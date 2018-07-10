/*************************************************************************
    > File Name: main.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-07-10 15:03:31
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
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

#include "stitchutils.h"
#define TRYS 10

using namespace std;
using namespace cv;
using namespace cv::detail;

int prepare_paras(vector<String> &img_names, vector<CameraParams> &save_cameras, Ptr<RotationWarper> &save_warper, vector<Mat> &save_masks_warped, vector<Point> &save_corners, vector<Size> &save_sizes, Ptr<ExposureCompensator> &save_compensator, Ptr<Blender> &save_blender);
void help()
{
    printf("Demonstrate the process of stitching several images in different dirs.\n");
    printf("Input dirs with images, get one from each then stitch them.\n");
    printf("Usage:\n\t./stitchPlane /path/to/dir0/ /path/to/dir1/ ....\n");
}
int main(int argc, char *argv[])
{
    if( argc < 3 )
    {
        printf("No enough input !\n");
        help();
        return -1;
    }

    // List and load all images path
    vector<vector<string> >ImageNames;
    vector<string> Dirs;
    for(int i = 1; i < argc; ++i)
    {
        vector<string> img_names = listDir( argv[i], ".jpg" );
        if(img_names.empty())
        {
            printf("%s has no images !\n", argv[i]);
            return -1;
        }
        else
        {
            printf("Load %d img_names from %s\n", static_cast<int>(img_names.size()), argv[i]);
            std::sort(img_names.begin(), img_names.end());
            ImageNames.push_back( img_names );
            Dirs.push_back( argv[i] );
        }
    }
    // find minimum number of images;
    int mincnt = static_cast<int>(ImageNames[0].size());
    int dirnum = static_cast<int>(Dirs.size());
    for(size_t i = 1; i < ImageNames.size(); i ++) mincnt = static_cast<int>(ImageNames[i].size()) < mincnt ? static_cast<int>(ImageNames[i].size()) : mincnt;
    printf("Selected %d dirs and %d frames in each dir.\n", dirnum, mincnt);

    // prepare paras
    vector<Mat> save_masks_warped;
    vector<CameraParams> save_cameras(dirnum);
    vector<Point> save_corners(dirnum);
    vector<Size> save_sizes(dirnum);
    Ptr<RotationWarper> save_warper;
    Ptr<ExposureCompensator> save_compensator;
    Ptr<Blender> save_blender;
    Mat K, img, img_warped, img_warped_s, mask_warped;
    Mat result, result_mask, cropped;
    Rect rect;
    bool para_prepared = false, crop_flag = false;
    
    int try_find_para_times = 0, para_flag = -1;
    for(int n = 0; n < mincnt; ++n)
    {
        vector<string> full_paths;
        for(size_t i = 0; i < dirnum; ++i)
        {
            string img_path = Dirs[i] + ImageNames[i][n];
            full_paths.push_back( img_path );
            vlog("%s\n", img_path.c_str());
        }
        if( !para_prepared )
        {
            try_find_para_times ++;
            para_flag = prepare_paras(full_paths, save_cameras, save_warper, save_masks_warped, save_corners, save_sizes, save_compensator, save_blender);
            if(para_flag != 0)
            {
                printf("#%d find failed !\n", try_find_para_times);
                if( try_find_para_times < TRYS ) continue;
                else
                {
                    printf("Prepare Paras Failed !\n");
                    break;
                }
            }
            else
            {
                para_prepared = true;
                printf("Prepare Paras Done.!\n");
                //save_blender->prepare(save_corners, save_sizes);
            }
        }
        
        save_blender->prepare(save_corners, save_sizes);

        // once the paras prepared
        for(size_t img_idx = 0; img_idx < full_paths.size(); ++img_idx)
        {
            img = imread(full_paths[img_idx], 1);
            save_cameras[img_idx].K().convertTo(K, CV_32F);
            // Warp the current image
            save_warper->warp(img, K, save_cameras[img_idx].R, INTER_LINEAR, BORDER_REFLECT, img_warped);
            mask_warped = save_masks_warped[img_idx];
            save_compensator->apply(img_idx, save_corners[img_idx], img_warped, mask_warped);
            img_warped.convertTo(img_warped_s, CV_16S);
            save_blender->feed(img_warped_s, mask_warped, save_corners[img_idx]);
        }
        save_blender->blend(result, result_mask);
        result.convertTo(result, CV_8U);
        if(!crop_flag) crop_flag = cropimg2rect(result, rect);
        if(crop_flag) cropped = result(rect);
        else
        {
            printf("Warning, cropped faild !\n");
            cropped = result;
        }
        imshow("panorama", cropped);
        char k = waitKey(30);
        if(k == 27) break;
    }
    return 0;
}
