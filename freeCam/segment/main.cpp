
#include<iostream>
#include<opencv2/opencv.hpp>
#include "segmentation.h"
using namespace std;
using namespace cv;

static void usage()
{
    cout<<"This program illustrate the use of segmentation.\n";
    cout<<"mainly roughSegment, singleConnect, the filled the holes;\n";
    cout<<"Call:\n";
    cout<<"\t ./bin image.jpg\n";
    cout<<"Using OpenCV version " << CV_VERSION <<endl;
}
int main(int argc, char *argv[])
{
    if( argc != 2 ) {
        usage();
        return -1;
    }
    Mat img = imread(argv[1], 1);
    if(img.rows > 640 || img.cols > 1280) resize(img, img, img.size() / 2);
    Mat gray, binary;
    cvtColor(img, gray, CV_BGR2GRAY);
    roughThreshSegment(img, binary);
    imshow("rough", binary);
    binary = refineBinarySegments(binary);
    imshow("refine", binary);
    waitKey();
    return 0;
}
