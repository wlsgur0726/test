// declare
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <exception>

// impl only
#include <cassert>
#include <iostream>



// std만 사용해서 구현한 쓰레드풀
class ThreadPool
{
public:
	typedef std::function<void()>					Task;
	typedef std::function<void(std::exception&)>	ExceptionHandler;
	typedef std::shared_ptr<ExceptionHandler>		ExceptionHandlerPtr;

	inline static size_t DefaultThreadCount()
	{
		return std::thread::hardware_concurrency();
	}

	inline static constexpr bool DefaultOverTime()
	{
		return true;
	}


protected:
	// 이벤트 큐
	std::unique_ptr<std::condition_variable>	m_event;
	std::unique_ptr<std::mutex>					m_taskQueueLock;
	std::queue<Task>							m_taskQueue;

	// 동작 상태
	bool m_run = false;

	// 이 변수가 true이면 
	// 종료명령이 떨어져도 남은 작업을 모두 처리한 후 종료한다.
	bool m_overtime = true;

	// 작업을 수행하는 쓰레드들
	const size_t m_threadCount;
	std::vector<std::thread> m_threads;

	// 사용자가 입력한 작업에서 발생한 예외를 처리하는 핸들러
	ExceptionHandlerPtr m_exceptionHandler;


public:
	ThreadPool(size_t threadCount = DefaultThreadCount())
		: m_threadCount(threadCount)
	{
		assert(threadCount > 0);
		m_event.reset(new std::condition_variable);
		m_taskQueueLock.reset(new std::mutex);
	}


	ThreadPool& Start()
	{
		std::unique_lock<std::mutex> lock(*m_taskQueueLock);
		if (m_threads.size() > 0)
			return *this;

		m_run = true;
		for (int i=0; i<m_threadCount; ++i) {
			m_threads.push_back(std::thread([this]()
			{
				std::unique_lock<std::mutex> lock(*m_taskQueueLock);
				// 1. 일감이 들어오거나 Stop요청을 기다림
				while (m_run || (m_overtime && !m_taskQueue.empty())) {
					if (m_taskQueue.empty()) {
						//m_event->wait_for(lock, std::chrono::milliseconds(100));  // polling
						m_event->wait(lock);
						continue;
					}

					// 2. 작업을 접수
					assert(m_taskQueue.empty() == false);
					Task task(std::move(m_taskQueue.front()));
					m_taskQueue.pop();

					// 3. 작업 수행
					lock.unlock();
					try {
						if (task != nullptr)
							task();
					}
					catch (std::exception& e) {
						ExceptionHandlerPtr eh = std::atomic_load(&m_exceptionHandler);
						if (eh != nullptr)
							(*eh)(e);
						else {
							// default handler
							// 우짤까..
							std::cerr << e.what() << " : " __FILE__ "(" << __LINE__ << ")\n";
						}
					}

					// 4. 1단계부터 반복
					lock.lock();
				}
			}));
		}
		return *this;
	}


	ThreadPool& SetExceptionHandler(const ExceptionHandler& eh)
	{
		// 이 함수의 Caller Thread와 Worker Thread 간의
		// 데이터 레이스를 방지하기 위해 atomic op 사용
		std::atomic_store(&m_exceptionHandler,
						  ExceptionHandlerPtr(new ExceptionHandler(eh)));
		return *this;
	}


	inline ThreadPool& PushTask(const Task& task)
	{
		return PushTask(std::move(Task(task)));
	}


	ThreadPool& PushTask(Task&& task)
	{
		std::unique_lock<std::mutex> lock(*m_taskQueueLock);
		m_taskQueue.push(std::move(task));
		m_event->notify_one();
		return *this;
	}


	void Stop(bool overtime = DefaultOverTime())
	{
		std::unique_lock<std::mutex> lock(*m_taskQueueLock);
		m_run = false;
		m_overtime = overtime;
		m_event->notify_all();
	}


	void Wait(bool overtime = DefaultOverTime())
	{
		Stop(overtime);
		std::unique_lock<std::mutex> lock(*m_taskQueueLock);
		while (m_threads.size() > 0) {
			auto t = std::move(m_threads[m_threads.size()-1]);
			m_threads.pop_back();
			lock.unlock();
			t.join();
			lock.lock();
		}
	}


