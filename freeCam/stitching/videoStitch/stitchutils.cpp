/*************************************************************************
    > File Name: cropping.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-06-26 16:48:16
 ************************************************************************/

#include<iostream>
#include<vector>
#include<string>
#include<stdio.h>
#include<stdarg.h>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include<opencv2/opencv.hpp>
#include "stitchutils.h"
using namespace std;
using namespace cv;

#ifndef DEBUG
#define DEBUG 1
#endif
//void vlog(const char *format,...);
void vlog(const char *format,...)
{
#if DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

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

bool cropimg2rect(const cv::Mat &src, cv::Rect &rect)
{
    cv::Mat gray;
    cvtColor(src, gray, CV_BGR2GRAY);//convert src to gray
    cv::Rect roiRect(0,0,gray.cols,gray.rows); // start as the source image - ROI is the complete SRC-Image

    while (1)
    {
        bool isTopNotBlack=checkBlackRow(gray, roiRect.y,roiRect);
        bool isLeftNotBlack=checkBlackColumn(gray, roiRect.x,roiRect);
        bool isBottomNotBlack=checkBlackRow(gray, roiRect.y+roiRect.height-1,roiRect);
        bool isRightNotBlack=checkBlackColumn(gray, roiRect.x+roiRect.width-1,roiRect);

        if(isTopNotBlack && isLeftNotBlack && isBottomNotBlack && isRightNotBlack) {
            rect = roiRect;
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

vector<string> listDir( const char* path, const char *ext )
{
    DIR* dirFile = opendir( path );
    vector<string> ret;
    if ( dirFile ) 
    {
        struct dirent* hFile;
        //errno = 0;
        while (( hFile = readdir( dirFile )) != NULL ) 
        {
            // skip . and ..
            if ( !strcmp( hFile->d_name, "."  )) continue;
            if ( !strcmp( hFile->d_name, ".." )) continue;

            // in linux hidden files all start with '.', skip it
            if (hFile->d_name[0] == '.' ) continue;

            // dirFile.name is the name of the file. Do whatever string comparison 
            // you want here. Something like:
            if(ext != NULL)
            {
                if ( strstr( hFile->d_name, ext ))
                    ret.push_back( hFile->d_name );
            }
            else
                ret.push_back( hFile->d_name );
      } 
      closedir( dirFile );
   }
    return ret;
}

void check_dir(const char *dirpath)
{
    struct stat st = {0};

    if (stat(dirpath, &st) == -1)
    {
        printf("mkdir %s\n", dirpath);
        mkdir(dirpath, 0777);
    }
}
