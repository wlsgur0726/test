#include <stdio.h>
#include <stdlib.h>

#define Specifier 3		// 0 ~ 4
#if Specifier == 0
#	define NOEXCEPT
#
#elif Specifier == 1
#	define NOEXCEPT noexcept
#
#elif Specifier == 2
#	define NOEXCEPT noexcept(false)
#
#elif Specifier == 3
#	define NOEXCEPT throw()
#
#elif Specifier == 4
#	define NOEXCEPT throw(const char*)
#
#endif


#define ThrowType 1		// 0 ~ 1
#if ThrowType == 0
#	define EXCEPT "Test"
#
#else
#	define EXCEPT 12345
#
#endif


struct MyStruct
{
	~MyStruct()
	{
		printf("~MyStruct\n");
	}
};


void ThrowException()
{
	throw EXCEPT;
}


void AAA() NOEXCEPT
{
	MyStruct s;
	printf("%s\n", __FUNCTION__);
	ThrowException();
}


void BBB()
{
	try {
		AAA();
	}
	catch (int e) {
		printf("[%s] Exception! : %d\n", __FUNCTION__, e);
	}
	printf("%s\n", __FUNCTION__);
}


void CCC()
{
	try {
		BBB();
	}
	catch (const char* e) {
		printf("[%s] Exception! : %s\n", __FUNCTION__, e);
	}
	printf("%s\n", __FUNCTION__);
}


int main()
{
	system("pwd");
	CCC();
	return 0;
}
