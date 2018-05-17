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
    Mat imgMatches;
    drawMatches(img, keypoints1, reference, keypoints2, matches, imgMatches);
    imwrite("matches.jpg", imgMatches);
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
 */
void getWarpMatrixORB(std::vector<Point2f> imgP, std::vector<Point2f> referenceP, Mat &warpMatrix, const int method)
{

    // Find homography.
    if( method ==  MOTION_HOMOGRAPHY)
        warpMatrix = findHomography( imgP, referenceP, RANSAC );
    else
        warpMatrix = estimateAffine2D( imgP, referenceP, noArray(), RANSAC );
        //warpMatrix = estimateAffinePartial2D( imgP, referenceP, noArray(), RANSAC );
        //warpMatrix = estimateRigidTransform( imgP, referenceP, false);

    // Use homography to warp image.
    //warpPerspective(img, imgWrap, warpMatrix, reference.size());
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
void warpImage(Mat &img, Mat &imgWarp, Size outSize, Mat &warpMatrix, const int method)
{
    // Use homography to warp image.
    if( method == MOTION_HOMOGRAPHY )
        warpPerspective(img, imgWarp, warpMatrix, outSize);
    else
        warpAffine(img, imgWarp, warpMatrix, outSize);
    //warpAffine(img, imgWarp, warpMatrix, outSize, INTER_LINEAR, BORDER_TRANSPARENT);
    return;
}
