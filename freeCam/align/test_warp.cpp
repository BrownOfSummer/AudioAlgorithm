/*************************************************************************
    > File Name: test_wrap.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-05-17 16:47:53
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
#include "registration.h"
using namespace std;
using namespace cv;

void generateWarpMatrix( Mat &warp_matrix, const int warpType )
{
    RNG rng( getTickCount() );
    double angle;
    switch( warpType )
    {
        case MOTION_TRANSLATION:
            warp_matrix = ( Mat_<float>(2,3) << 
                    1, 0, (rng.uniform(10.f, 20.f)),
                    0, 1, (rng.uniform(10.f, 20.f)));
            break;
        case MOTION_EUCLIDEAN:
            angle = CV_PI / 30 + CV_PI*rng.uniform((double)-2.f, (double)2.f)/180;
            cout<<"angel = "<<angle<<endl;
            //Mat rot_mat = getRotationMatrix2D(center, angle * 180 / CV_PI, scale=1 );
            warp_matrix = ( Mat_<float>(2,3) << 
                    cos(angle), sin(angle), (rng.uniform(10.f, 20.f)),
                    -sin(angle), cos(angle), (rng.uniform(10.f, 20.f)));
            break;
        case MOTION_AFFINE:
            warp_matrix = (Mat_<float>(2,3) << (1-rng.uniform(-0.05f, 0.05f)),
                (rng.uniform(-0.03f, 0.03f)), (rng.uniform(10.f, 20.f)),
                (rng.uniform(-0.03f, 0.03f)), (1-rng.uniform(-0.05f, 0.05f)),
                (rng.uniform(10.f, 20.f)));
            break;
        case MOTION_HOMOGRAPHY:
            warp_matrix = (Mat_<float>(3,3) << (1-rng.uniform(-0.05f, 0.05f)),
                (rng.uniform(-0.03f, 0.03f)), (rng.uniform(10.f, 20.f)),
                (rng.uniform(-0.03f, 0.03f)), (1-rng.uniform(-0.05f, 0.05f)),(rng.uniform(10.f, 20.f)),
                (rng.uniform(0.0001f, 0.0003f)), (rng.uniform(0.0001f, 0.0003f)), 1.f);
            break;
        default:
            cout<<"warpType must be one of: \n\tMOTION_TRANSLATION\n\tMOTION_EUCLIDEAN\n\tMOTION_AFFINE\n\tMOTION_HOMOGRAPHY\n";

    }
    return;
}
int main(int argc, char *argv[])
{
    Mat warp_matrix, src, src_warped;
    const int warpType = MOTION_EUCLIDEAN;
    //const int warpType = MOTION_TRANSLATION;
    generateWarpMatrix(warp_matrix, warpType);
    cout<<"WARP:\n"<<warp_matrix<<endl;

    src = imread( argv[1], 1 );
    warpImage( src, src_warped, src.size(), warp_matrix, warpType );
    Mat reference = src;

    imshow("reference", reference);
    imshow("src_warped", src_warped);
    Mat warp1, warp2;
    Mat warpImg1, warpImg2;
    
    vector<Point2f> imgP, referenceP;
    getMatchPoints(src_warped, reference, imgP, referenceP);
    getWarpMatrixORB(imgP, referenceP, warp1, warpType);
    cout<<"ORB:\n"<<warp1<<endl;
    warpImage(src_warped, warpImg1, reference.size(), warp1, warpType);
    imshow("ORB-warp", warpImg1);

#if CV_MAJOR_VERSION > 2
    getWarpMatrixECC(src_warped, reference, warp2, warpType);
    cout<<"ECC:\n"<<warp2<<endl;
    warpImageECC(src_warped, warpImg2, reference.size(), warp2, warpType);
    imshow("ECC-warp", warpImg2);
#endif
    waitKey();
    return 0;
}
