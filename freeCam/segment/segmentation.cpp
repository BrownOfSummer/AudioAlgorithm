// Intend to segment image.
#include<iostream>
#include<vector>
#include<string>
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
/*
 * Get the edge of a gray image, use Canny.
 * Input:   image,      gray;
 *          edgeThresh, threshold for the hysteresis procedure.
 *          use_scharr, select between sobel or Scharr;
 * OutPut: edge, output edge map; single channels 8-bit image, which has the same size as image.
 */
void getEdges(Mat image, Mat &edge, int edgeThresh, bool use_scharr)
{
    Mat gray, blurImage;
    if(image.channels() > 1) cvtColor(image, gray, CV_BGR2GRAY);
    else gray = image;
    blur(gray, blurImage, Size(3,3));
    if( ! use_scharr ) {
        // use sobel inner.
        Canny(blurImage, edge, edgeThresh, edgeThresh * 3, 3);
    }
    else {
        /// Canny detector with scharr
        Mat dx,dy;
        Scharr(blurImage, dx, CV_16S, 1, 0);
        Scharr(blurImage, dy, CV_16S, 0, 1);
        Canny( dx, dy, edge, edgeThresh, edgeThresh * 3 );
    }
    return;
}

/*
 * segment the image with otsu threshold, and edges;
 * Input:   gray img;
 * OutPut:  binary mask with same size of img;
 */
void roughThreshSegment(Mat img, Mat &mask)
{
    Mat gray, otsuArea, adapArea, edge, dst;
    if(img.channels() > 1) cvtColor(img, gray, CV_BGR2GRAY);
    else gray = img;
    getEdges(gray, edge, 30, false);
    //dilate(edge, edge, Mat(), Point(-1,-1));
    //MORPH_CROSS, MORPH_ELLIPSE, MORPH_RECT
    //Mat kernel = getStructuringElement(MORPH_CROSS, Size(3, 3), Point(1,1));
    //morphologyEx(edge, edge, MORPH_CLOSE, kernel, Point(1,1), 2 );
    morphologyEx(edge, edge, MORPH_CLOSE, Mat::ones(3,3,CV_8SC1), Point(1,1), 2 );

    GaussianBlur(gray, gray, Size(3, 3), 0, 0, BORDER_DEFAULT);
    
    threshold(gray, otsuArea, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);
    //dilate(otsuArea, otsuArea, Mat(), Point(-1,-1));
    morphologyEx(otsuArea, otsuArea, MORPH_CLOSE, Mat::ones(3,3,CV_8SC1), Point(1,1), 2 );
    
    adaptiveThreshold(gray, adapArea, 255, THRESH_BINARY_INV, ADAPTIVE_THRESH_GAUSSIAN_C, 3, 3);
    //dilate(adapArea, adapArea, Mat(), Point(-1,-1));
    morphologyEx(adapArea, adapArea, MORPH_CLOSE, Mat::ones(3,3,CV_8SC1), Point(1,1), 2 );

    bitwise_or(adapArea, otsuArea, dst, noArray());
    bitwise_or(dst, edge, dst, noArray());

    //Execute morphological-close
    //morphologyEx(dst, dst, MORPH_CLOSE, Mat::ones(9,9,CV_8SC1), Point(4,4), 2 );
    //morphologyEx(dst, dst, MORPH_OPEN, Mat::ones(3,3,CV_8SC1), Point(1,1), 2 );
    
    mask = dst;
    return;
}
/*
 * find the max connected area in the binary image.
 */
Mat singleConnetedDomain( Mat binary )
{
    // Maximum connected domain
    int connectivity = 8;
    Mat labels, centroids, stats;
    connectedComponentsWithStats(binary, labels, stats, centroids, connectivity);
    if(stats.rows < 2) return binary;
    int areaLabel = 0, areaMax = 0;
    for(int r = 1; r < stats.rows; r ++)
    {
        int maxPix = stats.at<int>(r, 4);
        if( maxPix > areaMax ) {
            areaMax = maxPix;
            areaLabel = r;
        }
    }
    Mat ret = Mat::zeros(binary.size(), CV_8UC1);
    for(int x = 0; x < binary.rows; x ++)
        for(int y = 0; y < binary.cols; y++)
            ret.at<uchar>(x, y) = labels.at<int>(x,y) == areaLabel ? 255 : 0;
    return ret;
}

