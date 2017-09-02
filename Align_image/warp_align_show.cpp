
#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

static void draw_warped_roi(Mat& image, const int width, const int height, Mat& W);
static void draw_warped_roi2(Mat& image, const int width, const int height, Mat& W);
bool find_xpix_on_line(const int img_height, const Point2f p1_in_img, const Point2f p2_in_img, double input_y, double &output_x);
bool find_ypix_on_line(const int img_height, const Point2f p1_in_img, const Point2f p2_in_img, double input_x, double &output_y);

#define HOMO_VECTOR(H, x, y)\
    H.at<float>(0,0) = (float)(x);\
    H.at<float>(1,0) = (float)(y);\
    H.at<float>(2,0) = 1.;

#define GET_HOMO_VALUES(X, x, y)\
    (x) = static_cast<float> (X.at<float>(0,0)/X.at<float>(2,0));\
    (y) = static_cast<float> (X.at<float>(1,0)/X.at<float>(2,0));
Mat GetWarpMatrix(Mat im1_gray, Mat im2_gray, const int warp_mode);
Mat GetGradient(Mat src_gray);
int main(int argc, char *argv[])
{
    Mat src = imread(argv[1], 1);
    Mat rotation_matrix = getRotationMatrix2D(Point2f(src.cols/2, src.rows/2), 45, 1);
    double angle = atan2(rotation_matrix.at<double>(1,0), rotation_matrix.at<double>(0,0));
    cout<<"angle = "<<angle * 180 / 3.1415926 <<endl;
    ///define shift_matrix
    Mat shift_matrix = Mat(2, 3, CV_32FC1);
    shift_matrix.at<float>(0, 0) = 1;
    shift_matrix.at<float>(0, 1) = 0;
    shift_matrix.at<float>(0, 2) = 10;
    shift_matrix.at<float>(1, 0) = 0;
    shift_matrix.at<float>(1, 1) = 1;
    shift_matrix.at<float>(1, 2) = 10;


    cout<<"rotation_matrix = "<<endl<<rotation_matrix<<endl;
    cout<<"shift_matrix = "<<endl<<shift_matrix<<endl;

    Mat rotation_dst, shift_dst, rotation_shift_dst, rotation_shift_dst2;
    warpAffine(src, rotation_dst, rotation_matrix, src.size());
    warpAffine(src, shift_dst, shift_matrix, src.size());
    warpAffine(rotation_dst, rotation_shift_dst, shift_matrix, src.size());

    const int warp_mode = MOTION_EUCLIDEAN;
    Mat src_gray, tmp_gray;
    cvtColor(src, src_gray, CV_BGR2GRAY);
    //cvtColor(rotation_shift_dst, tmp_gray, CV_BGR2GRAY);
    cvtColor(rotation_dst, tmp_gray, CV_BGR2GRAY);
    //cvtColor(shift_dst, tmp_gray, CV_BGR2GRAY);
    //Mat warp_matrix = GetWarpMatrix(src_gray, tmp_gray, warp_mode);
    Mat warp_matrix = GetWarpMatrix(tmp_gray, src_gray, warp_mode);
    cout<<"warp_matrix = "<<endl<<warp_matrix<<endl;
    Mat warp_dst;
    warpAffine(tmp_gray, warp_dst, warp_matrix, src.size());

    if(warp_matrix.at<float>(0,0) * warp_matrix.at<float>(1,0) > 0)
        draw_warped_roi (warp_dst,  src.cols, src.rows, warp_matrix);
    else if(warp_matrix.at<float>(0,0) * warp_matrix.at<float>(1,0) < 0)
        draw_warped_roi2 (warp_dst,  src.cols, src.rows, warp_matrix);
    else
        cout<<"Just TRANSLATION !"<<endl;
    //Mat identity_matrix = Mat::eye(3,3,CV_32F);
    //draw_warped_roi (template_image, template_image.cols-2, template_image.rows-2, identity_matrix);

    imshow("src", src);
    imshow("rotation_dst", rotation_dst);
    imshow("shift_dst", shift_dst);
    imshow("rotation_shift_dst", rotation_shift_dst);
    imshow("warp_dst", warp_dst);
    waitKey();
    return 0;
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

static void draw_warped_roi(Mat& image, const int width, const int height, Mat& W)
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
    circle(image, Point2f(left, top), 10, Scalar(255,255,255));
    circle(image, Point2f(left, bottom), 10, Scalar(255,255,255));
    circle(image, Point2f(right, top), 10, Scalar(255,255,255));
    circle(image, Point2f(right, bottom), 10, Scalar(255,255,255));
    line(image, Point2f(left, top), Point2f(left, bottom), Scalar(255,255,255));
    line(image, Point2f(right, top), Point2f(right, bottom), Scalar(255,255,255));
    line(image, Point2f(left, top), Point2f(right, top), Scalar(255,255,255));
    line(image, Point2f(left, bottom), Point2f(right, bottom), Scalar(255,255,255));
    // test find_ypix_on_line and find_xpix_on_line
    //line(image, Point2f(10, 10), Point2f(200, 100), Scalar(255,0,255));
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
static void draw_warped_roi2(Mat& image, const int width, const int height, Mat& W)
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
    circle(image, Point2f(left, top), 10, Scalar(255,255,255));
    circle(image, Point2f(left, bottom), 10, Scalar(255,255,255));
    circle(image, Point2f(right, top), 10, Scalar(255,255,255));
    circle(image, Point2f(right, bottom), 10, Scalar(255,255,255));
    line(image, Point2f(left, top), Point2f(left, bottom), Scalar(255,255,255));
    line(image, Point2f(right, top), Point2f(right, bottom), Scalar(255,255,255));
    line(image, Point2f(left, top), Point2f(right, top), Scalar(255,255,255));
    line(image, Point2f(left, bottom), Point2f(right, bottom), Scalar(255,255,255));
    // test find_ypix_on_line and find_xpix_on_line
    //line(image, Point2f(10, 10), Point2f(200, 100), Scalar(255,0,255));
}
