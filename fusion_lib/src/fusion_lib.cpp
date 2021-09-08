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
void image_fuser(unsigned int num_images, ms_image *&img, double *fused_img, unsigned int img_w, unsigned int img_h)
{
    unsigned int idx;
    
    // apply scale and inversion to images
    
    cv::Mat img = cv::Mat(*img_h, *img_w, CV_64FC1, fused_img);
    
    for (idx = 0; idx < num_images; ++idx)
    {
        if (img[idx]->use_image)
            img = img + img[idx]->weight * (img[idx]->invert ? (1.0 - img[idx]->image) : img[idx]->image);

    }
    
}   // end of image_fuser

