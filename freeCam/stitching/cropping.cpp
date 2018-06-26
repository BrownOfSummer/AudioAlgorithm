/*************************************************************************
    > File Name: cropping.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-06-26 16:48:16
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
#include "cropping.h"
using namespace std;
using namespace cv;

#define CUTBLACKTHREASHOLD 0.05

bool checkBlackRow(const cv::Mat& roi, int y, const cv::Rect &rect) {
    int zeroCount = 0;
    for(int x=rect.x; x<rect.width; x++) {
        if(roi.at<uchar>(y, x) == 0) {
            zeroCount++;
        }
    }
    if((zeroCount/(float)roi.cols)>CUTBLACKTHREASHOLD) {
        return false;
    }
    return true;
}


bool checkBlackColumn(const cv::Mat& roi, int x,const cv::Rect &rect) {
    int zeroCount = 0;
    for(int y=rect.y; y<rect.height; y++) {
        if(roi.at<uchar>(y, x) == 0) {
            zeroCount++;
        }
    }
    if((zeroCount/(float)roi.rows)>CUTBLACKTHREASHOLD) {
        return false;
    }
    return true;
}
//+ (bool) cropWithMat: (const cv::Mat &)src andResult: (cv::Mat &)dest {
bool cropWithMat(const cv::Mat &src, cv::Mat &dest) {
    cv::Mat gray;
    cvtColor(src, gray, CV_BGR2GRAY);//convert src to gray

    cv::Rect roiRect(0,0,gray.cols,gray.rows); // start as the source image - ROI is the complete SRC-Image

    while (1) {
        //printf("%d %d %d %d\n",roiRect.x,roiRect.y,roiRect.width,roiRect.height);

        bool isTopNotBlack=checkBlackRow(gray, roiRect.y,roiRect);
        bool isLeftNotBlack=checkBlackColumn(gray, roiRect.x,roiRect);
        //bool isBottomNotBlack=checkBlackRow(gray, roiRect.y+roiRect.height,roiRect);
        bool isBottomNotBlack=checkBlackRow(gray, roiRect.y+roiRect.height-1,roiRect);
        //bool isRightNotBlack=checkBlackColumn(gray, roiRect.x+roiRect.width,roiRect);
        bool isRightNotBlack=checkBlackColumn(gray, roiRect.x+roiRect.width-1,roiRect);

        if(isTopNotBlack && isLeftNotBlack && isBottomNotBlack && isRightNotBlack) {
            cv::Mat imageReference = src(roiRect);
            imageReference.copyTo(dest);
            return true;
        }
        // If not, scale ROI down
        // if x is increased, width has to be decreased to compensate
        if(!isLeftNotBlack) {
            roiRect.x++;
            roiRect.width--;
        }
        // same is valid for y
        if(!isTopNotBlack) {
            roiRect.y++;
            roiRect.height--;
        }
        if(!isRightNotBlack) {
            roiRect.width--;
        }
        if(!isBottomNotBlack) {
            roiRect.height--;
        }
        if(roiRect.width <= 0 || roiRect.height <= 0) {
            return false;
        }
    }
}
