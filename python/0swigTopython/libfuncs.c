#include<stdio.h>
#include "libfuncs.h"

int func1(int a, int b)
{
    return a + b;
}

double func2( double a, double b )
{
    return a * b;
}

void func3(int a, int times)
{
    int sum = 0;
    for(int i = 0; i < times; i ++)
        for(int j = 0; j < a; j ++)
            sum += j;
    printf("%d\n",sum);
}

testStruct func4(int a, double b)
{
    testStruct test;
    test.a = a;
    test.b = b;
    return test;
}
