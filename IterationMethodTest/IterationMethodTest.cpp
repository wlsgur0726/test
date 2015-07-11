#include <stdio.h>
#include <stdint.h>
#include <functional>
#include <vector>
#include <chrono>


/*
	=================================
	Windows7 Release x64 i5-4440
	---------------------------------
	Test_Functor start
	14.000800 ms
	714244.900291 count per millisec

	Test_VF start
	26.001500 ms
	384593.196546 count per millisec

	Test_Lambda start
	67.003900 ms
	149245.043945 count per millisec
	=================================
	

	=================================
	CentOS7 Release x64 VM
	---------------------------------
	Test_Functor start
	26.720104 ms
	374250.040344 count per millisec

	Test_VF start
	28.048034 ms
	356531.227822 count per millisec

	Test_Lambda start
	60.362184 ms
	165666.636582 count per millisec
	=================================
*/

const uint64_t TestObjectCount = 1000 * 10000;

struct Object_Fuctor
{
	int n = 1;
	inline void Work_Functor(uint64_t& out) {
		out += n;
	}
};


template <typename T>
struct Functor
{
	T* m_list;
	int m_count;
	inline Functor(T* list, int count)
		: m_list(list)
		, m_count(count)
	{}

	inline void Do(uint64_t& out) {
		for (int i=0; i<m_count; ++i) {
			m_list[i]->Work_Functor(out);
		}
	}
};


struct VirtualFunction
{
	virtual void Work_VF(uint64_t& out) = 0;
};

struct Object_VF : public VirtualFunction
{
	int n = 1;
	virtual void Work_VF(uint64_t& out) override {
		out += n;
	}
};


struct Object_Lambda
{
	int n = 1;
	std::function<void()> m_work_Lambda;
};


void Test_Functor()
{
	std::vector<Object_Fuctor*> list;
	list.reserve(TestObjectCount);
	for (int i=0; i<TestObjectCount; ++i) {
		list.push_back(new Object_Fuctor);
	}

	printf("%s start\n", __FUNCTION__);
	auto start = std::chrono::high_resolution_clock::now();

	uint64_t counter = 0;
	Functor<Object_Fuctor*> f(list.data(), list.size());
	f.Do(counter);

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	if (TestObjectCount!=counter)
		printf("Invalid Test!");
	printf(" %lf ms\n", elapsed.count());
	printf(" %lf count per millisec\n\n", TestObjectCount/elapsed.count());
	for (auto& o : list) {
		delete o;
	}
}

void Test_VF()
{
	std::vector<VirtualFunction*> list;
	list.reserve(TestObjectCount);
	for (int i=0; i<TestObjectCount; ++i) {
		list.push_back(new Object_VF);
	}

	printf("%s start\n", __FUNCTION__);
	auto start = std::chrono::high_resolution_clock::now();
	
	uint64_t counter = 0;
	for (auto& o : list) {
		o->Work_VF(counter);
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;

	if (TestObjectCount!=counter)
		printf("Invalid Test!");
	printf(" %lf ms\n", elapsed.count());
	printf(" %lf count per millisec\n\n", TestObjectCount/elapsed.count());
	for (auto& o : list) {
		delete o;
	}
}

void Test_Lambda()
{
	std::vector<Object_Lambda*> list;
	list.reserve(TestObjectCount);

	uint64_t counter = 0;
	for (int i=0; i<TestObjectCount; ++i) {
		auto o = new Object_Lambda;
		o->m_work_Lambda =[&]() {
			counter += o->n;
		};
		list.push_back(o);
	}

	printf("%s start\n", __FUNCTION__);
	auto start = std::chrono::high_resolution_clock::now();

	for (auto& o : list) {
		o->m_work_Lambda();
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	
	if (TestObjectCount!=counter)
		printf("Invalid Test!");
	printf(" %lf ms\n", elapsed.count());
	printf(" %lf count per millisec\n\n", TestObjectCount/elapsed.count());
	for (auto& o : list) {
		delete o;
	}
}

int main()
{
	Test_Functor();

	Test_VF();

	Test_Lambda();

	return 0;
}