/*
 * find the max area contours in the binary image and return the index in the contours vector;
 * Input:   binary, binary image.
 *          contours, hierarchy, to save contours; 
 * Return: max contours area index;
 */
int getAreaContours(Mat binary, vector<vector<Point> > &contours, vector<Vec4i> &hierarchy)
{
    //vector<vector<Point> > contours;
    //vector<Vec4i> hierarchy;
    if( !contours.empty() ) contours.clear();
    if( !hierarchy.empty() ) hierarchy.clear();

    findContours( binary, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE );
    
    if( contours.size() < 1) return -1;
    
    int largestComp = 0;
    double maxArea = 0;
    for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
    {
        const vector<Point>& c = contours[idx];
        double area = fabs(contourArea(Mat(c)));
        if( area > maxArea )
        {
            maxArea = area;
            largestComp = idx;
        }
    }

    //return contours[largestComp];
    return largestComp;
}

/*
 * find the max connected area in the binary image with contours.
 */
Mat singleConnetedDomainContour(Mat binary)
{
    vector<vector<Point> >contours;
    vector<Vec4i> hierarchy;
    int maxAreaIdx = getAreaContours(binary, contours, hierarchy);
    if( maxAreaIdx < 0 ) return binary;
    Mat ret = Mat::zeros( binary.size(), CV_8UC1 );
    //drawContours(ret, contours, 0, Scalar::all(255), -1);
    drawContours( ret, contours, maxAreaIdx, Scalar::all(255), FILLED, LINE_8, hierarchy );

    return ret;
}

/*
 * fill the black holes in the binary image;
 */
Mat fillHole(Mat binary)
{
    morphologyEx(binary, binary, MORPH_CLOSE, Mat::ones(3,3,CV_8SC1), Point(1,1), 2 );
    Mat filled;
    binary.copyTo(filled);
    floodFill( filled, Point(0,0), Scalar(255) );
    bitwise_not( filled, filled );
    bitwise_or(filled, binary, filled, noArray());
    //morphologyEx(filled, filled, MORPH_OPEN, Mat::ones(3,3,CV_8SC1), Point(1,1), 2 );
    return filled;
}

/*
 * refine the rough segment.
 */
Mat refineBinarySegments(Mat binary)
{
    //Mat new_mask = singleConnetedDomainContour(binary);
    Mat new_mask = singleConnetedDomain(binary);
    new_mask = fillHole( new_mask );
    //vector<vector<Point> >contours;
    //vector<Vec4i> hierarchy;
    //int maxAreaIdx = getAreaContours(new_mask, contours, hierarchy);
    //vector<Point> approx;
    //approxPolyDP(Mat(contours[maxAreaIdx]), approx, arcLength(Mat(contours[maxAreaIdx]), true)*0.02, true);
    //Mat temp = Mat::zeros( binary.size(), CV_8UC3 );
    //fillConvexPoly(temp, approx, Scalar(255, 0, 0));
    return new_mask;
}
/*
 * Demonstrate the threshold segmentation with color;
 * Input:   original,       3 channels BGR image.
 *          colorThresh,    6 colorThresh: [minBGR, maxBGR] | [minHSV | maxHSV] | [minLab, maxLab] | [minYCrCb | maxYCrCb]
 *                          example:[0, 0, 0, 255, 255, 255]
 *          mode:           BGR | HSV | YCB | YCrCb | LAB | Lab
 * Return:  BRG image with color and black background.
 */
