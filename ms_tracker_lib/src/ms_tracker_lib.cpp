#define _CRT_SECURE_NO_WARNINGS

#include <cstdint>
#include <string>
#include <iostream>

// OpenCV includes
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// Custom includes
#include "ms_tracker_lib.h"
#include "ms_tracker.h"

//----------------------------------------------------------------------------------
// library internal global/state variables:
ms_tracker trckr;

//----------------------------------------------------------------------------------
void create_tracker(int tracker_type)
{
    trckr = ms_tracker(tracker_type);
}

//----------------------------------------------------------------------------------
void init_tracker(unsigned char *img, int h, int w, int c, target_rect *target)
{

    trckr.add_target(*target);
    trckr.init(img, h, w, c, *target);

}

//----------------------------------------------------------------------------------
void update_tracker(unsigned char* img, int h, int w, int c, target_rect *target)
{
    trckr.track_object(img, h, w, c);
    *target = trckr.get_target();
}


//----------------------------------------------------------------------------------
bool tracker_status(void)
{
    return trckr.tracking;
}
