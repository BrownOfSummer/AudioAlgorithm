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

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#define METHOD 1
using namespace cv;
using namespace std;

//Mat cameraMatrix, distCoeffs;
//int width, height;
bool loadInfo(const string calibration_xml, Mat &cameraMatrix, Mat &distCoeffs, int *width, int *height);
void help(const char* s)
{
    printf("Demonstrate the rectify process with cameraMatrix.\n");
    printf("Load the cameraMatrix and distCoeffs, the rectify frames.\n");
    printf("Usage:\n\t%s out_camera_matrix.xml video.mp4 [out_video_path]\n", s);
}
int main(int argc, char *argv[])
{
    if(argc != 3 && argc != 4)
    {
        printf("ERROR, InPut ERROR.\n");
        help(argv[0]);
        return -1;
    }
    Mat cameraMatrix, distCoeffs, frame, dst, display, map1, map2;
    int width, height;
    bool is_show = argc == 3 ? true : false;
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
    VideoCapture cap(argv[2], CAP_FFMPEG);
    if( !cap.isOpened() ) {
        cout<<"ERROR: Open video failed !\n";
        help(argv[0]);
        return -1;
    }
    int video_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    int video_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int total_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
    double frame_rate = cap.get(CV_CAP_PROP_FPS);
    
    if(imageSize != Size(video_width, video_height))
    {
        printf("Valid Size Failed.\n");
        return -1;
    }
    
    VideoWriter outputVideo;
    if( !is_show )
    {
        printf("Prepare for writing video...\n");
        int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
        char EXT[] = { (char)(ex & 0XFF), (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};
        printf("Video Codec: %c%c%c%c%c\n", EXT[0],EXT[1],EXT[2],EXT[3],EXT[4]);
        outputVideo.open(argv[3], ex, frame_rate, Size(video_width, video_height), true);
        if( !outputVideo.isOpened() ) {
            cout<<"Failed to open outputVideo !\n";
            return -1;
        }
        else printf("Writting to %s ...\n", argv[3]);
    }
    
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
            outputVideo.write(dst);
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
