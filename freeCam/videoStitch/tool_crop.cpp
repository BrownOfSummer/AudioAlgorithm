/*************************************************************************
    > File Name: tool_crop.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-06-28 12:37:38
 ************************************************************************/

#include<stdio.h>
#include<opencv2/opencv.hpp>
#include "stitchutils.h"
using namespace cv;

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("INPUT ERROR:\nUsage: %s input.jpg\n", argv[0]);
        return -1;
    }
    Mat src, dst;
    bool flag = cropWithMat(src, dst);
    if(flag) imwrite("cropped.jpg", dst);
    else printf("Cropped Failed.\n");
    return 0;
}
