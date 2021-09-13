#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <string>
#include <iostream>

// OpenCV includes
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// Custom includes
#include "fusion_lib.h"

//----------------------------------------------------------------------------------
// library internal global/state variables:

//----------------------------------------------------------------------------------
void image_fuser(unsigned int num_images, ms_image* img, double* fused_data64_t, unsigned char* fused_data8_t, unsigned int img_w, unsigned int img_h)
{
    unsigned int idx;

    // assign the fused data pointer to an opencv container
    cv::Mat fused_img = cv::Mat(img_h, img_w, CV_64FC1, fused_data64_t);
    cv::Mat fused_img8_t = cv::Mat(img_h, img_w, CV_8UC1, fused_data8_t);
    cv::Mat tmp_img;

    for (idx = 0; idx < num_images; ++idx)
    {
        if (img[idx].use_img)
        {
            // get the pointer data into a cv::Mat container
            tmp_img = cv::Mat(img[idx].img_h, img[idx].img_w, CV_64FC1, img[idx].image);

            // center crop the image to fit the desired output image size
            if ((img_w < img[idx].img_w) || (img_h < img[idx].img_h))
            {
                cv::Rect roi((img[idx].img_w - img_w) >> 1, (img[idx].img_h - img_h) >> 1, img_h, img_w);
                tmp_img = tmp_img(roi);
            }

            // add the weighted image to the existing fused images
            fused_img = fused_img + img[idx].weight * (img[idx].invert_img ? (1.0 - tmp_img) : tmp_img);
          }
    }   // end of for loop

    fused_img.convertTo(fused_img8_t, CV_8UC1, 255.0, 0.0);

}   // end of image_fuser
