/*************************************************************************
    > File Name: registration.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-05-11 11:25:11
 ************************************************************************/

#include<iostream>
#include<vector>
#include<cstdio>
#include<opencv2/opencv.hpp>
#include<opencv2/xfeatures2d.hpp>
#include<opencv2/features2d.hpp>
#include "registration.h"

using namespace cv;
using namespace cv::xfeatures2d;
const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.15f;

#ifndef DEBUG
#define DEBUG 1
#endif

void vlog(const char *format,...);
void vlog(const char *format,...) {
#if DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}
/*
 * Get the match point pairs using ORB method.
 * Paras:
 *      img:        input, image to be warp;
 *      reference:  input, image as template;
 *      imgP:       output, match points in img;
 *      referenceP: output, match points in reference;
 */
void getMatchPoints(Mat img, Mat reference, std::vector<Point2f> &imgP, std::vector<Point2f> &referenceP)
{
    if(img.empty() || reference.empty()) {
        vlog("Input Image ERROR.\n");
        return;
    }
    if( !imgP.empty() ) imgP.clear();
    if( !referenceP.empty() ) referenceP.clear();

    Mat imgGray, referenceGray;
    if(img.channels() != 1) cvtColor(img, imgGray, CV_BGR2GRAY);
    else img.copyTo(imgGray);
    if(reference.channels() != 1) cvtColor(reference, referenceGray, CV_BGR2GRAY);
    else reference.copyTo(referenceGray);

    // Variables to store keypoints and descriptors.
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Detec ORB features and compute descriptors.
    Ptr<Feature2D> orb = ORB::create( MAX_FEATURES );
    orb->detectAndCompute(imgGray, Mat(), keypoints1, descriptors1);
    orb->detectAndCompute(referenceGray, Mat(), keypoints2, descriptors2);

    // Match features.
    std::vector<DMatch> matches;
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    matcher->match(descriptors1, descriptors2, matches, Mat());

    // Sort matches by score.
    std::sort(matches.begin(), matches.end());
    vlog("matches.size() = %d\n", matches.size());

    // Remove not so good matches.
    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin() + numGoodMatches, matches.end());
    vlog("good matches.size() = %d\n", matches.size());

    // Draw imgMatches.
#if DEBUG
    Mat imgMatches;
    drawMatches(img, keypoints1, reference, keypoints2, matches, imgMatches);
    imwrite("matches.jpg", imgMatches);
#endif
    // Extract location of good matches.
    // std::vector<Point2f> points1, points2;
    for(size_t i = 0; i < matches.size(); i ++)
    {
        imgP.push_back( keypoints1[ matches[i].queryIdx ].pt );
        referenceP.push_back( keypoints2[ matches[i].trainIdx ].pt );
    }
    return;
}
/*
 * Find the warpMatrix from img to reference image: warp(img, warpMatrix)->reference;
 * Paras:
 *      imgP:       input, match Points in img;
 *      referenceP: input, match Points in template img;
 *      warpMatrix: output, the warpMatrix result, [2x3] or [3x3].
 *      method:     input:  the warp method
 *                          MOTION_AFFINE
 *                          MOTION_EUCLIDEAN
 *                          MOTION_TRANSLATION
 *                          MOTION_HOMOGRAPHY
 *      return: warpMatrix; referenceP = warpMatrix * imgP
 */
void getWarpMatrixORB(std::vector<Point2f> imgP, std::vector<Point2f> referenceP, Mat &warpMatrix, const int method)
{

    // Find homography.
    if( method ==  MOTION_HOMOGRAPHY)
        warpMatrix = findHomography( imgP, referenceP, RANSAC );
    else 
    {
        // from destination to reference: M * imgP -> referenceP;
        warpMatrix = estimateAffine2D( imgP, referenceP, noArray(), RANSAC );
        //warpMatrix = estimateAffinePartial2D( imgP, referenceP, noArray(), RANSAC );
        //warpMatrix = estimateRigidTransform( imgP, referenceP, false);
    }
    return;
}

