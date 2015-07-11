#include <thread>
#include <stdio.h>

#define SOMEDATA SomeData(__LINE__)
struct SomeData
{
	int n = 0;
	SomeData(int ln) {
		n = ln;
		printf("%s %d\n", __FUNCTION__, ln);
	}
	SomeData(const SomeData& s) {
		printf("%s %d (copy)\n", __FUNCTION__, s.n);
	}
	~SomeData() {
		printf("%s %d\n", __FUNCTION__, n);
	}
};

struct SomeObject
{
	SomeObject() {
		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
	SomeObject(int) {
		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
	SomeObject(int, float) {
		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
	SomeObject(float& out) {
		out += 1.11;
		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
	SomeObject(SomeData d) {
		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
	~SomeObject() {
		printf("%s %d\n", __FUNCTION__, __LINE__);
	}
};


template<typename T>
struct Ptr
{
	T* m_data = nullptr;

	Ptr() {}

	template<typename... ARGS>
	Ptr(ARGS&&... args) {
		m_data = new T(args...);
	}
	~Ptr() {
		if (m_data != nullptr)
			delete m_data;
	}
};

int main()
{
	std::thread t;
	float f = 0;
	Ptr<SomeObject> p(SOMEDATA);
	printf("f : %f\n", f);
	return 0;
}