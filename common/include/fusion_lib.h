#ifndef MS_FUSION_LIB_H
#define MS_FUSION_LIB_H

#include "ms_image_struct.h"

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

    #ifdef LIB_EXPORTS
        #define MS_FUSION_LIB __declspec(dllexport)
    #else
        #define MS_FUSION_LIB __declspec(dllimport)
    #endif

#else
    #define MS_FUSION_LIB

#endif

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    MS_FUSION_LIB void image_fuser(unsigned int num_images, ms_image* img, double* fused_data64_t, unsigned char* fused_data8_t, unsigned int img_w, unsigned int img_h);
#ifdef __cplusplus
}
#endif


#endif  // MS_FUSION_LIB_H
