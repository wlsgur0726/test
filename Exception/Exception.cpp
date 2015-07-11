#ifdef _WIN32
#include <vld.h>
#endif
#include <exception>
#include <string>

struct TestException : public std::exception
{
	std::string m_msg;
	TestException(const char* str = "") {
		m_msg = str;
	}
	virtual const char* what() const throw() override {
		return m_msg.c_str();
	}
};


struct MyClass 
{
	struct Resource
	{
		char data[100];
		Resource() {
			puts(__FUNCTION__);
		}
		~Resource() {
			puts(__FUNCTION__);
		}
	};

	Resource* resource = nullptr;
	MyClass() {
		puts(__FUNCTION__);
		resource = new Resource;
	}
	~MyClass() {
		puts(__FUNCTION__);
		throw TestException("destructor");
		delete resource;
	}
};

void Test1();
void Test2();

int main()
{
	Test1();
	return 0;
}


void Test1()
{
	puts(__FUNCTION__);
	try {
		MyClass obj;
	}
	catch (std::exception& e) {
		printf("exception! %s\n", e.what());
	}
}

void Stack2() {
	throw TestException("TestException!!");
}
void Stack1() {
	try {
		Stack2();
	}
	catch (...) {
		printf("Stack1 Unknown exception!\n");
		throw;
	}
}
void Test2()
{
	puts(__FUNCTION__);
	try {
		Stack1();
	}
	catch (TestException& e) {
		printf("TestException : %s\n", e.what());
	}
	catch (std::exception& e) {
		printf("std::exception : %s\n", e.what());
	}
	catch (...) {
		printf("Unknown exception\n");
	}
}
