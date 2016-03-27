#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>
#include <vector>
#include <stack>
#include <boost/lockfree/stack.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif


/*** Test 환경 *********************************************************************************************/
//// Windows  :  i5-4440 3.10GHz (4 core), Windows7, VC2015
//// Linux    :  위의 Windows머신에서 돌아가는 VM (4 core), CentOS7, GCC 5.1.1
/***********************************************************************************************************/



/*** Test Option *******************************************************************************************/

//// 결과값이 너무 커서 보기 힘들면 이 숫자로 나눈다.
const double Test_Division = 1000.0;

//// 테스트를 수행 할 시간
const int Test_TimeMs = 1000 * 10;

//// 쓰레드 개수
#define Test_TheadCount 4

//// Windows에만 적용. CRITICAL_SECTION 객체의 스핀카운트
////  - 0이면 기본값. (제일 빠르다?)
const int Test_WinCS_SpinCount = 0;

//// 한 쓰레드에서 연속으로 Alloc하는 데이터 개수
const int Test_Alloc_Count = 10;

//// 테스트용 객체 하나의 크기
////  - 오브젝트풀 사용하지 않는 테스트의 경우, Windows는 12KB, Linux는 24KB 가량 부터 느려짐
////  - Windows  :  약 160,000 cnt/ms
////  - Linux    :  약 120,000 cnt/ms
const int Test_Object_Size = 1024 * 12;

//// 할당받은 객체를 초기화한다
//// 객체 크기와 그것을 사용하는 비용이 비례 할 것이라는 가정
//#define Test_MemSet

//// 오브젝트풀 사용 여부 (아래 평은 Test_Use_TLS를 사용하지 않은 경우)
////  - Test_Alloc_Count 또는 Test_Object_Size가 크건 작건 일관성 있는 속도 유지
////  - Windows  :  단일:25,000~30,000cnt/ms,  샤딩:약97,000cnt/ms
////  - Linux    :  단일:약15,000cnt/ms,  샤딩:약68,000cnt/ms
#define Test_Use_ObjPool Test_TheadCount
//#define Test_Use_ObjPool 1

//// 오브젝트풀을 std::stack으로 구현
////  - Windows  :  직접 stack 구현하는것에 비해 10%가량 느림
////  - Linux    :  비슷함
//#define Test_Use_StdStack

//// 오브젝트풀을 직접 락프리 스택으로 구현
//    - Windows  :  단일:약8,000cnt/ms,  샤딩:약200,000cnt/ms
//    - Linux    :  단일:약8,000cnt/ms,  샤딩:약160,000cnt/ms
//#define Test_Use_My_LockfreeStack

//// 오브젝트풀을 boost::lockfree::stack으로 구현
////  - Windows  :  약 9,200cnt/ms,  샤딩:약35,000cnt/ms
////  - Linux    :  약 11,500cnt/ms,  샤딩:약41,000cnt/ms
//#define Test_Use_Boost_LockfreeStack

//// 각 OS의 고유한 락(Windows:CRITICAL_SECTION, Linux:pthread_mutex) 대신 std::mutex를 사용
////  - Windows  :  단일:OS락과 비슷함,  샤딩:약31,000cnt/ms
////  - Linux    :  단일:OS락과 비슷함,  샤딩:약30,000cnt/ms
//#define Test_Use_StdMutex

//// 오브젝트풀 사용 시 스핀락 사용 여부
////  - Windows  :  단일:9,000~18,000cnt/ms,  샤딩:약36,000cnt/ms
////  - Linux    :  단일:10,000~16,000cnt/ms,  샤딩:약33,000cnt/ms
//#define Test_Use_SpinLock

//// 오브젝트 풀을 쓰레드 로컬 객체로 사용 여부 (락 미사용)
////  - 공통  :  속도가 가장 빠름 (약 950,000 cnt/ms)
////  - 풀을 사용하는 각 쓰레드들의 발급:반납 비율이 같아야 한다.
////    (예를들어 A쓰레드의 풀은 발급만 하고 B쓰레드의 풀은 반납만 받는다면 풀을 쓰는 의미가 없다)
//#define Test_Use_TLS

