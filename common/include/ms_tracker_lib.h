#ifndef MS_TRACKER_LIB_H
#define MS_TRACKER_LIB_H

#include "target_rect.h"

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

    #ifdef LIB_EXPORTS
        #define MS_TRACKER_LIB __declspec(dllexport)
    #else
        #define MS_TRACKER_LIB __declspec(dllimport)
    #endif

#else
    #define MS_TRACKER_LIB

#endif

// ----------------------------------------------------------------------------
/**
Tracker variables

These are the IDs that can be used to select various tracker types.
*/
#if !defined(LIB_EXPORTS)
enum tracker_types
{
    BOOSTING,
    MIL,
    KCF,
    TLD,
    MEDIANFLOW,
    GOTURN,
    MOSSE,
    CSRT
};
#endif


// ----------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    MS_TRACKER_LIB void create_tracker(int tracker_type);
#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    MS_TRACKER_LIB void init_tracker(unsigned char* img, int h, int w, int c, target_rect *target);
#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    MS_TRACKER_LIB void update_tracker(unsigned char* img, int h, int w, int c, target_rect *target);
#ifdef __cplusplus
}
#endif

// ----------------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
    // 
    MS_TRACKER_LIB bool tracker_status(void);
#ifdef __cplusplus
}
#endif

#endif  // MS_TRACKER_LIB_H
