#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <stack>
#include <queue>
#include <memory>
#include <exception>
#include <assert.h>




class Worker
{
public:
	typedef std::function<void()> Job;
	typedef std::unique_ptr<Job> JobPtr;
	typedef std::queue<JobPtr> JobQueue;
	typedef std::function<void(std::exception&)> ExceptionHandler;
	typedef std::shared_ptr<ExceptionHandler> ExceptionHandlerPtr;


private:
	std::mutex m_lock;
	std::condition_variable m_event;
	JobQueue m_jobQueue;
	bool m_run = true;
	bool m_overtime = true;
	std::thread m_thread;
	ExceptionHandlerPtr m_exceptionHandler;


public:
	Worker()
	{
		m_thread = std::thread([this]()
		{
			while (m_run || (m_overtime && !m_jobQueue.empty())) {
				std::unique_lock<std::mutex> lock(m_lock);
				if (m_jobQueue.empty())
					m_event.wait_for(lock, std::chrono::milliseconds(1000));

				assert(m_jobQueue.empty() == false);
				JobPtr job(std::move(m_jobQueue.front()));
				m_jobQueue.pop();
				assert(job != nullptr);

				lock.unlock();
				try {
					(*job)();
				}
				catch (std::exception& e) {
					ExceptionHandlerPtr eh;
					std::atomic_store(&eh, std::atomic_load(&m_exceptionHandler));
					if (eh != nullptr)
						(*eh)(e);
					else {
						// ...
					}
				}
			}
		});
	}


	inline void SetExceptionHandler(const ExceptionHandler& eh)
	{
		std::atomic_store(&m_exceptionHandler,
						  ExceptionHandlerPtr(new ExceptionHandler(eh)));
	}


	inline bool PushJob(const Job& newJob)
	{
		return PushJob(JobPtr(new Job(newJob)));
	}


	inline bool PushJob(Job&& newJob)
	{
		return PushJob(JobPtr(new Job(std::move(newJob))));
	}


	inline bool PushJob(JobPtr&& job)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		if (m_run == false)
			return false;
		m_jobQueue.push(std::move(job));
		lock.unlock();
		m_event.notify_one();
		return true;
	}


	void Stop(bool overtime = true)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		m_run = false;
		m_overtime = overtime;
		lock.unlock();
		m_event.notify_all();
	}


	~Worker()
	{
		Stop();
		m_thread.join();
	}

	Worker(const Worker&) = delete;
	Worker& operator = (const Worker&) = delete;
};


/////////////////
template <typename Key,
		  typename Hash = std::hash<Key>,
		  typename EqualTo = std::equal_to<Key>>
class ThreadPool
{
public:
	typedef std::function<void(std::exception&)> ExceptionHandler;
	typedef std::function<void(Key&)> Routine;

private:
	struct Job
	{
		Key m_key;
		Routine m_routine;
		bool m_inSequence;

		Job(const Key& key,
			const Routine& routine,
			bool inSequence)
			: m_key(key)
			, m_routine(routine)
			, m_inSequence(inSequence)
		{
		}

		Job(Key&& key,
			Routine&& routine,
			bool inSequence)
			: m_key(std::move(key))
			, m_routine(std::move(routine))
			, m_inSequence(inSequence)
		{
		}
	};
	typedef std::unique_ptr<Job> JobPtr;
	typedef std::queue<JobPtr> JobQueue;


	struct HashWrapper
	{
		size_t operator () (const Key* key) const
		{
			return Hash()(*key);
		}
	};

	struct EqualToWrapper
	{
		bool operator () (const Key* left, const Key* right) const
		{
			return EqualTo()(*left, *right);
		}
	};

	const uint32_t m_threadCount;
	std::unique_ptr<Worker> m_threads;
	std::stack<Worker*> m_restList;
	std::unordered_set<Worker*> m_runners;
	std::unordered_map<Key*, Worker*, HashWrapper, EqualToWrapper> m_workingKeys;
	Worker::JobQueue m_jobQueue;

	inline bool PushJob_Internal(Worker::Job&& job, Key* key)
	{
		Worker* worker = nullptr;
		if (key != nullptr) {
			auto it = m_workingKeys.find(key);
			if (it != m_workingKeys.end()) {
				worker = it->second;
			}
		}

		if (worker == nullptr) {
			if (m_restList.empty()) {
				m_jobQueue.push(Worker::JobPtr(new Worker::Job(std::move(job))));
			}
			else {
				worker = m_restList.top();
				m_restList.pop();
				m_runners.insert(worker);
			}
		}

		if (worker != nullptr)
			worker->PushJob(std::move(job));



		std::unique_lock<std::mutex> lock(m_lock);
		if (m_run == false)
			return false;
		m_jobQueue.push_back(std::move(job));
		lock.unlock();
		m_event.notify_one();
		return true;
	}