//// TLS의 오브젝트풀이라도 다른 쓰레드로부터 관리를 받을 필요가 있다면 불가피하게 락이 필요하다.
////  - Windows  :  OS락:약170,000cnt/ms, 스핀락:약80,000cnt/ms
////  - Linux    :  OS락:약150,000cnt/ms, 스핀락:약290,000cnt/ms
//#define Test_TLS_With_Lock

/***********************************************************************************************************/



/*** 결과 **************************************************************************************************/

//// new, delete횟수
uint64_t g_accessCount = 0;

//// 오브젝트 풀 사용 시 최대로 풀링한 데이터 개수
uint64_t g_pooledCount = 0;

/***********************************************************************************************************/




#if defined(_MSC_VER) && _MSC_VER <= 1800
#define TLS __declspec(thread) // VS2013 이하
#else
#define TLS thread_local // C++11
#endif

struct CS
{
#ifndef Test_Use_SpinLock

	#if defined(Test_Use_StdMutex)
		std::mutex m_lock;
		void lock() {
			m_lock.lock();
		}
		void unlock() {
			m_lock.unlock();
		}
	#elif !defined(_WIN32) // Test_Use_StdMutex 
		pthread_mutex_t m_lock;
		CS() {
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&m_lock, &attr);
		}
		~CS() {
			pthread_mutex_destroy(&m_lock);
		}
		void lock() {
			pthread_mutex_lock(&m_lock);
		}
		void unlock() {
			pthread_mutex_unlock(&m_lock);
		}
	#else // Test_Use_StdMutex
		CRITICAL_SECTION m_lock;
		CS() {
			if (Test_WinCS_SpinCount == 0)
				InitializeCriticalSection(&m_lock);
			else
				InitializeCriticalSectionAndSpinCount(&m_lock, Test_WinCS_SpinCount);
		}
		~CS() {
			DeleteCriticalSection(&m_lock);
		}
		void lock() {
			EnterCriticalSection(&m_lock);
		}
		void unlock() {
			LeaveCriticalSection(&m_lock);
		}
	#endif // Test_Use_StdMutex

#else // Test_Use_SpinLock

	std::atomic<bool> m_lock;
	CS() {
		m_lock = false;
	}
	void lock() {
		bool cmp;
		while (true) {
			if (m_lock.load(std::memory_order_acquire)) continue;
			cmp = false;
			if (m_lock.compare_exchange_strong(cmp, true, std::memory_order_acquire))
				break;
		}
	}
	void unlock() {
		m_lock.store(false, std::memory_order_release);
	}

#endif // Test_Use_SpinLock
};
CS g_mutex;


template<typename T>
class ObjectPool
{
public:
#ifdef Test_Use_Boost_LockfreeStack
	boost::lockfree::stack<T*> m_pool;
	ObjectPool()
		: m_pool(Test_TheadCount * Test_Alloc_Count * 10)
	{
	}
#else
#ifdef Test_Use_StdStack
	std::stack<T*> m_pool;
#else
	struct Node
	{
		Node* m_next = nullptr;
		uint8_t m_data[sizeof(T)];
		inline Node() {}
#ifdef Test_Use_My_LockfreeStack
		std::atomic<int>* m_popContention = nullptr;
		void SafeWait(const std::atomic<int>* a_diff)
		{
			if (m_popContention != nullptr && m_popContention == a_diff) {
				while (m_popContention->load(std::memory_order_acquire) > 0);
				m_popContention = nullptr;
			}
		}
		~Node()
		{
			SafeWait(m_popContention);
		}
#endif
	};
#ifdef Test_Use_My_LockfreeStack
	std::atomic<Node*> m_head;
	std::atomic<size_t> m_count;
	std::atomic<int> m_popContention;
	ObjectPool()
	{
		m_head = nullptr;
		m_count = 0;
		m_popContention = 0;
	}
#else
	Node* m_head = nullptr;
	size_t m_count = 0;
#endif
#endif

#if !defined(Test_Use_TLS) || defined(Test_TLS_With_Lock)
	CS m_lock;
#endif

