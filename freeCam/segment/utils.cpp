/*************************************************************************
    > File Name: utils.cpp
    > Author: li_pengju
    > Mail: li_pengju@vobile.cn
    > Copyright (c) Vobile Inc. All Rights Reserved
    > Created Time: 2018-05-26 15:05:20
 ************************************************************************/

#include<iostream>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

#ifndef DEBUG
#define DEBUG 1
#endif

void vlog(const char *format,...) {
#if DEBUG
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

/*
 * Morphological operation: Erosion
 * Paras: src mat
 *        erosion_elem: type of enrosion, 1 0f 3;
 *        erosion_size*/
Mat Erosion( Mat src, int erosion_elem, int erosion_size )
{
  int erosion_type;
  Mat erosion_dst;
  if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
  else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
  else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }

  Mat element = getStructuringElement( erosion_type,
                                       Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                       Point( erosion_size, erosion_size ) );

  /// Apply the erosion operation
  erode( src, erosion_dst, element );
  //imshow( "Erosion Demo", erosion_dst );
  return erosion_dst;
}

/*
 * Morphological operation: Dilation
 * Paras: src mat
 *        dilation_elem: type of dilation, 1 of 3
 *        dilation_size
 */
Mat Dilation( Mat src, int dilation_elem, int dilation_size)
{
  int dilation_type;
  Mat dilation_dst;
  if( dilation_elem == 0 ){ dilation_type = MORPH_RECT; }
  else if( dilation_elem == 1 ){ dilation_type = MORPH_CROSS; }
  else if( dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }

  Mat element = getStructuringElement( dilation_type,
                                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                       Point( dilation_size, dilation_size ) );
  /// Apply the dilation operation
  dilate( src, dilation_dst, element );
  //imshow( "Dilation Demo", dilation_dst );
  return dilation_dst;
}

Mat getGradient(Mat src)
{
    Mat src_gray;
    if(src.channels() != 1) cvtColor(src, src_gray, CV_BGR2GRAY);
    else src.copyTo(src_gray);

    // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
    GaussianBlur(src_gray, src_gray, Size(3, 3), 0, 0, BORDER_DEFAULT);
	Mat grad_x, grad_y, grad;
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
	addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
	return grad; 
}

void detectPeopleArea(Mat img)
{
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    vector<Rect> found, found_filtered;
    double t = (double) getTickCount();
    // Run the detector with default parameters. to get a higher hit-rate
    // (and more false alarms, respectively), decrease the hitThreshold and
    // groupThreshold (set groupThreshold to 0 to turn off the grouping completely).
    hog.detectMultiScale(img, found, 0, Size(8,8), Size(32,32), 1.05, 2);
    t = (double) getTickCount() - t;
    cout << "detection time = " << (t*1000./cv::getTickFrequency()) << " ms" << endl;

    for(size_t i = 0; i < found.size(); i++ )
    {
        Rect r = found[i];

        size_t j;
        // Do not add small detections inside a bigger detection.
        for ( j = 0; j < found.size(); j++ )
            if ( j != i && (r & found[j]) == r )
                break;

        if ( j == found.size() )
            found_filtered.push_back(r);
    }

    for (size_t i = 0; i < found_filtered.size(); i++)
    {
        Rect r = found_filtered[i];

        // The HOG detector returns slightly larger rectangles than the real objects,
        // so we slightly shrink the rectangles to get a nicer output.
        r.x += cvRound(r.width*0.1);
        r.width = cvRound(r.width*0.8);
        r.y += cvRound(r.height*0.07);
        r.height = cvRound(r.height*0.8);
        rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
    }
    imshow("people", img);
    waitKey();
    return;
}
