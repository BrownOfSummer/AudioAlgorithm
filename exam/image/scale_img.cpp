/*************************************************************************
    > File Name: scale_img2.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn 
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-08-28 14:45:38
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui.hpp>
using namespace std;
using namespace cv;

void Scale(unsigned char *pIn, unsigned char *pOut, int w, int h, double times);
int main(int argc, char *argv[])
{
    Mat src = imread(argv[1], 0);
    double scale = 0.8;
    double srcX = 0;
    double srcY = 0;
    double dot_tmp_y = 0.0;
    double dot_tmp_x = 0.0;
    int int_tmp_x = 0;
    int int_tmp_y = 0;
    double dst_cols = src.cols * scale;
    double dst_rows = src.rows * scale;
    //(int)(dst_cols) * (int)(dst_rows)
    Mat dst(dst_cols, dst_rows, src.type());// Do (int) when is not int
    cout<<dst.cols<<", "<<dst.rows<<endl;
    
    for(double x = 0; x < dst_cols; x++) {
        for(double y = 0; y < dst_rows; y ++) {
            if( x==0 && y==0 )
                dst.at<uchar>(x,y) = src.at<uchar>(x,y);
            else {
                srcX = x / scale;
                int_tmp_x = (int)(srcX);
                dot_tmp_x = srcX - int_tmp_x;

                srcY = y / scale;
                int_tmp_y = (int)(srcY);
                dot_tmp_y = srcY - (int)(srcY);
                dst.at<uchar>(x,y) = 
                    (1 - dot_tmp_x)*(1 - dot_tmp_y)*src.at<uchar>(int_tmp_x, int_tmp_y) +
                    (1 - dot_tmp_x)*dot_tmp_y*src.at<uchar>(int_tmp_x, int_tmp_y + 1) +
                    (1 - dot_tmp_y)*dot_tmp_x*src.at<uchar>(int_tmp_x + 1, int_tmp_y) + 
                    dot_tmp_y*dot_tmp_x*src.at<uchar>(int_tmp_x+1, int_tmp_y+1);
            }
        }
    }
    cout<<dst.size()<<endl;
    imshow("dst", dst);
    waitKey();

    // scale with function
    Mat dst2(dst_cols, dst_rows, src.type());
    Scale(src.data, dst2.data, src.cols, src.rows, scale);
    imshow("dst2", dst2);
    waitKey();
    return 0;
}

void Scale(unsigned char *pIn, unsigned char *pOut, int w, int h, double times)
{
    int dst_rows = (int)(h * times);
    int dst_cols = (int)(w * times);
    double srcX = 0;
    double dot_tmp_x = 0.0;
    double srcY = 0;
    double dot_tmp_y = 0.0;
    int int_tmp_x = 0;
    int int_tmp_y = 0;
    for(int y = 0; y < dst_rows; y++) {
        for(int x = 0; x < dst_cols; x ++) {
            if( x == 0 && y == 0 )
                pOut[0] = pIn[0];
            else {
                srcX = x / times;
                int_tmp_x = (int)(srcX);
                dot_tmp_y = srcX - int_tmp_x;

                srcY = y / times;
                int_tmp_y = (int)(srcY);
                dot_tmp_y = srcY - int_tmp_y;

                double tmp_result = 
                    (1 - dot_tmp_x) * (1 - dot_tmp_y) * pIn[int_tmp_y * w + int_tmp_x] + 
                    (1 - dot_tmp_x) * dot_tmp_y * pIn[(int_tmp_y + 1) * w + int_tmp_x] +
                    (1 - dot_tmp_y) * dot_tmp_x * pIn[int_tmp_y * w + int_tmp_x + 1] + 
                    dot_tmp_y * dot_tmp_x * pIn[(int_tmp_y + 1) * w + int_tmp_x + 1];
                pOut[y * dst_cols + x] = (int)(tmp_result + 0.5);
            }
        }
    }
    cout<<"Dst size: ( "<<dst_rows<<", "<<dst_cols<<" )"<<endl;
}

