/*************************************************************************
    > File Name: interactiveColor.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-06-01 15:21:35
 ************************************************************************/
#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
using namespace cv;
using namespace std;

//Global Variables
Mat img, placeholder;

//int _bgrPixel[3], _hsvPixel[3], _ycbPixel[3], _labPixel[3];
Vec3b _bgrPixel, _hsvPixel, _ycbPixel, _labPixel;
vector<Vec3b> bgrSave, hsvSave, ycbSave, labSave;
// Callback function for any event on he mouse
void onMouse( int event, int x, int y, int flags, void* userdata )
{   
    if( event == EVENT_MOUSEMOVE )
	{

     	Vec3b bgrPixel(img.at<Vec3b>(y, x));
        
        Mat3b hsv,ycb,lab;
        // Create Mat object from vector since cvtColor accepts a Mat object
        Mat3b bgr (bgrPixel);
        
        //Convert the single pixel BGR Mat to other formats
        cvtColor(bgr, ycb, COLOR_BGR2YCrCb);
        cvtColor(bgr, hsv, COLOR_BGR2HSV);
        cvtColor(bgr, lab, COLOR_BGR2Lab);
        
        //Get back the vector from Mat
        Vec3b hsvPixel(hsv.at<Vec3b>(0,0));
        Vec3b ycbPixel(ycb.at<Vec3b>(0,0));
        Vec3b labPixel(lab.at<Vec3b>(0,0));
       
        // Create an empty placeholder for displaying the values
        placeholder = Mat::zeros(img.rows,400,CV_8UC3);

        //fill the placeholder with the values of color spaces
        putText(placeholder, format("BGR [%d, %d, %d]",bgrPixel[0],bgrPixel[1],bgrPixel[2]), Point(20, 70), FONT_HERSHEY_COMPLEX, .9, Scalar(255,255,255), 1);
        putText(placeholder, format("HSV [%d, %d, %d]",hsvPixel[0],hsvPixel[1],hsvPixel[2]), Point(20, 140), FONT_HERSHEY_COMPLEX, .9, Scalar(255,255,255), 1);
        putText(placeholder, format("YCrCb [%d, %d, %d]",ycbPixel[0],ycbPixel[1],ycbPixel[2]), Point(20, 210), FONT_HERSHEY_COMPLEX, .9, Scalar(255,255,255), 1);
        putText(placeholder, format("LAB [%d, %d, %d]",labPixel[0],labPixel[1],labPixel[2]), Point(20, 280), FONT_HERSHEY_COMPLEX, .9, Scalar(255,255,255), 1);
        
        _bgrPixel = bgrPixel;
        _hsvPixel = hsvPixel;
        _ycbPixel = ycbPixel;
        _labPixel = labPixel;
        /*
        for(int i = 0; i < 3; i ++) {
            _bgrPixel[i] = bgrPixel[i];
            _hsvPixel[i] = hsvPixel[i];
            _ycbPixel[i] = ycbPixel[i];
            _labPixel[i] = labPixel[i];
        }
        */
	    Size sz1 = img.size();
	    Size sz2 = placeholder.size();
	    
        //Combine the two results to show side by side in a single image
        Mat combinedResult(sz1.height, sz1.width+sz2.width, CV_8UC3);
	    Mat left(combinedResult, Rect(0, 0, sz1.width, sz1.height));
	    img.copyTo(left);
	    Mat right(combinedResult, Rect(sz1.width, 0, sz2.width, sz2.height));
	    placeholder.copyTo(right);
	    imshow("ESC to Exit", combinedResult);
    }
    if(event == EVENT_FLAG_LBUTTON) {
        printf("\nBGR:( %3d, %3d, %3d )\n", _bgrPixel[0], _bgrPixel[1], _bgrPixel[2]);
        printf("HSV:( %3d, %3d, %3d )\n", _hsvPixel[0], _hsvPixel[1], _hsvPixel[2]);
        printf("YCB:( %3d, %3d, %3d )\n", _ycbPixel[0], _ycbPixel[1], _ycbPixel[2]);
        printf("LAB:( %3d, %3d, %3d )\n", _labPixel[0], _labPixel[1], _labPixel[2]);

        bgrSave.push_back(_bgrPixel);
        hsvSave.push_back(_hsvPixel);
        ycbSave.push_back(_ycbPixel);
        labSave.push_back(_labPixel);
    }
}

