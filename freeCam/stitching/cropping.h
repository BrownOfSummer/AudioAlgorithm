/*************************************************************************
    > File Name: cropping.h
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-06-26 16:59:22
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;

bool cropWithMat(const cv::Mat &src, cv::Mat &dest);
bool checkBlackColumn(const cv::Mat& roi, int x,const cv::Rect &rect);
bool checkBlackRow(const cv::Mat& roi, int y, const cv::Rect &rect);
