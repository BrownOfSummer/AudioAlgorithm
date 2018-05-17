/*************************************************************************
    > File Name: test_wrap.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-05-11 12:30:30
 ************************************************************************/

#include<iostream>
#include<vector>
#include<opencv2/opencv.hpp>
#include "registration.h"
using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    Mat reference = imread( argv[1] );
    cout<<"Read "<<argv[1]<<" as reference !\n";
    Mat im = imread( argv[2] );
    cout<<"Read "<<argv[2]<<" to align. \n";

    cout<<"Get match points ...\n";
    vector<Point2f> imgP, referenceP;
    getMatchPoints(im, reference, imgP, referenceP);
    
    Mat imRegistered, warpMatrix;
    cout<<"Get warpMatrix ...\n";
    getWarpMatrixORB(imgP, referenceP, warpMatrix, MOTION_AFFINE);
    cout<<warpMatrix<<endl;
    if(warpMatrix.empty()) {
        cout<<"Can not get warp matrix !\n";
        return 0;
    }
    cout<<"Warp images ...\n";
    warpImage(im, imRegistered, reference.size(), warpMatrix, MOTION_AFFINE);
    cout<<"Saving aligned image: aligned.jpg\n";
    imwrite("aligned.jpg", imRegistered);
    cout<<"Estimated homography:\n";
    return 0;
}
