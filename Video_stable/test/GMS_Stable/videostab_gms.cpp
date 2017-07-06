/*************************************************************************
    > File Name: image_align.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-06-22 10:52:35
 ************************************************************************/

#include "Header.h"
#include "gms_matcher.h"
#include<iostream>
#include<fstream>
#include <opencv2/opencv.hpp>
#include<string>
#include<vector>
#include<cstdlib>
using namespace cv;
using namespace std;
const int SMOOTHING_RADIUS = 5;
const int HORIZONTAL_BORDER_CROP = 5; // In pixels. Crops the border to reduce the black borders from stabilisation being too noticeable.
struct TransformParam
{
    TransformParam() {}
    TransformParam(double _dx, double _dy, double _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx;
    double dy;
    double da; // angle
};

struct Trajectory
{
    Trajectory() {}
    Trajectory(double _x, double _y, double _a) {
        x = _x;
        y = _y;
        a = _a;
    }

    double x;
    double y;
    double a; // angle
};
inline size_t getMatchPoint(Mat img1, Mat img2, vector<KeyPoint> &kp1, vector<KeyPoint> &kp2, vector<DMatch> &matches_gms);
int smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform);
int no_smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform);

int main(int argc, char* argv[])
{
    string ImgNamesLists[1000];
    char str[200];
    FILE *fImgList = fopen(argv[1], "rb");
    if (NULL == fImgList)
        return -1;
    int count = 0;
    while (fgets(str, 200, fImgList) != NULL && count < 1000) {
        memset(str + strlen(str)-1, 0, 1);
        ImgNamesLists[count] = str;
        count ++;
    }
    // Step 1, Get the rois
    cout<<"Get roi..."<<endl;
    Mat srcImage, roiImg;
    vector<Mat> rois;
    for(int i = 0; i < count; i ++)
    {
        // Read the images to be aligned
        srcImage = imread(ImgNamesLists[i]);
        if(srcImage.empty())
        {
            cout<< "Cannot open image ["<<argv[i]<<"]"<<endl;
            return -1;
        }
        
        //int roi_height = 300;
        //int roi_width = 200;
        //int sy = srcImage.rows / 4 + 20;
        //int sx = srcImage.cols / 3 + 50;
        //int sy = 205;
        //int sx = 390;
        //int roi_height = 460 - sy;
        //int roi_width = 570 - sx;
        int roi_height = srcImage.rows - 1;
        int roi_width = srcImage.cols - 1;
        int sy = 0;
        int sx = 0;
        if( sy + roi_height >= srcImage.rows || sx + roi_width >= srcImage.cols ){
            cout<< "ERROR: "<<"Region overstep border!"<<endl;
            return -1;
        }
        Rect rect(sx, sy, roi_width, roi_height);
        srcImage(rect).copyTo(roiImg);
        rois.push_back(roiImg.clone());
    }
    cout<<"rois size(71): "<<rois.size()<<endl;

    // Step 2. Generate warps
    Mat warp_matrix;
    Mat warpMatrix = Mat::eye(2, 3, CV_64F);
    vector<Mat> warps;
    vector<KeyPoint> kp1, kp2;
    vector<DMatch> matches_gms;
    vector<Point2f> prev, cur;

    cout<<"Generate warp matrix..."<<endl;
    for(int i = 0; i < count; i ++)
    {
        
        if( i == 0 ){
           // warps.push_back(warpMatrix.clone());// initial warp_matrix, only to hold the pos
            warpMatrix.copyTo(warp_matrix); // save the last_T
            continue;
        }
        
        int n_match = getMatchPoint(rois[i-1], rois[i], kp1, kp2, matches_gms);
        for(size_t j = 0; j < matches_gms.size(); j ++)
        {
            Point2f left = kp1[matches_gms[j].queryIdx].pt;
            Point2f right = kp2[matches_gms[j].trainIdx].pt;
            prev.push_back(left);
            cur.push_back(right);
        }
        Mat T;
        if( n_match > 0 )
            T = estimateRigidTransform(prev, cur, false);
        else {
            cout<<"ERROR: "<<ImgNamesLists[i-1]<<" and "<<ImgNamesLists[i]<<" do not match !"<<endl;
            return -1;
            //warpMatrix.copyTo(warp_matrix);
        }
        if( T.empty() ){
            cout<<"WARNING: "<<ImgNamesLists[i-1]<<" and "<<ImgNamesLists[i]<<" can not warp !"<<endl;
        }
        else {
            T.copyTo(warp_matrix);
        }

        warps.push_back(warp_matrix.clone());

        kp1.clear();
        kp2.clear();
        matches_gms.clear();
        prev.clear();
        cur.clear();
    }
    cout<<"warps size(70): "<<warps.size()<<endl;

    //Step 3. Deal with TransformParam
    cout<<"Deal with TransformParam.."<<endl;
    vector<TransformParam> prev_to_cur_transform;
    vector<TransformParam> new_prev_to_cur_transform;
    for(int i = 0; i < warps.size(); i ++)
    {
        double dx = warps[i].at<double>(0, 2);
        double dy = warps[i].at<double>(1, 2);
        double da = atan2(warps[i].at<double>(1, 0), warps[i].at<double>(0, 0));
        prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
    }
    smooth_warp_para(prev_to_cur_transform, new_prev_to_cur_transform);
    //no_smooth_warp_para(prev_to_cur_transform, new_prev_to_cur_transform);
    cout<<"new_prev_to_cur_transform size(70): " << new_prev_to_cur_transform.size()<<endl;

    //Step 4. Wrap the image
    cout<<"Warp the image..."<<endl;
    Mat T(2, 3, CV_64F);
    Mat im_aligned;
    vector<Mat> warped_rois; 
    //warped_rois.push_back(rois[0].clone());
    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++)
    {
        T.at<double>(0, 0) = cos(new_prev_to_cur_transform[i].da);
        T.at<double>(0, 1) = -sin(new_prev_to_cur_transform[i].da);

        T.at<double>(1, 0) = sin(new_prev_to_cur_transform[i].da);
        T.at<double>(1, 1) = cos(new_prev_to_cur_transform[i].da);

        T.at<double>(0, 2) = new_prev_to_cur_transform[i].dx;
        T.at<double>(1, 2) = new_prev_to_cur_transform[i].dy;

        //warpAffine(rois[i], im_aligned, T, rois[i].size(), INTER_LINEAR + WARP_INVERSE_MAP, BORDER_TRANSPARENT);
        srcImage = imread(ImgNamesLists[i]);
        warpAffine(srcImage, im_aligned, T, srcImage.size(), INTER_LINEAR, BORDER_TRANSPARENT);
        //warpAffine(rois[i], im_aligned, T, rois[i].size(), INTER_LINEAR, BORDER_TRANSPARENT);
        warped_rois.push_back(im_aligned.clone());
    }
    cout<<"warped_rois size(70): "<<warped_rois.size()<<endl;

    char tmp[200];
    string out_name;
    for(int i = 0; i < warped_rois.size(); i ++)
    {
        sprintf(tmp,"./data/%d.jpg", 1000+i+1);
        out_name = tmp;
        imwrite(out_name, warped_rois[i]);
        //imshow("warped", warped_rois[i]);
        //waitKey();
    }
    return 0;
}


