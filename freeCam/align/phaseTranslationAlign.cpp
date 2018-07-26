/*************************************************************************
    > File Name: phaseTranslationAlign.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-07-23 13:27:53
 ************************************************************************/

#include<iostream>
#include <cstdio>
#include <cstdarg>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
#ifndef DEBUG
#define DEBUG 1
#endif
void vlog(const char *format,...);
void vlog(const char *format,...) {
#if DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}
static void help()
{
    printf("Demonstrate the process of alignment based on Fourier shift theorem that detecting the translational shift in the frequency domain.\n");
    printf("Usage:\n\t./phaseTranslationAlign img1 img2....\n");
}
void contImg(const Mat &img, const Mat &reference, const Point2d shift, const char* outname1, const char* outname2);
void calcMove(const vector<Point2d> &Shifts, vector<Point2d> &out);
void contImgList(const vector<Mat> Imgs, const vector<Point2d> Shifts, const char* outImgName);
int main(int argc, char *argv[])
{
    if(argc < 3)
    {
        help();
        vlog("Input ERROR!\n");
        return -1;
    }
    vector<Mat> inputs;
    vector<Point2d> Shifts;
    Mat img, cur, reference, cur64f, reference64f;
    Mat hann, lhann, rhann, thann, bhann;

    // load images
    for(int i = 1; i < argc; ++i)
    {
        //vlog("%s\n", argv[i]);
        img = imread(argv[i], 1);
        if( img.empty() )
        {
            vlog("Read %s ERROR !\n", argv[i]);
            return -1;
        }
        inputs.push_back( img.clone() );
    }
    
    // deal with the whole, left half and right half image;
    // Rect(x, y, width, height)
    Rect lhalf = Rect(0, 0, img.cols / 2, img.rows);
    Rect rhalf = Rect(img.cols / 2, 0, img.cols / 2, img.rows);
    Rect thalf = Rect(0, 0, img.cols, img.rows / 2);
    Rect bhalf = Rect(0, img.rows / 2, img.cols, img.rows / 2);
    //cout<<"lhalf: "<<lhalf<<endl;
    //cout<<"rhalf: "<<rhalf<<endl;
    //cout<<"thalf: "<<thalf<<endl;
    //cout<<"bhalf: "<<bhalf<<endl;
    
    // create hann
    createHanningWindow(hann, img.size(), CV_64F);
    createHanningWindow(lhann, img(lhalf).size(), CV_64F);
    //createHanningWindow(rhann, img(rhalf).size(), CV_64F);
    createHanningWindow(thann, img(thalf).size(), CV_64F);
    createHanningWindow(bhann, img(bhalf).size(), CV_64F);

    for(size_t i = 1; i < inputs.size(); ++i)
    {
        cvtColor(inputs[i - 1], reference, CV_BGR2GRAY);
        cvtColor(inputs[i], cur, CV_BGR2GRAY);
        reference.convertTo(reference64f, CV_64F);
        cur.convertTo(cur64f, CV_64F);
        Point2d shift = phaseCorrelate(reference64f, cur64f, hann);

        if( shift.x > 0 )
        {
            // left_half
            reference(lhalf).convertTo(reference64f, CV_64F);
            cur(lhalf).convertTo(cur64f, CV_64F);
            shift = phaseCorrelate(reference64f, cur64f, lhann);
            //vlog("\tleft_half : x = %lf, y = %lf\n", shift.x, shift.y);
            //if(shift.x < 0) contImg(inputs[i], inputs[i-1], shift, "left_hcont.jpg", "left_vcont.jpg");
            if( shift.x > 0 )
            {
                // bottom_half
                reference(bhalf).convertTo(reference64f, CV_64F);
                cur(bhalf).convertTo(cur64f, CV_64F);
                shift = phaseCorrelate(reference64f, cur64f, bhann);
                //vlog("\tbottom_half: x = %lf, y = %lf\n", shift.x, shift.y);
                //if(shift.x < 0) contImg(inputs[i], inputs[i-1], shift, "bottom_hcont.jpg", "bottom_vcont.jpg");
                if( shift.x > 0 )
                {
                    // top_half
                    reference(thalf).convertTo(reference64f, CV_64F);
                    cur(thalf).convertTo(cur64f, CV_64F);
                    shift = phaseCorrelate( reference64f, cur64f, thann );
                    //vlog("\ttop_half  : x = %lf, y = %lf\n", shift.x, shift.y);
                    //if(shift.x < 0) contImg(inputs[i], inputs[i-1], shift, "top_hcont.jpg", "top_vcont.jpg");
                }
            }

            if( shift.x > 0 )
            {
                // set default value
                shift.x = -(double)(reference.cols) / 4;
                shift.y = 0;
            }
        }
        //vlog("%s ---> %s: x = %lf, y = %lf\n", argv[i], argv[i + 1], shift.x, shift.y);
        //vlog("%lf %lf\n", shift.x, shift.y);
        Shifts.push_back( shift );
    }
    vector<Point2d> out;
    calcMove(Shifts, out);
    for(size_t i = 0; i < out.size(); ++i) printf("%lf %lf\n", out[i].x, out[i].y);
    contImgList(inputs, Shifts, "result.jpg");
    return 0;

}

void calcMove(const vector<Point2d> &Shifts, vector<Point2d> &out)
{
    out = vector<Point2d>(Shifts.size() + 1);
    Point2d shift = Point2d(0, 0);
    out[0] = shift;
    for(size_t i = 0; i < Shifts.size(); ++i)
        out[i + 1] = -Shifts[i];
    for(size_t i = 1; i < out.size(); ++i)
    {
        out[i].x += out[i - 1].x;
        out[i].y += out[i - 1].y;
    }
    return;
}
void contImgList(const vector<Mat> Imgs, const vector<Point2d> Shifts, const char* outImgName)
{
    int numItems = static_cast<int> (Imgs.size());
    if( numItems < 2 )
    {
        vlog("Not enough images for vconcat !\n");
        return;
    }
    if( numItems != static_cast<int>(Shifts.size()) + 1) 
    {
        vlog("assert Imgs.size() == Shifts.size() + 1  error !\n");
        return;
    }
    int height = Imgs[0].rows;
    int width = Imgs[0].cols;

    // Check img size and Shifts
    for(int i = 1; i < numItems; ++i)
    {
        if(Imgs[i].rows != height || Imgs[i].cols != width)
        {
            vlog("Image size should same.\n");
            return;
        }
        if(Shifts[i - 1].x > 0)
        {
            vlog("Shifts[%d].x cannot > 0.\n", i - 1);
            return;
        }
    }
    vector<Point2i> intShifts(Imgs.size());
    vector<Point2i> intSumShifts(Imgs.size());
    vector<Rect> rects(Imgs.size());
    
    // img0 as reference
    intShifts[0] = Point2i(0, 0);
    intSumShifts[0] = Point2i(0, 0);

    // convet to coor(0, 0) at left-top, double2int
    vlog("intShifts:\n0 0\n");
    for(size_t i = 0; i < Shifts.size(); ++i)
    {
        intShifts[i + 1].x = Shifts[i].x > 0 ? (int)( Shifts[i].x + 0.5 ) : -(int)(-Shifts[i].x + 0.5);
        intShifts[i + 1].y = Shifts[i].y > 0 ? (int)( Shifts[i].y + 0.5 ) : -(int)(-Shifts[i].y + 0.5);
        intShifts[i + 1] = -intShifts[i + 1];
        vlog("%d %d\n", intShifts[i+1].x, intShifts[i+1].y);
        intSumShifts[i + 1] = intShifts[i + 1];
    }
    
    // Find max x,y move, maxposy for max step moving down, minnegy for max step moving up.
    int maxposy = 0, minnegy = 0;
    vlog("intSumShifts:\n0 0\n");
    for(size_t i = 1; i < intSumShifts.size(); ++i)
    {
        intSumShifts[i] += intSumShifts[i - 1];
        vlog("%d %d\n", intSumShifts[i].x, intSumShifts[i].y);
        if( intSumShifts[i].y > 0 && intSumShifts[i].y > maxposy ) maxposy = intSumShifts[i].y;
        if( intSumShifts[i].y < 0 && intSumShifts[i].y < minnegy ) minnegy = intSumShifts[i].y;
    }
    vlog("\nmaxposy = %d; minnegy = %d\n", maxposy, minnegy);

    int dsth = height - maxposy + minnegy;
    //Mat dst = Mat::zeros(dsth, width + intSumShifts[ intSumShifts.size() - 1 ].x, CV_8UC3);
    Mat dst = Mat::zeros(dsth, width + intSumShifts[ numItems - 1 ].x, CV_8UC3);
    //cout<<dst.size()<<endl;
    //|---->+
    //|
    //+
    for(size_t i = 0; i < intSumShifts.size() - 1; ++i)
    {
        rects[i] = Rect( 0, maxposy - intSumShifts[i].y, intShifts[i + 1].x, dsth);
    }
    //rects[ intSumShifts.size() - 1 ] = Rect(0, maxposy - intSumShifts[ intSumShifts.size() - 1 ].y, 1280, dsth);
    rects[ numItems - 1 ] = Rect(0, maxposy - intSumShifts[ numItems - 1 ].y, 1280, dsth);
    for(size_t i = 0; i < rects.size() - 1; ++i)
    {
        Rect dstrect = Rect(intSumShifts[i].x, 0, intShifts[i + 1].x, dsth);
        //cout<<i<<"-->"<<rects[i]<<"---"<<dstrect<<endl;
        Imgs[i](rects[i]).copyTo(dst( dstrect ));
    }
    //Rect dstrect = Rect(intSumShifts[ intSumShifts.size() - 1 ].x, 0, 1280, dsth);
    //Imgs[ intSumShifts.size() - 1 ](rects[ intSumShifts.size() - 1 ]).copyTo( dst(dstrect) );
    Rect dstrect = Rect(intSumShifts[ numItems - 1 ].x, 0, 1280, dsth);
    Imgs[ numItems - 1 ](rects[ numItems - 1 ]).copyTo( dst(dstrect) );
    imwrite(outImgName, dst);
    return;
}
void contImg(const Mat &img1, const Mat &img2, const Point2d shift, const char* outname1, const char* outname2)
{
    Mat img, reference;
    img1.copyTo(img);
    img2.copyTo(reference);
    line(img, Point(0, reference.rows / 2), Point(reference.cols, reference.rows / 2), Scalar(0, 0, 255));
    line(img, Point(reference.cols / 2, 0), Point(reference.cols / 2, reference.rows), Scalar(0, 255, 0));
    line(reference, Point(0, reference.rows / 2), Point(reference.cols, reference.rows / 2), Scalar(0, 0, 255));
    line(reference, Point(reference.cols / 2, 0), Point(reference.cols / 2, reference.rows), Scalar(0, 255, 0));

    int xmove, ymove;
    double newx = shift.x;
    double newy = shift.y;
    /*
    if(newx < 0) xmove = -(int)(-(newx - 1));
    else xmove = (int)(newx + 1);
    if(newy < 0) ymove = -(int)(-(newy - 1));
    else ymove = (int)(newy + 1);
    */
    //round
    if(newx < 0) xmove = -(int)(-(newx) + 0.5);
    else xmove = (int)(newx + 0.5);
    if(newy < 0) ymove = -(int)(-(newy) + 0.5);
    else ymove = (int)(newy + 0.5);
    // trans (0,0) to left-top
    xmove = -xmove;
    ymove = -ymove;

    Rect rect1, rect2;
    int dsth, dstw;
    if(xmove > 0) //move right
    {
        dstw = reference.cols + xmove;
        rect1.x = 0;
        rect1.width = xmove;
        rect2.x = 0;
        rect2.width = reference.cols;
        if(ymove > 0)
        {
            cout<<"ymove > 0\n";
            dsth = reference.rows - ymove;
            rect1.y = ymove;
            rect1.height = dsth;
            rect2.y = 0;
            rect2.height = dsth;
        }
        else
        {
            cout<<"ymove < 0\n";
            dsth = reference.rows + ymove;
            rect1.y = 0;
            rect1.height = dsth;
            rect2.y = -ymove;
            rect2.height = dsth;
        }
    }
    else
    {
        //TODO
        cout<<"uncomplete!\n";
        return;
    }


    Mat dst = Mat::zeros(dsth, dstw, CV_8UC3);

    Rect dstrect1, dstrect2;
    dstrect1.x = 0;
    dstrect1.y = 0;
    dstrect1.height = dsth;
    dstrect1.width = rect1.width;
    
    dstrect2.x = rect1.width;
    dstrect2.y = 0;
    dstrect2.height = dsth;
    dstrect2.width = rect2.width;

    /*
    cout<<"Rect1 = "<<rect1<<endl;
    cout<<"Rect2 = "<<rect2<<endl;
    cout<<"Dst.size() = "<<dst.size()<<endl;
    cout<<"dstrect1 = "<<dstrect1<<endl;
    cout<<"dstrect2 = "<<dstrect2<<endl;
    */
    reference(rect1).copyTo(dst(dstrect1));
    img(rect2).copyTo(dst(dstrect2));
    imwrite(outname1, dst);

    Mat dst2 = Mat::zeros(reference.rows * 2, reference.cols, CV_8UC3);
    reference.copyTo( dst2( Rect(0, 0, reference.cols, reference.rows) ) );

    Rect rect3 = Rect(0, 0, reference.cols - xmove, reference.rows);
    Rect rect4 = Rect(xmove, reference.rows, reference.cols - xmove, reference.rows);
    img(rect3).copyTo(dst2(rect4));
    imwrite(outname2, dst2);
}
