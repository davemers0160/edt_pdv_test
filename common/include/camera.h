/*
* Copyright 2021 Amy Wagoner, NSWC Crane
* 
* TODO: Add licensing statement. 
*/

// Stores values for each camera

#ifndef INCLUDED_CAMERA_H
#define INCLUDED_CAMERA_H

#include <cstdint>
#include <string>
//#include <list>
//#include <algorithm>
//#include <iostream>

//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/features2d.hpp>
//#include <opencv2/opencv.hpp>
//#include <opencv2/tracking.hpp>
//#include <opencv2/core/ocl.hpp>

//#include "image_proc.h"

//using namespace cv;

class Camera
{
public: 

    std::string name;
    //bool has_detection;
    //bool has_target;
    //std::vector<cv::KeyPoint> keypoints;

    uint32_t height, width; // frame height and width
    //cv::Rect2d roi;
    //int lost_track; // how many frames camera has lost track
        
    // Configurable for each camera
    //int max_lost; // keeps track of how many frames camera unable to track object before starting over with detect 
    //int roi_margin; // Margin (in pixels) for bounding box of object

    // ----------------------------------------------------------------------------
    Camera() = default;

    Camera(std::string cam_name) : name(cam_name) 
    {
        height = 0;
        width = 0;
        pitch_x = 0.0;
        pitch_y = 0.0;
        integration_time = 0.0;
        blending_weight = 0.5;
        frame_rate = 30.0;
    }

    Camera(std::string cam_name, uint32_t h, uint32_t w) : name(cam_name), height(h), width(w)
    {
        pitch_x = 0.0;
        pitch_y = 0.0;
        integration_time = 0.0;
        blending_weight = 0.5;
        frame_rate = 30.0;
    }

    ~Camera()
    {

    }

        
    // ----------------------------------------------------------------------------
    std::string get_name() { return name; }

    // ----------------------------------------------------------------------------
    void set_pitch(double x, double y)
    {
        pitch_x = x;
        pitch_y = y;
    }

    double get_x_pitch() { return pitch_x; }
    double get_y_pitch() { return pitch_y; }

    // ----------------------------------------------------------------------------
    void set_integration_time(double it)
    {
        integration_time = it;
    }

    double get_integration_time() { return integration_time; }

    // ----------------------------------------------------------------------------
    void set_blending_weight(double w) // Sets the weight for image blending - must be between 0-1. 
    {
        blending_weight = w;
    }

    double get_blending_weight() { return blending_weight; }

    // ----------------------------------------------------------------------------
    void set_frame_rate(double fr)
    {
        frame_rate = fr;
    }

    double get_frame_rate() { return frame_rate; }

    // ----------------------------------------------------------------------------


    //void set_blending_images(std::vector<Camera*> *cams);
    //void blend();
    //
        
    //template<typename img_type1, typename img_type2>
    //void detect(cv::Mat& img, cv::Mat& img_with_keypoints);

    ////template<typename img_type1>
    //void start_tracker(cv::Mat& img);

    ////template<typename img_type1>
    //bool update_tracker(cv::Mat& img);

private:
    //image_proc img_prc; 
    //double mean_intensity; 

    // Configurable for each camera
    double pitch_x, pitch_y;
    double integration_time; 
    double frame_rate;
    //double cam_zoom, cam_focus;
    double blending_weight;     // weight used for blending images - could use logistic regression or NN to determine 
        
    //void convert_to_8bit(cv::Mat& img);
        
};

#endif
