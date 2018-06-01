/*************************************************************************
    > File Name: interactiveColorSegment2.cpp
    > Author: li_pengju
    > Created Time: 2018-06-01 13:06:09
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
// trackbar function;
void bgrTrackbarActivity(int value, void*);
void hsvTrackbarActivity(int value, void*);
void ycbTrackbarActivity(int value, void*);
void labTrackbarActivity(int value, void*);
// global variable to input and output trackbar.
int minr = 0, ming = 0, minb = 0, maxr = 255, maxg = 255, maxb = 255;
int minh = 0, mins = 0, minv = 0, maxh = 255, maxs = 255, maxv = 255;
int miny = 0, mincr = 0, mincb = 0, maxy = 255, maxcr = 255, maxcb = 255;
int minL = 0, minA = 0, minB = 0, maxL = 255, maxA = 255, maxB = 255;
// global image deal with in trackbar function
Mat original, imageBGR, imageHSV, imageYCrCb, imageLab;
void help( const char* program )
{
    printf("This program demonstrate the color Thresh segment in an interactive way.\n");
    printf("Get value in trackbar and deal with image, then show.\n");
    printf("Usage:\n\t%s imgname.jpg\n", program);
}
int main(int argc, char *argv[])
{
    if( argc != 2 ) {
        printf("Input ERROR !\n");
        help(argv[0]);
        return -1;
    }
    original = imread(argv[1], 1);
    if( original.empty() ) {
        printf("image is empty !\n");
        return -1;
    }
   
    int resizeWidth = original.cols;
    int resizeHeight = original.rows;
    if( original.rows > 640 || original.cols > 720 ) {
        printf("Resize image to a samller size.\n");
        resizeWidth = original.cols / 2;
        resizeHeight = original.rows / 2;
        Size rsize(resizeHeight, resizeWidth);
        resize(original, original, rsize);
    }
    
    cvtColor(original, imageHSV, COLOR_BGR2HSV);
    cvtColor(original, imageYCrCb, COLOR_BGR2YCrCb);
    cvtColor(original, imageLab, COLOR_BGR2Lab);
    original.copyTo(imageBGR);
	// position on the screen where the windows start 
	int initialX = 50;
	int	initialY = 50;

    namedWindow("ESC to Exit", WINDOW_AUTOSIZE);
	namedWindow("SelectBGR", WINDOW_AUTOSIZE);
	namedWindow("SelectHSV", WINDOW_AUTOSIZE);
	namedWindow("SelectYCB", WINDOW_AUTOSIZE);
	namedWindow("SelectLAB", WINDOW_AUTOSIZE);

	// moving the windows to stack them horizontally 
	moveWindow("ESC to Exit", initialX, initialY);
	moveWindow("SelectBGR", initialX + 1 * (resizeWidth + 5), initialY);
	moveWindow("SelectHSV", initialX + 2 * (resizeWidth + 5), initialY);
	moveWindow("SelectYCB", initialX + 3 * (resizeWidth + 5), initialY);
	moveWindow("SelectLAB", initialX + 4 * (resizeWidth + 5), initialY);

	createTrackbar("Min Cr", "SelectYCB", &mincr, 255, ycbTrackbarActivity);
	createTrackbar("Max Cr", "SelectYCB", &maxcr, 255, ycbTrackbarActivity);
	createTrackbar("Min Cb", "SelectYCB", &mincb, 255, ycbTrackbarActivity);
	createTrackbar("Max Cb", "SelectYCB", &maxcb, 255, ycbTrackbarActivity);
	createTrackbar("Min Y", "SelectYCB", &miny, 255, ycbTrackbarActivity);
	createTrackbar("Max Y", "SelectYCB", &maxy, 255, ycbTrackbarActivity);

	// creating trackbars to get values for HSV 
	createTrackbar("Min H", "SelectHSV", &minh, 255, hsvTrackbarActivity); //180
	createTrackbar("Max H", "SelectHSV", &maxh, 255, hsvTrackbarActivity);
	createTrackbar("Min S", "SelectHSV", &mins, 255, hsvTrackbarActivity);
	createTrackbar("Max S", "SelectHSV", &maxs, 255, hsvTrackbarActivity);
	createTrackbar("Min V", "SelectHSV", &minv, 255, hsvTrackbarActivity);
	createTrackbar("Max V", "SelectHSV", &maxv, 255, hsvTrackbarActivity);

	// creating trackbars to get values for BGR 
	createTrackbar("Min B", "SelectBGR", &minb, 255, bgrTrackbarActivity);
	createTrackbar("Max B", "SelectBGR", &maxb, 255, bgrTrackbarActivity);
	createTrackbar("Min G", "SelectBGR", &ming, 255, bgrTrackbarActivity);
	createTrackbar("Max G", "SelectBGR", &maxg, 255, bgrTrackbarActivity);
	createTrackbar("Min R", "SelectBGR", &minr, 255, bgrTrackbarActivity);
	createTrackbar("Max R", "SelectBGR", &maxr, 255, bgrTrackbarActivity);

	// creating trackbars to get values for LAB 
	createTrackbar("Min L", "SelectLAB", &minL, 255, labTrackbarActivity);
	createTrackbar("Max L", "SelectLAB", &maxL, 255, labTrackbarActivity);
	createTrackbar("Min A", "SelectLAB", &minA, 255, labTrackbarActivity);
	createTrackbar("Max A", "SelectLAB", &maxA, 255, labTrackbarActivity);
	createTrackbar("Min B", "SelectLAB", &minB, 255, labTrackbarActivity);
	createTrackbar("Max B", "SelectLAB", &maxB, 255, labTrackbarActivity);


	// show all images initially 
	imshow("SelectHSV", original);
	imshow("SelectYCB", original);
	imshow("SelectLAB", original);
	imshow("SelectBGR", original);
    char k;
    while(1)
    {
        imshow("ESC to Exit", original);
        k = waitKey(1) & 0xFF;
        if(k == 27) break;
        
    }
	destroyAllWindows();
    printf("BGR Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", minb, ming, minr, maxb, maxg, maxr);
    printf("HSV Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", minh, mins, minv, maxh, maxs, maxv);
    printf("YCB Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", miny, mincr, mincb, maxy, maxcr, maxcb);
    printf("LAB Thresh: %3d, %3d, %3d, %3d, %3d, %3d\n", minL, minA, minB, maxL, maxA, maxB);
    return 0;
}
void bgrTrackbarActivity(int value, void*)
{
    // Get values from the BGR trackbar
    /*
    int BMin = getTrackbarPos("BMin", "SelectBGR");
    int GMin = getTrackbarPos("GMin", "SelectBGR");
    int RMin = getTrackbarPos("RMin", "SelectBGR");

    int BMax = getTrackbarPos("BMax", "SelectBGR");
    int GMax = getTrackbarPos("GMax", "SelectBGR");
    int RMax = getTrackbarPos("RMax", "SelectBGR");

    Scalar minBGR = Scalar(BMin, GMin, RMin);
    Scalar maxBGR = Scalar(BMax, GMax, RMax);
    */
    Scalar minBGR = Scalar(minb, ming, minr);
    Scalar maxBGR = Scalar(maxb, maxg, maxr);
    //printf("%3d %3d %3d %3d %3d %3d\n", minb, ming, minr, maxb, maxg, maxr);
    Mat maskBGR;
    // Create the mask using the min and max values obtained from trackbar and apply bitwise and operation to get the results
    inRange(imageBGR, minBGR, maxBGR, maskBGR);
    Mat resultBGR = Mat::zeros(original.rows, original.cols, CV_8UC3);
    bitwise_and(original, original, resultBGR, maskBGR);
    imshow("SelectBGR", resultBGR);
}
void hsvTrackbarActivity(int value, void*)
{
    Scalar minHSV = Scalar(minh, mins, minv);
    Scalar maxHSV = Scalar(maxh, maxs, maxv);
    Mat maskHSV;
    inRange(imageHSV, minHSV, maxHSV, maskHSV);
    Mat resultHSV = Mat::zeros(original.rows, original.cols, CV_8UC3);
    bitwise_and(original, original, resultHSV, maskHSV);
    imshow("SelectHSV", resultHSV);
}
void ycbTrackbarActivity(int value, void*)
{
    Scalar minYCrCb = Scalar(miny, mincr, mincb);
    Scalar maxYCrCb = Scalar(maxy, maxcr, maxcb);
    Mat maskYCrCb;
    inRange(imageYCrCb, minYCrCb, maxYCrCb, maskYCrCb);
    Mat resultYCrCb = Mat::zeros(original.rows, original.cols, CV_8UC3);
    bitwise_and(original, original, resultYCrCb, maskYCrCb);
    imshow("SelectYCB", resultYCrCb);
}
void labTrackbarActivity(int value, void*)
{
    Scalar minLab = Scalar(minL, minA, minB);
    Scalar maxLab = Scalar(maxL, maxA, maxB);
    Mat maskLab;
    inRange(imageLab, minLab, maxLab, maskLab);
    Mat resultLab = Mat::zeros(original.rows, original.cols, CV_8UC3);
    bitwise_and(original, original, resultLab, maskLab);
    imshow("SelectLAB", resultLab);
}
