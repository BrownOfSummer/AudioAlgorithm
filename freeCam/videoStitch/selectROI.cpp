/*************************************************************************
    > File Name: select_roi.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-08-22 15:49:39
 ************************************************************************/

#include<iostream>
#include<cstdio>
#include<string>
#include<vector>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

#define WIDTH_FOR_SHOW 720
// paras for select
Point point1, point2;
int drag = 0, select_flag = 0;
Mat img;
char win_name[200];

void mouseHandler(int event, int x, int y, int flags, void* param)
{
    if (event == CV_EVENT_LBUTTONDOWN && !drag)
    {
        /* left button clicked. ROI selection begins */
        point1 = Point(x, y);
        drag = 1;
                            
    }
         
    if (event == CV_EVENT_MOUSEMOVE && drag)
    {
        /* mouse dragged. ROI being selected */
        Mat img1 = img.clone();
        point2 = Point(x, y);
        rectangle(img1, point1, point2, CV_RGB(0, 255, 0), 1, 8, 0);
        imshow(win_name, img1);                                       
    }
             
    if (event == CV_EVENT_LBUTTONUP && drag)
    {
        point2 = Point(x, y);
        //rect = Rect(point1.x,point1.y,x-point1.x,y-point1.y);
        drag = 0;
        //roiImg = img(rect);                                            
    }
                 
    if (event == CV_EVENT_LBUTTONUP)
    {
        /* ROI selected */
        select_flag = 1;
        drag = 0;                                 
    }
}

void help()
{
    printf("Select the common image ROI with mouse. Images should have same size.\n");
    printf("Useage:\n\t./selectROI img1 img2 ...\n");
    printf("Move mouse with left button down, and a rectangle with green color will draw to confirm.\n");
    printf("Keys:\n");
    printf("\tESC - to break.\n");
    printf("\t s - to select the area, then do the next image.\n");
}

int main(int argc, char *argv[])
{
    if( argc < 2)
    {
        help();
        return -1;
    }
    vector<Point> left_tops, bottom_rights;
    vector<Mat> Imgs;
    float scale = 1.0;
    bool flag = false, need_resize = false;
    for(int i = 1; i < argc; i ++)
    {
        img = imread(argv[i], 1);
        Imgs.push_back( img.clone() );
        if( img.cols > WIDTH_FOR_SHOW ) 
        {
            scale = (float)(WIDTH_FOR_SHOW) / img.cols;
            resize(img, img, Size(), scale, scale);
            need_resize = true;
        }
        printf("select image #%d\n", i);
        sprintf(win_name, "image %d, ESC to exit, s to select",i);
        namedWindow(win_name, WINDOW_AUTOSIZE);   
        imshow(win_name, img);
        cvSetMouseCallback(win_name, mouseHandler, NULL);
        for(;;)
        {
            char c = (char)waitKey(0);
            if(c == 27)
            {
                flag = true;
                break;
            }
            else if( c == 's' )
            {
                printf("Select: left_top( %d, %d ), bottom_right( %d, %d )\n", point1.x, point1.y, point2.x, point2.y);
                left_tops.push_back( point1 );
                bottom_rights.push_back( point2 );
                break;
            }
            else
                printf("ESC to exit, s for select. \n");
        }
        destroyWindow(win_name);
        if( flag ) return 0;
    }

    if( left_tops.size() == bottom_rights.size() && left_tops.size() > 0)
    {
        int top, left, bottom, right;
        if(need_resize)
        {
            for(size_t i = 0; i < left_tops.size(); ++i)
            {
                left_tops[i].x = (int)( left_tops[i].x / scale);
                left_tops[i].y = (int)( left_tops[i].y / scale);
                bottom_rights[i].x = (int)( bottom_rights[i].x / scale );
                bottom_rights[i].y = (int)( bottom_rights[i].y / scale );
            }
        }
        for(size_t i = 0; i < left_tops.size(); ++i)
        {
            if(i == 0)
            {
                top = left_tops[i].y;
                left = left_tops[i].x;
                bottom = bottom_rights[i].y;
                right = bottom_rights[i].x;
            }
            top = top < left_tops[i].y ? top : left_tops[i].y;
            left = left < left_tops[i].x ? left : left_tops[i].x;
            bottom = bottom > bottom_rights[i].y ? bottom : bottom_rights[i].y;
            right = right > bottom_rights[i].x ? right : bottom_rights[i].x;
        }
        Rect final_rect(left, top, right - left, bottom - top);
        printf("Final rect: [%d %d %d %d]\n", left, top, right - left, bottom - top);
        char out_name[200];
        for(int i = 0; i < Imgs.size(); ++i)
        {
            sprintf(out_name, "out_image_%02d.jpg", i);
            printf("%s\n", out_name);
            imwrite( out_name, Imgs[i](final_rect) );
        }
    }
    else
        printf("No rect select !\n");

    return 0;
}