void getColorThresh(vector<Vec3b> &bgrSave, vector<Vec3b> &hsvSave, vector<Vec3b> &ycbSave, vector<Vec3b> &labSave)
{
    // min0, min1, min2, max0, max1, max2;
    int threshBGR[6] = {256, 256, 256, -1, -1, -1};
    int threshHSV[6] = {256, 256, 256, -1, -1, -1};
    int threshYCB[6] = {256, 256, 256, -1, -1, -1};
    int threshLAB[6] = {256, 256, 256, -1, -1, -1};
    int size = bgrSave.size();
    if(size != hsvSave.size() || size != ycbSave.size() || size != labSave.size()) {
        cout<<"Save error !\n"<<endl;
        return;
    }
    for(int i = 0; i < size; i ++) {
        for(int j = 0; j < 3; j ++) {
            threshBGR[j] = bgrSave[i][j] < threshBGR[j] ? bgrSave[i][j] : threshBGR[j]; //min
            threshBGR[j+3] = bgrSave[i][j] > threshBGR[j + 3] ? bgrSave[i][j] : threshBGR[j + 3]; //max
            threshHSV[j] = hsvSave[i][j] < threshHSV[j] ? hsvSave[i][j] : threshHSV[j]; //min
            threshHSV[j+3] = hsvSave[i][j] > threshHSV[j + 3] ? hsvSave[i][j] : threshHSV[j + 3]; //max
            threshYCB[j] = ycbSave[i][j] < threshYCB[j] ? ycbSave[i][j] : threshYCB[j]; //min
            threshYCB[j+3] = ycbSave[i][j] > threshYCB[j + 3] ? ycbSave[i][j] : threshYCB[j + 3]; //max
            threshLAB[j] = labSave[i][j] < threshLAB[j] ? labSave[i][j] : threshLAB[j]; //min
            threshLAB[j+3] = labSave[i][j] > threshLAB[j + 3] ? labSave[i][j] : threshLAB[j + 3]; //max
        }
    }
    printf("BGR Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", threshBGR[0], threshBGR[1], threshBGR[2], threshBGR[3], threshBGR[4], threshBGR[5]);
    printf("HSV Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", threshHSV[0], threshHSV[1], threshHSV[2], threshHSV[3], threshHSV[4], threshHSV[5]);
    printf("YCB Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", threshYCB[0], threshYCB[1], threshYCB[2], threshYCB[3], threshYCB[4], threshYCB[5]);
    printf("LAB Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", threshLAB[0], threshLAB[1], threshLAB[2], threshLAB[3], threshLAB[4], threshLAB[5]);
}

void help(const char *program)
{
    printf("This program demonstrate the different color space.\n");
    printf("Moving mouse on the image, detect color in BGR and transform to HSV, YCrCb, Lab.\n");
    printf("Left button to save the current color to vector and calc a thresh [min0 min1 min2 max0 max1 max2] for inRange.\n");
    printf("ESC to Exit.\n");
    printf("Usage:\n\t%s imgName.jpg\n", program);
}
int main( int argc, const char** argv )
{
    // filename
    // Read the input image
    if( argc != 2 ) {
        printf("Input ERROR.\n");
        help(argv[0]);
    }
    
    img = imread(argv[1]);
    // Resize the image to 400x400
    //Size rsize(400,400);
    //resize(img,img,rsize);

    if(img.empty())
    {
        printf("Input image empty !\n");
        help(argv[0]);
        return -1;
    }
    
    // Create an empty window
    namedWindow("ESC to Exit", WINDOW_AUTOSIZE);   
    // Create a callback function for any event on the mouse
    setMouseCallback( "ESC to Exit", onMouse );
    
    imshow( "ESC to Exit", img );
    
    while(1)
    {
        char k = waitKey(1) & 0xFF;
        if (k == 27)
            break;
    }
    
    getColorThresh(bgrSave, hsvSave, ycbSave, labSave);
    return 0;
}
