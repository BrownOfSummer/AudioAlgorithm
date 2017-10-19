/*************************************************************************
    > File Name: test_std_queue.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-10-17 16:06:41
 ************************************************************************/

#include<iostream>
#include<random>
using namespace std;

bool FindConfirmMatch( int *num, int num_length, int thresh_num, int confirm_length ); 
bool FindConfirmMatch2( int *num, int num_length, int thresh_num, int confirm_length ); 
int main()
{
    //std::queue<int> score;
    int num1[] = {1,1,0,0,0,0,1,1,1,0,0,1,0,1,0,1,1,1,1,0};
    int num2[] = {1,1,0,0,0,0,1,0,1,0,0,1,0,1,0,1,1,1,1,1};
    int num3[] = {0,0,0,0,0,0,1,0,1,0,0,1,0,1,0,1,0,1,1,1};
    int num4[] = {1,1,1,1,0,0,1,0,1,0,0,1,0,1,0,1,1,1,1,1};
    int num[20];
    //FindConfirmMatch( num4, 20, 0, 3 );
    //FindConfirmMatch2( num4, 20, 0, 3 );
    std::srand(std::time(0)); // use current time as seed for random generator
    for(int i = 0; i < 20; i++) {
        num[i] = rand() % 2;
    }
    FindConfirmMatch( num, 20, 0, 3 );
    FindConfirmMatch2( num, 20, 0, 3 );
    return 0;
}

bool FindConfirmMatch2( int *num, int num_length, int thresh_num, int confirm_length ) 
{
    int cnt = 0;
    int tmpIdx = -1, startIdx = -1, endIdx = -1;
    int i = 0;
    for(i = 0; i < num_length; i ++) {
        cout<<"i = "<<i<<"; "<<num[i]<<endl;
        if( num[i] > thresh_num ) {
            cnt ++;
            endIdx = i;
            if( cnt >= confirm_length )
                break;
        }
        else {
            cnt = 0;
        }
    }
    if( cnt >= confirm_length ) {
        cout<<"cnt = "<<cnt<<endl;
        startIdx = endIdx - cnt + 1;
        cout<<"start = "<<startIdx<<"; endIdx = "<<endIdx<<endl;
    }
    else {
        cout<<"No Confirm Match !"<<endl;
        return false;
    }
    return true;
}
bool FindConfirmMatch( int *num, int num_length, int thresh_num, int confirm_length ) 
{
    int cnt = 0;
    int tmpIdx = -1, startIdx = -1, endIdx = -1;
    bool need_skip = true;
    int i = 0;
    for(i = 0; i < num_length; i ++) {
        cout<<"i = "<<i<<"; "<<num[i]<<endl;
        if( need_skip  ) {
            if( num[i] > thresh_num ) {
                need_skip = false;
                cnt ++;
                tmpIdx = i;
            }
            continue;
        }
        if( num[i] > thresh_num) {
            cnt ++;
            if( cnt >= confirm_length ) {
                startIdx = tmpIdx;
                endIdx = i;
                break;
            }
        }
        else {
            need_skip = true;
            cnt = 0;
        }
    }
    if( cnt >= confirm_length ) {
        cout<<"cnt = "<<cnt<<endl;
        cout<<"start = "<<startIdx<<"; endIdx = "<<endIdx<<endl;
    }
    else {
        cout<<"No Confirm Match !"<<endl;
        return false;
    }
    return true;
}
