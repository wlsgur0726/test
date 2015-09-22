#include "temp.h"

template<>
int TestClass::Func<int>(int &a)
{
	std::cout << __FUNCTION__ << " int \n";
	a = 123;
	return a;
}

template<>
float TestClass::Func<float>(float& a)
{
	std::cout << __FUNCTION__ << " float \n";
	return a;
}

template<>
const char* TestClass::Func<const char*>(const char*& a)
{
	std::cout << __FUNCTION__ << " const char* \n";
	a = __FUNCTION__;
	return a;
}

template<>
TestStruct TestClass::Func<TestStruct>(TestStruct& a)
{
	std::cout << __FUNCTION__ << " TestStruct \n";
	a.n = 321;
	return a;
}
