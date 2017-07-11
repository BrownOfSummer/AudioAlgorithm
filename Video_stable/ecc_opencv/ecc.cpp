/* 
 * Enhanced Correlation Coefficient (ECC) Image Alignment Algorithm 
 * https://sites.google.com/site/georgeevangelidis/ecc
 *
 * Authors: Georgios Evangelidis
 * e-mail: evagelid@ceid.upatras.gr
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
 *
 */
 /*
 * Acknowledgements: Many thanks to Mark Asbach (Fraunhofer IAIS)
 * for code refactoring!
 *
 */


#include <cassert>
#include <cv.h>
//#include <math.h>
#include "ecc.h"

static void calculate_jacobian_translation(const CvMat * src1, const CvMat * src2, CvMat * jacobian);
static void calculate_jacobian_euclidean  (const CvMat * src1, const CvMat * src2, CvMat * jacobian, const CvMat * map_matrix);
static void calculate_jacobian_affine     (const CvMat * src1, const CvMat * src2, CvMat * jacobian);
static void calculate_jacobian_homography (const CvMat * src1, const CvMat * src2, CvMat * jacobian, const CvMat * map_matrix);
static void update_warping_matrix (CvMat * map_matrix, const CvMat * update, const WARP_MODE warp_mode);
//static void update_warping_matrix (CvMat * map_matrix, const CvMat * update);
using namespace std;


static void calculate_jacobian_affine (const CvMat * src1, const CvMat * src2, CvMat * jacobian)
{
  assert (src1);
  assert (src2);
  assert (src1->width  == src2->width);
  assert (src1->height == src2->height);
  assert (jacobian);
  assert (jacobian->cols == 6);
  assert (jacobian->rows == src1->width * src1->height);
  
  for (int y = 0; y < src1->height; y++)
  {
    for (int x = 0; x < src1->width; x++)
    {
      // access source and destination values
      const float   src1_pix =   CV_MAT_ELEM (*src1,     float, y, x);
      const float   src2_pix =   CV_MAT_ELEM (*src2,     float, y, x);
            float * line     = & CV_MAT_ELEM (*jacobian, float, y*src1->width+x, 0);
      
      // perform dot product directly
      line[0] = x * src1_pix;
      line[1] =                x * src2_pix;
      line[2] = y * src1_pix;
      line[3] =                y * src2_pix;
      line[4] =     src1_pix;
      line[5] =                    src2_pix;
		}
	}
}


static void calculate_jacobian_translation (const CvMat * src1, const CvMat * src2, CvMat * jacobian)
{
  assert (src1);
  assert (src2);
  assert (src1->width  == src2->width);
  assert (src1->height == src2->height);
  assert (jacobian);
  assert (jacobian->cols == 2);
  assert (jacobian->rows == src1->width * src1->height);
  
  for (int y = 0; y < src1->height; y++)
  {
    for (int x = 0; x < src1->width; x++)
    {
      // access source and destination values
      const float   src1_pix =   CV_MAT_ELEM (*src1,     float, y, x);
      const float   src2_pix =   CV_MAT_ELEM (*src2,     float, y, x);
            float * line     = & CV_MAT_ELEM (*jacobian, float, y*src1->width+x, 0);
      
      line[0] = src1_pix;
      line[1] = src2_pix;
      
		}
	}
}


