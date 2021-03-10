#ifndef MAIN_HEADER_H
#define MAIN_HEADER_H

#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// ----------------------------------------------------------------------------
bool get_video(std::string image_path, std::vector<cv::Mat> &imgs)
{
    bool has_video = cv::imreadmulti(image_path, imgs, cv::IMREAD_GRAYSCALE);

    // If successful, start iterator at first image
    //if (has_video)
    //{
    //    img_it = std::begin(imgs);
    //    height = img_it->rows;
    //    width = img_it->cols;
    //}

    return has_video;
}

// ----------------------------------------------------------------------------
template<typename T>
std::string num2str(T val, std::string fmt)
{
    char in_string[64];
    sprintf(in_string, fmt.c_str(), val);
    return std::string(in_string);
}   // end of num2str  


#endif	// MAIN_HEADER_H
