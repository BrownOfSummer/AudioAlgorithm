/*************************************************************************
    > File Name: registration.h
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-05-11 12:28:41
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
/*
 * MOTION_AFFINE
 * MOTION_EUCLIDEAN
 * MOTION_TRANSLATION
 * MOTION_HOMOGRAPHY 
 */

void getMatchPoints(Mat img, Mat reference, std::vector<Point2f> &imgP, std::vector<Point2f> &referenceP);
void getWarpMatrixORB(std::vector<Point2f> imgP, std::vector<Point2f> referenceP, Mat &warpMatrix, const int method);
void warpImage(Mat img, Mat &imgWrap, Size outSize, Mat warpMatrix, const int method);
Mat getGradient(Mat src);
void getWarpMatrixECC(Mat img, Mat reference, Mat &warpMatrix, const int warp_mode);
