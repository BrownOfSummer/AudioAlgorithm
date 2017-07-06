#include <opencv2/opencv.hpp>
#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>
#include <string>

#include "Header.h"
#include "gms_matcher.h"
using namespace std;
using namespace cv;

const int SMOOTHING_RADIUS = 5; // In frames. The larger the more stable the video, but less reactive to sudden panning
const int HORIZONTAL_BORDER_CROP = 0; // In pixels. Crops the border to reduce the black borders from stabilisation being too noticeable.
struct TransformParam
{
    TransformParam() {}
    TransformParam(double _dx, double _dy, double _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx;
    double dy;
    double da; // angle
};

struct Trajectory
{
    Trajectory() {}
    Trajectory(double _x, double _y, double _a) {
        x = _x;
        y = _y;
        a = _a;
    }

    double x;
    double y;
    double a; // angle
};

inline size_t getMatchPoint(Mat img1, Mat img2, vector<KeyPoint> &kp1, vector<KeyPoint> &kp2, vector<DMatch> &matches_gms);
int smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform);

    ofstream out_transform("prev_to_cur_transformation.txt");
    ofstream out_trajectory("trajectory.txt");
    ofstream out_smoothed_trajectory("smoothed_trajectory.txt");
    ofstream out_new_transform("new_prev_to_cur_transformation.txt");
int main(int argc, char **argv)
{
    string ImgNamesLists[1000];
    char str[200];
    FILE *fImgList = fopen(argv[1], "rb");
    if (NULL == fImgList)
        return -1;
    int count = 0;
    while (fgets(str, 200, fImgList) != NULL && count < 1000) {
        memset(str + strlen(str)-1, 0, 1);
        ImgNamesLists[count] = str;
        count ++;
    }

    if( argc < 2 ) {
        cout<<argv[0]<<" [image.list]"<<endl;
        return -1;
    }

    Mat cur, cur_grey;
    Mat prev, prev_grey;

    prev = imread(ImgNamesLists[0]);

    // Step 1 - Get previous to current frame transformation (dx, dy, da) for all frames
    vector <TransformParam> prev_to_cur_transform; // previous to current

    int k=1;
    int max_frames = count;
    Mat last_T;

    for(int i = 1; i < count; i ++){
        cur = imread(ImgNamesLists[i]);
        if(cur.data == NULL) {
            break;
        }

        /*
         * New method for trans matrix
         */

        vector<KeyPoint> kp1, kp2;
        vector<DMatch> matches_gms;
        int n_match = getMatchPoint(prev, cur, kp1, kp2, matches_gms);
        cout<<n_match<<" "<<kp1.size()<<" "<<kp2.size()<<endl;
        
        vector<Point2f> prev_corner2;
        vector<Point2f> cur_corner2;
        for (size_t i = 0; i < matches_gms.size(); i++)
        {
            Point2f left = kp1[matches_gms[i].queryIdx].pt;
            Point2f right = kp2[matches_gms[i].trainIdx].pt;
            prev_corner2.push_back(left);
            cur_corner2.push_back(right);
        }
        
        Mat T = estimateRigidTransform(prev_corner2, cur_corner2, false); // false = rigid transform, no scaling/shearing


        // in rare cases no transform is found. We'll just use the last known good transform.
        if(T.data == NULL) {
            last_T.copyTo(T);
        }

        T.copyTo(last_T);

        // decompose T
        double dx = T.at<double>(0,2);
        double dy = T.at<double>(1,2);
        double da = atan2(T.at<double>(1,0), T.at<double>(0,0));

        prev_to_cur_transform.push_back(TransformParam(dx, dy, da));

        out_transform << k << " " << dx << " " << dy << " " << da << endl;

        cur.copyTo(prev);
        cur_grey.copyTo(prev_grey);

        cout << "Frame: " << k << "/" << max_frames << " - good optical flow: " << prev_corner2.size() << endl;
        k++;
    }

    vector<TransformParam> new_prev_to_cur_transform;
    smooth_warp_para(prev_to_cur_transform, new_prev_to_cur_transform);
    // Step 5 - Apply the new transformation to the video
    //cap.set(CV_CAP_PROP_POS_FRAMES, 0);
    Mat T(2,3,CV_64F);

    int vert_border = HORIZONTAL_BORDER_CROP * prev.rows / prev.cols; // get the aspect ratio correct

    for(int k = 0; k < max_frames - 1; k ++){
        //cap >> cur;
        cur = imread(ImgNamesLists[k]);

        if(cur.data == NULL) {
            break;
        }

        T.at<double>(0,0) = cos(new_prev_to_cur_transform[k].da);
        T.at<double>(0,1) = -sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1,0) = sin(new_prev_to_cur_transform[k].da);
        T.at<double>(1,1) = cos(new_prev_to_cur_transform[k].da);

        T.at<double>(0,2) = new_prev_to_cur_transform[k].dx;
        T.at<double>(1,2) = new_prev_to_cur_transform[k].dy;

        Mat cur2;

        warpAffine(cur, cur2, T, cur.size());

        cur2 = cur2(Range(vert_border, cur2.rows-vert_border), Range(HORIZONTAL_BORDER_CROP, cur2.cols-HORIZONTAL_BORDER_CROP));

        // Resize cur2 back to cur size, for better side by side comparison
        resize(cur2, cur2, cur.size());

        // Now draw the original and stablised side by side for coolness
        //Mat canvas = Mat::zeros(cur.rows, cur.cols*2+10, cur.type());
        Mat canvas = Mat::zeros(cur.rows, cur.cols, cur.type());

        //cur.copyTo(canvas(Range::all(), Range(0, cur2.cols)));
        //cur2.copyTo(canvas(Range::all(), Range(cur2.cols+10, cur2.cols*2+10)));
        cur2.copyTo(canvas(Range::all(), Range(0, cur2.cols)));

        // If too big to fit on the screen, then scale it down by 2, hopefully it'll fit :)
        if(canvas.cols > 1920) {
            resize(canvas, canvas, Size(canvas.cols/2, canvas.rows/2));
        }

        imshow("before and after", canvas);

        char str[256];
        sprintf(str, "data/%08d.jpg", k);
        imwrite(str, canvas);

        waitKey(20);
    }

    return 0;
}

