#include <iostream>
#include <thread>
#include <memory>
#include <stdint.h>

const int TestTimeSec = 5;
template <size_t SIZE>
struct _Obj
{
	static constexpr size_t Size()
	{
		return SIZE;
	}

	char data[SIZE];
};


template <typename Func>
uint64_t Test(const Func& f)
{
	volatile bool run = true;
	std::thread t([&run]()
	{
		std::this_thread::sleep_for(std::chrono::seconds(TestTimeSec));
		run = false;
	});
	t.detach();

	uint64_t count = 0;
	while (run) {
		f();
		++count;
	}
	return count;
}

template <size_t SIZE>
void TestMain()
{
	std::cout << "\n SIZE : " << SIZE << "\n";
	typedef _Obj<SIZE> Obj;
	uint64_t result;


	printf("  Test new&delete...\n");
	Obj* p = nullptr;
	result = Test([&p]()
	{
		if (p == nullptr)
			p = new Obj;
		else {
			delete p;
			p = nullptr;
		}
	});
	std::cout << "  Result : " << (result/(1000*TestTimeSec)) << " per ms\n\n";


	printf("  Test copy...\n");
	Obj a, b;
	bool s = true;
	result = Test([&a, &b, &s]()
	{
		if (s)
			a = b;
		else
			b = a;
		s = !s;
	});
	std::cout << "  Result : " << (result/(1000*TestTimeSec)) << " per ms\n\n";


}


void PtrTest()
{
	std::cout << "\n ptr swap test\n";
	typedef _Obj<512> Obj;
	uint64_t result;

	printf("  Test nomal ptr...\n");
	Obj* p1 = new Obj;
	Obj* p2 = nullptr;
	result = Test([&p1, &p2]()
	{
		std::swap(p1, p2);
	});
	std::cout << "  Result : " << (result/(1000*TestTimeSec)) << " per ms\n\n";


	printf("  Test shared_ptr...\n");
	std::shared_ptr<Obj> sp1(new Obj);
	std::shared_ptr<Obj> sp2(nullptr);
	result = Test([&sp1, &sp2]()
	{
		sp1.swap(sp2);
	});
	std::cout << "  Result : " << (result/(1000*TestTimeSec)) << " per ms\n\n";


	printf("  Test unique_ptr...\n");
	std::unique_ptr<Obj> up1(new Obj);
	std::unique_ptr<Obj> up2(nullptr);
	result = Test([&up1, &up2]()
	{
		up1.swap(up2);
	});
	std::cout << "  Result : " << (result/(1000*TestTimeSec)) << " per ms\n\n";
}


int main()
{
	std::cout << "nothing : " << (Test([](){})/(1000*TestTimeSec)) << " per ms\n\n";
	const int TestCount = 1;
	for (int i=0; i<TestCount; ++i) {
		printf("\n==================================================\n");
		printf("Test %d\n", i+1);
		PtrTest();
		TestMain<8>();
		TestMain<32>();
		TestMain<128>();
		TestMain<512>();
		TestMain<1024>();
		TestMain<1024*4>();
		TestMain<1024*256>();
	}
	return 0;
}