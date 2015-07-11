#include <stdio.h>
#include <new>

struct Data
{
	int a = 0;
	int b = 1;
	int c = 2;
	Data() {
		printf("[%p] Constructor1\n", this);
	}
	Data(int _a, int _b, int _c) {
		a = _a;
		b = _b;
		c = _c;
		printf("[%p] Constructor2 %d %d %d\n", this, a, b, c);
	}
	Data(Data& _a, Data& _b, Data& _c) {
		a = _a.a;
		b = _b.b;
		c = _c.c;
		printf("[%p] Constructor3 %d %d %d\n", this, a, b, c);
	}
	~Data() {
		printf("[%p] Destructor\n", this);
	}
	void Print() {
		printf("[%p] %d %d %d\n", this, a, b, c);
	}
};

template<typename T>
struct Node
{
	char data[sizeof(T)];

	template<typename... ARGS>
	Node(ARGS&&... args) {
		T* _this = (T*)data;
		::new(_this) T(args...);
	}
	~Node() {
		T* _this = (T*)data;
		_this->~T();
	}
};

void Test1()
{
	Data g;
	{
		printf("Start %s\n", __FUNCTION__);
		auto node = new Node<Data>(g, g, g);
		Data* obj = (Data*)node;
		obj->Print();
		delete node;
	}
}



struct TestClass
{
	char data[100];
	TestClass() {
		printf("constructor %p\n", this);
	}

	virtual ~TestClass() {
		printf("destructor %p\n", this);
	}

	static void* operator new (std::size_t sz){
		void* p = ::operator new(sz);
		printf("operator new %p\n", p);
		return p;
	}

		static void operator delete (void* ptr){
		printf("operator delete %p\n", ptr);
		::operator delete(ptr);
	}
};

void Test2()
{
	{
		TestClass c;
	}

	printf("---------------------------------\n");

	{
		TestClass* c = new TestClass;
		delete c;
	}

	printf("---------------------------------\n");

	{
		TestClass* c = (TestClass*) operator new (sizeof(TestClass));
		operator delete(c);
	}
}

int main()
{
	Test1();
	printf("===================================\n");
	Test2();
	return 0;
}