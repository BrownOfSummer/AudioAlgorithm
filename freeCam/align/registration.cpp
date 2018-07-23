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
#include "ransac_match.h"

using namespace cv;
using namespace cv::xfeatures2d;
const int MAX_FEATURES = 1500;
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
    //drawMatches(img, keypoints1, reference, keypoints2, matches, imgMatches);
    imgMatches = DrawInlier(img, reference, keypoints1, keypoints2, matches, 2);
    imwrite("matches-before-ransac.jpg", imgMatches);
#endif
    // Extract location of good matches.
    // std::vector<Point2f> points1, points2;
    vector<point_pair> match_pairs;
    vector<int> inlierIdx;
    for(size_t i = 0; i < matches.size(); i ++)
    {
        //imgP.push_back( keypoints1[ matches[i].queryIdx ].pt );
        //referenceP.push_back( keypoints2[ matches[i].trainIdx ].pt );
        point_pair pair = point_pair( keypoints1[ matches[i].queryIdx ].pt, keypoints2[ matches[i].trainIdx ].pt);
        match_pairs.push_back(pair);
    }
    ransacit(match_pairs, inlierIdx);
    vlog("Inliers after RANSAC: %d\n", inlierIdx.size());
    for(size_t i = 0; i < inlierIdx.size(); ++i)
    {
        imgP.push_back( match_pairs[i].a );
        referenceP.push_back( match_pairs[i].b );
    }
#if DEBUG
    Mat ret = drawPointPairs(img, reference, imgP, referenceP, 2);
    imwrite("matches-after-ransac.jpg", ret);
