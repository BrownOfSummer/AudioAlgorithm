// Module: invcomp.h
// Brief:  
// Author: Oleg A. Krivtsov
// Email:  olegkrivtsov@mail.ru
// Date:  March 2008

#ifndef __INVERSECOMP_H__
#define __INVERSECOMP_H__

#include "cv.h"

void align_image_inverse_compositional(IplImage* pImgT, CvRect omega, IplImage* pImgI);

#endif //__INVERSECOMP_H__