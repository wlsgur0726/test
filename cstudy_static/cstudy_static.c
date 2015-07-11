/*
 * cstudy_static.c
 *
 *  Created on: 2015. 4. 25.
 *      Author: kjh
 */

#include <stdio.h>

void funcA()
{
	static int num = 0;
	printf("funcA %d\n", ++num);
}

void funcB()
{
	static int num = 0;
	printf("funcB %d\n", ++num);
}

int main()
{
	funcA();
	funcA();

	funcB();
	funcB();

	funcA();
	funcA();

	funcB();
	funcB();

	return 0;
}
