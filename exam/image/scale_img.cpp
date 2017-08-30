/*************************************************************************
    > File Name: scale_img.cpp
    > Created Time: 2017-08-30 14:15:48
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void ScaleImg(unsigned char *pIn, unsigned char *pOut, int src_h, int src_w, double scale);
int main(int argc, char *argv[])
{
    if( argc != 3 ) {
        cout<<"Usage: "<<argv[0]<<" img scale"<<endl;
        return -1;
    }
    Mat src = imread(argv[1], 0);
    //double scale = 1.8;
    double scale = atof(argv[2]);
    int dst_height = (int)(src.rows * scale);
    int dst_width = (int)(src.cols * scale);
    //Mat::Mat(int rows, int cols, int type, void* data, size_t step=AUTO_STEP)
    Mat dst(dst_height, dst_width, src.type());// Mat do (int) auto for the cols and rows
    if( dst_width < 2 || dst_height < 2 ) {
        cout<< "Scale is too small !"<<endl;
        return 0;
    }

    double srcY, srcX, dot_x, dot_y;
    int x, y;   //coord in src image
    for(int dst_y = 0; dst_y < dst_height; dst_y ++) {
        for(int dst_x = 0; dst_x < dst_width; dst_x ++) {
            srcY = 1.0 * dst_y / scale;
            srcX = 1.0 * dst_x / scale;
            //(y,x) in src image Q00
            y = (int)(srcY);
            x = (int)(srcX);
            dot_y = srcY - y;
            dot_x = srcX - x;
            double tmp_pixv = 
                (1 - dot_y) * (1 - dot_x) * src.at<uchar>(y, x) +
                (1 - dot_y) * dot_x * src.at<uchar>(y, x + 1) +
                (1 - dot_x) * dot_y * src.at<uchar>(y + 1, x) +
                dot_x * dot_y * src.at<uchar>(y+1, x + 1);
            int pixv = (int)(tmp_pixv + 0.5);
            //pixv = pixv > 255 ? 255 : pixv;
            dst.at<uchar>(dst_y, dst_x) = pixv;
        }
    }
    cout<<"src size: height="<<src.rows<<"; width="<<src.cols<<endl;
    cout<<"dst size: height="<<dst.rows<<"; width="<<dst.cols<<endl;
    imshow("src", src);
    imshow("dst", dst);

    /* 
     * Two ways to use ScaleImg:
     * (1) define Mat first and then calc
     * (2) malloc some memory to store result, the put it in Mat
     * */

    //Mat dst2(dst_height, dst_width, src.type());
    //ScaleImg(src.data, dst2.data, src.rows, src.cols, scale);

    unsigned char* pout_data = (unsigned char *)malloc(dst_height * dst_width * sizeof(char));
    ScaleImg(src.data, pout_data, src.rows, src.cols, scale);
    Mat dst2(dst_height, dst_width, CV_8UC1, pout_data, dst_width);

    imshow("dst2", dst2);
    cout<<"dst size: height="<<dst.rows<<"; width="<<dst.cols<<endl;
    waitKey();
    return 0;
}

void ScaleImg(unsigned char *pIn, unsigned char *pOut, int src_h, int src_w, double scale)
{
    int dst_height = (int)(src_h * scale);
    int dst_width = (int)(src_w * scale);
    double srcX, srcY, dot_y, dot_x, tmp_pixv;
    unsigned char pixv;
    int y, x; //record pix coord in src image
    
    for(int dst_y = 0; dst_y < dst_height; dst_y ++) {
        for(int dst_x = 0; dst_x < dst_width; dst_x ++) {
            if(dst_y == 0 && dst_x == 0)
                pOut[0] = pIn[0];
            else {
                srcY = (double)(dst_y) / scale;
                srcX = (double)(dst_x) / scale;
                //(x,y): left-top pix of the float coord (srcY, srcX)
                y = (int)(srcY);
                x = (int)(srcX);
                dot_y = srcY - y;
                dot_x = srcX - x;
                
                tmp_pixv = 
                    (1 - dot_y) * (1 - dot_x) * pIn[y * src_w + x] +
                    (1 - dot_y) * dot_x * pIn[y * src_w + x + 1] +
                    (1 - dot_x) * dot_y * pIn[(y+1) * src_w + x] +
                    dot_x * dot_y * pIn[(y+1) * src_w + x + 1];

                pixv = (unsigned char)(tmp_pixv);
                pOut[dst_y * dst_width + dst_x] = pixv;
                //pOut[dst_y * dst_width + dst_x] = (int)(tmp_pixv + 0.5);
            }
        }
    }
    //cout<<"dst size: height="<<dst_height<<"; width="<<dst_width<<endl;
}
