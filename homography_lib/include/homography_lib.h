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


//-----------------------------------------------------------------------------
typedef struct image_struct
{
    double* image;

    unsigned int img_w;
    unsigned int img_h;

} image_struct;

//-----------------------------------------------------------------------------
inline image_struct init_image_struct(double* image, unsigned int img_w, unsigned int img_h, bool use_img, bool invert_img, double weight, double scale, bool scale_img)
{
    image_struct tmp;
    tmp.image = image;
    tmp.img_w = img_w;
    tmp.img_h = img_h;

    return tmp;
}


//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    HOMOGRAPHY_LIB void generate_h_matrix(image_struct* ref_img_t, image_struct* img_t, double* h_t);
#ifdef __cplusplus
}
#endif


#endif  // HOMOGRAPHY_LIB_H
