#pragma once

#include <iostream>

struct TestStruct
{
	int n;

	TestStruct() {
		std::cout << "TestStruct() \n";
	}
	TestStruct(const TestStruct&) {
		std::cout << "TestStruct(const TestStruct&) \n";
	}
	TestStruct(TestStruct&&) {
		std::cout << "TestStruct(TestStruct&&) \n";
	}
	TestStruct& operator = (const TestStruct&) {
		std::cout << "operator = (const TestStruct&) \n";
		return *this;
	}
	TestStruct& operator = (TestStruct&&) {
		std::cout << "operator = (TestStruct&&) \n";
		return *this;
	}
	~TestStruct() {
		std::cout << "~TestStruct() \n";
	}
};

struct TestClass
{
	template <typename T>
	T Func(T&);
};