inline size_t getMatchPoint(Mat img1, Mat img2, vector<KeyPoint> &kp1, vector<KeyPoint> &kp2, vector<DMatch> &matches_gms)
{
	//vector<KeyPoint> kp1, kp2;
	Mat d1, d2;
	//vector<DMatch> matches_all, matches_gms;
	vector<DMatch> matches_all;

	Ptr<ORB> orb = ORB::create(10000);
	orb->setFastThreshold(0);
	orb->detectAndCompute(img1, Mat(), kp1, d1);
	orb->detectAndCompute(img2, Mat(), kp2, d2);
    
	BFMatcher matcher(NORM_HAMMING);
	matcher.match(d1, d2, matches_all);

	// GMS filter
	int num_inliers = 0;
	std::vector<bool> vbInliers;
	gms_matcher gms(kp1,img1.size(), kp2,img2.size(), matches_all);
	num_inliers = gms.GetInlierMask(vbInliers, false, false);

	// draw matches
	for (size_t i = 0; i < vbInliers.size(); ++i)
	{
		if (vbInliers[i] == true)
		{
			matches_gms.push_back(matches_all[i]);
		}
	}

    return matches_gms.size();
}

int smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform)
{

    // - Accumulate the transformations to get the image trajectory

    // Accumulated frame to frame transform
    double a = 0;
    double x = 0;
    double y = 0;

    vector <Trajectory> trajectory; // trajectory at all frames

    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {
        x += prev_to_cur_transform[i].dx;
        y += prev_to_cur_transform[i].dy;
        a += prev_to_cur_transform[i].da;

        trajectory.push_back(Trajectory(x,y,a));

    }

    // - Smooth out the trajectory using an averaging window
    vector <Trajectory> smoothed_trajectory; // trajectory at all frames

    for(int i=0; i < trajectory.size(); i++) {
        double sum_x = 0;
        double sum_y = 0;
        double sum_a = 0;
        int count = 0;

        for(int j=-SMOOTHING_RADIUS; j <= SMOOTHING_RADIUS; j++) {
            if(i+j >= 0 && i+j < trajectory.size()) {
                sum_x += trajectory[i+j].x;
                sum_y += trajectory[i+j].y;
                sum_a += trajectory[i+j].a;

                count++;
            }
        }

        double avg_a = sum_a / count;
        double avg_x = sum_x / count;
        double avg_y = sum_y / count;

        smoothed_trajectory.push_back(Trajectory(avg_x, avg_y, avg_a));

    }

    // - Generate new set of previous to current transform, such that the trajectory ends up being the same as the smoothed trajectory
    //vector <TransformParam> new_prev_to_cur_transform;

    // Accumulated frame to frame transform
    a = 0;
    x = 0;
    y = 0;

    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {
        x += prev_to_cur_transform[i].dx;
        y += prev_to_cur_transform[i].dy;
        a += prev_to_cur_transform[i].da;

        // target - current
        double diff_x = smoothed_trajectory[i].x - x;
        double diff_y = smoothed_trajectory[i].y - y;
        double diff_a = smoothed_trajectory[i].a - a;

        double dx = prev_to_cur_transform[i].dx + diff_x;
        double dy = prev_to_cur_transform[i].dy + diff_y;
        double da = prev_to_cur_transform[i].da + diff_a;

        new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));

    }
