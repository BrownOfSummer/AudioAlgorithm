/*************************************************************************
    > File Name: utils.h
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-05-26 15:08:15
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
#ifndef DEBUG
#define DEBUG 1
#endif

void vlog(const char *format,...);
Mat Erosion( Mat src, int erosion_elem, int erosion_size );
Mat Dilation( Mat src, int dilation_elem, int dilation_size);
Mat getGradient(Mat src);
void detectPeopleArea(Mat img);
