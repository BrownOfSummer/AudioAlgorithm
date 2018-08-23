/*************************************************************************
    > File Name: match_surf.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-08-21 13:30:01
 ************************************************************************/

#include<iostream>
#include<cstdio>
#include<opencv2/opencv.hpp>
#include<string>
#include<vector>
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"
using namespace std;
using namespace cv;
using namespace cv::detail;
void help()
{
    printf("Demonstrate the match process of surf matches used in stitching detail.\n");
    printf("Decrease the \"match_conf\" to get more matches.\n");
    printf("Usage:\n\t./match_surf img1 img2 [match_conf]\n");
}
float match_conf = 0.6; // higher is more robust, lower more matches.
Mat DrawInlier(Mat &src1, Mat &src2, std::vector<KeyPoint> &kpt1, std::vector<KeyPoint> &kpt2, std::vector<DMatch> &inlier, int type);
int main(int argc, char *argv[])
{
    if( argc != 3 && argc != 4)
    {
        help();
        return -1;
    }
    if( argc == 4 ) match_conf = atof( argv[3] );
    Ptr<FeaturesFinder> finder = new SurfFeaturesFinder();
    int num_images = 2;
    vector<ImageFeatures> features(num_images);
    vector<Mat> images;
    for( int i = 1; i < 3; i ++ )
    {
        printf("%s\n", argv[i]);
        Mat img = imread(argv[i]);
        if( img.empty() ) printf("Read %s ERROR!\n", argv[i]);
        images.push_back(img);
    }
    for (int i = 0; i < num_images; ++i)
    {
        Mat img = images[i];
        (*finder)(img, features[i]);
        features[i].img_idx = i;
        printf("Features in image # %d: %ld\n", i + 1, features[i].keypoints.size());
    }
    finder->collectGarbage();
    printf("Pairwise matching\n");

    vector<MatchesInfo> pairwise_matches;
    BestOf2NearestMatcher matcher(false, match_conf);
    matcher(features, pairwise_matches);
    //printf("pairwise_matches.size() = %ld\n", pairwise_matches.size());
    /*
    for(size_t i = 0; i < pairwise_matches.size(); i ++)
    {
        printf("src_idx=%d, dst_idx=%d, num_inliers=%d, DMatch_size=%ld; conf = %lf\n", 
        pairwise_matches[i].src_img_idx, pairwise_matches[i].dst_img_idx, pairwise_matches[i].num_inliers, pairwise_matches[i].matches.size(), pairwise_matches[i].confidence);
    }
    */
    matcher.collectGarbage();
    Mat img_corr = DrawInlier(images[0], images[1], features[0].keypoints, features[1].keypoints, pairwise_matches[1].matches, 3);
    imwrite("match.jpg", img_corr);
    printf("match_conf = %.3f; %ld matches .\n", match_conf, pairwise_matches[1].matches.size());
    return 0;
}
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
    else if(type == 2)
    {
        for (size_t i = 0; i < inlier.size(); i++)
        {
            Point2f left = kpt1[inlier[i].queryIdx].pt;
            Point2f right = (kpt2[inlier[i].trainIdx].pt + Point2f((float)src1.cols, 0.f));
            Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            line(output, left, right, color, 2);
            circle(output, left, 1, color, 5);
            circle(output, right, 1, color, 5);
        }
    }
    else
    {
        for(size_t i = 0; i < kpt1.size(); ++i)
        {
            circle(output, kpt1[i].pt, 1, Scalar(0, 0, 255), 3);
        }
        for(size_t i = 0; i < kpt2.size(); ++i)
        {
            Point2f right = kpt2[i].pt + Point2f( (float)src1.cols, 0.f );
            circle(output, right, 1, Scalar(0, 255, 0), 3);
        }
        for (size_t i = 0; i < inlier.size(); i++)
        {
            Point2f left = kpt1[inlier[i].queryIdx].pt;
            Point2f right = (kpt2[inlier[i].trainIdx].pt + Point2f((float)src1.cols, 0.f));
            Scalar color = Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            line(output, left, right, color, 2);
        }
    }

    return output;
}
