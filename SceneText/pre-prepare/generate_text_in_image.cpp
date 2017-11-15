/*************************************************************************
    > File Name: generate_text_in_image2.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2017-11-06 10:11:38
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
#include<cstdio>
#include<cstdlib>
#include<time.h>
using namespace std;
using namespace cv;
#define WIDTH 640
#define HEIGHT 480
#define TOTAL_TEXT_CLASS 52
/// Function headers
static Scalar randomColor( RNG& rng );
int Drawing_Random_Filled_Polygons( Mat image, int number, RNG rng );

int main(int argc, char *argv[])
{
    bool out_flag = false;
    if( argc ==2 ) {
        out_flag = true;
    }
    char texts[] = {'1','2','3','4','5','6','7','8','9','0'
    ,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T'
    ,'U','V','W','X','Y','Z',
    'a','b','d','e','f','g','h','i','j','l','m','n','q','r','t','y'};
    //char texts2[] = {'C','c','K','k','O','o','U','u','S','s','V', 'v', 'W', 'w', 'X', 'x', 'Z', 'z'};
    //srand(time(NULL));
    srand(getTickCount());
    //RNG rng( 0xFFFFFFFF );
    RNG rng( rand() );
    Mat img(HEIGHT, WIDTH, CV_8UC3, Scalar::all(0));
    int flag = Drawing_Random_Filled_Polygons( img, 50, rng );
    if( flag != 0 ) return -1;

    int fontFace = 5; // text type
    int thickness=3; // text line thickness
    int baseline = 0;
    int text_num = rng.uniform(5, 11); //5~10 chars, generate 5~10 letters on one image
    for(int i = 0; i < text_num; i ++) {
        int text_class = rng.uniform(0, TOTAL_TEXT_CLASS);
        string text(1, texts[text_class]);
        double fontScale = rng.uniform(2, 5);
        Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
        //baseline += thickness;
        int x = rng.uniform(thickness + 2, WIDTH - textSize.width);
        int y = rng.uniform(textSize.height + thickness + 2, HEIGHT);
        Point textOrg(x, y);
        Point textStart = textOrg + Point(0, -textSize.height - thickness);
        Point textEnd = textOrg + Point(textSize.width, baseline);

        //putText(img, text, textOrg, fontFace, fontScale, Scalar::all(255), thickness, 8, false);
        putText(img, text, textOrg, fontFace, fontScale, randomColor(rng), thickness, 8, false);

        //cout<<"textStart = ( "<<textStart.x<<", "<<textStart.y<<" )"<<endl;
        //cout<<"textEnd = ( "<<textEnd.x<<", "<<textEnd.y<<" )"<<endl;
        if( !out_flag ) {
            cout<<"(xmin,ymin,xmax,ymax): ["<<textStart.x<<" "<<textStart.y<<" "<<textEnd.x<<" "<<textEnd.y<<"]; class = "<<text_class + 1<<endl;
            rectangle(img, textStart , textEnd, Scalar(0,0,255));
            circle(img, textStart, 5, Scalar(255, 0, 0));
            circle(img, textEnd, 5, Scalar(0, 255, 0));
            imshow("image", img);
            waitKey();
        }
        else{
            //fprintf(fp, "%d %d %d %d %d\n", textStart.x, textStart.y, textEnd.x, textEnd.y, text_class + 1);
            /* python scrips with read this cout for xml */
            textStart.x = MAX(textStart.x, 1);
            textStart.y = MAX(textStart.y, 1);
            textEnd.x = MIN(textEnd.x, WIDTH - 1);
            textEnd.y = MIN(textEnd.y, HEIGHT - 1);
            cout<<texts[text_class]<<" "<<textStart.x<<" "<<textStart.y<<" "<<textEnd.x<<" "<<textEnd.y<<endl;
        }

    }
    if( out_flag )
        imwrite(argv[1], img);
    return 0;
}

/**
 * @function randomColor
 * @brief Produces a random color given a random object
 */
static Scalar randomColor( RNG& rng )
{
  int icolor = (unsigned) rng;
  return Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

/**
 * @function Drawing_Random_Filled_Polygons
 */
int Drawing_Random_Filled_Polygons( Mat image, int number, RNG rng )
{
    int lineType = 8;

    /// Global Variables
    const int window_width = WIDTH;
    const int window_height = HEIGHT;
    int x_1 = -window_width/2;
    int x_2 = window_width*3/2;
    int y_1 = -window_width/2;
    int y_2 = window_width*3/2;
  for ( int i = 0; i < number; i++ )
  {
    Point pt[2][3];
    pt[0][0].x = rng.uniform(x_1, x_2);
    pt[0][0].y = rng.uniform(y_1, y_2);
    pt[0][1].x = rng.uniform(x_1, x_2);
    pt[0][1].y = rng.uniform(y_1, y_2);
    pt[0][2].x = rng.uniform(x_1, x_2);
    pt[0][2].y = rng.uniform(y_1, y_2);
    pt[1][0].x = rng.uniform(x_1, x_2);
    pt[1][0].y = rng.uniform(y_1, y_2);
    pt[1][1].x = rng.uniform(x_1, x_2);
    pt[1][1].y = rng.uniform(y_1, y_2);
    pt[1][2].x = rng.uniform(x_1, x_2);
    pt[1][2].y = rng.uniform(y_1, y_2);

    const Point* ppt[2] = {pt[0], pt[1]};
    int npt[] = {3, 3};

    fillPoly( image, ppt, npt, 2, randomColor(rng), lineType );
  }
  return 0;
}