Mat colorThreshSegment(Mat original, const vector<int> colorThresh, const string mode)
{
    if( original.channels() != 3 ) {
        vlog("Input image must be BGR 3 channels\n");
        return original;
    }
    if( colorThresh.size() != 6 ) {
        vlog("Input colorThresh must be 6 elements:\n");
        vlog("[minBGR, maxBGR] | [minHSV | maxHSV] | [minLab, maxLab] | [minYCrCb | maxYCrCb]\n");
        return original;
    }
    Mat ret = Mat::zeros( original.rows, original.cols, CV_8UC3 );
    if( mode == "BGR" ) {
        Mat maskBGR, imageBGR;
        original.copyTo(imageBGR);
        Scalar minBGR = Scalar(colorThresh[0], colorThresh[1], colorThresh[2]);
        Scalar maxBGR = Scalar(colorThresh[3], colorThresh[4], colorThresh[5]);
        // Create the mask using the min and max values obtained from trackbar and apply bitwise and operation to get the results
        inRange(imageBGR, minBGR, maxBGR, maskBGR);
        bitwise_and(original, original, ret, maskBGR);
        return ret;
    }
    else if( mode == "HSV" ) {
        Mat maskHSV, imageHSV;
        cvtColor(original, imageHSV, COLOR_BGR2HSV);
        Scalar minHSV = Scalar(colorThresh[0], colorThresh[1], colorThresh[2]);
        Scalar maxHSV = Scalar(colorThresh[3], colorThresh[4], colorThresh[5]);
        inRange(imageHSV, minHSV, maxHSV, maskHSV);
        bitwise_and(original, original, ret, maskHSV);
        return ret;
    }
    else if( mode == "LAB" || mode == "Lab" ) {
        Mat maskLab, imageLab;
        cvtColor(original, imageLab, COLOR_BGR2Lab);
        Scalar minLab = Scalar(colorThresh[0], colorThresh[1], colorThresh[2]);
        Scalar maxLab = Scalar(colorThresh[3], colorThresh[4], colorThresh[5]);
        inRange(imageLab, minLab, maxLab, maskLab);
        bitwise_and(original, original, ret, maskLab);
        return ret;
    }
    else if( mode == "YCB" || mode == "YCrCb" ) {
        Mat maskYCrCb, imageYCrCb;
        cvtColor(original, imageYCrCb, COLOR_BGR2YCrCb);
        Scalar minYCrCb = Scalar(colorThresh[0], colorThresh[1], colorThresh[2]);
        Scalar maxYCrCb = Scalar(colorThresh[3], colorThresh[4], colorThresh[5]);
        inRange(imageYCrCb, minYCrCb, maxYCrCb, maskYCrCb);
        bitwise_and(original, original, ret, maskYCrCb);
        return ret;
    }
    else {
        vlog("mode type must be one of: BGR | HSV | LAB | Lab |YCB | YCrCb\n");
        return original;
    }
}
/*
 * Demonstrate the skin tone threshold to detect skin.
 * two method provided, BGR + HSV or BGR + YCrCb, and the second is a little better in some case.
 * Paras:
 *      src,    Input, 3 channel BGR color image.
 *      dst,    Output, same size with src, remove non-skin-tone pixels;
 *      method, Input,  <=0 for hsv, others for YCrCb
 * Return:
 *      mask, binary image with same size with src, skin-tone is marked as 255;
 * Reference: https://arxiv.org/pdf/1708.02694.pdf
 */
