/*************************************************************************
    > File Name: mystitching.cpp
    > Author: li_pengju
    > Mail: 
    > Copyright: All Rights Reserved
    > Created Time: 2018-06-21 15:14:07
 ************************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include "opencv2/opencv_modules.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/timelapsers.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"

#define ENABLE_LOG 1
#define LOG(msg) std::cout << msg
#define LOGLN(msg) std::cout << msg << std::endl
using namespace std;
using namespace cv;
using namespace cv::detail;
// Default command line args
vector<String> img_names;
bool preview = false;
bool try_cuda = false;
double work_megapix = 0.6;
double seam_megapix = 0.1;
double compose_megapix = -1;
float conf_thresh = 1.f;
string features_type = "surf";
string matcher_type = "homography";
string estimator_type = "homography";
string ba_cost_func = "ray";
string ba_refine_mask = "xxxxx";
bool do_wave_correct = true;
WaveCorrectKind wave_correct = detail::WAVE_CORRECT_HORIZ;
bool save_graph = false;
std::string save_graph_to;
string warp_type = "spherical";
int expos_comp_type = ExposureCompensator::GAIN_BLOCKS;
float match_conf = 0.3f;
string seam_find_type = "gc_color";
int blend_type = Blender::MULTI_BAND;
int timelapse_type = Timelapser::AS_IS;
float blend_strength = 5;
string result_name = "result.jpg";
bool timelapse = false;
int range_width = -1;

bool load_find_features(vector<String> &img_names, vector<ImageFeatures> &features, vector<Mat> &images, vector<Size> &full_img_sizes);
bool load_find_features(vector<String> &img_names, vector<ImageFeatures> &features, vector<Mat> &images, vector<Size> &full_img_sizes, double *work_scale, double *seam_scale, bool &is_work_scale_set, bool &is_seam_scale_set);
bool get_pairwise_matches(vector<ImageFeatures> &features, vector<MatchesInfo> &pairwise_matches);
bool estimate_adjuster_cameras(vector<ImageFeatures> &features, vector<MatchesInfo> &pairwise_matches, vector<CameraParams> &cameras, vector<double> &focals, float *warped_image_scale);
bool generate_warp_creator(Ptr<WarperCreator> &warper_creator, string warp_type );
bool warp_images(Ptr<WarperCreator> &warper_creator, vector<Mat> &images, vector<CameraParams> &cameras, float warped_image_scale, double seam_work_aspect,
        vector<Point> &corners, vector<UMat> &masks_warped, vector<UMat> &images_warped, vector<Size> &sizes, Ptr<RotationWarper> &warper);
//bool find_seam(vector<UMat> &images_warped, vector<UMat> &masks_warped, vector<Point> &corners);
bool find_seam(Ptr<ExposureCompensator> &compensator, vector<UMat> &images_warped, vector<UMat> &masks_warped, vector<Point> &corners);
int main(int argc, char *argv[])
{
    // 1 Load images to images, and detect features to features.
    if(argc < 3) return -1;
    for(int i = 1; i < argc; ++i) img_names.push_back(argv[i]);
    int num_images = static_cast<int>(img_names.size());
    if( num_images < 2 )
    {
        LOGLN("Need more images");
        return -1;
    }
    else cout<< "Input "<<num_images<<" images.\n";
    double work_scale = 1, seam_scale = 1, compose_scale = 1;
    double seam_work_aspect = 1;
    //bool is_work_scale_set = false, is_seam_scale_set = false, is_compose_scale_set = false;
    bool is_work_scale_set = true, is_seam_scale_set = true, is_compose_scale_set = false;
    Mat full_img, img;
    vector<ImageFeatures> features(num_images);
    vector<Mat> images(num_images);
    vector<Size> full_img_sizes(num_images);

    //bool flag1 = load_find_features(img_names, features, images, full_img_sizes);
    bool flag1 = load_find_features(img_names, features, images, full_img_sizes, &work_scale, &seam_scale, is_work_scale_set, is_seam_scale_set);
    if( !flag1 ) return -1;
    // 2 Pairwise matching
    LOG("Pairwise matching");
    vector<MatchesInfo> pairwise_matches;
    bool flag2 = get_pairwise_matches(features, pairwise_matches);
    if( !flag2 ) return -1;

    // 3 Check again, Leave only images we are sure are from the same panorama
    vector<int> indices = leaveBiggestComponent(features, pairwise_matches, conf_thresh);
    if( static_cast<int>(indices.size()) != num_images)
    {
        cout<<"Not all images have enough match confidence!\n";
        return -1;
    }

    // 4 Estimate Camera
    // Estimate
    vector<CameraParams> cameras;
    // Find median focal length
    vector<double> focals;
    float warped_image_scale;
    bool flag3 = estimate_adjuster_cameras(features, pairwise_matches, cameras, focals, &warped_image_scale);
    if( !flag3 ) return -1;
    // 5 warp images
    vector<Point> corners(num_images);
    vector<UMat> masks_warped(num_images);
    vector<UMat> images_warped(num_images);
    vector<Size> sizes(num_images);
    Ptr<WarperCreator> warper_creator;
    Ptr<RotationWarper> warper;
    bool flag4 = generate_warp_creator(warper_creator, warp_type);
    bool flag5 = warp_images(warper_creator, images, cameras, warped_image_scale, seam_work_aspect, corners, masks_warped, images_warped, sizes, warper);
    for(size_t i = 0; i < images.size(); ++i)
    {
        imshow("src", images[i]);
        imshow("warped", images_warped[i]);
        imshow("masks_warped", masks_warped[i]);
        waitKey();
    }
    Ptr<ExposureCompensator> compensator = ExposureCompensator::createDefault(expos_comp_type);
    bool flag6 = find_seam(compensator, images_warped, masks_warped, corners);
    for(size_t i = 0; i < images.size(); ++i)
    {
        imshow("src", images[i]);
        imshow("warped", images_warped[i]);
        imshow("masks_warped", masks_warped[i]);
        waitKey();
    }

    //bool flag7 = compositing(warper_creator, warper, img_names, masks_warped, cameras, full_img_sizes, sizes, corners, is_compose_scale_set, compose_scale, work_scale, warped_image_scale);
    // 5 warp images

    LOGLN("Compositing...");
    int64 t = getTickCount();

    Mat img_warped, img_warped_s;
    Mat dilated_mask, seam_mask, mask, mask_warped;
    Ptr<Blender> blender;
    Ptr<Timelapser> timelapser;
    //double compose_seam_aspect = 1;
    double compose_work_aspect = 1;

    for (int img_idx = 0; img_idx < num_images; ++img_idx)
    {
        LOGLN("Compositing image #" << indices[img_idx]+1);

        // Read image and resize it if necessary
        full_img = imread(img_names[img_idx]);
        if (!is_compose_scale_set)
        {
            if (compose_megapix > 0)
                compose_scale = min(1.0, sqrt(compose_megapix * 1e6 / full_img.size().area()));
            is_compose_scale_set = true;

            // Compute relative scales
            //compose_seam_aspect = compose_scale / seam_scale;
            compose_work_aspect = compose_scale / work_scale;

            // Update warped image scale
            warped_image_scale *= static_cast<float>(compose_work_aspect);
            warper = warper_creator->create(warped_image_scale);

            // Update corners and sizes
            for (int i = 0; i < num_images; ++i)
            {
                // Update intrinsics
                cameras[i].focal *= compose_work_aspect;
                cameras[i].ppx *= compose_work_aspect;
                cameras[i].ppy *= compose_work_aspect;

                // Update corner and size
                Size sz = full_img_sizes[i];
                if (std::abs(compose_scale - 1) > 1e-1)
                {
                    sz.width = cvRound(full_img_sizes[i].width * compose_scale);
                    sz.height = cvRound(full_img_sizes[i].height * compose_scale);
                }

                Mat K;
                cameras[i].K().convertTo(K, CV_32F);
                Rect roi = warper->warpRoi(sz, K, cameras[i].R);
                corners[i] = roi.tl();
                sizes[i] = roi.size();
            }
        }
        if (abs(compose_scale - 1) > 1e-1)
            resize(full_img, img, Size(), compose_scale, compose_scale);
        else
            img = full_img;
        full_img.release();
        Size img_size = img.size();

        Mat K;
        cameras[img_idx].K().convertTo(K, CV_32F);

        // Warp the current image
        warper->warp(img, K, cameras[img_idx].R, INTER_LINEAR, BORDER_REFLECT, img_warped);

        // Warp the current image mask
        mask.create(img_size, CV_8U);
        mask.setTo(Scalar::all(255));
        warper->warp(mask, K, cameras[img_idx].R, INTER_NEAREST, BORDER_CONSTANT, mask_warped);

        // Compensate exposure
        compensator->apply(img_idx, corners[img_idx], img_warped, mask_warped);

        img_warped.convertTo(img_warped_s, CV_16S);
        img_warped.release();
        img.release();
        mask.release();

        dilate(masks_warped[img_idx], dilated_mask, Mat());
        resize(dilated_mask, seam_mask, mask_warped.size());
        mask_warped = seam_mask & mask_warped;

        if (!blender && !timelapse)
        {
            blender = Blender::createDefault(blend_type, try_cuda);
            Size dst_sz = resultRoi(corners, sizes).size();
            float blend_width = sqrt(static_cast<float>(dst_sz.area())) * blend_strength / 100.f;
            if (blend_width < 1.f)
                blender = Blender::createDefault(Blender::NO, try_cuda);
            else if (blend_type == Blender::MULTI_BAND)
            {
                MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(blender.get());
                mb->setNumBands(static_cast<int>(ceil(log(blend_width)/log(2.)) - 1.));
                LOGLN("Multi-band blender, number of bands: " << mb->numBands());
            }
            else if (blend_type == Blender::FEATHER)
            {
                FeatherBlender* fb = dynamic_cast<FeatherBlender*>(blender.get());
                fb->setSharpness(1.f/blend_width);
                LOGLN("Feather blender, sharpness: " << fb->sharpness());
            }
            blender->prepare(corners, sizes);
        }
        else if (!timelapser && timelapse)
        {
            timelapser = Timelapser::createDefault(timelapse_type);
            timelapser->initialize(corners, sizes);
        }

        // Blend the current image
        if (timelapse)
        {
            timelapser->process(img_warped_s, Mat::ones(img_warped_s.size(), CV_8UC1), corners[img_idx]);
            String fixedFileName;
            size_t pos_s = String(img_names[img_idx]).find_last_of("/\\");
            if (pos_s == String::npos)
            {
                fixedFileName = "fixed_" + img_names[img_idx];
            }
            else
            {
                fixedFileName = "fixed_" + String(img_names[img_idx]).substr(pos_s + 1, String(img_names[img_idx]).length() - pos_s);
            }
            imwrite(fixedFileName, timelapser->getDst());
        }
        else
        {
            blender->feed(img_warped_s, mask_warped, corners[img_idx]);
        }
    }

    if (!timelapse)
    {
        Mat result, result_mask;
        blender->blend(result, result_mask);

        LOGLN("Compositing, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");

        imwrite(result_name, result);
    }

    //LOGLN("Finished, total time: " << ((getTickCount() - app_start_time) / getTickFrequency()) << " sec");
    return 0;

}

bool load_find_features(vector<String> &img_names, vector<ImageFeatures> &features, vector<Mat> &images, vector<Size> & full_img_sizes, double *work_scale, double *seam_scale, bool &is_work_scale_set, bool &is_seam_scale_set)
{
    LOGLN("Finding features...");
    int64 t = getTickCount();

    Ptr<FeaturesFinder> finder;
    if(features_type == "surf")
    {
        finder = makePtr<SurfFeaturesFinder>();
    }
    else if(features_type == "orb")
    {
        finder = makePtr<OrbFeaturesFinder>();
    }
    else
    {
        cout<<"Unknown 2D features type: '"<<features_type<<"'.\n";
        return -1;
    }

    Mat full_img, img;
    double seam_work_aspect = 1;
    int num_images = static_cast<int>(img_names.size());
    for(int i = 0; i < num_images; ++i)
    {
        full_img = imread(img_names[i]);
        full_img_sizes[i] = full_img.size();
        if( full_img.empty() )
        {
            LOGLN("Can't open image "<< img_names[i]);
            return -1;
        }
        if(work_megapix < 0)
        {
            img = full_img;
            *work_scale = 1;
            is_work_scale_set = true;
        }
        else
        {
            if( !is_work_scale_set )
            {
                *work_scale = min(1.0, sqrt(work_megapix * 1e6 / full_img.size().area()));
                is_work_scale_set = true;
            }
            resize(full_img, img, Size(), *work_scale, *work_scale);
        }
        if( !is_seam_scale_set )
        {
            *seam_scale = min(1.0, sqrt(seam_megapix * 1e6 / full_img.size().area()));
            seam_work_aspect = *seam_scale / *work_scale;
            is_seam_scale_set = true;
        }

        (*finder)(img, features[i]);
        features[i].img_idx = i;
        LOGLN("Features in image #" << i+1 << ": " << features[i].keypoints.size());
        resize(full_img, img, Size(), *seam_scale, *seam_scale);
        images[i] = img.clone(); /// save image after seam_scale
    }
    // Find feature done;
    finder->collectGarbage();
    full_img.release();
    img.release();
    
    LOGLN("Finding features, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");
    return true;
}
bool load_find_features(vector<String> &img_names, vector<ImageFeatures> &features, vector<Mat> &images, vector<Size> & full_img_sizes)
{
    LOGLN("Finding features...");
    int64 t = getTickCount();
    Ptr<FeaturesFinder> finder;
    if(features_type == "orb") finder = makePtr<OrbFeaturesFinder>();
    else finder = makePtr<SurfFeaturesFinder>(); //"surf"
    
    Mat full_img, img;
    //double seam_work_aspect = 1;

    for(size_t i = 0; i < img_names.size(); ++i)
    {
        full_img = imread(img_names[i]);
        if( full_img.empty() )
        {
            LOGLN("Can't open image "<< img_names[i]);
            return false;
        }
        full_img_sizes[i] = full_img.size();
        img = full_img;

        (*finder)(img, features[i]);
        features[i].img_idx = i;
        LOGLN("Features in image #" << i+1 << ": " << features[i].keypoints.size());
        //resize(full_img, img, Size(), seam_scale, seam_scale);
        images[i] = img.clone(); /// save image after seam_scale
    }
    // Find feature done;
    finder->collectGarbage();
    full_img.release();
    img.release();
    
    LOGLN("Finding features, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");
    return true;
}
bool get_pairwise_matches(vector<ImageFeatures> &features, vector<MatchesInfo> &pairwise_matches)
{
    int64 t = getTickCount();
    //vector<MatchesInfo> pairwise_matches;
    Ptr<FeaturesMatcher> matcher;
    if (matcher_type == "affine")
        matcher = makePtr<AffineBestOf2NearestMatcher>(false, try_cuda, match_conf);
    else if (range_width==-1)
        matcher = makePtr<BestOf2NearestMatcher>(try_cuda, match_conf);
    else
        matcher = makePtr<BestOf2NearestRangeMatcher>(range_width, try_cuda, match_conf);

    (*matcher)(features, pairwise_matches);
    matcher->collectGarbage();

    LOGLN("Pairwise matching, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec; pairwise_matches.size() = "<<pairwise_matches.size());
    return true;
}
bool estimate_adjuster_cameras(vector<ImageFeatures> &features, vector<MatchesInfo> &pairwise_matches, vector<CameraParams> &cameras, vector<double> &focals, float *warped_image_scale)
{
    // 4 Estimate Camera
    // Estimate
    Ptr<Estimator> estimator;
    if(estimator_type == "affine")
        estimator = makePtr<AffineBasedEstimator>();
    else
        estimator = makePtr<HomographyBasedEstimator>();

    //vector<CameraParams> cameras;
    if( !(*estimator)(features, pairwise_matches, cameras) )
    {
        cout<<"Homography estimation failed.\n";
        return -1;
    }
    for(size_t i = 0; i < cameras.size(); ++i)
    {
        Mat R;
        cameras[i].R.convertTo(R, CV_32F);
        cameras[i].R = R;
        //LOGLN("Initial camera intrinsics #" << indices[i]+1 << ":\nK:\n" << cameras[i].K() << "\nR:\n" << cameras[i].R);
        LOGLN("Initial camera intrinsics #" << i+1 << ":\nK:\n" << cameras[i].K() << "\nR:\n" << cameras[i].R);
    }

    // 5 adjuster the cameras
    Ptr<detail::BundleAdjusterBase> adjuster;
    if (ba_cost_func == "reproj") adjuster = makePtr<detail::BundleAdjusterReproj>();
    else if (ba_cost_func == "ray") adjuster = makePtr<detail::BundleAdjusterRay>();
    else if (ba_cost_func == "affine") adjuster = makePtr<detail::BundleAdjusterAffinePartial>();
    else if (ba_cost_func == "no") adjuster = makePtr<NoBundleAdjuster>();
    else
    {
        cout << "Unknown bundle adjustment cost function: '" << ba_cost_func << "'.\n";
        return -1;
    }
    adjuster->setConfThresh(conf_thresh);
    Mat_<uchar> refine_mask = Mat::zeros(3, 3, CV_8U);
    if (ba_refine_mask[0] == 'x') refine_mask(0,0) = 1;
    if (ba_refine_mask[1] == 'x') refine_mask(0,1) = 1;
    if (ba_refine_mask[2] == 'x') refine_mask(0,2) = 1;
    if (ba_refine_mask[3] == 'x') refine_mask(1,1) = 1;
    if (ba_refine_mask[4] == 'x') refine_mask(1,2) = 1;
    adjuster->setRefinementMask(refine_mask);
    if (!(*adjuster)(features, pairwise_matches, cameras))
    {
        cout << "Camera parameters adjusting failed.\n";
        return -1;
    }

    // Find median focal length

    //vector<double> focals;
    for (size_t i = 0; i < cameras.size(); ++i)
    {
        //LOGLN("Ajuster Camera #" << indices[i]+1 << ":\nK:\n" << cameras[i].K() << "\nR:\n" << cameras[i].R);
        LOGLN("Ajuster Camera #" << i+1 << ":\nK:\n" << cameras[i].K() << "\nR:\n" << cameras[i].R);
        focals.push_back(cameras[i].focal);
    }

    sort(focals.begin(), focals.end());
    //float warped_image_scale;
    if (focals.size() % 2 == 1)
        *warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
    else
        *warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;

    if (do_wave_correct)
    {
        vector<Mat> rmats;
        for (size_t i = 0; i < cameras.size(); ++i)
            rmats.push_back(cameras[i].R.clone());
        waveCorrect(rmats, wave_correct);
        for (size_t i = 0; i < cameras.size(); ++i)
            cameras[i].R = rmats[i];
    }
    return true;
}
bool generate_warp_creator(Ptr<WarperCreator> &warper_creator, string warp_type )
{
    // Warp images and their masks
    //Ptr<WarperCreator> warper_creator;
    if (warp_type == "plane")
        warper_creator = makePtr<cv::PlaneWarper>();
    else if (warp_type == "affine")
        warper_creator = makePtr<cv::AffineWarper>();
    else if (warp_type == "cylindrical")
        warper_creator = makePtr<cv::CylindricalWarper>();
    else if (warp_type == "spherical")
        warper_creator = makePtr<cv::SphericalWarper>();
    else if (warp_type == "fisheye")
        warper_creator = makePtr<cv::FisheyeWarper>();
    else if (warp_type == "stereographic")
        warper_creator = makePtr<cv::StereographicWarper>();
    else if (warp_type == "compressedPlaneA2B1")
        warper_creator = makePtr<cv::CompressedRectilinearWarper>(2.0f, 1.0f);
    else if (warp_type == "compressedPlaneA1.5B1")
        warper_creator = makePtr<cv::CompressedRectilinearWarper>(1.5f, 1.0f);
    else if (warp_type == "compressedPlanePortraitA2B1")
        warper_creator = makePtr<cv::CompressedRectilinearPortraitWarper>(2.0f, 1.0f);
    else if (warp_type == "compressedPlanePortraitA1.5B1")
        warper_creator = makePtr<cv::CompressedRectilinearPortraitWarper>(1.5f, 1.0f);
    else if (warp_type == "paniniA2B1")
        warper_creator = makePtr<cv::PaniniWarper>(2.0f, 1.0f);
    else if (warp_type == "paniniA1.5B1")
        warper_creator = makePtr<cv::PaniniWarper>(1.5f, 1.0f);
    else if (warp_type == "paniniPortraitA2B1")
        warper_creator = makePtr<cv::PaniniPortraitWarper>(2.0f, 1.0f);
    else if (warp_type == "paniniPortraitA1.5B1")
        warper_creator = makePtr<cv::PaniniPortraitWarper>(1.5f, 1.0f);
    else if (warp_type == "mercator")
        warper_creator = makePtr<cv::MercatorWarper>();
    else if (warp_type == "transverseMercator")
        warper_creator = makePtr<cv::TransverseMercatorWarper>();
    if (!warper_creator)
    {
        cout << "Can't create the following warper '" << warp_type << "'\n";
        return false;
    }
    return true;
}
bool warp_images(Ptr<WarperCreator> &warper_creator, vector<Mat> &images, vector<CameraParams> &cameras, float warped_image_scale, double seam_work_aspect,
        vector<Point> &corners, vector<UMat> &masks_warped, vector<UMat> &images_warped, vector<Size> &sizes, Ptr<RotationWarper> &warper)
{
    int num_images = static_cast<int>(images.size());
    LOGLN("Warping images (auxiliary)...");
    int64 t = getTickCount();

    //vector<Point> corners(num_images);
    //vector<UMat> masks_warped(num_images);
    //vector<UMat> images_warped(num_images);
    //vector<Size> sizes(num_images);
    vector<UMat> masks(num_images);

    // Preapre images masks
    for (int i = 0; i < num_images; ++i)
    {
        masks[i].create(images[i].size(), CV_8U);
        masks[i].setTo(Scalar::all(255));
    }

    //Ptr<RotationWarper> warper = warper_creator->create(static_cast<float>(warped_image_scale * seam_work_aspect));
    warper = warper_creator->create(static_cast<float>(warped_image_scale * seam_work_aspect));

    for(int i = 0; i < num_images; ++i)
    {
        Mat_<float> K;
        cameras[i].K().convertTo(K, CV_32F);
        float swa = (float)seam_work_aspect;
        K(0, 0) *= swa; K(0, 2) *= swa;
        K(1, 1) *= swa; K(1, 2) *= swa;
        
        corners[i] = warper->warp(images[i], K, cameras[i].R, INTER_LINEAR, BORDER_REFLECT, images_warped[i]);
        sizes[i] = images_warped[i].size();

        warper->warp(masks[i], K, cameras[i].R, INTER_NEAREST, BORDER_CONSTANT, masks_warped[i]);
    }
    LOGLN("Warping images, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");
    return true;
}

bool find_seam(Ptr<ExposureCompensator> &compensator, vector<UMat> &images_warped, vector<UMat> &masks_warped, vector<Point> &corners)
{
    int num_images = static_cast<int> (images_warped.size());
    vector<UMat> images_warped_f(num_images);
    for(int i = 0; i < num_images; ++i)
        images_warped[i].convertTo(images_warped_f[i], CV_32F);
    //Ptr<ExposureCompensator> compensator = ExposureCompensator::createDefault(expos_comp_type);
    compensator->feed(corners, images_warped, masks_warped);

    Ptr<SeamFinder> seam_finder;
    if(seam_find_type == "no")
        seam_finder = makePtr<detail::NoSeamFinder>();
    else if(seam_find_type == "voronoi")
        seam_finder = makePtr<detail::VoronoiSeamFinder>();
    else if(seam_find_type == "gc_color")
        seam_finder = makePtr<detail::GraphCutSeamFinder>(GraphCutSeamFinderBase::COST_COLOR);
    else if(seam_find_type == "gc_colorgrad")
        seam_finder = makePtr<detail::GraphCutSeamFinder>(GraphCutSeamFinderBase::COST_COLOR_GRAD);
    else if(seam_find_type == "dp_color")
        seam_finder = makePtr<detail::DpSeamFinder>(DpSeamFinder::COLOR);
    else if(seam_find_type == "dp_colorgrad")
        seam_finder = makePtr<detail::DpSeamFinder>(DpSeamFinder::COLOR_GRAD);

    if (!seam_finder)
    {
        cout << "Can't create the following seam finder '" << seam_find_type << "'\n";
        return 1;
    }

    seam_finder->find(images_warped_f, corners, masks_warped);
    /*
    for(int i = 0; i < masks_warped.size(); ++i) {
        imshow("masks_warped", masks_warped[i]);
        imshow("images_warped", images_warped[i]);
        waitKey();
    }
    return 0;
    */
    // Release unused memory
    //images.clear();
    //images_warped.clear();
    //images_warped_f.clear();
    //masks.clear();
    return true;
}
