
#include "Header.h"
#include "gms_matcher.h"
#include <ctime>

#include<iostream>
using namespace std;

inline size_t getMatchPoint(Mat img1, Mat img2, vector<KeyPoint> &kp1, vector<KeyPoint> &kp2, vector<DMatch> &matches_gms);
int main(int argc, char* argv[])
{

    if(argc != 3){
        cout<<"ERROR: Wrong input-> "<<argv[0]<<" img1 img2"<<endl;
        return -1;
    }
    Mat img1 = imread(argv[1]);
    Mat img2 = imread(argv[2]);
	vector<KeyPoint> kp1, kp2;
	vector<DMatch> matches_gms;
    int n_match = getMatchPoint(img1, img2, kp1, kp2, matches_gms);
    cout<<n_match<<" "<<kp1.size()<<" "<<kp2.size()<<endl;
	Mat show = DrawInlier(img1, img2, kp1, kp2, matches_gms, 1);
	//imshow("show", show);
	//waitKey();
    
    /*test*/
    vector<Point2f> Pre;
    vector<Point2f> Next;
    for (size_t i = 0; i < matches_gms.size(); i++)
    {
        Point2f left = kp1[matches_gms[i].queryIdx].pt;
        Point2f right = kp2[matches_gms[i].trainIdx].pt;
        //std::cout<<"left = "<<left<<"; right = "<<right<<std::endl;
        Pre.push_back(left);
        Next.push_back(right);
    }
    Mat T, im_aligned;
    if( n_match > 0 )
        T = estimateRigidTransform(Pre, Next, false);
    else
    {
        cout<<"WARNING, Not point for estimateRigidTransform !"<<endl;
    }
    if( T.empty() )
    {
        cout<<"WARNING, Cannot warpAffine the image !"<<endl;
        return -1;
    }
    else
    {
        warpAffine(img2, im_aligned, T, img2.size(), INTER_LINEAR + WARP_INVERSE_MAP, BORDER_TRANSPARENT);
        imshow("align", im_aligned);
    }
    imshow("img1", img1);
    imshow("img2", img2);
    imshow("show", show);
    waitKey();
    std::cout<<T<<std::endl;

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