/*
    vector<TransformParam> tmp;
    int W = 3;
    double weight[7] = {0.025, 0.075, 0.1, 0.7, 0.1, 0.075, 0.025};
    //int W = 2;
    //double weight[5] = {0.0, 0.15, 0.7, 0.15, 0.0};
    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++)
    {
        a = 0;
        x = 0;
        y = 0;
        int index = 0, k = 0;

        for(int j = i-W; j <= i+W; j ++)
        {
            if(j < 0)
                index = j + new_prev_to_cur_transform.size();
            else if(j > new_prev_to_cur_transform.size() - 1)
                index = j - new_prev_to_cur_transform.size();
            else
                index = j;
            a += weight[k] * new_prev_to_cur_transform[k].da;
            x += weight[k] * new_prev_to_cur_transform[k].dx;
            y += weight[k] * new_prev_to_cur_transform[k].dy;
            k++;
        }
        tmp.push_back(TransformParam(x, y, a));
    }

    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++){
        new_prev_to_cur_transform[i].da = tmp[i].da;
        new_prev_to_cur_transform[i].dx = tmp[i].dx;
        new_prev_to_cur_transform[i].dy = tmp[i].dy;
    }
*/    
    return new_prev_to_cur_transform.size();
}

int no_smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform)
{
    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {

        double dx = prev_to_cur_transform[i].dx;
        double dy = prev_to_cur_transform[i].dy;
        double da = prev_to_cur_transform[i].da;
        new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
    }
    vector<TransformParam> tmp;
    //int W = 3;
    //double weight[7] = {0.025, 0.075, 0.15, 0.5, 0.15, 0.075, 0.025};
    int W = 2;
    double weight[5] = {0.0, 0.15, 0.7, 0.15, 0.0};
    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++)
    {
        double a = 0;
        double x = 0;
        double y = 0;
        int index = 0, k = 0;

        for(int j = i-W; j <= i+W; j ++)
        {
            if(j < 0)
                index = j + new_prev_to_cur_transform.size();
            else if(j > new_prev_to_cur_transform.size() - 1)
                index = j - new_prev_to_cur_transform.size();
            else
                index = j;
            a += weight[k] * new_prev_to_cur_transform[k].da;
            x += weight[k] * new_prev_to_cur_transform[k].dx;
            y += weight[k] * new_prev_to_cur_transform[k].dy;
            k++;
        }
        tmp.push_back(TransformParam(x, y, a));
    }

    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++){
        new_prev_to_cur_transform[i].da = tmp[i].da;
        new_prev_to_cur_transform[i].dx = tmp[i].dx;
        new_prev_to_cur_transform[i].dy = tmp[i].dy;
    }
    return new_prev_to_cur_transform.size();
}
