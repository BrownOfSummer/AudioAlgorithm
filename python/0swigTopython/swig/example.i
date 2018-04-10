/* File : example.i */
%module example
%{
/* Put headers and other declarations here */
#include "example.h"
%}
int fact(int n);
int my_mod(int x, int y);
char *get_time();
