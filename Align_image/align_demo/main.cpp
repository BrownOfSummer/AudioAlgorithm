// Module: main.cpp
// Brief:  
// Author: Oleg A. Krivtsov
// Email:  olegkrivtsov@mail.ru
// Date:  March 2008

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "auxfunc.h"
#include "forwadditive.h"
#include "invcomp.h"

// Our plan:
// 
// 1. Ask user to enter image warp parameter vector p=(wz, tx, ty, s)
// 2. Define our template rectangle to be bounding rectangle of the butterfly
// 3. Warp image I with warp matrix W(p)
// 4. Show template T and image T, wait for a key press
// 5. Estimate warp parameters using Lucas-Kanade method, wait for a key press
// 6. Estimate warp parameters using Baker-Dellaert-Matthews, wait for a key press
//

int main(int argc, char* argv[])
{
	// Ask user to enter image warp paremeters vector.
	// p = (wz, tx, ty),
	
	float WZ=0, TX=0, TY=0;
	printf("Please enter WZ, TX and TY, separated by space.\n");
	printf("Example: -0.01 5 -3\n");
	printf(">");
	scanf("%f %f %f", &WZ, &TX, &TY);

	// Here we will store our images.
	IplImage* pColorPhoto = 0; // Photo of a butterfly on a flower.
	IplImage* pGrayPhoto = 0;  // Grayscale copy of the photo.
	IplImage* pImgT = 0;	   // Template T.
	IplImage* pImgI = 0;	   // Image I.

	// Here we will store our warp matrix.
	CvMat* W = 0;  // Warp W(p)
	
	// Create two simple windows. 
	cvNamedWindow("template"); // Here we will display T(x).
	cvNamedWindow("image"); // Here we will display I(x).

	// Load photo.
	//pColorPhoto = cvLoadImage("..\\data\\photo.jpg");
	pColorPhoto = cvLoadImage(argv[1]);

	// Create other images.
	CvSize photo_size = cvSize(pColorPhoto->width,pColorPhoto->height);
	pGrayPhoto = cvCreateImage(photo_size, IPL_DEPTH_8U, 1);
	pImgT = cvCreateImage(photo_size, IPL_DEPTH_8U, 1);
	pImgI = cvCreateImage(photo_size, IPL_DEPTH_8U, 1);

	// Convert photo to grayscale, because we need intensity values instead of RGB.	
	cvCvtColor(pColorPhoto, pGrayPhoto, CV_RGB2GRAY);
	
	// Create warp matrix, we will use it to create image I.
	W = cvCreateMat(3, 3, CV_32F);

	// Create template T
	// Set region of interest to butterfly's bounding rectangle.
	cvCopy(pGrayPhoto, pImgT);
	CvRect omega = cvRect(110, 100, 200, 150);
	
	// Create I by warping photo with sub-pixel accuracy. 
	init_warp(W, WZ, TX, TY);
	warp_image(pGrayPhoto, pImgI, W);
	
	// Draw the initial template position approximation rectangle.
	cvSetIdentity(W);
	draw_warped_rect(pImgI, omega, W);

	// Show T and I and wait for key press.
	cvSetImageROI(pImgT, omega);
	cvShowImage("template", pImgT);
	cvShowImage("image", pImgI);
	printf("Press any key to start Lucas-Kanade algorithm.\n");
	cvWaitKey(0);
	cvResetImageROI(pImgT);

	// Print what parameters were entered by user.
	printf("==========================================================\n");
	printf("Ground truth:  WZ=%f TX=%f TY=%f\n", WZ, TX, TY);
	printf("==========================================================\n");

	init_warp(W, WZ, TX, TY);

	// Restore image I
	warp_image(pGrayPhoto, pImgI, W);

	/* Lucas-Kanade */	
	align_image_forwards_additive(pImgT, omega, pImgI);

	// Restore image I
	warp_image(pGrayPhoto, pImgI, W);

	printf("Press any key to start Baker-Dellaert-Matthews algorithm.\n");
	cvWaitKey();

	/* Baker-Dellaert-Matthews*/
	align_image_inverse_compositional(pImgT, omega, pImgI);
	
	printf("Press any key to exit the demo.\n");
	cvWaitKey();

	// Release all used resources and exit
	cvDestroyWindow("template");
	cvDestroyWindow("image");
	cvReleaseImage(&pImgT);
	cvReleaseImage(&pImgI);
	cvReleaseMat(&W);
	
	return 0;
}