	virtual ~ThreadPool()
	{
		Wait();
	}


	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator = (const ThreadPool&) = delete;
};




// Key의 Hash가 동일한 작업이 수행중인 경우 
// 선작업이 끝날 때까지 실행을 보류하여 
// 실행순서를 보장해주는 쓰레드풀
template <typename Key,
		  typename Hash = std::hash<Key>>
class SequentialThreadPool : public ThreadPool
{
public:
	typedef std::function<void(Key&)>		SequentialTask;
	typedef size_t							HashKey;


private:
	typedef std::pair<Key, SequentialTask>	KeyAndTask;
	typedef std::queue<KeyAndTask>			KeyAndTaskQueue;
	struct AlreadyHashed
	{
		inline constexpr size_t operator()(const HashKey h) const
		{
			return h;
		}
	};

	std::unordered_map<	HashKey,
						KeyAndTaskQueue,
						AlreadyHashed>	m_workingMap;
	std::unique_ptr<std::mutex>			m_workingMapLock;


public:
	SequentialThreadPool(size_t threadCount = DefaultThreadCount())
		: ThreadPool(threadCount)
	{
		m_workingMapLock.reset(new std::mutex);
	}


	virtual ~SequentialThreadPool()
	{
		// SequentialThreadPool클래스의 영역이 delete된 상태에서 Wait하는것을 방지하기 위해 재정의
		ThreadPool::Wait();
	}


	inline SequentialThreadPool& PushTask(const Key& key,
										  const SequentialTask& task)
	{
		return PushTask(std::move(Key(key)),
						std::move(SequentialTask(task)));
	}


	SequentialThreadPool& PushTask(Key&& key,
								   SequentialTask&& task)
	{
		// Key객체를 m_workingMap의 key로 등록하지 않는 이유 : 
		//   만약 서로 다른 Key객체가 동일한 내부데이터를 가리키고,
		//   이러한 내부데이터를 기반으로 해싱을 하도록 설계한 경우
		//   task(사용자로직)의 인자로 넘어간 key객체로 인하여
		//   task를 수행중인 thread와 sequence check를 수행하는 thread간에 data race나
		//   access violation 등의 문제를 유발할 수 있으므로
		//   Push 시점에 미리 key를 해싱해서 이것을 sequence check의 기준으로 사용한다.
		const HashKey hashKey = Hash()(key);
		auto sequentialTask = [this,
							   hashKey,
							   keyAndTask = KeyAndTask({std::move(key),
														std::move(task)})]() mutable
		{
			auto& key  = keyAndTask.first;
			auto& task = keyAndTask.second;

			// 1. sequence check
			std::unique_lock<std::mutex> lock(*m_workingMapLock);
			auto it = m_workingMap.find(hashKey);
			if (it != m_workingMap.end()) {
				// 1-A. 동일한 작업이 이미 수행중인 경우
				//      해당 작업을 수행중인 쓰레드가 처리하도록 큐잉하고 종료
				it->second.push(std::move(keyAndTask));
			}
			else {
				// 1-B. 동일한 작업이 수행중이지 않은 경우
				// 1-B-1. 키를 등록하여 수행중임을 표시
				m_workingMap.emplace(hashKey, KeyAndTaskQueue());
				while (true) {
					// 1-B-2. 작업 수행
					lock.unlock();
					if (task != nullptr)
						task(key);
					
					// 1-B-3. 작업 수행하는 도중 동일한 작업이 큐잉되었는지 확인
					lock.lock();
					it = m_workingMap.find(hashKey);
					auto& queue = it->second;
					if (queue.empty()) {
						// 1-B-3-A. 동일한 작업이 없는 경우 등록을 해제하고 종료
						m_workingMap.erase(it);
						break;
					}

					// 1-B-3-B. 동일한 작업이 또 들어온 경우 해당 작업을 우선 처리
					keyAndTask = std::move(queue.front());
					queue.pop();
					// 1-B-4. 1-B-2단계부터 반복
				}
			}
			// 2. 완료
		};
		ThreadPool::PushTask(std::move(sequentialTask));
		return *this;
	}
};