/*
 *  Warp the image with the warpMatrix.
 *  Paras: 
 *      img: input, image to be warp;
 *      outSize:    input, the image size after warp;
 *      imgWarp:    output, the image be warped;
 *      warpMatrix: input, the warpMatrix of size[2 x 3] or [3 x 3]
 *      method:     input:  the warp method
 *                          MOTION_AFFINE
 *                          MOTION_EUCLIDEAN
 *                          MOTION_TRANSLATION
 *                          MOTION_HOMOGRAPHY 
 */
void warpImage(Mat img, Mat &imgWarp, Size outSize, Mat warpMatrix, const int method)
{
    // Use homography to warp image.
    if( method == MOTION_HOMOGRAPHY )
    {
        warpPerspective(img, imgWarp, warpMatrix, outSize);
        //warpPerspective(img, imgWarp, warpMatrix, outSize, INTER_LINEAR + WARP_INVERSE_MAP);
    }
    else
    {
        warpAffine(img, imgWarp, warpMatrix, outSize);
        //warpAffine(img, imgWarp, warpMatrix, outSize, INTER_LINEAR + WARP_INVERSE_MAP);
    }
    return;
}
/*
 * Calc the gradients of a image using sobel method.
 * Paras:
 *      src: gray image to calc gradient.
 *      return the sobel gradient
 */
Mat getGradient(Mat src)
{
    Mat src_gray;
    if(src.channels() != 1) cvtColor(src, src_gray, CV_BGR2GRAY);
    else src.copyTo(src_gray);

    // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
    GaussianBlur(src_gray, src_gray, Size(3, 3), 0, 0, BORDER_DEFAULT);
	Mat grad_x, grad_y, grad;
	Mat abs_grad_x, abs_grad_y;

	int scale = 1;	
	int delta = 0;
	int ddepth = CV_32FC1; ;
    
    // Calculate the x and y gradients using Sobel operator

	Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
	convertScaleAbs( grad_x, abs_grad_x );

	Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
	convertScaleAbs( grad_y, abs_grad_y );

    // Combine the two gradients
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
	return grad; 
}

#if CV_MAJOR_VERSION > 2

/* Calc the warp_matrix to align img to reference.
 * Paras:
 *      img,        input, image to be warp;
 *      reference,  input, image as template;
 *      warpMatrix, output, the warp matrix;
 *      warp_mode, input:
 *          MOTION_AFFINE
 *          MOTION_EUCLIDEAN
 *          MOTION_TRANSLATION
 *          MOTION_HOMOGRAPHY
 *      four transfor methods.
 */
void getWarpMatrixECC(Mat img, Mat reference, Mat &warpMatrix, const int warp_mode)
{
    // Set a 2x3 or 3x3 warp matrix depending on the motion model.
    // Initialize the matrix to identity
    if ( warp_mode == MOTION_HOMOGRAPHY )
        warpMatrix = Mat::eye(3, 3, CV_32F);
    else
        warpMatrix = Mat::eye(2, 3, CV_32F);

    // Specify the number of iterations.
    int number_of_iterations = 50;
    
    // Specify the threshold of the increment in the correlation coefficient between two iterations
    double termination_eps = 1e-8;
    
    // Define termination criteria
    TermCriteria criteria (TermCriteria::COUNT+TermCriteria::EPS, number_of_iterations, termination_eps);

    // Run the ECC algorithm. The results are stored in warp_matrix.
    findTransformECC(
                     getGradient(reference),
                     getGradient(img),
                     warpMatrix,
                     warp_mode,
                     criteria
                 );
    return;
}


void warpImageECC(Mat img, Mat &imgWarp, Size outSize, Mat warpMatrix, const int method)
{
    // Use homography to warp image.
    if( method == MOTION_HOMOGRAPHY )
    {
        //warpPerspective(img, imgWarp, warpMatrix, outSize);
        warpPerspective(img, imgWarp, warpMatrix, outSize, INTER_LINEAR + WARP_INVERSE_MAP);
    }
    else
    {
        //warpAffine(img, imgWarp, warpMatrix, outSize);
        warpAffine(img, imgWarp, warpMatrix, outSize, INTER_LINEAR + WARP_INVERSE_MAP);
    }
    return;
}
#endif
