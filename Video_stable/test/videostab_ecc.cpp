/*************************************************************************
    > File Name: image_align.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-06-22 10:52:35
 ************************************************************************/

#include<iostream>
#include<fstream>
#include <opencv2/opencv.hpp>
#include<string>
#include<vector>
#include<cstdlib>
using namespace cv;
using namespace std;
bool USE_ROI = 0;
const int SMOOTHING_RADIUS = 5;
const int HORIZONTAL_BORDER_CROP = 5; // In pixels. Crops the border to reduce the black borders from stabilisation being too noticeable.
Mat GetGradient(Mat src_gray)
{ 
	Mat grad_x, grad_y;
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
    Mat grad;
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

	return grad; 

} 

struct TransformParam
{
    TransformParam() {}
    TransformParam(float _dx, float _dy, float _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    float dx;
    float dy;
    float da; // angle
};

struct Trajectory
{
    Trajectory() {}
    Trajectory(float _x, float _y, float _a) {
        x = _x;
        y = _y;
        a = _a;
    }

    float x;
    float y;
    float a; // angle
};
int smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform);
int no_smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform);

ofstream out_transform("prev_to_cur_transformation.txt");
ofstream out_trajectory("trajectory.txt");
ofstream out_smoothed_trajectory("smoothed_trajectory.txt");
ofstream out_new_transform("new_prev_to_cur_transformation.txt");
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
        int roi_width, roi_height, sy, sx;
        if( USE_ROI ){
            roi_height = 300;
            roi_width = 200;
            sy = srcImage.rows / 4 + 20;
            sx = srcImage.cols / 3 + 50;
        }
        else{
            roi_height = srcImage.rows - 1;
            roi_width = srcImage.cols - 1;
            sy = 0;
            sx = 0;
        }
        //int sy = 205;
        //int sx = 390;
        //int roi_height = 460 - sy;
        //int roi_width = 570 - sx;
        if( sy + roi_height >= srcImage.rows || sx + roi_width >= srcImage.cols ){
            cout<< "ERROR: "<<"Region overstep border!"<<endl;
            return -1;
        }
        Rect rect(sx, sy, roi_width, roi_height);
        srcImage(rect).copyTo(roiImg);
        //imshow("roi",roiImg);
        //waitKey();
        rois.push_back(roiImg.clone());
    }
    cout<<"rois size(71): "<<rois.size()<<endl;

    // Step 2. Generate warps
    /*
     * MOTION_AFFINE
     * MOTION_EUCLIDEAN
     * MOTION_TRANSLATION
     * MOTION_HOMOGRAPHY 
     * */
    const int warp_mode = MOTION_AFFINE;
    int number_of_iterations = 100;
    float termination_eps = 1e-10;
    TermCriteria criteria (TermCriteria::COUNT+TermCriteria::EPS, number_of_iterations, termination_eps);
    Mat roiGray1, roiGray2;

    Mat warp_matrix;
    Mat warpMatrix = Mat::eye(2, 3, CV_32F);
    vector<Mat> warps;

    cout<<"Generate warp matrix..."<<endl;
    for(int i = 0; i < count; i ++)
    {
        if( i == 0 ){
           // warps.push_back(warpMatrix.clone());// initial warp_matrix, only to hold the pos
            warpMatrix.copyTo(warp_matrix); // save the last_T
            cvtColor(rois[count - 1], roiGray1, CV_BGR2GRAY);// last image as template image
            cvtColor(rois[0], roiGray2, CV_BGR2GRAY);
            findTransformECC( GetGradient(roiGray1), GetGradient(roiGray2), warp_matrix, warp_mode, criteria);
            warps.push_back(warp_matrix.clone());// initial warp_matrix, only to hold the pos
            continue;
        }
        
        warpMatrix.copyTo(warp_matrix);
        cvtColor(rois[i-1], roiGray1, CV_BGR2GRAY);
        cvtColor(rois[i], roiGray2, CV_BGR2GRAY);
        findTransformECC( GetGradient(roiGray1), GetGradient(roiGray2), warp_matrix, warp_mode, criteria);
        warps.push_back(warp_matrix.clone());
    }
    cout<<"warps size(71): "<<warps.size()<<endl;

    //Step 3. Deal with TransformParam
    cout<<"Deal with TransformParam.."<<endl;
    vector<TransformParam> prev_to_cur_transform;
    vector<TransformParam> new_prev_to_cur_transform;
    for(int i = 0; i < warps.size(); i ++)
    {
        float dx = warps[i].at<float>(0, 2);
        float dy = warps[i].at<float>(1, 2);
        float da = atan2(warps[i].at<float>(1, 0), warps[i].at<float>(0, 0));
        prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
        out_transform << i+1 << " " << dx << " " << dy << " " << da << endl;
    }
    smooth_warp_para(prev_to_cur_transform, new_prev_to_cur_transform);
    //no_smooth_warp_para(prev_to_cur_transform, new_prev_to_cur_transform);
    cout<<"new_prev_to_cur_transform size(71): " << new_prev_to_cur_transform.size()<<endl;

    //Step 4. Wrap the image
    cout<<"Warp the image..."<<endl;
    Mat T(2, 3, CV_32F);
    Mat im_aligned;
    vector<Mat> warped_rois; 
    //warped_rois.push_back(rois[0].clone());
    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++)
    {
        T.at<float>(0, 0) = cos(new_prev_to_cur_transform[i].da);
        T.at<float>(0, 1) = -sin(new_prev_to_cur_transform[i].da);

        T.at<float>(1, 0) = sin(new_prev_to_cur_transform[i].da);
        T.at<float>(1, 1) = cos(new_prev_to_cur_transform[i].da);

        T.at<float>(0, 2) = new_prev_to_cur_transform[i].dx;
        T.at<float>(1, 2) = new_prev_to_cur_transform[i].dy;

        //warpAffine(rois[i], im_aligned, T, rois[i].size(), INTER_LINEAR + WARP_INVERSE_MAP, BORDER_TRANSPARENT);
        //srcImage = imread(ImgNamesLists[i]);
        //warpAffine(srcImage, im_aligned, T, srcImage.size(), INTER_LINEAR, BORDER_TRANSPARENT);
        warpAffine(rois[i], im_aligned, T, rois[i].size(), INTER_LINEAR, BORDER_TRANSPARENT);
        warped_rois.push_back(im_aligned.clone());
    }
    cout<<"warped_rois size(71): "<<warped_rois.size()<<endl;

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


int smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform)
{

    // - Accumulate the transformations to get the image trajectory

    // Accumulated frame to frame transform
    float a = 0;
    float x = 0;
    float y = 0;

    vector <Trajectory> trajectory; // trajectory at all frames

    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {
        x += prev_to_cur_transform[i].dx;
        y += prev_to_cur_transform[i].dy;
        a += prev_to_cur_transform[i].da;

        trajectory.push_back(Trajectory(x,y,a));
        out_trajectory << (i+1) << " " << x << " " << y << " " << a << endl;
    }

    // - Smooth out the trajectory using an averaging window
    vector <Trajectory> smoothed_trajectory; // trajectory at all frames

    for(int i=0; i < trajectory.size(); i++) {
        float sum_x = 0;
        float sum_y = 0;
        float sum_a = 0;
        int count = 0;

        for(int j=-SMOOTHING_RADIUS; j <= SMOOTHING_RADIUS; j++) {
            if(i+j >= 0 && i+j < trajectory.size()) {
                sum_x += trajectory[i+j].x;
                sum_y += trajectory[i+j].y;
                sum_a += trajectory[i+j].a;

                count++;
            }
        }

        float avg_a = sum_a / count;
        float avg_x = sum_x / count;
        float avg_y = sum_y / count;

        smoothed_trajectory.push_back(Trajectory(avg_x, avg_y, avg_a));
        out_smoothed_trajectory << (i+1) << " " << avg_x << " " << avg_y << " " << avg_a << endl;

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
        float diff_x = smoothed_trajectory[i].x - x;
        float diff_y = smoothed_trajectory[i].y - y;
        float diff_a = smoothed_trajectory[i].a - a;

        float dx = prev_to_cur_transform[i].dx + diff_x;
        float dy = prev_to_cur_transform[i].dy + diff_y;
        float da = prev_to_cur_transform[i].da + diff_a;

        new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
        out_new_transform << (i+1) << " " << dx << " " << dy << " " << da << endl;

    }
/*
    vector<TransformParam> tmp;
    int W = 3;
    float weight[7] = {0.025, 0.075, 0.1, 0.7, 0.1, 0.075, 0.025};
    //int W = 2;
    //float weight[5] = {0.0, 0.15, 0.7, 0.15, 0.0};
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

        float dx = prev_to_cur_transform[i].dx;
        float dy = prev_to_cur_transform[i].dy;
        float da = prev_to_cur_transform[i].da;
        new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
    }
    vector<TransformParam> tmp;
    //int W = 3;
    //float weight[7] = {0.025, 0.075, 0.15, 0.5, 0.15, 0.075, 0.025};
    int W = 2;
    float weight[5] = {0.0, 0.15, 0.7, 0.15, 0.0};
    for(int i = 0; i < new_prev_to_cur_transform.size(); i ++)
    {
        float a = 0;
        float x = 0;
        float y = 0;
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