static void calculate_jacobian_homography (const CvMat * src1, const CvMat * src2, CvMat * jacobian, const CvMat * map_matrix)
{
  assert (src1);
  assert (src2);
  assert (src1->width  == src2->width);
  assert (src1->height == src2->height);
  assert (jacobian);
  assert (jacobian->cols == 8);
  assert (jacobian->rows == src1->width * src1->height);
  assert (map_matrix->cols == 3);
  assert (map_matrix->rows == 3);
  
  // direct access to map matrix entries
  const float w00 = CV_MAT_ELEM(*map_matrix,float,0,0);
  const float w01 = CV_MAT_ELEM(*map_matrix,float,0,1);
  const float w02 = CV_MAT_ELEM(*map_matrix,float,0,2);
  const float w10 = CV_MAT_ELEM(*map_matrix,float,1,0);
  const float w11 = CV_MAT_ELEM(*map_matrix,float,1,1);
  const float w12 = CV_MAT_ELEM(*map_matrix,float,1,2);
  const float w20 = CV_MAT_ELEM(*map_matrix,float,2,0);
  const float w21 = CV_MAT_ELEM(*map_matrix,float,2,1);
  
  for (int y = 0; y < src1->height; y++)
  {
    for (int x = 0; x < src1->width; x++)
    {
      // project coordinates along homography and normalize them
      const float den = 1 / (w20 * x + w21 * y + 1);
      const float u   =   - (w00 * x + w01 * y + w02) * den;
      const float v   =   - (w10 * x + w11 * y + w12) * den;
      const float x_s =                             x * den;
      const float y_s =                             y * den;
      
      // access source and destination values
      const float   src1_pix =   CV_MAT_ELEM (*src1,     float, y, x);
      const float   src2_pix =   CV_MAT_ELEM (*src2,     float, y, x);
            float * line     = & CV_MAT_ELEM (*jacobian, float, y*src1->width+x, 0);
      
      // perform dot product directly
      line[0] =     x_s * src1_pix;
      line[1] =                          x_s * src2_pix;
      line[2] = u * x_s * src1_pix + v * x_s * src2_pix;
      line[3] =     y_s * src1_pix;
      line[4] =                          y_s * src2_pix;
      line[5] = u * y_s * src1_pix + v * y_s * src2_pix;
      line[6] =     den * src1_pix;
      line[7] =                          den * src2_pix;
		}
	}
}

static void calculate_jacobian_euclidean (const CvMat * src1, const CvMat * src2, CvMat * jacobian, const CvMat * map_matrix)
{
  assert (src1);
  assert (src2);
  assert (src1->width  == src2->width);
  assert (src1->height == src2->height);
  assert (jacobian);
  assert (jacobian->cols == 3);
  assert (jacobian->rows == src1->width * src1->height);
  assert (map_matrix->cols == 3);
  assert (map_matrix->rows == 2);
  
  // direct access to map matrix entries
  const float w00 = CV_MAT_ELEM(*map_matrix,float,0,0); //cos(theta)
  const float w10 = CV_MAT_ELEM(*map_matrix,float,1,0); //sin(theta)
  
  for (int y = 0; y < src1->height; y++)
  {
    for (int x = 0; x < src1->width; x++)
    {
      // project coordinates wrt to current angle
      const float x_s =   -w10 * x - w00 * y;
      const float y_s =    w00 * x - w10 * y;
      
      // access source and destination values
      const float   src1_pix =   CV_MAT_ELEM (*src1,     float, y, x);
      const float   src2_pix =   CV_MAT_ELEM (*src2,     float, y, x);
            float * line     = & CV_MAT_ELEM (*jacobian, float, y*src1->width+x, 0);
      
      // perform dot product directly
      line[0] =  x_s * src1_pix + y_s * src2_pix;
      line[1] =     src1_pix;
      line[2] =     src2_pix;
		}
	}
}


