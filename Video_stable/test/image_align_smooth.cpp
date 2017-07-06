/*************************************************************************
    > File Name: image_align.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-06-22 10:52:35
 ************************************************************************/

#include<iostream>
#include<fstream>
#include "opencv2/opencv.hpp"
#include<string>
#include<vector>
#include<cstdlib>
#define USE_ROI 1
using namespace cv;
using namespace std;

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

int main(int argc, char* argv[])
{
    string ImgNamesLists[100];
    char str[200];
    FILE *fImgList = fopen(argv[1], "rb");
    if (NULL == fImgList)
        return -1;
    int count = 0;
    while (fgets(str, 200, fImgList) != NULL && count < 100) {
        memset(str + strlen(str)-1, 0, 1);
        ImgNamesLists[count] = str;
        count ++;
    }
    
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
        int sy = 205;
        int sx = 390;
        int roi_height = 460 - sy;
        int roi_width = 570 - sx;
        if( sy + roi_height >= srcImage.rows || sx + roi_width >= srcImage.cols ){
            cout<< "ERROR: "<<"Region overstep border!"<<endl;
            return -1;
        }
        Rect rect(sx, sy, roi_width, roi_height);
        srcImage(rect).copyTo(roiImg);
        rois.push_back(roiImg.clone());
    }
    cout<<"rois size(71): "<<rois.size()<<endl;
    // Define the motion model
    /*
     * MOTION_AFFINE
     * MOTION_EUCLIDEAN
     * MOTION_TRANSLATION
     * MOTION_HOMOGRAPHY 
     * */
    const int warp_mode = MOTION_EUCLIDEAN;
    // Set a 2x3 or 3x3 warp matrix depending on the motion model.
    Mat warp_matrix, warpMatrix;
    // Initialize the matrix to identity
    if ( warp_mode == MOTION_HOMOGRAPHY )
        warpMatrix = Mat::eye(3, 3, CV_32F);
    else
        warpMatrix = Mat::eye(2, 3, CV_32F);
    // Specify the number of iterations.
    int number_of_iterations = 100;
    // Specify the threshold of the increment
    // in the correlation coefficient between two iterations
    double termination_eps = 1e-10;
    // Define termination criteria
    TermCriteria criteria (TermCriteria::COUNT+TermCriteria::EPS, number_of_iterations, termination_eps);

    Mat template_roi, roiGray1, roiGray2, im_aligned;
    vector<Mat> warps;
    vector<Mat> warped_rois;

    cout<<"Generate warp matrix..."<<endl;
    for(int i = 0; i < count; i ++)
    {
        if( i == 0 ){
            warpMatrix.copyTo(warp_matrix);
            cvtColor(rois[count - 1], roiGray1, CV_BGR2GRAY);// last image as template image
            cvtColor(rois[0], roiGray2, CV_BGR2GRAY);
            findTransformECC( GetGradient(roiGray1), GetGradient(roiGray2), warp_matrix, warp_mode, criteria);

            //rois[i].copyTo(template_roi);
            warps.push_back(warp_matrix.clone());// initial warp_matrix, only to hold the pos
            continue;
        }
        
        warpMatrix.copyTo(warp_matrix);
        cvtColor(rois[i-1], roiGray1, CV_BGR2GRAY);
        cvtColor(rois[i], roiGray2, CV_BGR2GRAY);

        // Run the ECC algorithm. The results are stored in warp_matrix.
        findTransformECC( GetGradient(roiGray1), GetGradient(roiGray2), warp_matrix, warp_mode, criteria);
        warps.push_back(warp_matrix.clone());
        //rois[i].copyTo(template_roi);
    }
    cout<<"warps size(71): "<<warps.size()<<endl;

    cout<<"Warp the image..."<<endl;

    // deal with warp_matrix
    ofstream out_smoothed_warps("smoothed_warp_matrix");
    int W = 7 / 2;
    double weight[7] = {0.05, 0.1, 0.15, 0.4, 0.15, 0.1, 0.05};
    for(int i = 0; i < count; i ++)
    {
        int index = 0, k = 0;
        warp_matrix.setTo(Scalar(0));
        for(int j = i-W; j <= i+W; j ++)
        {
            if(j < 0)
                index = j + count;
            else if(j > count - 1)
                index = j - count;
            else
                index = j;
            warp_matrix = warp_matrix + weight[k] * warps[index];
            //cout<<index<<" x "<<weight[k]<<" + ";
            k++;
        }
        //cout<<endl;
        out_smoothed_warps<<warp_matrix<<endl<<endl;

        // Storage for warped image.
        if (warp_mode != MOTION_HOMOGRAPHY)
            // Use warpAffine for Translation, Euclidean and Affine
            warpAffine(rois[i], im_aligned, warp_matrix, rois[i].size(), INTER_LINEAR + WARP_INVERSE_MAP, BORDER_TRANSPARENT);
        else
            // Use warpPerspective for Homography
            warpPerspective (rois[i], im_aligned, warp_matrix, rois[i].size(),INTER_LINEAR + WARP_INVERSE_MAP);

        warped_rois.push_back(im_aligned.clone());
    }
    cout<<"warped_rois size(71): "<<warped_rois.size()<<endl;

    char tmp[200];
    string out_name;
    for(int i = 0; i < count; i ++)
    {
        sprintf(tmp,"./roi_align_results/%d.jpg", 100+i+1);
        out_name = tmp;
        imwrite(out_name, warped_rois[i]);
        //imshow("warped", warped_rois[i]);
        //waitKey();
    }
    return 0;
}
