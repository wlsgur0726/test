#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "tbb/reader_writer_lock.h"
#include "tbb/spin_rw_mutex.h"
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"

// std의 unordered_map과 tbb의 concurrent_unordered_map 비교.
typedef tbb::concurrent_unordered_map<intptr_t, intptr_t> TBBMAP;
typedef std::unordered_map<intptr_t, intptr_t> STDMAP;
TBBMAP tbbmap;
STDMAP stdmap;

// tbb concurrent_unordered_map의 erase가 unsafe이므로 rw락을 사용한다.
tbb::reader_writer_lock tbbrwlock;
// std는 exclusive lock
std::mutex stdmutex;

void tbbInsert(intptr_t key, intptr_t value) {
	tbbrwlock.lock_read();
	tbbmap.insert(TBBMAP::value_type(key, value));
	tbbrwlock.unlock();
}
void stdInsert(intptr_t key, intptr_t value) {
	stdmutex.lock();
	stdmap.insert(STDMAP::value_type(key, value));
	stdmutex.unlock();
}
intptr_t tbbGetAndWork(intptr_t key) {
	intptr_t r = 0;
	//tbbrwlock.lock_read();

	auto i = tbbmap.find(key);
	if (i != tbbmap.end()) {
		for (r = 0; r<i->second; ++r);
	}

	//tbbrwlock.unlock();
	return r;
}
intptr_t stdGetAndWork(intptr_t key) {
	intptr_t r = 0;
	//stdmutex.lock();

	auto i = stdmap.find(key);
	if (i != stdmap.end()) {
		for (r = 0; r<i->second; ++r);
	}

	//stdmutex.unlock();
	return r;
}
void tbbErase(intptr_t key) {
	tbbrwlock.lock();
	tbbmap.unsafe_erase(key);
	tbbrwlock.unlock();
}
void stdErase(intptr_t key) {
	stdmutex.lock();
	stdmap.erase(key);
	stdmutex.unlock();
}

void SleepWrap(int sec) {
#ifdef _WIN32
	Sleep(sec * 1000);
#else
	sleep(sec);
#endif
}


const int ThreadCount = 4;
const int WorkCount = 1000000;
// ThreadCount 개수 만큼의 쓰레드가 삽입&삭제를,

void Test_TBB() {
	std::mutex sumlock;
	uint64_t sum = 0;
	volatile bool run = true;
	std::thread threads[ThreadCount];
	for (auto& t : threads) {
		t = std::thread([&]() {
			uint64_t count = 0;
			while (run) {
				int r1 = rand();
				tbbInsert(r1, WorkCount);
				int r2 = rand();
				tbbInsert(r2, WorkCount);

				tbbErase(r1);
				++count;
				tbbErase(r2);
				++count;
			}
			sumlock.lock();
			sum += count;
			sumlock.unlock();
		});
	}
	std::thread iterThread([&](){
		uint64_t count = 0;
		while (run) {
			tbbrwlock.lock();
			for (auto it = tbbmap.begin(); it != tbbmap.end(); ++it) {
				tbbGetAndWork(it->first);
			}
			tbbrwlock.unlock();
			++count;
			SleepWrap(1);
		}
	});

	SleepWrap(10);

	run = false;
	for (auto& t : threads)
		t.join();
	iterThread.join();

	printf("TBB : %llu\n", sum);
}

void Test_STD() {
	std::mutex sumlock;
	uint64_t sum = 0;
	volatile bool run = true;
	std::thread threads[ThreadCount];
	for (auto& t : threads) {
		t = std::thread([&]() {
			uint64_t count = 0;
			while (run) {
				int r1 = rand();
				stdInsert(r1, WorkCount);
				int r2 = rand();
				stdInsert(r2, WorkCount);

				stdErase(r1);
				++count;
				stdErase(r2);
				++count;
			}
			sumlock.lock();
			sum += count;
			sumlock.unlock();
		});
	}
	std::thread iterThread([&](){
		uint64_t count = 0;
		while (run) {
			stdmutex.lock();
			for (auto it = stdmap.begin(); it != stdmap.end(); ++it) {
				stdGetAndWork(it->first);
			}
			stdmutex.unlock();
			++count;
			SleepWrap(1);
		}
	});
	SleepWrap(10);

	run = false;
	for (auto& t : threads)
		t.join();
	iterThread.join();

	printf("STD : %llu\n", sum);
}

int main(int argc, char* argv[]) {
	srand((unsigned int)&argc);

	Test_STD();
	Test_TBB();

	getchar();
	return 0;
}