#endif

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
        //warpMatrix = estimateAffine2D( imgP, referenceP, noArray(), RANSAC );
        //warpMatrix = estimateAffinePartial2D( imgP, referenceP, noArray(), RANSAC );
        warpMatrix = estimateRigidTransform( imgP, referenceP, false);
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
    Mat img_gray, reference_gray;
    if(img.channels() > 1) cvtColor(img, img_gray, CV_BGR2GRAY);
    else img.copyTo(img_gray);
    if(reference.channels() > 1) cvtColor(reference, reference_gray, CV_BGR2GRAY);
    else reference.copyTo(reference_gray);
    if ( warp_mode == MOTION_HOMOGRAPHY )
        warpMatrix = Mat::eye(3, 3, CV_32F);
    else
        warpMatrix = Mat::eye(2, 3, CV_32F);

    // Specify the number of iterations.
    int number_of_iterations = 100;
    
    // Specify the threshold of the increment in the correlation coefficient between two iterations
    double termination_eps = 1e-8;
    
    // Define termination criteria
    TermCriteria criteria (TermCriteria::COUNT+TermCriteria::EPS, number_of_iterations, termination_eps);

    // Run the ECC algorithm. The results are stored in warp_matrix.
    findTransformECC(
                     //getGradient(reference),
                     reference_gray,
                     //getGradient(img),
                     img_gray,
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

Mat DrawInlier(Mat &src1, Mat &src2, std::vector<KeyPoint> &kpt1, std::vector<KeyPoint> &kpt2, std::vector<DMatch> &inlier, int type)
{
	const int height = max(src1.rows, src2.rows);
	const int width = src1.cols + src2.cols;
	Mat output(height, width, CV_8UC3, Scalar(0, 0, 0));
	src1.copyTo(output(Rect(0, 0, src1.cols, src1.rows)));
	src2.copyTo(output(Rect(src1.cols, 0, src2.cols, src2.rows)));

    RNG rng(12345);
	if (type == 1)
	{
		for (size_t i = 0; i < inlier.size(); i++)
		{
			Point2f left = kpt1[inlier[i].queryIdx].pt;
			Point2f right = (kpt2[inlier[i].trainIdx].pt + Point2f((float)src1.cols, 0.f));
            Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
			line(output, left, right, color);
		}
	}
	else if (type == 2)
	{
		for (size_t i = 0; i < inlier.size(); i++)
		{
			Point2f left = kpt1[inlier[i].queryIdx].pt;
			Point2f right = (kpt2[inlier[i].trainIdx].pt + Point2f((float)src1.cols, 0.f));
            Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
			line(output, left, right, color);
			circle(output, left, 1, color, 2);
			circle(output, right, 1, color, 2);
		}
        /*
		for (size_t i = 0; i < inlier.size(); i++)
		{
			Point2f left = kpt1[inlier[i].queryIdx].pt;
			Point2f right = (kpt2[inlier[i].trainIdx].pt + Point2f((float)src1.cols, 0.f));
			circle(output, left, 1, Scalar(0, 255, 255), 2);
			circle(output, right, 1, Scalar(0, 255, 0), 2);
		}
        */
	}

	return output;
}

Mat drawPointPairs(const Mat &img1, const Mat&img2, const vector<Point2f> &p1, const vector<Point2f> &p2, const int type)
{
    const int height = std::max(img1.rows, img2.rows);
    const int width = img1.cols + img2.cols;
    Mat output(height, width, CV_8UC3, Scalar(0,0,0));
    img1.copyTo(output(Rect(0, 0, img1.cols, img1.rows)));
    img2.copyTo(output(Rect(img1.cols, 0, img2.cols, img2.rows)));
    RNG rng(12345);
    if(type == 1)
    {
        for(size_t i = 0; i < p1.size(); ++i)
        {
            Point2f left = p1[i];
            Point2f right = p2[i] + Point2f( (float)(img1.cols), 0.0 );
            Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            line(output, left, right, color);
        }
    }
    else
    {
        for(size_t i = 0; i < p1.size(); ++i)
        {
            Point2f left = p1[i];
            Point2f right = p2[i] + Point2f( (float)(img1.cols), 0.0 );

            Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            line(output, left, right, color);
            circle(output, left, 1, color, 2);
            circle(output, right, 1, color, 2);
        }
    }
    return output;
}
/* calc translation based phase. */
Point2d phaseMove(const Mat &img, const Mat &reference)
{
    Mat prev, curr, curr64f, prev64f, hann;
    if( img.channels() > 1 ) cvtColor(img, curr, CV_BGR2GRAY);
    else curr = img.clone();
    if( reference.channels() > 1 ) cvtColor(reference, prev, CV_BGR2GRAY);
    else prev = reference.clone();

    prev.convertTo(prev64f, CV_64F);
    curr.convertTo(curr64f, CV_64F);

    createHanningWindow(hann, curr.size(), CV_64F);
    Point2d shift = phaseCorrelate(prev64f, curr64f, hann);
    
    //cout<<"shift.x = "<<shift.x<<"; shift.y = "<<shift.y<<endl;
    return shift;
}

/* Estimate warp based on solve.
 * Warp Point2f shape1 to shape2.
 * 6 paras for affine:
 * P1[0].x, P1[0].y, 1, 0,       0,       0  --> P2[0].x
 * 0,       0,       0, P1[0].x, P1[0].y, 1  --> P2[0].y
 * P1[1].x, P1[1].y, 1, 0,       0,       0  --> P2[1].x
 * 0        0        0, P1[1].x, P1[1].y, 1  --> P2[1].y
 * ....
 *
 * 4 paras for affine:[(0,0)==(1,1); (0,1) = -(1,0)]
 *  P1[0].x, P1[0].y, 1, 0   --> P2[0].x
 * -P1[0].y, P1[0].x, 0, 1   --> P2[0].y
 *  P1[1].x, P1[1].y, 1, 0   --> P2[1].x
 * -P1[1].y, P1[1].x, 0, 1   --> P2[1].y
 *
 * 4 paras for translation:
 * P1[0].x,     0,      1,  0 --> P2[0].x
 * 0,       P1[0].y,    0,  1 --> P2[0].y
 * P1[1].x,     0,      1,  0 --> P2[1].x
 * 0        P1[1].y,    0,  1 --> P2[1].y
 */
Mat localWarpEstimate(const std::vector<Point2f>& shape1, const std::vector<Point2f>& shape2, int type=1)
{
    Mat out(2,3,CV_32F);
    int siz=2*(int)shape1.size();
    
    /*6 paras affine*/
    if (type == 1)
    {
        Mat matM(siz, 6, CV_32F);
        Mat matP(siz,1,CV_32F);
        int contPt=0;
        for (int ii=0; ii<siz; ii++)
        {
            Mat therow = Mat::zeros(1,6,CV_32F);
            if (ii%2==0)
            {
                therow.at<float>(0,0)=shape1[contPt].x;
                therow.at<float>(0,1)=shape1[contPt].y;
                therow.at<float>(0,2)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].x;
            }
            else
            {
                therow.at<float>(0,3)=shape1[contPt].x;
                therow.at<float>(0,4)=shape1[contPt].y;
                therow.at<float>(0,5)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].y;
                contPt++;
            }
        }
        Mat sol;
        solve(matM, matP, sol, DECOMP_SVD);
        out = sol.reshape(0,2);
    }
    /* 4 paras affine*/
    else if(type == 2)
    {
        Mat matM(siz, 4, CV_32F);
        Mat matP(siz,1,CV_32F);
        int contPt=0;
        for (int ii=0; ii<siz; ii++)
        {
            Mat therow = Mat::zeros(1,4,CV_32F);
            if (ii%2==0)
            {
                therow.at<float>(0,0)=shape1[contPt].x;
                therow.at<float>(0,1)=shape1[contPt].y;
                therow.at<float>(0,2)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].x;
            }
            else
            {
                therow.at<float>(0,0)=-shape1[contPt].y;
                therow.at<float>(0,1)=shape1[contPt].x;
                therow.at<float>(0,3)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].y;
                contPt++;
            }
        }
        Mat sol;
        solve(matM, matP, sol, DECOMP_SVD);
        out.at<float>(0,0)=sol.at<float>(0,0);
        out.at<float>(0,1)=sol.at<float>(1,0);
        out.at<float>(0,2)=sol.at<float>(2,0);
        out.at<float>(1,0)=-sol.at<float>(1,0);
        out.at<float>(1,1)=sol.at<float>(0,0);
        out.at<float>(1,2)=sol.at<float>(3,0);
    }
    /* 4 paras for simple translation */
    else if(type == 3)
    {
        Mat matM(siz, 4, CV_32F);
        Mat matP(siz,1,CV_32F);
        int contPt=0;
        for (int ii=0; ii<siz; ii++)
        {
            Mat therow = Mat::zeros(1,4,CV_32F);
            if (ii%2==0)
            {
                therow.at<float>(0,0)=shape1[contPt].x;
                therow.at<float>(0,1)=0;
                therow.at<float>(0,2)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].x;
            }
            else
            {
                therow.at<float>(0,0)=0;
                therow.at<float>(0,1)=shape1[contPt].y;
                therow.at<float>(0,3)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].y;
                contPt++;
            }
        }
        Mat sol;
        solve(matM, matP, sol, DECOMP_SVD);
        //cout<<sol<<endl;
        out.at<float>(0,0)=sol.at<float>(0,0);
        out.at<float>(0,1)=0;
        out.at<float>(0,2)=sol.at<float>(0,2);
        out.at<float>(1,0)=0;
        out.at<float>(1,1)=sol.at<float>(0,1);
        out.at<float>(1,2)=sol.at<float>(0,3);
    }
    /* 3 paras for translation*/
    else
    {
        Mat matM(siz, 3, CV_32F);
        Mat matP(siz,1,CV_32F);
        int contPt=0;
        for (int ii=0; ii<siz; ii++)
        {
            Mat therow = Mat::zeros(1,3,CV_32F);
            if (ii%2==0)
            {
                therow.at<float>(0,0)=shape1[contPt].x;
                therow.at<float>(0,1)=1;
                therow.at<float>(0,2)=0;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].x;
            }
            else
            {
                therow.at<float>(0,0)=shape1[contPt].y;
                therow.at<float>(0,1)=0;
                therow.at<float>(0,2)=1;
                therow.row(0).copyTo(matM.row(ii));
                matP.at<float>(ii,0) = shape2[contPt].y;
                contPt++;
            }
        }
        Mat sol;
        solve(matM, matP, sol, DECOMP_SVD);
        //cout<<sol<<endl;
        out.at<float>(0,0)=sol.at<float>(0,0);
        out.at<float>(0,1)=0;
        out.at<float>(0,2)=sol.at<float>(0,1);
        out.at<float>(1,0)=0;
        out.at<float>(1,1)=sol.at<float>(0,0);
        out.at<float>(1,2)=sol.at<float>(0,2);
    }
    return out;
}
