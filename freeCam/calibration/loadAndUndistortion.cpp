/*************************************************************************
    > File Name: loadAndUndistortion.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-07-01 15:43:22
 ************************************************************************/
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>

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

Mat cameraMatrix, distCoeffs;
int width, height;
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

int main(int argc, char *argv[])
{
    bool flag = loadInfo(argv[1], cameraMatrix, distCoeffs, &width, &height);
    if(flag)
    {
        cout<<"camera_matrix:\n"<<cameraMatrix<<endl;
        cout<<"distCoeffs:\n"<<distCoeffs<<endl;
        cout<<"(width, height) = ("<<width<<", "<<height<<")\n";
    }
    else cout<<"Error\n";

    Mat src = imread(argv[2], 1);
    if(src.size() != Size(width, height))
    {
        cout<<"Validation error !\n"<<endl;
    }
    Mat dst1, dst2;
    // Method 1, slower. 5.65s / 1000
    int64 t1 = getTickCount();
    undistort(src, dst1, cameraMatrix, distCoeffs);
    cout<< (getTickCount() - t1) / getTickFrequency()<<"s\n";
    
    // Method 2, faster. 1.28s / 1000
    Size imageSize = src.size();
    Mat map1, map2, newCamMat;
    Rect rect;
    double alpha = 0;
    // alpha=0 means that the rectified images are zoomed and shifted so that only valid pixels are visible (no black areas after rectification)
    // alpha=1 means that the rectified image is decimated and shifted so that all the pixels from the original images from the cameras are retained in the rectified images (no source image pixels are lost)
    t1 = getTickCount();
    newCamMat = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, alpha, imageSize, &rect, false);
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), newCamMat, imageSize, CV_16SC2, map1, map2);
    remap(src, dst2, map1, map2, INTER_LINEAR);
    cout<< (getTickCount() - t1) / getTickFrequency()<<"s\n";

    rectangle(dst2,Point(rect.x,rect.y), Point(rect.x + rect.width, rect.y+rect.height), Scalar(0,255,0));

    if(src.cols > 640)
    {
        double ratio = 640 / (double)(src.cols);
        resize(src, src, Size(), ratio, ratio);
        resize(dst1, dst1, Size(), ratio, ratio);
        resize(dst2, dst2, Size(), ratio, ratio);
    }
    imshow("src", src);
    imshow("dst1", dst1);
    imshow("dst2", dst2);
    waitKey();
    return 0;
}
