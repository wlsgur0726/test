/*
 * ThreadLocalStroageTest.cpp
 *
 *  Created on: 2015. 4. 27.
 *      Author: root
 */


#include <stdio.h>
#include <thread>
using namespace std;

#if 0
	#define TLS
#else
	#if defined(_MSC_VER) && _MSC_VER <= 1800
		#define TLS __declspec(thread) // VS2013 이하
	#else
		#define TLS thread_local
	#endif
#endif

struct TestClass
{
	int n = 0;
	TestClass(){
		printf("create %p\n", this);
	}
	void Func(){
		printf("TestClass %p %d\n", this, ++n);
	}
	~TestClass() {
		printf("delete %p %d\n", this, n);
	}
};

TLS int t_testVal = 0;
TLS TestClass t_testClass;

const int TestCount = 1000000;
int main()
{
	thread threads[4];
	for (auto& t : threads) {
		t = thread([](){
			for (int i=0; i<TestCount; ++i)
				++t_testVal;
			t_testClass.Func();
			t_testClass.Func();
			printf("%p : %d\n", &t_testVal, t_testVal);
		});
	}
	
	for (auto& t : threads) {
		t.join();
	}
	
	return 0;
}
