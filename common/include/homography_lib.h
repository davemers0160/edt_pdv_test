#ifndef HOMOGRAPHY_LIB_H
#define HOMOGRAPHY_LIB_H


#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

    #ifdef BUILD_LIB
        #ifdef LIB_EXPORTS
            #define HOMOGRAPHY_LIB __declspec(dllexport)
        #else
            #define HOMOGRAPHY_LIB __declspec(dllimport)
        #endif
    #else
        #define HOMOGRAPHY_LIB
    #endif

#else
    #define HOMOGRAPHY_LIB

#endif

#include "ms_image_struct.h"
#include "target_rect.h"

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
#ifdef __cplusplus
extern "C" {
#endif

    //-----------------------------------------------------------------------------
    // take a double image and scale between 0 and 1
    HOMOGRAPHY_LIB void normalize_img(unsigned int img_w, unsigned int img_h, double* img_t, double* norm_img_t);

    //-----------------------------------------------------------------------------
    // 
    HOMOGRAPHY_LIB void transform_single_image(ms_image r_img,
        ms_image t_img,
        double* fused_data64_t,
        unsigned char* fused_data8_t
    );

    //-----------------------------------------------------------------------------
    // 
    HOMOGRAPHY_LIB void transform_multi_image(uint32_t N,
        ms_image r_img,
        ms_image* t_img,
        double* fused_data64_t,
        unsigned char* fused_data8_t
    );

    //-----------------------------------------------------------------------------
    HOMOGRAPHY_LIB void transform_multi_image_rect(uint32_t N,
        ms_image r_img,
        target_rect r_rect,
        ms_image* imgs,
        target_rect* img_rects,
        double* fused_data64_t,
        unsigned char* fused_data8_t,
        bool tight_box
    );




#ifdef __cplusplus
}
#endif

#endif  // HOMOGRAPHY_LIB_H
