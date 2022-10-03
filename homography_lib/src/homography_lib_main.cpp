#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>

#include "homography_lib.h"
//#include "img_registration.h"
#include "ms_image_struct.h"
#include "homography_class.h"

//std::atomic<bool> homography_complete = false;
//cv::Point2f initial_point, final_point;


//-----------------------------------------------------------------------------
void transform_single_image(ms_image r_img, 
    ms_image t_img,
    double* fused_data64_t,
    unsigned char* fused_data8_t,
    unsigned int &img_w,
    unsigned int &img_h
)
{
    cv::Mat ref_img_c, img_c, tmp_img;

    // get the images into the opencv containers
    cv::Mat ref_img = cv::Mat(r_img.img_h, r_img.img_w, CV_64FC1, r_img.image);
    cv::Mat img = cv::Mat(t_img.img_h, t_img.img_w, CV_64FC1, t_img.image);

    // assign the fused data pointer to an opencv container
    img_h = r_img.img_h;
    img_w = r_img.img_w;
    cv::Mat fused_img = cv::Mat(img_h, img_w, CV_64FC1, fused_data64_t);
    cv::Mat fused_img8_t = cv::Mat(img_h, img_w, CV_8UC1, fused_data8_t);

    // create a homography class for each image
    homography ref_h(r_img.threshold);
    homography t_img_h(t_img.threshold);

    // get the bounding box for the reference image
    ref_h.get_bounding_box(ref_img, ref_img_c, r_img.invert_img);

    // get the bounding box for the input image
    t_img_h.get_bounding_box(img, img_c, t_img.invert_img);

    t_img_h.calc_homography_matrix(ref_h.get_rect());

    t_img_h.transform_image(img, tmp_img, ref_img.size());

    // check for needed scaling
    if (t_img.scale_img)
        tmp_img *= t_img.scale;

    // add the weighted image to the existing fused images
    fused_img = fused_img + t_img.weight * (t_img.invert_img ? (1.0 - tmp_img) : tmp_img);

    fused_img.convertTo(fused_img8_t, CV_8UC1, 255.0, 0.0);

}   // end of transform_single_image

//-----------------------------------------------------------------------------
void transform_multi_image(uint32_t N, 
    ms_image r_img, 
    ms_image *t_img, 
    double* fused_data64_t, 
    unsigned char* fused_data8_t, 
    unsigned int img_w, 
    unsigned int img_h
)
{
    uint32_t idx;

    cv::Mat tmp_img, tmp_img_c;
    std::vector<homography> t_img_h(N);

    // get the images into the opencv containers
    cv::Mat ref_img = cv::Mat(r_img.img_h, r_img.img_w, CV_64FC1, r_img.image);

    // assign the fused data pointer to an opencv container
    img_h = r_img.img_h;
    img_w = r_img.img_w;
    cv::Mat fused_img = cv::Mat(img_h, img_w, CV_64FC1, fused_data64_t);
    cv::Mat fused_img8_t = cv::Mat(img_h, img_w, CV_8UC1, fused_data8_t);

    // create a homography class for the reference image
    homography ref_h(r_img.threshold);

    fused_img = fused_img + r_img.weight * (r_img.invert_img ? (1.0 - ref_img) : ref_img);

    for (idx = 0; idx < N; ++idx)
    {
        if (t_img[idx].use_img)
        {
            // get the pointer data into a cv::Mat container
            tmp_img = cv::Mat(t_img[idx].img_h, t_img[idx].img_w, CV_64FC1, t_img[idx].image);
            
            t_img_h[idx].threshold = t_img[idx].threshold;

            t_img_h[idx].get_bounding_box(tmp_img, tmp_img_c, t_img[idx].invert_img);

            t_img_h[idx].calc_homography_matrix(ref_h.get_rect());

            t_img_h[idx].transform_image(tmp_img, tmp_img, ref_img.size());

            // check for needed scaling
            if (t_img[idx].scale_img)
                tmp_img *= t_img[idx].scale;

            // add the weighted image to the existing fused images
            fused_img = fused_img + t_img[idx].weight * (t_img[idx].invert_img ? (1.0 - tmp_img) : tmp_img);

        }
    }   // end of for loop

    fused_img.convertTo(fused_img8_t, CV_8UC1, 255.0, 0.0);

}   // end of transform_multi_image
