/*************************************************************************
    > File Name: test_opencv_version.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-07-16 15:17:35
 ************************************************************************/

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
  cout << "OpenCV version : " << CV_VERSION << endl;
  cout << "Major version : " << CV_MAJOR_VERSION << endl;
  cout << "Minor version : " << CV_MINOR_VERSION << endl;
  cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;

  if ( CV_MAJOR_VERSION < 3)
  {
      // Old OpenCV 2 code goes here. 
      cout<<"opencv2\n";
  } else
  {
      // New OpenCV 3 code goes here.
      cout<<"opencv3\n";
  }
}