static void update_warping_matrix (CvMat * map_matrix, const CvMat * update, const WARP_MODE warp_mode)
{
  assert (map_matrix); assert (CV_MAT_TYPE (map_matrix->type) == CV_32FC1);
  assert (update);     assert (CV_MAT_TYPE (update->type)     == CV_32FC1);

  assert (warp_mode == WARP_MODE_AFFINE || warp_mode == WARP_MODE_HOMOGRAPHY ||
	  warp_mode == WARP_MODE_EUCLIDEAN || warp_mode == WARP_MODE_TRANSLATION);
  if (warp_mode == WARP_MODE_HOMOGRAPHY)
	   assert (map_matrix->rows == 3 && update->rows == 8);
  else if (warp_mode == WARP_MODE_AFFINE)
		assert(map_matrix->rows == 2 && update->rows == 6);
  else if (warp_mode == WARP_MODE_EUCLIDEAN)
	   assert (map_matrix->rows == 2 && update->rows == 3);
  else 
	   assert (map_matrix->rows == 2 && update->rows == 2);
  
  assert (update->cols == 1);
  
  const int cols = map_matrix->cols;
  const int rows = map_matrix->rows;

  if (warp_mode == WARP_MODE_HOMOGRAPHY || warp_mode == WARP_MODE_AFFINE) {
  for (ptrdiff_t y = 0; y < rows; y++)
		for (ptrdiff_t x = 0; x < cols; x++)
      if (y != 2 || x != 2) // the last element (2,2) is not represented in update (homography case)
				CV_MAT_ELEM(*(map_matrix), float, y, x) += CV_MAT_ELEM(*(update), float, rows*x+y, 0);

  }
  else if (warp_mode == WARP_MODE_TRANSLATION) {
	  			CV_MAT_ELEM(*(map_matrix), float, 0, 2) += CV_MAT_ELEM(*(update), float, 0, 0);
	  			CV_MAT_ELEM(*(map_matrix), float, 1, 2) += CV_MAT_ELEM(*(update), float, 1, 0);
  }
  else {
      double new_theta = acos(CV_MAT_ELEM(*(map_matrix), float, 0, 0));
      if (CV_MAT_ELEM(*(map_matrix), float, 1, 0)<0){
          new_theta = -new_theta;
      }
	  new_theta = new_theta + CV_MAT_ELEM(*(update), float, 0, 0);

	  CV_MAT_ELEM(*(map_matrix), float, 0, 2) += CV_MAT_ELEM(*(update), float, 1, 0);
	  CV_MAT_ELEM(*(map_matrix), float, 1, 2) += CV_MAT_ELEM(*(update), float, 2, 0);

	  CV_MAT_ELEM(*(map_matrix), float, 0, 0) = CV_MAT_ELEM(*(map_matrix), float, 1, 1) = cos(new_theta);
	  CV_MAT_ELEM(*(map_matrix), float, 1, 0) = sin(new_theta);
	  CV_MAT_ELEM(*(map_matrix), float, 0, 1) = -CV_MAT_ELEM(*(map_matrix), float, 1, 0);

	
  }

}