	inline void Lock() 
	{
#if !defined(Test_Use_TLS) || defined(Test_TLS_With_Lock)
		m_lock.lock();
#endif
	}

	inline void Unlock()
	{
#if !defined(Test_Use_TLS) || defined(Test_TLS_With_Lock)
		m_lock.unlock();
#endif
	}
#endif // Test_Use_Boost_LockfreeStack


	~ObjectPool()
	{
		ShrinkAll();
	}

	T* New() 
	{
#ifdef Test_Use_Boost_LockfreeStack
		T* ret;
		if (m_pool.pop(ret) == false)
			ret = (T*) ::operator new(sizeof(T));
#elif defined(Test_Use_StdStack)
		Lock();
		T* ret;
		if (m_pool.empty()) {
			Unlock();
			ret = (T*) ::operator new(sizeof(T));
		}
		else {
			ret = m_pool.top();
			m_pool.pop();
			Unlock();
		}
#elif defined(Test_Use_My_LockfreeStack)
		++m_popContention;
		Node* snapshot;
		while (true) {
			snapshot = m_head.load(std::memory_order_acquire);
			if (snapshot == nullptr)
				break;

			Node* next = snapshot->m_next;
			if (m_head.compare_exchange_strong(snapshot, next, std::memory_order_acq_rel))
				break;
		}
		int cnt = --m_popContention;
		if (snapshot == nullptr)
			snapshot = new Node;
		else {
			//--m_count;
			if (cnt == 0)
				snapshot->m_popContention = nullptr;
			else
				snapshot->m_popContention = &m_popContention;
		}
		T* ret = (T*)snapshot->m_data;
#else
		Lock();
		Node* head = m_head;
		if (head == nullptr) {
			Unlock();
			head = new Node;
		}
		else {
			m_head = m_head->m_next;
			--m_count;
			Unlock();
		}
		T* ret = (T*)head->m_data;
#endif // Test_Use_Boost_LockfreeStack
		new(ret) T();
		return ret;
	}

	void Free(T* a_data)
	{
		a_data->~T();

#ifdef Test_Use_Boost_LockfreeStack
		m_pool.push(a_data);
#elif defined(Test_Use_StdStack)
		Lock();
		m_pool.push(a_data);
		Unlock();
#elif defined(Test_Use_My_LockfreeStack)
		Node* node = (Node*)((uint8_t*)a_data - offsetof(Node, m_data));
		Node* snapshot;
		node->SafeWait(&m_popContention);
		do {
			snapshot = m_head.load(std::memory_order_acquire);
			node->m_next = snapshot;
		} while (false == m_head.compare_exchange_strong(snapshot, node, std::memory_order_acq_rel));
		//++m_count;
#else
		Node* node = (Node*)((uint8_t*)a_data - offsetof(Node, m_data));
		Lock();
		node->m_next = m_head;
		m_head = node;
		++m_count;
		Unlock();
#endif // Test_Use_Boost_LockfreeStack
	}

	size_t ShrinkAll() 
	{
		size_t cnt = 0;
#if defined(Test_Use_Boost_LockfreeStack)
		T* del;
		while (m_pool.pop(del)) {
			::operator delete(del);
			++cnt;
		}

#elif defined(Test_Use_StdStack)
		Lock();
		while (m_pool.empty()==false) {
			T* del = m_pool.top();
			m_pool.pop();
			::operator delete(del);
			++cnt;
		}
		Unlock();
#else
		Lock();
		while (true) {
			Node* del = m_head;
			if (del == nullptr)
				break;
			m_head = del->m_next;
			delete del;
			++cnt;
		}
		Unlock();
#endif
		return cnt;
	}
};

struct TestObject
{
	char m_data[Test_Object_Size];
	inline TestObject() 
	{
		// 빈 생성자를 정의해주지 않으면
		// 오브젝트풀에서 명시적으로 생성자를 호출 할 때 ( new(ret) T(); )
		// memset(0)이 수행된다.
	}
};

