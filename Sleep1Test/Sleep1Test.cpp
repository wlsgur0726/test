#include <iostream>
#include <thread>
using namespace std;

int main()
{
	const int Count = 1000;
	auto start = chrono::high_resolution_clock::now();
	for (int i=0; i<Count; ++i) {
		this_thread::sleep_for(chrono::nanoseconds(1));
	}
	std::chrono::duration<double, std::milli> elapsed;
	elapsed = chrono::high_resolution_clock::now() - start;

	cout << "elapsed : " << elapsed.count() << '\n';
	cout << (elapsed.count() / Count) << " counts per ms\n";
	return 0;
}