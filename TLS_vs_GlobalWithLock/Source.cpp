#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <mutex>
#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>
#include <stack>

#ifdef _WIN32
#include <Windows.h>
#endif


/*** Test 환경 ********************************************************/
//// Windows : i5-4440 3.10GHz (4 core), Windows7
//// Linux   : 위 스펙의 리얼머신 위에서 돌아가는 VM (4 core), CentOS7 
/**********************************************************************/



/*** Test Option ******************************************************/

//// 결과값이 너무 커서 보기 힘들면 이 숫자로 나눈다.
const double Test_Division = 100.0;

//// 테스트를 수행 할 시간
const int Test_TimeMs = 1000 * 5;

//// 테스트용 객체 하나의 크기
////  - 오브젝트풀 사용하지 않는 테스트의 경우, 1024*12 가량 부터 느려짐
const int Test_Object_Size = 1024 * 12;

//// Windows에만 적용. CRITICAL_SECTION 객체의 스핀카운트
////  - 0이 제일 빠르다?
const int Test_WinCS_SpinCount = 0;

//// 한 쓰레드에서 연속으로 Alloc하는 데이터 개수
////  - 테스트 결과값에 큰 영향을 미친다
const int Test_Alloc_Count = 10;

//// 할당받은 객체를 초기화한다
//// 객체 크기와 그것을 사용하는 비용이 비례 할 것이라는 가정
//#define Test_MemSet

//// 오브젝트 풀 사용 여부 (아래 평은 Test_Use_TLS를 사용하지 않은 경우)
////  - Test_Alloc_Count 또는 Test_Object_Size가 작다면 속도 면에서 손해
////  - Test_Alloc_Count 또는 Test_Object_Size가 큰 값이라도 일관성 있는 속도 유지
//#define Test_Use_ObjPool

//// 각 OS의 고유한 락(Windows:CRITICAL_SECTION, Linux:pthread_mutex) 대신 std::mutex를 사용
////  - Windows : CRITICAL_SECTION이 10배가량 빠름
////  - Linux   : 비슷함
//#define Test_Use_StdMutex

//// 오브젝트 풀 사용 시 순수 스핀락 사용 여부
////  - Windows : std::mutex보다 빠르고 CRITICAL_SECTION보다 느리다
////  - Linux   : std::mutex보다 느리다
//#define Test_Use_SpinLock

//// 오브젝트 풀 사용 시 쓰레드 로컬 스토리지 사용 여부 (락 미사용)
////  - 공통 : 속도가 가장 빠름
//#define Test_Use_TLS

//// TLS를 사용하는 오브젝트 풀이라도 다른 쓰레드로부터 관리를 받을 필요가 있다면(shrink 등) 불가피하게 락이 필요하다.
////  - Windows : CRITICAL_SECTION을 사용 할 때 성능손실이 적다.
////  - Linux   : SpinLock을 사용 할 때 성능손실이 적다.
//#define Test_TLS_With_Lock

/**********************************************************************/



/*** 결과 *************************************************************/

//// new횟수 + delete횟수
uint64_t g_accessCount = 0;

//// 오브젝트 풀 사용 시 최대로 풀링한 데이터 개수
uint64_t g_pooledCount = 0;

/**********************************************************************/



#ifndef Test_Use_TLS

#define TLS 

#else // Test_Use_TLS

#if defined(_MSC_VER) && _MSC_VER <= 1800
#define TLS __declspec(thread) // VS2013 이하
#else
#define TLS thread_local // C++11
#endif

#endif // Test_Use_TLS

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
		while (true) {
			if (m_lock == false) {
				bool cmp = false;
				if (m_lock.compare_exchange_weak(cmp, true, std::memory_order_acquire))
					break;
			}
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
	std::stack<T*> m_pool;

#if defined(Test_Use_TLS) && defined(Test_TLS_With_Lock)
	CS m_lock;
#endif

	void Lock() 
	{
#if defined(Test_Use_TLS) && defined(Test_TLS_With_Lock)
		m_lock.lock();
#endif
	}

	void Unlock()
	{
#if defined(Test_Use_TLS) && defined(Test_TLS_With_Lock)
		m_lock.unlock();
#endif
	}

	~ObjectPool()
	{
		ShrinkAll();
	}

	T* New() 
	{
		Lock();
		T* ret;
		if (m_pool.empty()) {
			ret = new T;
		}
		else {
			ret = m_pool.top();
			m_pool.pop();
		}
		Unlock();
		return ret;
	}

	void Free(T* a_data)
	{
		Lock();
		m_pool.push(a_data);
		Unlock();
	}

	void ShrinkAll() 
	{
		Lock();
		while (m_pool.empty()==false) {
			T* del = m_pool.top();
			m_pool.pop();
			delete del;
		}
		Unlock();
	}
};

struct TestObject
{
	char m_data[Test_Object_Size];
};
TLS ObjectPool<TestObject>* t_ObjectPool = nullptr;

inline void InitObjectPool() {
	g_mutex.lock();
	if (t_ObjectPool == nullptr)
		t_ObjectPool = new ObjectPool<TestObject>;
	g_mutex.unlock();
}
inline void UnInitObjectPool() {
	g_mutex.lock();
	g_pooledCount += t_ObjectPool->m_pool.size();
	if (t_ObjectPool != nullptr) {
		delete t_ObjectPool;
		t_ObjectPool = nullptr;
	}
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

	#ifndef Test_Use_TLS

		inline TestObject* New() {
			g_mutex.lock();
			TestObject* ret = t_ObjectPool->New();
			g_mutex.unlock();
			return ret;
		}
		inline void Free(TestObject* data) {
			g_mutex.lock();
			t_ObjectPool->Free(data);
			g_mutex.unlock();
		}

	#else // Test_Use_TLS
		inline TestObject* New() {
			return t_ObjectPool->New();
		}
		inline void Free(TestObject* data) {
			t_ObjectPool->Free(data);
		}

	#endif // Test_Use_TLS

#endif // Test_Use_ObjPool


/////////////////////////////////////////////////////////////////////////////
volatile bool g_Run = true;
int main()
{
	std::cout << "Start\n";

	std::thread threads[4];
	for (auto& t : threads) {
		t = std::thread([](){
			InitObjectPool();

			TestObject* objarr[Test_Alloc_Count] = {nullptr};
			int index = 0;
			uint32_t access = 0;
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

	printf("g_accessCount : %lf count per millisec\n", (g_accessCount / elapsed.count()) / Test_Division);
	printf("g_pooledCount : %llu\n", g_pooledCount);

	return 0;
}