Mat detectSkinTone(Mat src, Mat &dst, int method)
{
    Mat hsv, ycb;
    dst = Mat::zeros(src.size(), CV_8UC3);
    Mat mask = Mat::zeros(src.size(), CV_8UC1);
    if(method < 1)
    {
        cvtColor(src, hsv, CV_BGR2HSV);
        for(size_t i = 0; i < src.rows; i ++) {
            for(size_t j = 0; j < src.cols; j++) {
                Vec3b hsvPixel = hsv.at<Vec3b>(i, j);
                Vec3b bgrPixel = src.at<Vec3b>(i, j);
                int B = bgrPixel[0], G = bgrPixel[1], R = bgrPixel[2];
                int H = hsvPixel[0], S = hsvPixel[1], V = hsvPixel[2];
                if(     H >= 0 && H <= 50 && S >= 58 && S <= 174
                        && R > 95 && G > 40 && B > 20 && R > G  && R > B && R - G > 15 ) {
                    dst.at<Vec3b>(i, j) = src.at<Vec3b>(i, j);
                    mask.at<uchar>(i, j) = 255;
                }
            }
        }
    }
    else 
    {
        cvtColor(src, ycb, CV_BGR2YCrCb);
        for(size_t i = 0; i < src.rows; i ++) {
            for(size_t j = 0; j < src.cols; j ++) {
                Vec3b ycbPixel = ycb.at<Vec3b>(i, j);
                Vec3b bgrPixel = src.at<Vec3b>(i, j);
                int B = bgrPixel[0], G = bgrPixel[1], R = bgrPixel[2];
                int Y = ycbPixel[0], Cr = ycbPixel[1], Cb = ycbPixel[2];
                if(     R > 95 && G > 40 && B > 20 && R > G && R > B && R - G > 15
                        && Cr > 135 && Cb > 85 && Y > 80
                        && Cr <= (1.5862 * Cb) + 20
                        && Cr >= (0.3448 * Cb) + 76.2069
                        && Cr >= (-4.5652 * Cb) + 234.5652
                        && Cr <= (-1.15 * Cb) + 301.75
                        && Cr <= (-2.2857 * Cb) + 432.85 ) {
                    dst.at<Vec3b>(i, j) = src.at<Vec3b>(i, j);
                    mask.at<uchar>(i, j) = 255;
                }
            }
        }
    }
    
    return mask;
}
void getShowContours(Mat binary)
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( binary, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE );

    Mat dst = Mat::zeros(binary.size(), CV_8UC3);

    if( contours.size() == 0 ) {
        vlog("No contours !\n");
        return;
    }
    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int largestComp = 0, cnt = 0;
    double maxArea = 0;
    vlog("Draw each contours, ESC to break:\n");
    for( int idx = 0; idx >= 0; idx = hierarchy[idx][0] )
    {
        cnt ++;
        int b = theRNG().uniform(0, 255);
        int g = theRNG().uniform(0, 255);
        int r = theRNG().uniform(0, 255);
        Scalar color = Scalar((uchar)b, (uchar)g, (uchar)r);

        const vector<Point>& c = contours[idx];
        double area = fabs(contourArea(Mat(c)));
        if( area > maxArea )
        {
            maxArea = area;
            largestComp = idx;
        }
        drawContours( dst, contours, idx, color, FILLED, LINE_8, hierarchy );
        imshow("contours", dst);
        char key = waitKey();
        if( key == 27 ) break;
    }
    vlog("%d contours been plot.\nThe largest one shown in green.\n");
    Scalar color( 0, 255, 0 );
    drawContours( dst, contours, largestComp, color, FILLED, LINE_8, hierarchy );
    imshow("contours", dst);
    waitKey();
}

void refineSegments(const Mat img, Mat mask, Mat& dst)
{
    int niters = 3;

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Mat temp;
    //Mat kernel = Mat::ones(3,3,CV_8UC1);
    //dilate(mask, mask, kernel);

    dilate(mask, temp, Mat(), Point(-1,-1), niters);
    //erode(temp, temp, Mat(), Point(-1,-1), niters);
    //dilate(temp, temp, Mat(), Point(-1,-1), niters);
    imshow("mask", temp);
    waitKey();

    findContours( temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE );

    dst = Mat::zeros(img.size(), CV_8UC3);

    if( contours.size() == 0 )
        return;

    // iterate through all the top-level contours,
    // draw each connected component with its own random color
    int idx = 0, largestComp = 0;
    double maxArea = 0;

    for( ; idx >= 0; idx = hierarchy[idx][0] )
    {
        const vector<Point>& c = contours[idx];
        double area = fabs(contourArea(Mat(c)));
        if( area > maxArea )
        {
            maxArea = area;
            largestComp = idx;
        }
    }
    Scalar color( 146, 168, 209 );
    drawContours( dst, contours, largestComp, color, FILLED, LINE_8, hierarchy );
}
