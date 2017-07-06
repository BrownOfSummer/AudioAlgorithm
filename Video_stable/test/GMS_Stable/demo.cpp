// GridMatch.cpp : Defines the entry point for the console application.

//#define USE_GPU 

#include "Header.h"
#include "gms_matcher.h"
#include <ctime>

void GmsMatch(Mat &img1, Mat &img2);

void runImagePair(){
	Mat img1 = imread("./data/nn_left.jpg");
	Mat img2 = imread("./data/nn_right.jpg");

	imresize(img1, 480);
	imresize(img2, 480);

	GmsMatch(img1, img2);
}

void runImagePair2(Mat &img1, Mat &img2){
	//Mat img1 = imread("./data/nn_left.jpg");
	//Mat img2 = imread("./data/nn_right.jpg");

	//imresize(img1, 480);
	//imresize(img2, 480);

	GmsMatch(img1, img2);
}

int main(int argc, char* argv[])
{
#ifdef USE_GPU
	int flag = cuda::getCudaEnabledDeviceCount();
	if (flag != 0){ cuda::setDevice(0); }
#endif // USE_GPU

    if(argc < 2)
	    runImagePair();
    else if(argc == 3){
        Mat img1 = imread(argv[1]);
        Mat img2 = imread(argv[2]);
        runImagePair2(img1, img2);
    }
    else{
        std::cout<<"Without img or with img1 and img1"<<std::endl;
    }
        

	return 0;
}


void GmsMatch(Mat &img1, Mat &img2){
    clock_t start = clock();
    clock_t ORB_start = clock();
	vector<KeyPoint> kp1, kp2;
	Mat d1, d2;
	vector<DMatch> matches_all, matches_gms;

	Ptr<ORB> orb = ORB::create(10000);
	orb->setFastThreshold(0);
	orb->detectAndCompute(img1, Mat(), kp1, d1);
	orb->detectAndCompute(img2, Mat(), kp2, d2);
    
    clock_t ORB_end = clock();
    cout << "ORB took "<< 1000.0*(ORB_end - ORB_start)/CLOCKS_PER_SEC<< "ms"<<endl;
    clock_t BF_start = clock();
#ifdef USE_GPU
	GpuMat gd1(d1), gd2(d2);
	Ptr<cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher(NORM_HAMMING);
	matcher->match(gd1, gd2, matches_all);
#else
	BFMatcher matcher(NORM_HAMMING);
	matcher.match(d1, d2, matches_all);
#endif

    clock_t BF_end = clock();
    cout<< "BF took "<< 1000.0*(BF_end - BF_start)/CLOCKS_PER_SEC<< "ms"<<endl;
	// GMS filter
    clock_t GMS_start = clock();
	int num_inliers = 0;
	std::vector<bool> vbInliers;
	gms_matcher gms(kp1,img1.size(), kp2,img2.size(), matches_all);
	num_inliers = gms.GetInlierMask(vbInliers, false, false);

    clock_t GMS_end = clock();
    cout<< "GMS took "<< 1000.0*(GMS_end - GMS_start)/CLOCKS_PER_SEC<<" ms "<<endl;
	cout << "Get total " << num_inliers << " matches." << endl;
    clock_t end = clock();
    float len = 1000.0 * (end - start)/CLOCKS_PER_SEC;
    cout<< "Totally took "<< len << "ms" <<endl;
	// draw matches
	for (size_t i = 0; i < vbInliers.size(); ++i)
	{
		if (vbInliers[i] == true)
		{
			matches_gms.push_back(matches_all[i]);
		}
	}

	Mat show = DrawInlier(img1, img2, kp1, kp2, matches_gms, 2);
	imshow("show", show);
	waitKey();
    //imwrite("match.jpg", show);

    /*test*/
    /*
    std::cout<<matches_gms.size()<<std::endl;
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
    Mat T = estimateRigidTransform(Pre, Next, false);
    Mat im_aligned;
    warpAffine(img2, im_aligned, T, img2.size(), INTER_LINEAR + WARP_INVERSE_MAP, BORDER_TRANSPARENT);
    imshow("img1", img1);
    imshow("img2", img2);
    imshow("align", im_aligned);
    waitKey();
    std::cout<<T<<std::endl;
    */
}


