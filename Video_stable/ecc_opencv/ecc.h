#ifndef _ECC_H_
#define _ECC_H_

/* 
 * Enhanced Correlation Coefficient (ECC) Image Alignment Algorithm 
 * http://xanthippi.ceid.upatras.gr/people/evangelidis/ecc
 *
 * Authors: Georgios Evangelidis
 * e-mail: evagelid@ceid.upatras.gr
 * e-mail: georgios.evangelidis@inria.fr
 *
 * Copyright (2010): G.D.Evangelidis
 *
 * For details, see the paper:
 * G.D. Evangelidis, E.Z. Psarakis, "Parametric Image Alignment using 
 * Enhanced Correlation Coefficient Maximization, IEEE Transaction on
 * Pattern Analysis & Machine Intelligence, vol. 30, no. 10, 2008
 */

#include <cxcore.h>

enum WARP_MODE
{
	WARP_MODE_TRANSLATION,
	WARP_MODE_EUCLIDEAN,
	WARP_MODE_AFFINE,
	WARP_MODE_HOMOGRAPHY
};

CvMat * cvFindTransform(const IplImage * src_image, const IplImage * dst_image, CvMat * map_matrix, const WARP_MODE warp_mode = WARP_MODE_AFFINE, const CvTermCriteria & termination = cvTermCriteria (CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 50, 0.001));

#endif //_ECC_H_
