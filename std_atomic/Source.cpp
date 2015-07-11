#include <thread>
#include <atomic>
#include <iostream>

const int ThreadCount = 4;
const int AddCount = 1000000;
int g_Count = 0;
std::atomic<bool> g_lock = false;
void lock() {
	bool cmp;
	do {
		cmp = false;
	}
	while (false == g_lock.compare_exchange_strong(cmp, true));
}
void unlock() {
	g_lock = false;
}

void TestBehavior()
{
	std::atomic<int> val = 11;
	int cmp = 22;
	int newVal = 11;
	std::cout << "Before \n";
	std::cout << " cmp    :  " << cmp << "\n";
	std::cout << " newVal :  " << newVal << "\n";
	std::cout << " val    :  " << val << "\n\n";

	bool ret = val.compare_exchange_strong(cmp, newVal);

	std::cout << "After \n";
	std::cout << " cmp    :  " << cmp << "\n";
	std::cout << " newVal :  " << newVal << "\n";
	std::cout << " val    :  " << val << "\n";
	std::cout << " ret    :  " << ret << "\n\n";

}

void TestThread()
{
	std::cout << "Start\n";

	std::thread threads[4];
	for (auto& t : threads) {
		t = std::thread([](){
			for (int i = 0; i < AddCount; ++i) {
				lock();
				++g_Count;
				unlock();
			}
		});
	}

	for (auto& t : threads) {
		t.join();
	}

	std::cout << g_Count << "\n";
}
int main()
{
	TestThread();
	return 0;
}