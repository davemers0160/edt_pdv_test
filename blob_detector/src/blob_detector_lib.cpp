#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <atomic>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>

//#include "target_rect.h"
#include "blob_detector_lib.h"

// ----------------------------------------------------------------------------
const cv::Mat SE3_rect = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
const cv::Mat SE5_rect = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

static cv::Rect previous_rect;
static double bb_iou_threshold = 0.8;
static double alpha = 0.6;

// ----------------------------------------------------------------------------
template <typename T>
static inline bool max_vector_size(const std::vector<T>& lhs, const std::vector<T>& rhs)
{
    return lhs.size() < rhs.size();
}   // end of max_vector_size
    
// ----------------------------------------------------------------------------
inline double calc_iou(cv::Rect& r1, cv::Rect& r2)
{

    double intersection = (r1 & r2).area();
    double rect_union = (r1 | r2).area();

    return ((rect_union == 0) ? 0.0 : intersection / rect_union);

}   // end of calc_iou

// ----------------------------------------------------------------------------
inline void get_rect(std::vector<cv::Point>& p, cv::Rect& r)
{
    uint64_t idx, jdx;
    uint64_t min_x = ULLONG_MAX, min_y = ULLONG_MAX;
    uint64_t max_x = 0, max_y = 0;

    for (idx = 0; idx < p.size(); ++idx)
    {
        min_x = std::min(min_x, (uint64_t)p[idx].x);
        min_y = std::min(min_y, (uint64_t)p[idx].y);
        max_x = std::max(max_x, (uint64_t)p[idx].x);
        max_y = std::max(max_y, (uint64_t)p[idx].y);

    }

    r = cv::Rect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
}   // end of get_rect

//-----------------------------------------------------------------------------
void blob_detector(unsigned int img_w, unsigned int img_h, unsigned int img_c, uint8_t *img_t, double threshold, unsigned int *num_dets, detection_struct*& dets)
{
    uint32_t idx;
    double min_val, max_val;
    std::vector<std::vector<cv::Point> > img_contours;
    std::vector<cv::Vec4i> img_hr;
    cv::Mat img_pyr, img_grad, img2, img_blur, img_blur_abs;
    std::vector<cv::Rect> img_rect;
    std::string label;

    double max_iou = 0.0;
    double tmp_iou;

    cv::Mat img, converted_img;
    
    // check channel count
    if (img_c == 1)
    {
        img = cv::Mat(img_h, img_w, CV_8UC1, img_t);
        converted_img = img.clone();
    }
    else if (img_c == 3)
    {
        img = cv::Mat(img_h, img_w, CV_8UC3, img_t);
        cv::cvtColor(img, converted_img, CV_8UC1, cv::COLOR_RGB2GRAY);
    }
    else
    {
        std::cout << "Error, unsupported image channel number: " << img_c << std::endl;
        return;
    }

    cv::minMaxLoc(converted_img, &min_val, &max_val);
    converted_img.convertTo(converted_img, CV_8UC1, 255.0 / (max_val - min_val), -(255.0 * min_val) / (max_val - min_val));

    // calculate the image mean
    auto img_mean = cv::mean(converted_img)[0];

    // blur the image using a sigma == 1.0 with border reflection
    cv::GaussianBlur(converted_img, img_blur, cv::Size(0, 0), 1.0, 1.0, cv::BORDER_REFLECT_101);

    // calculate the absolute difference of the blurred image and the image mean
    cv::absdiff(img_blur, img_mean, img_blur_abs);

    // threshold the image: src(x,y) if src(x,y) > threshold, 0 otherwise
    cv::threshold(img_blur_abs, img2, threshold, 0, cv::THRESH_TOZERO);

    // perform some morphologies to remove additional noise and pull out an object of interest
    cv::morphologyEx(img2, img2, cv::MORPH_DILATE, SE3_rect);
    //cv::morphologyEx(img2, img2, cv::MORPH_TOPHAT, SE5_rect);
    cv::morphologyEx(img2, img2, cv::MORPH_CLOSE, SE5_rect);

    // find the contours of the remaining shapes
    cv::findContours(img2, img_contours, img_hr, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // check the number of contours found and figure out which one to use
    if (img_contours.size() == 1)
    {
        img_rect.resize(1);
        get_rect(img_contours[0], img_rect[0]);
    }
    else if (img_contours.size() > 1)
    {
        img_rect.resize(1); // img_rect.resize(img_contours.size())

        // TODO: find the rect with the highest IOU, if IOU doesn't meet a certain threshold then use previous_rect
        // TODO: figure out how to dampen a rapid change in rect size, look at IOU and scaling to slowly move towards the current rect
        // TODO: look at exponential weighted moving average or some other FIFO like thing with weights.  3-tap FIR
        get_rect(*std::max_element(img_contours.begin(), img_contours.end(), max_vector_size<cv::Point>), img_rect[0]);

        //for (idx = 0; idx < img_contours.size(); ++idx)
        //{
        //    tmp_iou = calc_iou(previous_rect, img_rect);
        //    
        //    if (tmp_iou > max_iou)
        //    {
        //        max_iou = tmp_iou;
        //        max_iou_index = idx;
        //    }
        //}

        //get_rect(img_contours[max_iou_index], img_rect);
    }
    else
    {
        img_rect[0] = previous_rect;
    }

    max_iou = calc_iou(previous_rect, img_rect[0]);

    if (max_iou < bb_iou_threshold)
    {
        img_rect[0].width = floor(img_rect[0].width * alpha + (1.0 - alpha) * previous_rect.width);
        img_rect[0].height = floor(img_rect[0].height * alpha + (1.0 - alpha) * previous_rect.height);
    }
    // do some bounding box conditioning  
    // New average = old average * (n-1)/n + new value /n
    //sma_width = floor(img_rect.width * alpha + (1.0 - alpha) * sma_width);
    //sma_height = floor(img_rect.height * alpha + (1.0 - alpha) * sma_height);

    //img_rect.width = sma_width;
    //img_rect.height = sma_height;

    previous_rect = img_rect[0];

    *num_dets = img_rect.size();

    // assigned image_rect data to detection_struct
    dets = new detection_struct[img_rect.size()];

    for (idx = 0; idx < img_rect.size(); ++idx)
    {
        label = "blob";
        dets[idx] = detection_struct(img_rect[idx].x, img_rect[idx].y, img_rect[idx].width, img_rect[idx].height, label.c_str(), 0.99, 0);
    }

}   // end of blob_detector