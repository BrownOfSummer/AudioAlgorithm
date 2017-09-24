/*************************************************************************
    > File Name: test.c
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-08-16 16:51:57
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>

#define FLAG 0
int main()
{
#if FLAG
    printf("In position %d\n", 1);
#if 0
    printf("In position %d\n", 2);
#else
    printf("In position %d\n", 3);
#endif
#else
    printf("In position %d\n", 4);
#endif

    int a = 5, b = 5, c = 2;
    //printf("a && b\n", a && b);
    return 0;
}