#if Test_Use_ObjPool && !defined(Test_Use_TLS)
ObjectPool<TestObject> t_ObjectPool[Test_Use_ObjPool];
#elif defined(Test_Use_TLS)
TLS ObjectPool<TestObject>* t_ObjectPool;
#else
ObjectPool<TestObject>* t_ObjectPool;
#endif

static std::atomic<uint32_t> g_LastNumber;
TLS uint32_t t_shard = g_LastNumber++;


inline void InitObjectPool() {
#if Test_Use_ObjPool && !defined(Test_Use_TLS)
#else
	g_mutex.lock();
	if (t_ObjectPool == nullptr)
		t_ObjectPool = new ObjectPool<TestObject>;
	g_mutex.unlock();
#endif
}
inline void UnInitObjectPool() {
	g_mutex.lock();
#if Test_Use_ObjPool && !defined(Test_Use_TLS)
	for (int i=0; i<Test_Use_ObjPool; ++i) {
		size_t pooledCount = t_ObjectPool[i].ShrinkAll();
		printf("pool[%d] : %llu\n", i, pooledCount);
		g_pooledCount += pooledCount;
	}
#else
	g_pooledCount += t_ObjectPool->ShrinkAll();
	if (t_ObjectPool != nullptr) {
		delete t_ObjectPool;
		t_ObjectPool = nullptr;
	}
#endif
	g_mutex.unlock();
}

#ifndef Test_Use_ObjPool

inline TestObject* New() {
	return new TestObject;
}
inline void Free(TestObject* data) {
	delete data;
}
#else // Test_Use_ObjPool

	inline TestObject* New() {
#if Test_Use_ObjPool && !defined(Test_Use_TLS)
		uint32_t shard = t_shard % Test_Use_ObjPool;
		//uint32_t shard = rand() % Test_Use_ObjPool;
		return t_ObjectPool[shard].New();
#else
		return t_ObjectPool->New();
#endif
	}
	inline void Free(TestObject* data) {
#if Test_Use_ObjPool && !defined(Test_Use_TLS)
		//uint32_t shard = std::hash<void*>()(data) % Test_Use_ObjPool;
		static_assert(997 > Test_Use_ObjPool*2, "해시키가 너무 작음");
		uint32_t shard = ((size_t)data % 997) % Test_Use_ObjPool;
		return t_ObjectPool[shard].Free(data);
#else
		t_ObjectPool->Free(data);
#endif
	}

#endif // Test_Use_ObjPool


/////////////////////////////////////////////////////////////////////////////
volatile bool g_Run = true;
int main()
{
	std::cout << "Start\n";

	std::vector<std::thread> threads;
	threads.resize(Test_TheadCount);
	for (auto& t : threads) {
		t = std::thread([](){
			InitObjectPool();

			TestObject* objarr[Test_Alloc_Count] = {nullptr};
			int index = 0;
			uint64_t access = 0;
			while (g_Run) {
				if (index < Test_Alloc_Count) {
					objarr[index] = New();
#ifdef Test_MemSet
					memset(arr[index], index, Test_Object_Size);
#endif
				}
				else {
					int i = index - Test_Alloc_Count;
					Free(objarr[i]);
					objarr[i] = nullptr;
				}

				index = (index + 1) % (Test_Alloc_Count * 2);
				++access;
			}
			
			for (auto obj : objarr) {
				if (obj != nullptr)
					Free(obj);
			}

			g_mutex.lock();
			g_accessCount += access;
			g_mutex.unlock();
#ifdef Test_Use_TLS
			UnInitObjectPool();
#endif
		});
	}

	auto start = std::chrono::high_resolution_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(Test_TimeMs));

	g_Run = false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	for (auto& t : threads) {
		t.join();
	}

#ifndef Test_Use_TLS
	UnInitObjectPool();
#endif

	printf("g_accessCount : %lf count per millisec (*Test_Division:%lf)\n",
		   (g_accessCount / elapsed.count()) / Test_Division,
		   Test_Division);
	printf("g_pooledCount : %llu\n", g_pooledCount);

	return 0;
}

