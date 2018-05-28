
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
