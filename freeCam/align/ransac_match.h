#ifndef _MATCH_H
#define _MATCH_H

#include <iostream>
#include <map>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>
//#include "CImg.h"

//using namespace cimg_library;
using namespace std;

#define NUM_OF_PAIR 4
#define CONFIDENCE 0.99
#define INLINER_RATIO 0.5
#define RANSAC_THRESHOLD 4.0
struct point_pair {
    cv::Point2f a;
    cv::Point2f b;
    point_pair(cv::Point2f _a, cv::Point2f _b)
    {
        a = _a;
        b = _b;
    }
};
using namespace std;

struct Parameters {
	float c1;
	float c2;
	float c3;
	float c4;
	float c5;
	float c6;
	float c7;
	float c8;
	Parameters(float _c1, float _c2, float _c3, float _c4, float _c5, float _c6, float _c7, float _c8) {
		c1 = _c1;
		c2 = _c2;
		c3 = _c3;
		c4 = _c4;
		c5 = _c5;
		c6 = _c6;
		c7 = _c7;
		c8 = _c8;
	}
	void print() {
		cout << c1 << " " << c2 << " " << c3 << " " << c4 << endl;
		cout << c5 << " " << c6 << " " << c7 << " " << c8 << endl;
	}
};

float getXAfterWarping(float x, float y, Parameters H);
float getYAfterWarping(float x, float y, Parameters H);

Parameters getHomographyFromPoingPairs(const vector<point_pair> &pairs);
void ransacit(const vector<point_pair> &pairs, vector<int> &retInlierIdx);

#endif
