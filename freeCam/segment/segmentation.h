
#include<iostream>
#include<string>
#include<vector>
#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void getEdges(Mat image, Mat &edge, int edgeThresh, bool use_scharr);
void roughThreshSegment(Mat img, Mat &mask);
Mat singleConnetedDomain( Mat binary );
int getAreaContours(Mat binary, vector<vector<Point> > &contours, vector<Vec4i> &hierarchy);
Mat singleConnetedDomainContour(Mat binary);
Mat fillHole(Mat binary);
Mat refineBinarySegments(Mat binary);
Mat colorThreshSegment(Mat original, const vector<int> colorThresh, const string mode);
void getShowContours(Mat binary);

void refineSegments(const Mat img, Mat mask, Mat& dst);
