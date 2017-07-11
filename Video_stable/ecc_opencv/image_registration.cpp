/* 
 * Enhanced Correlation Coefficient (ECC) Image Alignment Algorithm
 * https://sites.google.com/site/georgeevangelidis/ecc
 *
 * Authors: Georgios Evangelidis
 * e-mail: george.evangelidis@gmail.com
 *
 * Copyright (2010): G.D.Evangelidis
 *
 * 13/5/2012, Update: ECC deals with translation, eucelidean, affine and homography transform.
 *
 * For details, see the paper:
 * G.D. Evangelidis, E.Z. Psarakis, "Parametric Image Alignment using 
 * Enhanced Correlation Coefficient Maximization, IEEE Transaction on
 * Pattern Analysis & Machine Intelligence, vol. 30, no. 10, 2008
 */

 /*
 * Acknowledgements: Many thanks to Mark Asbach (Fraunhofer IAIS)
 * for code refactoring.
 *
 */


#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <iostream>

#include <cv.h>
#include <highgui.h>

#include "ecc.h"

void draw_warped_roi(IplImage* image, const int width, const int height, CvMat* W);

using namespace std;

int main (const int argc, const char * argv[])
{
  // algorithm parameters and commandline arguments
  int          number_of_iterations = 50;
  double       termination_eps      = 0.001;
  const char * motion               = "affine";
  const char * warp_init            = NULL;
  const char * warp_to_file         = NULL;
  const char * image_to_file        = "warped.png";
  bool         verbose              = false;
  IplImage   * target_image         = NULL;
  IplImage   * template_image       = NULL;
  CvMat      * warp_matrix          = NULL;
  
  // print help message
  if (argc < 7) 
  {
	  cerr << "Usage: ecc -i image.pgm -t template.pgm -o warp.ecc [options]" << endl
         << endl
         << "  number of iteration:              -n N                                           (default:     50)" << endl
         << "  termination epsilon:              -eps X.XX                                      (default:  0.001)" << endl
         << "  verbose output flag:              -v [0|1]                                       (default:      0)" << endl
         << "  geometric transformation:         -m [translation|euclidean|affine|homograhpy]   (default: affine)" << endl
         << "  output (warped) image:            -oim <filename>" << endl
         << "  warp initialization file:         -init <filename>" << endl
         << endl
         << "  Example: ecc -i image.pgm -t template.pgm -o warp.ecc -m homography -n 25 -v 1 -init init.txt -oim warped.pgm"  << endl << flush;
    
    return 0;
  }
  
  // handle command line arguments
  for (int arg = 1; arg < argc; arg++)
  {
    if (! strcmp(argv[arg], "-i"))
    {
      target_image = cvLoadImage(argv[++arg], CV_LOAD_IMAGE_GRAYSCALE);
      if (! target_image)
      {
        cerr << "Cannot load image." << endl << flush;
        return 0;
      }
    }
    if (! strcmp(argv[arg], "-t"))
    {
      template_image = cvLoadImage(argv[++arg], CV_LOAD_IMAGE_GRAYSCALE);
      if (! template_image)
      {
        cerr << "Cannot load image." << endl << flush;
        return 0;
      }
    }
    if (! strcmp(argv[arg],"-o"))
      warp_to_file=(argv[++arg]);
    if (! strcmp(argv[arg],"-n"))
      number_of_iterations=atoi(argv[++arg]);
    if (! strcmp(argv[arg],"-eps"))
      termination_eps=atof(argv[++arg]);
    if (! strcmp(argv[arg],"-m"))
    {
      motion=(argv[++arg]);
      if ((strcmp(motion,"affine")) && (strcmp(motion,"homography")) && (strcmp(motion,"translation")) && (strcmp(motion,"euclidean")))
      {
        cerr << "Wrong transformation name." << endl << flush;
        return 0;
      }
    }
    if (! strcmp(argv[arg],"-v"))
      verbose = atoi(argv[++arg]);
    if (! strcmp(argv[arg],"-init"))
      warp_init=(argv[++arg]);
    if (! strcmp(argv[arg],"-oim"))
      image_to_file=(argv[++arg]);
  }
  if (! warp_to_file)
  {
    cerr << "Output filename (-o [filename]) is missing." << endl << flush;
    return 0;
  }
  
  // initialize the warp matrix or load it appropriately
  if (! warp_init)
  {
    warp_matrix = cvCreateMat (3, 3, CV_32FC1);
    cvSetIdentity (warp_matrix);
  }
  else
  {
    warp_matrix = (CvMat*) cvLoad (warp_init);
    if (! warp_matrix)
    {
      cerr << "Check warp initialization file" << endl << flush;
      return 0;
    }
  }
  
  if (! warp_init)
  {
    if ((target_image->width!=template_image->width) || (target_image->height!=template_image->height))
    {
      cerr << endl << "Check the size of images and the warp initialization file (if any)." << endl;
      cerr << "The execution may be interrupted and the result may be biased." << endl;
      cerr <<	"Identity transformation ideally assumes images with equal resolution." << endl << endl<< flush;
    }
  }
  
  // make string into enum
  WARP_MODE mode_temp;
  if (!strcmp(motion,"translation"))
  	  mode_temp = WARP_MODE_TRANSLATION;
  else if (!strcmp(motion,"euclidean"))
  	  mode_temp = WARP_MODE_EUCLIDEAN;
  else if (!strcmp(motion,"affine"))
	  mode_temp = WARP_MODE_AFFINE;//warp_mode = WARP_MODE_AFFINE;
  else 
	  mode_temp = WARP_MODE_HOMOGRAPHY;//warp_mode = WARP_MODE_HOMOGRAPHY;


  const WARP_MODE warp_mode = mode_temp;

  if (warp_mode != WARP_MODE_HOMOGRAPHY)
      warp_matrix->rows = 2;
  
  // start timing
  const double tic_init = (double) cvGetTickCount ();
  
  // this single call does all the work
  CvMat * result_matrix = cvFindTransform (template_image, target_image, warp_matrix, warp_mode, 
                                                    cvTermCriteria (CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 
                                                                    number_of_iterations, termination_eps));
  if (! result_matrix)
  {
    cerr << "The execution was interrupted. The correlation value is going to be minimized." << endl;
    cerr << "Please check the warp initialization or the size of images." << endl << flush;
  }
  
  // end timing
  const double toc_final  = (double) cvGetTickCount ();
  const double total_time = (toc_final-tic_init)/((double)cvGetTickFrequency()*1000.);
  if (verbose)
    cout << "Total time: " << total_time / 1000 << " sec" << endl << flush;
    
  // save the final warp matrix
  cvSave (warp_to_file, result_matrix);
  if (verbose)
    cout << "Warp matrix has been saved in the file: " << warp_to_file << endl << flush;
  
  // save the final warped image
  IplImage * warped_image = cvCreateImage (cvGetSize(template_image), template_image->depth, target_image->nChannels);
  if (warp_mode != WARP_MODE_HOMOGRAPHY)
    cvWarpAffine      (target_image, warped_image, result_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
  else
    cvWarpPerspective (target_image, warped_image, result_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
  cvSaveImage (image_to_file, warped_image);
  if (verbose)
    cout << "Warped image has been saved in the file: " << image_to_file << endl << flush;
  
  // display resulting images
  if (verbose)
  {
    cvNamedWindow ("image",    CV_WINDOW_AUTOSIZE);
    cvNamedWindow ("template", CV_WINDOW_AUTOSIZE);
    cvNamedWindow ("warped",   CV_WINDOW_AUTOSIZE);
    
    cvMoveWindow  ("template", 300, 300);
    cvMoveWindow  ("warped",   500, 500);
    
    // draw boundaries of corresponding regions
    CvMat* identity_matrix = cvCreateMat (3, 3, CV_32F);
    cvSetIdentity (identity_matrix);
    draw_warped_roi (target_image,   template_image->width-2, template_image->height-2, result_matrix);
    draw_warped_roi (template_image, template_image->width-2, template_image->height-2, identity_matrix);
    cvReleaseMat (& identity_matrix);
    
    // show images
    cout << "Press any key to exit the demo (you might need to click on one of the windows before)." << endl << flush;
    cvShowImage ("image",    target_image);
    cvWaitKey (200);
    cvShowImage ("template", template_image);
    cvWaitKey (200);
    cvShowImage ("warped",   warped_image);
    cvWaitKey(0);
    
    // remove windows
    cvDestroyWindow("warped");
    cvDestroyWindow("template");
    cvDestroyWindow("image");
  }
  
  // release memory
  cvReleaseImage (& warped_image);
  cvReleaseMat   (& warp_matrix);
  cvReleaseImage (& template_image);
  cvReleaseImage (& target_image);
  
  // done
  return 0;
}

#define HOMO_VECTOR(H, x, y)\
CV_MAT_ELEM(*(H), float, 0, 0) = (float)(x);\
CV_MAT_ELEM(*(H), float, 1, 0) = (float)(y);\
CV_MAT_ELEM(*(H), float, 2, 0) = 1.;

#define GET_HOMO_VALUES(X, x, y)\
(x) = static_cast<float> (CV_MAT_ELEM(*(X), float, 0, 0)/CV_MAT_ELEM(*(X), float, 2, 0));\
(y) = static_cast<float> (CV_MAT_ELEM(*(X), float, 1, 0)/CV_MAT_ELEM(*(X), float, 2, 0));

void draw_warped_roi(IplImage* image, const int width, const int height, CvMat* W)
{
	CvPoint top_left, top_right, bottom_left, bottom_right;
  
	CvMat * H = cvCreateMat (3, 1, CV_32FC1);
	CvMat * U = cvCreateMat (3, 1, CV_32FC1);
  
  CvMat * warp_mat = cvCreateMat (3, 3, CV_32FC1);
  cvSetIdentity (warp_mat);
  for (int y = 0; y < W->rows; y++)
    for (int x = 0; x < W->cols; x++)
        cvmSet (warp_mat, y, x, cvmGet (W, y, x));
  
	//warp the corners of rectangle
  
	// top-left
	HOMO_VECTOR(H, 1, 1);
	cvGEMM(warp_mat, H, 1, 0, 0, U);
	GET_HOMO_VALUES(U, top_left.x, top_left.y);
  
	// top-right
	HOMO_VECTOR(H, width, 1);
	cvGEMM(warp_mat, H, 1, 0, 0, U);
	GET_HOMO_VALUES(U, top_right.x, top_right.y);
  
	// bottom-left
	HOMO_VECTOR(H, 1, height);
	cvGEMM(warp_mat, H, 1, 0, 0, U);
	GET_HOMO_VALUES(U, bottom_left.x, bottom_left.y);
  
	// bottom-right
	HOMO_VECTOR(H, width, height);
	cvGEMM(warp_mat, H, 1, 0, 0, U);
	GET_HOMO_VALUES(U, bottom_right.x, bottom_right.y);
  
	// draw the warped perimeter
	cvLine(image, top_left, top_right, cvScalar(255,0,255));
	cvLine(image, top_right, bottom_right, cvScalar(255,0,255));
	cvLine(image, bottom_right, bottom_left, cvScalar(255,0,255));
	cvLine(image, bottom_left, top_left, cvScalar(255,0,255));
  

	// release resources and exit
    cvReleaseMat(&warp_mat);
	cvReleaseMat(&H);
	cvReleaseMat(&U);
}

