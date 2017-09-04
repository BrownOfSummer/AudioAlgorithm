
#include<iostream>
#include<opencv2/opencv.hpp>

#define USE_ROI 1
using namespace std;
using namespace cv;

typedef struct{
    int left;
    int right;
    int top;
    int bottom;
}overlap;
#ifndef MAX
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#endif
#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif
#define HOMO_VECTOR(H, x, y)\
    H.at<float>(0,0) = (float)(x);\
    H.at<float>(1,0) = (float)(y);\
    H.at<float>(2,0) = 1.;

#define GET_HOMO_VALUES(X, x, y)\
    (x) = static_cast<float> (X.at<float>(0,0)/X.at<float>(2,0));\
    (y) = static_cast<float> (X.at<float>(1,0)/X.at<float>(2,0));

static void draw_warped_roi(Mat& image, const int width, const int height, Mat& W, overlap& area);
static void draw_warped_roi2(Mat& image, const int width, const int height, Mat& W, overlap& area);
bool find_xpix_on_line(const int img_height, const Point2f p1_in_img, const Point2f p2_in_img, double input_y, double &output_x);
bool find_ypix_on_line(const int img_height, const Point2f p1_in_img, const Point2f p2_in_img, double input_x, double &output_y);
Mat GetGradient(Mat src_gray)
{ 
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;

	int scale = 1; 	
	int delta = 0; 
	int ddepth = CV_32FC1; ;
    
    // Calculate the x and y gradients using Sobel operator

	Sobel( src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
	convertScaleAbs( grad_x, abs_grad_x );

	Sobel( src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
	convertScaleAbs( grad_y, abs_grad_y );

    // Combine the two gradients
    Mat grad;
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );

	return grad; 

} 

Mat GetWarpMatrix(Mat im1_gray, Mat im2_gray, const int warp_mode) {

    // Define the motion model
    /*
     * MOTION_AFFINE
     * MOTION_EUCLIDEAN
     * MOTION_TRANSLATION
     * MOTION_HOMOGRAPHY 
     * */
    //const int warp_mode = MOTION_TRANSLATION;

    // Set a 2x3 or 3x3 warp matrix depending on the motion model.
    Mat warp_matrix;
    // Initialize the matrix to identity
    if ( warp_mode == MOTION_HOMOGRAPHY )
        warp_matrix = Mat::eye(3, 3, CV_32F);
    else
        warp_matrix = Mat::eye(2, 3, CV_32F);

    // Specify the number of iterations.
    int number_of_iterations = 100;
    
    // Specify the threshold of the increment
    // in the correlation coefficient between two iterations
    double termination_eps = 1e-10;
    
    // Define termination criteria
    TermCriteria criteria (TermCriteria::COUNT+TermCriteria::EPS, number_of_iterations, termination_eps);

    // Run the ECC algorithm. The results are stored in warp_matrix.
    findTransformECC(
                     GetGradient(im1_gray),
                     GetGradient(im2_gray),
                     warp_matrix,
                     warp_mode,
                     criteria
                 );
    return warp_matrix;
}

int main(int argc, char *argv[])
{
    if( argc != 2 ) {
        cout<<"Usage: "<<argv[0]<<" image.list"<<endl;
        return -1;
    }
    int roi_height = 250;
    int roi_width = 250;
    int roi_sx = 530;
    int roi_sy = 150;
    int count = 0;
    int img_height, img_width;
    Mat im1, im2, roiImage, roiGray;
    vector<Mat> rois;
    
    string ImgNamesLists[1000];
    char str[200];
    FILE *fImgList = fopen(argv[1], "r");
    if(NULL == fImgList) {
        cout<<"Read file error!"<<endl;
        cout<<argv[1]<<" is not valid !"<<endl;
        return -1;
    }

    while (fgets(str, 200, fImgList) != NULL && count < 1000) {
        memset(str + strlen(str)-1, 0, 1);
        ImgNamesLists[count] = str;
        count ++;
    }
    bool is_first_read = true;
    for(int i = 0; i < count; i ++) {
        im1 = imread(ImgNamesLists[i], 1);
        if( is_first_read ){
            img_height = im1.rows;
            img_width = im1.cols;
            is_first_read = false;
        }
        else {
            if( img_height != im1.rows || img_width != im1.cols ) {
                cout<<"ERROR, all image should be same size !"<<endl;
                return -1;
            }
        }
        if( USE_ROI ) {
            Rect rect(roi_sx, roi_sy, roi_width, roi_height);
            im1(rect).copyTo(roiImage);
            cvtColor(roiImage, roiGray, CV_BGR2GRAY);
            rois.push_back(roiGray.clone());
        }
        else {
            cvtColor(im1, roiGray, CV_BGR2GRAY);
            rois.push_back(roiGray.clone());
        }
        //imshow("rois", roiGray);
        //imshow("roi-Gradient", GetGradient(roiGray));
        //waitKey();
    }

    int template_index = (rois.size() + 1) / 2 - 1;
    cout<<"Select "<<ImgNamesLists[template_index]<<" as template !"<<endl;

    const int warp_mode = MOTION_EUCLIDEAN;
    Mat warp_matrix;
    // to record the max overlapping area
    //int right = 1280, left = 0, bottom = 720, top = 0;
    int right = img_width, left = 0, bottom = img_height, top = 0;
    // move pixels to align template image
    vector<int> xmove; // + for leftmove, - for rightmove
    vector<int> ymove; // + for upmove, - for downmove
    vector<Mat> warped_img;
    overlap area;
    for(int i = 0; i < rois.size(); i ++) {
        if( i == template_index ) {
            xmove.push_back(0);
            ymove.push_back(0);
            im1 = imread(ImgNamesLists[i], 1);
            warped_img.push_back(im1.clone());

            right = right < im1.cols ? right : im1.cols;
            left = left > 0 ? left : 0;
            top = top > 0 ? top : 0;
            bottom = bottom < im1.rows ? bottom : im1.rows;
            continue;
        }
        //Mat warp_matrix = GetWarpMatrix(rois[template_index], rois[i], warp_mode);
        warp_matrix = GetWarpMatrix(rois[i], rois[template_index], warp_mode);
        double warp_angle = atan2(warp_matrix.at<double>(1, 0), warp_matrix.at<double>(0, 0));
        double angle = warp_angle < 0 ? -warp_angle : warp_angle;
        im1 = imread(ImgNamesLists[i], 1);
        if(angle * 180 / 3.1415927 < 1) {
            warp_matrix = GetWarpMatrix(rois[i], rois[template_index], MOTION_TRANSLATION);
            float xx = warp_matrix.at<float>(0,2);
            float yy = warp_matrix.at<float>(1,2);
            // Get integer move steps
            int xstep = xx > 0 ? (int)(xx + 0.5) : -(int)(-xx + 0.5);
            int ystep = yy > 0 ? (int)(yy + 0.5) : -(int)(-yy + 0.5);
            xmove.push_back(xstep);
            ymove.push_back(ystep);
            // Do align , align to template
            warpAffine(im1, im2, warp_matrix, im1.size());
            //draw_warped_roi(im2, im2.cols, im2.rows, warp_matrix);
            warped_img.push_back(im2.clone());

            // Get max overlapping area
            int endx = xstep > 0 ? img_width : img_width + xstep;
            int endy = ystep > 0 ? img_height : img_height + ystep;
            int sx = xstep > 0 ? xstep : 0;
            int sy = ystep > 0 ? ystep : 0;

            // max overlapping area in template record in left right bottom top
            right = endx < right ? endx : right;
            left = sx > left ? sx : left;
            bottom = endy < bottom ? endy : bottom;
            top = sy > top ? sy : top;
        }
        else if( warp_matrix.at<double>(0, 0) * warp_matrix.at<double>(1, 0) > 0 ) {
            warpAffine(im1, im2, warp_matrix, im1.size());
            draw_warped_roi2(im2,  im1.cols, im1.rows, warp_matrix, area);
            warped_img.push_back(im2.clone());
        }
        else if( warp_matrix.at<double>(0, 0) * warp_matrix.at<double>(1, 0) < 0 ) {
            warpAffine(im1, im2, warp_matrix, im1.size());
            draw_warped_roi (im2,  im1.cols, im1.rows, warp_matrix, area);
            warped_img.push_back(im2.clone());
        }
        else {
            cout<<"ERROR, warp angle error !"<<endl;
            return -1;
        }
        right = MIN(right, area.right);
        left = MAX(left, area.left);
        top = MAX(top, area.top);
        bottom = MIN(bottom, area.bottom);
    }
    int area_width = right - left;
    int area_height = bottom - top;
    cout<<"Overlaping area in template: left-up( "<<left<<", "<<top<<" ); right-bottom:( "<<right<<", "<<bottom<<" )"<<endl;
    cout<<"Area height: "<<area_height<<"; Area width: "<<area_width<<endl;

    // Align and resize
    for(int j = 0; j < 20; j ++) {
        // for show
        for(int i = 0; i < rois.size(); i ++) {
            int startx = left;
            int endx = startx + area_width;
            int starty = top;
            int endy = top + area_height;
            //int startx = 0;
            //int endx = img_width;
            //int starty = 0;
            //int endy = img_height;

            //im1 = imread(ImgNamesLists[i], 1);
            im1 = warped_img[i];
            Rect rect(startx, starty, area_width, area_height);
            im1(rect).copyTo(roiImage);
            resize(roiImage, im1, Size(640, 360));

            // ori image
            im2 = imread(ImgNamesLists[i], 1);
            resize(im2, im2, Size(640, 360));
            Mat rowshow, colshow;
            hconcat(im2, im1, rowshow);
            //vconcat(im2, im1, colshow);
            imshow("aligned", rowshow);
            waitKey(100);
        }
        waitKey(500);
    }

    return 0;
}

static void draw_warped_roi(Mat& image, const int width, const int height, Mat& W, overlap &area)
{
    Point2f top_left, top_right, bottom_left, bottom_right;

    Mat  H = Mat (3, 1, CV_32F);
    Mat  U = Mat (3, 1, CV_32F);

    Mat warp_mat = Mat::eye (3, 3, CV_32F);

    for (int y = 0; y < W.rows; y++)
        for (int x = 0; x < W.cols; x++)
            warp_mat.at<float>(y,x) = W.at<float>(y,x);

    //warp the corners of rectangle

    // top-left
    HOMO_VECTOR(H, 1, 1);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, top_left.x, top_left.y);

    // top-right
    HOMO_VECTOR(H, width, 1);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, top_right.x, top_right.y);

    // bottom-left
    HOMO_VECTOR(H, 1, height);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, bottom_left.x, bottom_left.y);

    // bottom-right
    HOMO_VECTOR(H, width, height);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, bottom_right.x, bottom_right.y);

    // draw the warped perimeter
    line(image, top_left, top_right, Scalar(255,0,255));
    line(image, top_right, bottom_right, Scalar(255,0,255));
    line(image, bottom_right, bottom_left, Scalar(255,0,255));
    line(image, bottom_left, top_left, Scalar(255,0,255));

    cout<<"top_left(x,y)     :( "<<top_left.x<<", "<<top_left.y<<" )"<<endl;
    cout<<"top_right(x,y)    :( "<<top_right.x<<", "<<top_right.y<<" )"<<endl;
    cout<<"bottom_left(x,y)  :( "<<bottom_left.x<<", "<<bottom_left.y<<" )"<<endl;
    cout<<"bottom_right(x,y) :( "<<bottom_right.x<<", "<<bottom_right.y<<" )"<<endl;
    cout<<endl;

    cout<<"top_left(x,y)     :( "<<top_left.x<<", "<<height - top_left.y<<" )"<<endl;
    cout<<"top_right(x,y)    :( "<<top_right.x<<", "<<height - top_right.y<<" )"<<endl;
    cout<<"bottom_left(x,y)  :( "<<bottom_left.x<<", "<<height - bottom_left.y<<" )"<<endl;
    cout<<"bottom_right(x,y) :( "<<bottom_right.x<<", "<<height - bottom_right.y<<" )"<<endl;

    double tmpx, tmpy;
    Point2f center;
    bool flag;
    int left = width, right = 0, top = height, bottom = 0;
    // points on col = 0
    flag = find_ypix_on_line(height, top_left, bottom_left, 0, tmpy);
    if( flag ) {
        center.x = 0;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255, 255, 255));
        cout<<"center on col=0:( "<<center.x<<", "<<center.y<<" )"<<endl;

        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    flag = find_ypix_on_line(height, bottom_left, bottom_right, 0, tmpy);
    if( flag ) {
        center.x = 0;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on col=0:( "<<center.x<<", "<<center.y<<" )"<<endl;
        
        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    // points on col = width
    flag = find_ypix_on_line(height, top_left, top_right, width, tmpy);
    if( flag ) {
        center.x = width;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on col=width:( "<<center.x<<", "<<center.y<<" )"<<endl;
        
        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    flag = find_ypix_on_line(height, bottom_right, top_right, width, tmpy);
    if( flag ) {
        center.x = width;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on col=width:( "<<center.x<<", "<<center.y<<" )"<<endl;

        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    
    //point on row = 0
    flag = find_xpix_on_line(height, top_left, bottom_left, 0, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = 0;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=0:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }
    flag = find_xpix_on_line(height, top_left, top_right, 0, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = 0;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=0:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }

    //point on row = height
    flag = find_xpix_on_line(height, bottom_left, bottom_right, height, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = height;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=height:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }
    flag = find_xpix_on_line(height, bottom_right, top_right, height, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = height;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=height:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }

    cout<<"( left, right, top, bottom ): ( "<<left<<", "<<right<<", "<<top<<", "<<bottom<<" )"<<endl;
    left = MAX(left, 0);
    right = MIN(right, width);
    top = MAX(top, 0);
    bottom = MIN(bottom, height);
    cout<<"( left, right, top, bottom ): ( "<<left<<", "<<right<<", "<<top<<", "<<bottom<<" )"<<endl;
    circle(image, Point2f(left, top), 10, Scalar(255,255,255));
    circle(image, Point2f(left, bottom), 10, Scalar(255,255,255));
    circle(image, Point2f(right, top), 10, Scalar(255,255,255));
    circle(image, Point2f(right, bottom), 10, Scalar(255,255,255));
    line(image, Point2f(left, top), Point2f(left, bottom), Scalar(0,255,0));
    line(image, Point2f(right, top), Point2f(right, bottom), Scalar(0,255,0));
    line(image, Point2f(left, top), Point2f(right, top), Scalar(0,255,0));
    line(image, Point2f(left, bottom), Point2f(right, bottom), Scalar(0,255,0));
    // test find_ypix_on_line and find_xpix_on_line
    //line(image, Point2f(10, 10), Point2f(200, 100), Scalar(255,0,255));
    area.left = left;
    area.right = right;
    area.top = top;
    area.bottom = bottom;
}

bool find_ypix_on_line(const int img_height, const Point2f p1_in_img, const Point2f p2_in_img, double input_x, double &output_y)
{
    // convert image coordinate to Cartesian coordinate
    double y1 = img_height - p1_in_img.y;
    double x1 = p1_in_img.x;
    double y2 = img_height - p2_in_img.y;
    double x2 = p2_in_img.x;
    double y = 0;
    // deal with vertical line
    if( x1 == x2 ) {
        cout<<"Warning, vertical line: x = "<<x1<<endl;
        return false;
    }
    else {
        y = y1 + ( (y2-y1) / (x2-x1) ) * (input_x - x1); 
        // covert to image coordinate
        y = img_height - y;
        output_y = y;
        return true;
    }
}
bool find_xpix_on_line(const int img_height, const Point2f p1_in_img, const Point2f p2_in_img, double input_y, double &output_x)
{
    // convert image coordinate to Cartesian coordinate
    double y1 = img_height - p1_in_img.y;
    double x1 = p1_in_img.x;
    double y2 = img_height - p2_in_img.y;
    double x2 = p2_in_img.x;
    double x = 0;
    input_y = img_height - input_y;
    // deal with horizontal line
    if( y1 == y2 ) {
        cout<<"Warning, horizontal line: y = "<<y1<<endl;
        return false;
    }
    else {
        //cout<<"x1, y1: "<<x1<<", "<<y1<<endl;
        //cout<<"x2, y2: "<<x2<<", "<<y2<<endl;
        x = x1 + ( (x2 -x1) / (y2 - y1) ) * (input_y - y1);
        output_x = x;
        return true;
    }
}
static void draw_warped_roi2(Mat& image, const int width, const int height, Mat& W, overlap &area)
{
    Point2f top_left, top_right, bottom_left, bottom_right;

    Mat  H = Mat (3, 1, CV_32F);
    Mat  U = Mat (3, 1, CV_32F);

    Mat warp_mat = Mat::eye (3, 3, CV_32F);

    for (int y = 0; y < W.rows; y++)
        for (int x = 0; x < W.cols; x++)
            warp_mat.at<float>(y,x) = W.at<float>(y,x);

    //warp the corners of rectangle

    // top-left
    HOMO_VECTOR(H, 1, 1);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, top_left.x, top_left.y);

    // top-right
    HOMO_VECTOR(H, width, 1);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, top_right.x, top_right.y);

    // bottom-left
    HOMO_VECTOR(H, 1, height);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, bottom_left.x, bottom_left.y);

    // bottom-right
    HOMO_VECTOR(H, width, height);
    gemm(warp_mat, H, 1, 0, 0, U);
    GET_HOMO_VALUES(U, bottom_right.x, bottom_right.y);

    // draw the warped perimeter
    line(image, top_left, top_right, Scalar(255,0,255));
    line(image, top_right, bottom_right, Scalar(255,0,255));
    line(image, bottom_right, bottom_left, Scalar(255,0,255));
    line(image, bottom_left, top_left, Scalar(255,0,255));

    cout<<"top_left(x,y)     :( "<<top_left.x<<", "<<top_left.y<<" )"<<endl;
    cout<<"top_right(x,y)    :( "<<top_right.x<<", "<<top_right.y<<" )"<<endl;
    cout<<"bottom_left(x,y)  :( "<<bottom_left.x<<", "<<bottom_left.y<<" )"<<endl;
    cout<<"bottom_right(x,y) :( "<<bottom_right.x<<", "<<bottom_right.y<<" )"<<endl;
    cout<<endl;

    cout<<"top_left(x,y)     :( "<<top_left.x<<", "<<height - top_left.y<<" )"<<endl;
    cout<<"top_right(x,y)    :( "<<top_right.x<<", "<<height - top_right.y<<" )"<<endl;
    cout<<"bottom_left(x,y)  :( "<<bottom_left.x<<", "<<height - bottom_left.y<<" )"<<endl;
    cout<<"bottom_right(x,y) :( "<<bottom_right.x<<", "<<height - bottom_right.y<<" )"<<endl;

    double tmpx, tmpy;
    Point2f center;
    bool flag;
    int left = width, right = 0, top = height, bottom = 0;
    // points on col = 0
    flag = find_ypix_on_line(height, top_left, bottom_left, 0, tmpy);
    if( flag ) {
        center.x = 0;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255, 255, 255));
        cout<<"center on col=0:( "<<center.x<<", "<<center.y<<" )"<<endl;

        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    flag = find_ypix_on_line(height, top_left, top_right, 0, tmpy);
    if( flag ) {
        center.x = 0;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on col=0:( "<<center.x<<", "<<center.y<<" )"<<endl;
        
        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    // points on col = width
    flag = find_ypix_on_line(height, top_right, bottom_right, width, tmpy);
    if( flag ) {
        center.x = width;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on col=width:( "<<center.x<<", "<<center.y<<" )"<<endl;
        
        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    flag = find_ypix_on_line(height, bottom_left, bottom_right, width, tmpy);
    if( flag ) {
        center.x = width;
        center.y = tmpy;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on col=width:( "<<center.x<<", "<<center.y<<" )"<<endl;

        top = tmpy < top ? tmpy : top;
        bottom = tmpy > bottom ? tmpy : bottom;
    }
    
    //point on row = 0
    flag = find_xpix_on_line(height, top_left, top_right, 0, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = 0;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=0:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }
    flag = find_xpix_on_line(height, bottom_right, top_right, 0, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = 0;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=0:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }

    //point on row = height
    flag = find_xpix_on_line(height, bottom_left, bottom_right, height, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = height;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=height:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }
    flag = find_xpix_on_line(height, bottom_left, top_left, height, tmpx);
    if( flag ) {
        center.x = tmpx;
        center.y = height;
        circle(image, center, 5, Scalar(255,255,255));
        cout<<"center on row=height:( "<<center.x<<", "<<center.y<<" )"<<endl;

        left = tmpx < left ? tmpx : left;
        right = tmpx > right ? tmpx : right;
    }

    cout<<"( left, right, top, bottom ): ( "<<left<<", "<<right<<", "<<top<<", "<<bottom<<" )"<<endl;
    left = MAX(left, 0);
    right = MIN(right, width);
    top = MAX(top, 0);
    bottom = MIN(bottom, height);
    cout<<"( left, right, top, bottom ): ( "<<left<<", "<<right<<", "<<top<<", "<<bottom<<" )"<<endl;
    circle(image, Point2f(left, top), 10, Scalar(255,255,255));
    circle(image, Point2f(left, bottom), 10, Scalar(255,255,255));
    circle(image, Point2f(right, top), 10, Scalar(255,255,255));
    circle(image, Point2f(right, bottom), 10, Scalar(255,255,255));
    line(image, Point2f(left, top), Point2f(left, bottom), Scalar(0,255,0));
    line(image, Point2f(right, top), Point2f(right, bottom), Scalar(0,255,0));
    line(image, Point2f(left, top), Point2f(right, top), Scalar(0,255,0));
    line(image, Point2f(left, bottom), Point2f(right, bottom), Scalar(0,255,0));
    // test find_ypix_on_line and find_xpix_on_line
    //line(image, Point2f(10, 10), Point2f(200, 100), Scalar(255,0,255));
    area.left = left;
    area.right = right;
    area.top = top;
    area.bottom = bottom;
}