//-------------------------------------------------------------------------------------------------------
// 테스트 설정

#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint64_t G = 0;


#define BeforehandPush 1
#if BeforehandPush
#	define Info_BeforehandPush "BeforehandPush  "
#
#else
#	define Info_BeforehandPush ""
#
#endif


void Work(uint64_t n)
{
#define NoWork 0
#if NoWork > 1
#	define Info_Work "NoWork2  "
#
#elif NoWork
#	define Info_Work "NoWork1  "
#
#else
#	define SleepWork 0
#	if SleepWork
#		define Info_Work "SleepWork  "
		std::this_thread::sleep_for(std::chrono::milliseconds(SleepWork));
#
#	else
#		define Info_Work "CPUBoundWork  "
		n *= 1.0;
		for (; n>0; --n)
			G += n;
#	endif
#
#endif
}


#if !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
#	define Info_Config "Debug  "
#	define TaskCount 1000000
#
#else
#	define Info_Config "Release  "
#	define TaskCount 10000000
#
#endif


#if SleepWork
#	undef  TaskCount
#	define TaskCount 1000000
#	define ThreadCount (50 * SleepWork)
#
#else
#	define ThreadCount 4
#
#endif
const int KeyCount = ThreadCount * 4;


#define Sequential 1
#if Sequential
#	define Info_Sequential "SequentialThreadPool  "
#
#else
#	define Info_Sequential "ThreadPool  "
#
#endif


#define CheckDupKey (NoWork<=1 && Sequential)
#if CheckDupKey
#	define Info_CheckDupKey "CheckDupKey  "
#
#else
#	define Info_CheckDupKey ""
#
#endif


//-------------------------------------------------------------------------------------------------------
// 테스트

std::atomic_int workingCount[KeyCount];
std::atomic_bool spinlock;
std::set<int> result;

int Test()
{
	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed;
	srand(time(nullptr));
	printf("시작. ThreadCount:%d, KeyCount:%d, TaskCount:%d\n", ThreadCount, KeyCount, (int)TaskCount);
	printf(Info_Config Info_Sequential Info_BeforehandPush Info_Work Info_CheckDupKey "\n");

#if Sequential
	SequentialThreadPool<int> tp(ThreadCount);
#else
	ThreadPool tp(ThreadCount);
#endif

#if !BeforehandPush
	tp.Start();
#endif

	for (int i=0; i<TaskCount; ++i) {
		int key = key = i % KeyCount;
#if Sequential
		tp.PushTask(key, [i](int key)
#else
		tp.PushTask([i, key]()
#endif
		{
#if NoWork > 1
			Work(0);
#else
#	if CheckDupKey
			struct _CheckDupKey
			{
				std::atomic_int& cnt;
				_CheckDupKey(int key)
					: cnt(workingCount[key])
				{
					assert(key < KeyCount);
					auto incafter = ++cnt;
					assert(incafter == 1);
				}
				~_CheckDupKey()
				{
					--cnt;
				}
			} task(key);
#	endif

			bool cmp;
			do {
				cmp = false;
				spinlock.compare_exchange_strong(cmp, true);
			} while (cmp);

			uint64_t rnd = rand() % 10000 + 10000;
			result.insert(i);

			cmp = true;
			bool r = spinlock.compare_exchange_strong(cmp, false);
			assert(cmp);

			Work(rnd);
#endif
		});
	}
	elapsed = std::chrono::high_resolution_clock::now() - start;

#if BeforehandPush
	printf("Push를 모두 완료, 작업 개시 (%.3lf sec)\n", elapsed.count()/1000);
	tp.Start();
#else
	printf("Push를 모두 완료 (%.3lf sec)\n", elapsed.count()/1000);
#endif

	tp.Wait(true);
	elapsed = std::chrono::high_resolution_clock::now() - start;

#if NoWork <= 1
	if (result.size() != TaskCount) {
		printf("test fail %llu\n", (uint64_t)result.size());
		return (int)G;
	}
#endif

	printf("완료 (%.3lf sec)\n", elapsed.count()/1000);
	return 0;
}


int main()
{
	return Test();
}
