#ifndef HOMOGRAPHY_TEST_H
#define HOMOGRAPHY_TEST_H

#include "ms_image_struct.h"

//-----------------------------------------------------------------------------
//typedef struct image_struct
//{
//    double* image;
//
//    unsigned int img_w;
//    unsigned int img_h;
//
//} image_struct;
//
////-----------------------------------------------------------------------------
//inline image_struct init_image_struct(double* image, unsigned int img_w, unsigned int img_h)
//{
//    image_struct tmp;
//    tmp.image = image;
//    tmp.img_w = img_w;
//    tmp.img_h = img_h;
//
//    return tmp;
//}


//-----------------------------------------------------------------------------
// homography library specific functions
//typedef void (*image_fuser)(unsigned int num_images, ms_image* img, double* fused_data64_t, unsigned char* fused_data8_t, unsigned int img_w, unsigned int img_h);

typedef void (*lib_transform_single_image)(ms_image r_img,
    ms_image t_img,
    double* fused_data64_t,
    unsigned char* fused_data8_t,
    unsigned int& img_w,
    unsigned int& img_h
);

typedef void (*lib_transform_multi_image)(uint32_t N,
    ms_image r_img,
    ms_image* t_img,
    double* fused_data64_t,
    unsigned char* fused_data8_t,
    unsigned int img_w,
    unsigned int img_h
);

#endif  // HOMOGRAPHY_TEST_H
