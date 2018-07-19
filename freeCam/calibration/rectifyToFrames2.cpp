/*************************************************************************
    > File Name: loadAndUndistortionVideo.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-07-02 14:50:34
 ************************************************************************/
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>
#include <stdio.h>
/*
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
*/
#include<opencv2/opencv.hpp>
#define METHOD 1
using namespace cv;
using namespace std;

//Mat cameraMatrix, distCoeffs;
//int width, height;
bool loadInfo(const string calibration_xml, Mat &cameraMatrix, Mat &distCoeffs, int *width, int *height);
bool str2secs(string time_str, double *secs);
void help(const char* s)
{
    printf("Demonstrate the rectify process with cameraMatrix.\n");
    printf("Load the cameraMatrix and distCoeffs, the rectify frames.\n");
    printf("Usage:\n\t%s out_camera_matrix.xml video.mp4 [out_frames_dir/] [used_for_prepare]\n", s);
    printf("\t%s out_camera_matrix.xml video.mp4 [out_frames_dir/] [-ss hh:mm:ss -t hh:mm:ss]\n", s);
}
int main(int argc, char *argv[])
{
    if(argc != 3 && argc != 4 && argc != 5 && argc != 8)
    {
        printf("ERROR, InPut ERROR.\n");
        help(argv[0]);
        return -1;
    }
    Mat cameraMatrix, distCoeffs, frame, dst, display, map1, map2;
    int width, height;
    bool is_show = argc == 3 ? true : false;
    bool used_for_prepare = argc == 5 ? true : false;
    bool need_offset = argc == 8 ? true : false;
    double ss = 0, dur = 0;
    if( need_offset )
    {
        if( !str2secs(string(argv[5]), &ss) & !str2secs(string(argv[7]), &dur) )
        {
            printf("Decode time string error !");
            return -1;
        }
    }
    bool loadFine = loadInfo(argv[1], cameraMatrix, distCoeffs, &width, &height);
    if(loadFine)
    {
        cout<<"camera_matrix:\n"<<cameraMatrix<<endl;
        cout<<"distCoeffs:\n"<<distCoeffs<<endl;
        cout<<"Size(width, height) = ("<<width<<", "<<height<<")\n";
    }
    else {
        printf("Load cameraMatrix Faild !\n");
        return -1;
    }
    double alpha = 0; // 0 for all rectified; 1 keep all src pixes and black warp;
    Size imageSize = Size(width, height);
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
                getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, alpha, imageSize, 0), imageSize,
                CV_16SC2, map1, map2);
    //VideoCapture cap(argv[2], CAP_FFMPEG);
    VideoCapture cap(argv[2]);
    if( !cap.isOpened() ) {
        cout<<"ERROR: Open video failed !\n";
        help(argv[0]);
        return -1;
    }
    int video_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    int video_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int total_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
    double frame_rate = cap.get(CV_CAP_PROP_FPS);
    
    int skip_start_frames = ss * frame_rate;
    int tdur_frames = dur * frame_rate;
    if(need_offset)
    {
        if( skip_start_frames >= total_frames)
        {
            printf("-ss is too large !\n");
            return -1;
        }
        if( tdur_frames < 1 )
        {
            printf("-t is too short !\n");
            return -1;
        }
        //tdur_frames = tdur_frames < (video_len - ss) * frame_rate ? tdur_frames : (video_len - ss) * frame_rate;
        tdur_frames = tdur_frames < (total_frames - skip_start_frames) ? tdur_frames : (total_frames - skip_start_frames);
        printf("Skip %d frames from start, retrive %d frames\n", skip_start_frames, tdur_frames);
        cap.set(CV_CAP_PROP_POS_FRAMES, skip_start_frames);
        //cap.set(CV_CAP_PROP_POS_MSEC, ss);
    }
    
    if(imageSize != Size(video_width, video_height))
    {
        printf("Valid Size Failed.\n");
        return -1;
    }
    char save_path[200];
    int fcnt = 0;
    printf("Start Rectify...\n");
    for(;;)
    {
        cap.grab();
        cap.retrieve(frame);
        if( frame.empty() ) break;
        remap(frame, dst, map1, map2, INTER_LINEAR);
        if( is_show )
        {
            if(frame.cols > 640)
            {
                double ratio = 640 / (double)(frame.cols);
                resize(frame, frame, Size(), ratio, ratio);
                resize(dst, dst, Size(), ratio, ratio);
                hconcat(frame, dst, display);
                //vconcat(frame, dst, display);
                imshow("Live", display);
                char k = waitKey(20);
                if(k == 27) break;
            }
        }
        else
        {
            //if(fcnt > 8000) break;
            if(need_offset && fcnt >= tdur_frames) break;
            sprintf(save_path,"%s%05d.jpg", argv[3],fcnt++);
            printf("Save to %s\n", save_path);
            imwrite(save_path, dst);
            
            if (used_for_prepare) {
                break;
            }
        }
    }
    return 0;
}
bool loadInfo(const string calibration_xml, Mat &cameraMatrix, Mat &distCoeffs, int *width, int *height)
{
    FileStorage fs( calibration_xml, FileStorage::READ );
    if( !fs.isOpened() )
    {
        cout<<"Open Error\n"<<endl;
        return false;
    }
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    fs["image_width"] >> (*width);
    fs["image_height"] >> (*height);
    fs.release();
    if(!cameraMatrix.empty() && !distCoeffs.empty() && *height > 0 && *width > 0)
        return true;
    else
        return false;
}
bool str2secs(string time_str, double *secs)
{
    int h = 0, m = 0;
    double s = 0;
    *secs = 0;
    int ret = sscanf(time_str.c_str(), "%d:%d:%lf", &h, &m, &s);
    if(ret == 3)
    {
        if(h < 0 || m >= 60 || m < 0 || s >= 60 || s < 0)
        {
            printf("Time string invalid !\n");
            return false;
        }
        *secs = h * 3600 + m * 60 + s;
    }
    else
    {
        printf("Time string invalide.\n");
        return false;
    }
    return true;
}