CvMat * cvFindTransform (const IplImage * src_image, const IplImage * dst_image, CvMat * map_matrix, const WARP_MODE warp_mode, const CvTermCriteria & termination)
{
  assert (src_image); assert (src_image->nChannels == 1); assert (src_image->width > 0 && src_image->height > 0);
  assert (dst_image); assert (dst_image->nChannels == 1); assert (dst_image->width > 0 && dst_image->height > 0);
  //assert (src_image->width <= dst_image->width && src_image->height <= dst_image->height);
  assert (map_matrix); assert (map_matrix->cols == 3); assert (map_matrix->rows == 2 || map_matrix->rows ==3);
  assert (warp_mode == WARP_MODE_AFFINE || warp_mode == WARP_MODE_HOMOGRAPHY ||
	  warp_mode == WARP_MODE_EUCLIDEAN || warp_mode == WARP_MODE_TRANSLATION);
  if (warp_mode == WARP_MODE_HOMOGRAPHY)
	  assert (map_matrix->rows ==3);

  assert (termination.type & CV_TERMCRIT_ITER || termination.type & CV_TERMCRIT_EPS);
  
  const int    number_of_iterations = (termination.type & CV_TERMCRIT_ITER) ? termination.max_iter : 200;
  const double termination_eps      = (termination.type & CV_TERMCRIT_EPS)  ? termination.epsilon  :  -1;
  
//  printf("number of iterations: %d \n", number_of_iterations);
//  printf("epsilon:%f\n",termination_eps);

 int param_temp = 6;//default: affine
  switch (warp_mode){
	  case WARP_MODE_TRANSLATION:
		  param_temp = 2;
		  break;
	  case WARP_MODE_EUCLIDEAN:
		 param_temp = 3;
		 break;
	  case WARP_MODE_HOMOGRAPHY:
		  param_temp = 8;
		  break;
  }

  const int number_of_model_parameters = param_temp; 
  const int number_of_pixels           = src_image->width * src_image->height;
  
  CvMat * src_image_zeromean = cvCreateMat (src_image->height, src_image->width, CV_32F); // smoothed zero-mean template
  CvMat * dst_image_float    = cvCreateMat (dst_image->height, dst_image->width, CV_32F); // smoothed input image
  CvMat * dst_image_warped   = cvCreateMat (src_image->height, src_image->width, CV_32F); // warped zero-mean input image
  
  // gaussian smoothing and zero-mean shifting (template image)
  cvConvert (src_image, src_image_zeromean);
  cvSmooth  (src_image_zeromean, src_image_zeromean, CV_GAUSSIAN, 5, 5);
  cvSubS    (src_image_zeromean, cvAvg (src_image_zeromean), src_image_zeromean);
  
  // gaussian smoothing (warped image)
  cvConvert (dst_image, dst_image_float);
  cvSmooth  (dst_image_float, dst_image_float, CV_GAUSSIAN, 5, 5);
  
  // needed matrices for gradients and warped gradients
  CvMat * dst_gradient_x        = cvCreateMat (dst_image->height, dst_image->width, CV_32FC1);
  CvMat * dst_gradient_y        = cvCreateMat (dst_image->height, dst_image->width, CV_32FC1);
  CvMat * dst_gradient_x_warped = cvCreateMat (src_image->height, src_image->width, CV_32FC1);
  CvMat * dst_gradient_y_warped = cvCreateMat (src_image->height, src_image->width, CV_32FC1);
  
  // calculate first order image derivatives
  CvMat * dx = cvCreateMat (1, 3, CV_32FC1);
  CvMat * dy = cvCreateMat (3, 1, CV_32FC1);
  CV_MAT_ELEM(* dx, float, 0, 0) = -1.0f/2 ;
  CV_MAT_ELEM(* dx, float, 0, 1) =  0.0f   ;
  CV_MAT_ELEM(* dx, float, 0, 2) =  1.0f/2 ;
  CV_MAT_ELEM(* dy, float, 0, 0) = -1.0f/2 ;
  CV_MAT_ELEM(* dy, float, 1, 0) =  0.0f   ;
  CV_MAT_ELEM(* dy, float, 2, 0) =  1.0f/2 ;
  cvFilter2D (dst_image_float, dst_gradient_x, dx);
  cvFilter2D (dst_image_float, dst_gradient_y, dy);
  cvReleaseMat (& dy);
  cvReleaseMat (& dx);
  
  // linear equation system for maximizing the enhanced correlation coefficient rho = normalized(dst_image_warped_flat * src_image_zeromean_flat)
  CvMat * jacobian                     = cvCreateMat (number_of_pixels,           number_of_model_parameters, CV_32F); // jacobian of warped image
  CvMat * jacobian_T                   = cvCreateMat (number_of_model_parameters, number_of_pixels,           CV_32F); // jacobian transposed
  CvMat * hessian                      = cvCreateMat (number_of_model_parameters, number_of_model_parameters, CV_32F); // Hessian
  CvMat * hessian_inv                  = cvCloneMat  (hessian);                                                        // inverse of Hessian
  CvMat * dst_image_projected          = cvCreateMat (number_of_model_parameters, 1, CV_32F);                          // destination image projected into jacobian
  CvMat * src_image_projected          = cvCloneMat  (dst_image_projected);                                            // source image projected into jacobian
  CvMat * dst_image_projected_hessian  = cvCloneMat  (dst_image_projected);                                            // auxiliary matrix
  CvMat * error_projected              = cvCloneMat  (dst_image_projected);                                            // error projected into jacobian
  
  // flattened (i.e. onedimensional) representations 
  CvMat src_image_zeromean_header, dst_image_warped_header;
  CvMat * src_image_zeromean_flat = cvReshape   (src_image_zeromean, & src_image_zeromean_header, 0, number_of_pixels);
  CvMat * dst_image_warped_flat   = cvReshape   (dst_image_warped,   & dst_image_warped_header,   0, number_of_pixels);
  CvMat * delta_p_flat            = cvCreateMat (number_of_model_parameters, 1, CV_32FC1);
  CvMat * error                   = cvCloneMat  (src_image_zeromean_flat);
  
  const double src_norm = cvNorm(src_image_zeromean_flat, 0, CV_L2); 
  
  // iteratively update map_matrix
  double rho      = -1;
  double last_rho = - termination_eps;
  for (int i = 1; (i <= number_of_iterations) && (fabs(rho-last_rho)>= termination_eps); i++)
  {  

    // warp-back portion of the dst_image to the coordinate space of the src_image
    if (warp_mode != WARP_MODE_HOMOGRAPHY)
    {
      cvWarpAffine      (dst_image_float, dst_image_warped,      map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
      cvWarpAffine      (dst_gradient_x,  dst_gradient_x_warped, map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
      cvWarpAffine      (dst_gradient_y,  dst_gradient_y_warped, map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
    }
    else
    {
      cvWarpPerspective (dst_image_float, dst_image_warped,      map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
      cvWarpPerspective (dst_gradient_x,  dst_gradient_x_warped, map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
      cvWarpPerspective (dst_gradient_y,  dst_gradient_y_warped, map_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP);
    }
    
    // zero-mean
    cvSubS (dst_image_warped, cvAvg (dst_image_warped), dst_image_warped);
    
    // calculate jacobian of image wrt parameters
	switch (warp_mode){
	  case WARP_MODE_AFFINE:
		  calculate_jacobian_affine (dst_gradient_x_warped, dst_gradient_y_warped, jacobian);
		  break;
	  case WARP_MODE_HOMOGRAPHY:
		  calculate_jacobian_homography (dst_gradient_x_warped, dst_gradient_y_warped, jacobian, map_matrix);
		  break;
	  case WARP_MODE_TRANSLATION:
	      calculate_jacobian_translation (dst_gradient_x_warped, dst_gradient_y_warped, jacobian);
		  break;
	  case WARP_MODE_EUCLIDEAN:
		  calculate_jacobian_euclidean (dst_gradient_x_warped, dst_gradient_y_warped, jacobian, map_matrix);
		 break;
  }
  
    cvTranspose (jacobian, jacobian_T);
    
    // calculate Hessian and its inverse
    cvGEMM   (jacobian_T, jacobian, 1, 0, 0, hessian);
    cvInvert (hessian, hessian_inv);

	const double dst_norm = cvNorm(dst_image_warped_flat, 0, CV_L2); 
	const double correlation = cvDotProduct (dst_image_warped_flat,src_image_zeromean_flat);


	// calculate enhanced correlation coefficiont (ECC) rho
    last_rho = rho;
    rho = correlation/dst_norm/src_norm;

	//      printf("iteration: %d\n", i);
	//		printf("iteration's contribution: %f\n", rho-last_rho);
	//      printf("rho: %f\n", rho);

    cvGEMM (jacobian_T,  dst_image_warped_flat,   1, NULL, 0, dst_image_projected);
  	cvGEMM (jacobian_T,  src_image_zeromean_flat, 1, NULL, 0, src_image_projected);
    cvGEMM (hessian_inv, dst_image_projected,     1, NULL, 0, dst_image_projected_hessian);
    
    // calculate the parameter lambda to account for illumination variation
    const double lambda_n = pow (dst_norm, 2.0) - cvDotProduct (dst_image_projected, dst_image_projected_hessian);
    const double lambda_d = correlation - cvDotProduct (src_image_projected, dst_image_projected_hessian);
    if (lambda_d <= 0.0)
    {
      // The execution was interrupted. The correlation value is going to be minimized.
      map_matrix = NULL;
      break;
    }
    const double lambda = (lambda_n/lambda_d);

    // estimate the update step delta_p
    cvScale (src_image_zeromean_flat, error, lambda);
    cvSub  (error, dst_image_warped_flat, error);
    cvGEMM (jacobian_T,  error,           1, NULL, 0, error_projected);
    cvGEMM (hessian_inv, error_projected, 1, NULL, 0, delta_p_flat);


    // update warping matrix
    update_warping_matrix (map_matrix, delta_p_flat, warp_mode);

  }


  // free intermediate buffers in reverse order
  cvReleaseMat (& error);
  cvReleaseMat (& delta_p_flat);

  cvReleaseMat (& error_projected);
  cvReleaseMat (& dst_image_projected_hessian);
  cvReleaseMat (& src_image_projected);
  cvReleaseMat (& dst_image_projected);
  cvReleaseMat (& hessian_inv);
  cvReleaseMat (& hessian);
  cvReleaseMat (& jacobian_T);
  cvReleaseMat (& jacobian);
  cvReleaseMat (& dst_gradient_y_warped);
  cvReleaseMat (& dst_gradient_x_warped);
  cvReleaseMat (& dst_gradient_y);
  cvReleaseMat (& dst_gradient_x);
  cvReleaseMat (& dst_image_warped);
  cvReleaseMat (& dst_image_float);
  cvReleaseMat (& src_image_zeromean);
  
  // return updated map matrix
  return map_matrix;
}
