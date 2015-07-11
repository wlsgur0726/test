/*
 * cstudy_div.c
 *
 *  Created on: 2015. 4. 25.
 *      Author: kjh
 */


#include <stdio.h>

void TestInt(int a,
			 int b,
			 int c)
{
	printf("TestInt\n");
	printf(" a = %d\n", a);
	printf(" b = %d\n", b);
	printf(" c = %d\n", c);
	printf(" (a/c)+(b/c) = %d\n", (a/c)+(b/c));
	printf(" (a+b)/c = %d\n", (a+b)/c);
	printf("\n");
}

void TestFloat(float a,
			   float b,
			   float c)
{
	printf("TestFloat\n");
	printf(" a = %f\n", a);
	printf(" b = %f\n", b);
	printf(" c = %f\n", c);
	printf(" (a/c)+(b/c) = %f\n", (a/c)+(b/c));
	printf(" (a+b)/c = %f\n", (a+b)/c);
	printf("\n");
}

#define A 10
#define B 20
#define C 3

int main()
{
	TestInt(A, B, C);
	TestFloat(A, B, C);
	return 0;
}
