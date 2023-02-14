#ifndef BLOB_DETECTOR_LIB_H_
#define BLOB_DETECTOR_LIB_H_


#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

    #ifdef BUILD_LIB
        #ifdef LIB_EXPORTS
            #define BLOB_DETECTOR_LIB __declspec(dllexport)
        #else
            #define BLOB_DETECTOR_LIB __declspec(dllimport)
        #endif
    #else
        #define BLOB_DETECTOR_LIB
    #endif

#else
    #define BLOB_DETECTOR_LIB

#endif

//#include "ms_image_struct.h"

// ----------------------------------------------------------------------------------------
typedef struct detection_struct
{
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

    char name[256];

    double score;

    unsigned int class_id;

    detection_struct()
    {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
        score = 0.0;
        class_id = 0;
        name[0] = 0;
    }

    detection_struct(unsigned int x_, unsigned int y_, unsigned int w_, unsigned int h_, const char name_[], double s_, unsigned int c_id)
    {
        x = x_;
        y = y_;
        w = w_;
        h = h_;
        score = s_;
        class_id = c_id;
        strcpy(name, name_);
    }

} detection_struct;

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

    //-----------------------------------------------------------------------------
    BLOB_DETECTOR_LIB void blob_detector(unsigned int img_w,
        unsigned int img_h, 
        unsigned int img_c, 
        uint8_t* img_t, 
        double threshold,
        unsigned int *num_dets,
        detection_struct*& dets
    );

#ifdef __cplusplus
}
#endif

#endif  // BLOB_DETECTOR_LIB_H_
