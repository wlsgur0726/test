/*
 * SemaphoreTest.cpp
 *
 *  Created on: 2015. 5. 2.
 *      Author: root
 */

#include <stdio.h>
#include <unistd.h>
#include <limits>
#include <stdint.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/eventfd.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <exception>
#include <thread>
#include <chrono>

struct SystemvSemaphore
{
	union semun{
		int val;
		struct semid_ds *buf;
		unsigned short int *arrary;
	};
	
	int m_semid = -1;
	SystemvSemaphore() {
		m_semid = semget(IPC_PRIVATE, 1, IPC_CREAT|0666);
		if (m_semid == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			return;
		}
		
		semun arg;
		arg.val = 0;
		if (semctl(m_semid, 0, SETVAL, arg) == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
		}
		
		int cnt = GetCount();
		if (cnt != 0) {
			printf("semcount is not 0 (%d)\n", cnt);
		}
	}
	
	~SystemvSemaphore() {
		if (m_semid != -1) {
			if (semctl(m_semid, 0, IPC_RMID, NULL) == -1) {
				int e = errno;
				char buf[BUFSIZ];
				printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			}
		}
	}
	
	void Release(int count = 1) {
		sembuf buf = {0, count, SEM_UNDO};
		if (semop(m_semid, &buf, 1) == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s (count:%d)\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)), GetCount());
		}
	}
	void Get(int count = 1) {
		sembuf buf = {0, -count, SEM_UNDO};
		if (semop(m_semid, &buf, 1) == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
		}
	}
	int GetCount() {
		int n = semctl(m_semid, 0, GETVAL);
		if (n == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
		}
		return n;
	}
};

struct PosixSemaphore
{
	sem_t m_sem;
	bool m_init = true;
	PosixSemaphore() {
		if (sem_init(&m_sem, 0, 0) == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			m_init = false;
		}
	}
	~PosixSemaphore() {
		if (m_init) {
			if (sem_destroy(&m_sem) == -1) {
				int e = errno;
				char buf[BUFSIZ];
				printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			}
		}
	}
	void Release(int count = 1) {
		for (int i=0; i<count; ++i) {
			if (sem_post(&m_sem) == -1) {
				int e = errno;
				char buf[BUFSIZ];
				printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			}
		}
	}
	void Get(int count = 1) {
		for (int i=0; i<count; ++i) {
			if (sem_wait(&m_sem) == -1) {
				int e = errno;
				char buf[BUFSIZ];
				printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			}	
		}
	}
};

struct EventfdSemaphore
{
	int m_fd = -1;
	EventfdSemaphore() {
		m_fd = eventfd(0, EFD_SEMAPHORE);
		if (m_fd == -1) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
		}
	}
	~EventfdSemaphore() {
		if (m_fd != -1) {
			close(m_fd);
		}
	}
	void Release(int count = 1) {
		uint64_t buf = count;
		if ( sizeof(buf) != write(m_fd, &buf, sizeof(buf))) {
			int e = errno;
			char buf[BUFSIZ];
			printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
		}
	}	
	void Get(int count = 1) {
		for (int i=0; i<count; ++i) {
			uint64_t buf = 0;
			if ( sizeof(buf) != read(m_fd, &buf, sizeof(buf))) {
				int e = errno;
				char buf[BUFSIZ];
				printf("error %s(line:%d), err:%d, %s\n", __FUNCTION__, __LINE__, e, strerror_r(e, buf, sizeof(buf)));
			}
		}
	}
};

const int Test_TimeMs = 3000;

template<typename SemType>
void Test_Create_And_Delete(const char* typeName) {
	printf("%s (%s)\n", __FUNCTION__, typeName);
	
	volatile bool run = true;
	uint64_t count = 0;
	std::thread t([&]() {
		while(run) {
			SemType* sem = new SemType;
			++count;
			delete sem;
		}
	});
	
	auto start = std::chrono::high_resolution_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(Test_TimeMs));

	run = false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	t.join();
	printf("  %lf count per millisec\n\n", count/elapsed.count());
}

template<typename SemType>
void Test_Release_And_Get(const char* typeName, int releaseCount) {
	printf("%s (%s, %d)\n", __FUNCTION__, typeName, releaseCount);
	
	volatile bool run = true;
	uint64_t count = 0;
	
	SemType sem;
	std::thread t1([&]() {
		while(run) {
			sem.Get(releaseCount);
			++count;
		}
	});
	std::thread t2([&]() {
		while (run) {
			sem.Release(releaseCount);
		}
	});
	
	auto start = std::chrono::high_resolution_clock::now();
	std::this_thread::sleep_for(std::chrono::milliseconds(Test_TimeMs));

	run = false;
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	sem.Release(releaseCount);
	t1.join();
	t2.join();
	printf("  %lf count per millisec\n\n", count/elapsed.count());
}

int main()
{
	// SystemV, Posix, eventfd를 기반으로 하는 각 세마포어들의 성능을 비교한다.
	
	// 객체 생성/삭제 비용
	Test_Create_And_Delete<SystemvSemaphore>("SystemvSemaphore");
	Test_Create_And_Delete<PosixSemaphore>("PosixSemaphore");
	Test_Create_And_Delete<EventfdSemaphore>("EventfdSemaphore");
	printf("===================================================\n");
	
	// 이벤트 포스팅, 감지 비용 (생산자-소비자)
	Test_Release_And_Get<SystemvSemaphore>("SystemvSemaphore", 1);
	Test_Release_And_Get<PosixSemaphore>("PosixSemaphore", 1);
	Test_Release_And_Get<EventfdSemaphore>("EventfdSemaphore", 1);
	printf("===================================================\n");
	
	// 한번에 집어넣고 꺼내는 이벤트의 개수를 늘려본다.
	Test_Release_And_Get<SystemvSemaphore>("SystemvSemaphore", 4);
	Test_Release_And_Get<PosixSemaphore>("PosixSemaphore", 4);
	Test_Release_And_Get<EventfdSemaphore>("EventfdSemaphore", 4);
	printf("===================================================\n");
	
	// SystemV Semaphore는 한번에 Release하는 개수가 많을 수록 
	// Release속도가 Get속도보다 월등히 빠르기 때문에 금방 한계값(32767)에 도달하여 ERANGE 에러가 발생한다.
	Test_Release_And_Get<SystemvSemaphore>("SystemvSemaphore", 8); 
	Test_Release_And_Get<PosixSemaphore>("PosixSemaphore", 8);
	Test_Release_And_Get<EventfdSemaphore>("EventfdSemaphore", 8);

	printf("end\n");
	return 0;
}