	std::mutex m_lock;
	std::condition_variable m_event;
	std::unordered_set<Key*, HashWrapper, EqualToWrapper> m_workingKeys;
	bool m_run = true;
	bool m_overtime = true;
	std::vector<std::thread> m_workers;

	ExceptionHandler m_exceptionHandler = nullptr;

	inline bool PushJob_Internal(JobPtr&& job)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		if (m_run == false)
			return false;
		m_jobQueue.push_back(std::move(job));
		lock.unlock();
		m_event.notify_one();
		return true;
	}

public:
	ThreadPool(uint32_t threadCount = std::thread::hardware_concurrency())
	{
		m_workers.reserve(threadCount);
		for (auto i=threadCount; i>0; --i) {
			m_workers.push_back(std::thread([this]()
			{
				std::stack<JobPtr> tempStack;
				std::unique_lock<std::mutex> lock(m_lock);
				while (m_run || (m_overtime && !m_jobQueue.empty())) {
					// 1. Event를 기다림
					if (m_jobQueue.empty())
						m_event.wait_for(lock, std::chrono::milliseconds(1000));

					// 2. 수행할 작업 선별
					JobPtr job(nullptr);
					while (m_jobQueue.empty() == false) {
						// 2-1. 맨 앞의 것을 꺼낸다.
						job = std::move(m_jobQueue.front());
						m_jobQueue.pop_front();
						// 2-2. Key가 같은 작업이 이미 수행중인지 검사
						if (job->m_inSequence && m_workingKeys.find(&job->m_key)!=m_workingKeys.end()) {
							// 2-2-1. 수행중이면 보류
							tempStack.push(std::move(job));
							assert(job.get() == nullptr);
						}
						else {
							// 2-2-2. 수행중이지 않으면 해당 작업을 선택
							if (job->m_inSequence)
								m_workingKeys.insert(&job->m_key);
							break;
						}
					}

					// 3. 위에서 보류처리된 것들을 다시 큐에 집어넣는다.
					while (tempStack.empty() == false) {
						m_jobQueue.push_front(std::move(tempStack.top()));
						tempStack.pop();
					}
					lock.unlock();

					// 4. 작업 수행
					try {
						if (job != nullptr)
							job->m_routine(job->m_key);
					}
					catch (std::exception& e) {
						if (m_exceptionHandler != nullptr)
							m_exceptionHandler(e);
						else {
							// ...
						}
					}

					// 5. 완료한 작업을 제거한다.
					lock.lock();
					if (job!=nullptr && job->m_inSequence)
						m_workingKeys.erase(&job->m_key);
				}
			}));
		}
	}


	inline void SetExceptionHandler(const ExceptionHandler& eh)
	{
		m_exceptionHandler = eh;
	}


	inline void SetExceptionHandler(ExceptionHandler&& eh)
	{
		m_exceptionHandler = std::move(eh);
	}


	inline bool PushJob(const Key& key,
						const Routine& routine,
						bool inSequence = true)
	{
		return PushJob_Internal(JobPtr(new Job(key, routine, inSequence)));
	}


	inline bool PushJob(Key&& key,
						Routine&& routine,
						bool inSequence = true)
	{
		return PushJob_Internal(JobPtr(new Job(std::move(key), std::move(routine), inSequence)));
	}


	inline bool PushJob(const Routine& routine)
	{
		return PushJob_Internal(JobPtr(new Job(Key(), routine, false)));
	}


	inline bool PushJob(Routine&& routine)
	{
		return PushJob_Internal(JobPtr(new Job(Key(), std::move(routine), false)));
	}


	void Stop(bool overtime = true)
	{
		std::unique_lock<std::mutex> lock(m_lock);
		m_run = false;
		m_overtime = overtime;
		lock.unlock();
		m_event.notify_all();
	}


	~ThreadPool()
	{
		Stop();
		for (auto& t : m_workers)
			t.join();
	}
};


// Test
#include <set>
#include <atomic>

std::atomic_bool lock;
std::set<int> result;

int main()
{
	{
		ThreadPool<int> tp;
		for (int i=0; i<1000000; ++i) {
			tp.PushJob(i % 8, [i](int key)
			{
				bool cmp;
				do {
					cmp = false;
					lock.compare_exchange_strong(cmp, true, std::memory_order::memory_order_release);
				} while (cmp);

				result.insert(i);

				cmp = true;
				bool r = lock.compare_exchange_strong(cmp, false);
				assert(cmp);
			});
		}
	}
	for (auto& i : result) {
		printf("%d %d\n", (int)result.size(), i);
	}
	return 0;
}