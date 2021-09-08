#ifndef MS_FUSION_LIB_H
#define MS_FUSION_LIB_H


#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

    #ifdef LIB_EXPORTS
        #define MS_FUSION_LIB __declspec(dllexport)
    #else
        #define MS_FUSION_LIB __declspec(dllimport)
    #endif

#else
    #define MS_FUSION_LIB

#endif

// ----------------------------------------------------------------------------
typedef struct ms_image
{
    double *image;
    unsigned int img_w;
    unsigned int img_h;
    
    bool use_img;
    bool invert_img;
    
    double weight;
    
} ms_image;


// ----------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    MS_FUSION_LIB void image_fuser(unsigned int num, ms_image *&img, double *fused_img, unsigned int img_w, unsigned int img_h);
#ifdef __cplusplus
}
#endif



#endif  // MS_FUSION_LIB_H
