/*************************************************************************
    > File Name: test_static.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-08-28 18:45:29
 ************************************************************************/

#include<iostream>
using namespace std;
void add_some(int a)
{
    static int b = 0;
    b = b + a;
    cout<<b<<endl;
}

int main()
{
    for(int i = 0; i < 3; i ++) {
        add_some(10);
    }
    return 0;
}

