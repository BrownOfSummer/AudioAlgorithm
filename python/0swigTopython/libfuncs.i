/* File: libfuncs.i */
%module libfuncs
%{
#include "libfuncs.h"
%}
/*%include "libfuncs.h"*/
typedef struct {
    int a;
    double b;
}testStruct;
int func1(int a, int b);
double func2(double a, double b);
void func3(int a, int times);
testStruct func4(int a, double b);
/*Or %include libfuncs.h*/