inline size_t getMatchPoint(Mat img1, Mat img2, vector<KeyPoint> &kp1, vector<KeyPoint> &kp2, vector<DMatch> &matches_gms)
{
	//vector<KeyPoint> kp1, kp2;
	Mat d1, d2;
	//vector<DMatch> matches_all, matches_gms;
	vector<DMatch> matches_all;

	Ptr<ORB> orb = ORB::create(10000);
	orb->setFastThreshold(0);
	orb->detectAndCompute(img1, Mat(), kp1, d1);
	orb->detectAndCompute(img2, Mat(), kp2, d2);
    
	BFMatcher matcher(NORM_HAMMING);
	matcher.match(d1, d2, matches_all);

	// GMS filter
	int num_inliers = 0;
	std::vector<bool> vbInliers;
	gms_matcher gms(kp1,img1.size(), kp2,img2.size(), matches_all);
	num_inliers = gms.GetInlierMask(vbInliers, false, false);

	// draw matches
	for (size_t i = 0; i < vbInliers.size(); ++i)
	{
		if (vbInliers[i] == true)
		{
			matches_gms.push_back(matches_all[i]);
		}
	}

    return matches_gms.size();
}

int smooth_warp_para(vector<TransformParam> prev_to_cur_transform, vector<TransformParam> &new_prev_to_cur_transform)
{

    // Step 2 - Accumulate the transformations to get the image trajectory

    // Accumulated frame to frame transform
    double a = 0;
    double x = 0;
    double y = 0;

    vector <Trajectory> trajectory; // trajectory at all frames

    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {
        x += prev_to_cur_transform[i].dx;
        y += prev_to_cur_transform[i].dy;
        a += prev_to_cur_transform[i].da;

        trajectory.push_back(Trajectory(x,y,a));

        out_trajectory << (i+1) << " " << x << " " << y << " " << a << endl;
    }

    // Step 3 - Smooth out the trajectory using an averaging window
    vector <Trajectory> smoothed_trajectory; // trajectory at all frames

    for(size_t i=0; i < trajectory.size(); i++) {
        double sum_x = 0;
        double sum_y = 0;
        double sum_a = 0;
        int count = 0;

        for(int j=-SMOOTHING_RADIUS; j <= SMOOTHING_RADIUS; j++) {
            if(i+j >= 0 && i+j < trajectory.size()) {
                sum_x += trajectory[i+j].x;
                sum_y += trajectory[i+j].y;
                sum_a += trajectory[i+j].a;

                count++;
            }
        }

        double avg_a = sum_a / count;
        double avg_x = sum_x / count;
        double avg_y = sum_y / count;

        smoothed_trajectory.push_back(Trajectory(avg_x, avg_y, avg_a));

        out_smoothed_trajectory << (i+1) << " " << avg_x << " " << avg_y << " " << avg_a << endl;
    }

    // Step 4 - Generate new set of previous to current transform, such that the trajectory ends up being the same as the smoothed trajectory
    //vector <TransformParam> new_prev_to_cur_transform;

    // Accumulated frame to frame transform
    a = 0;
    x = 0;
    y = 0;

    for(size_t i=0; i < prev_to_cur_transform.size(); i++) {
        x += prev_to_cur_transform[i].dx;
        y += prev_to_cur_transform[i].dy;
        a += prev_to_cur_transform[i].da;

        // target - current
        double diff_x = smoothed_trajectory[i].x - x;
        double diff_y = smoothed_trajectory[i].y - y;
        double diff_a = smoothed_trajectory[i].a - a;

        double dx = prev_to_cur_transform[i].dx + diff_x;
        double dy = prev_to_cur_transform[i].dy + diff_y;
        double da = prev_to_cur_transform[i].da + diff_a;

        new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
        out_new_transform << (i+1) << " " << dx << " " << dy << " " << da << endl;

    }

    return new_prev_to_cur_transform.size();
}
