/*
* Copyright 2021 Amy Wagoner, NSWC Crane
* 
* TODO: Add licensing statement. 
*/

#ifndef INCLUDED_IMAGE_PROC_H
#define INCLUDED_IMAGE_PROC_H


#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

#include <iostream>

//using namespace cv;
//
//class image_proc
//{
//    private:
//        // Histogram variables 
//        int histSize = 256;
//        bool uniform = true;
//        bool accumulate = false;
//        int hist_w = 512;
//        int hist_h = 400;
//        int bin_w = cvRound( (double) hist_w/histSize );
//        
//        // Tracker variables
//        //enum trackers
//        //{
//        //    BOOSTING, 
//        //    MIL, 
//        //    KCF, 
//        //    TLD,
//        //    MEDIANFLOW, 
//        //    GOTURN, 
//        //    MOSSE, 
//        //    CSRT
//        //};
//        //
//        //enum trackers trackerType;
//        //
//        //cv::Ptr<cv::Tracker> tracker;
//        //
//        //bool tracking;
//        
//        
//    public: 
//        image_proc();
//        ~image_proc();
//        
//        void blob_detection(cv::Mat img, std::vector<cv::KeyPoint> *keypoints, cv::Mat *im_with_keypoints, double mean);
//        void calc_hist(cv::Mat& img);
//        void init_tracker(cv::Mat &img, cv::Rect2d &roi);
//        bool update_tracker(cv::Mat &img, cv::Rect2d &roi);
//        void setTrackerType(std::string type);
//        //void blend_img(cv::Mat &img1, cv::Mat &img2, double alpha, cv::Mat &dst);
//}; 


// ----------------------------------------------------------------------------
// https://docs.opencv.org/master/d5/dc4/tutorial_adding_images.html
void blend_img(cv::Mat& img1, double w1, cv::Mat& img2, double w2, cv::Mat& blended_img)
{
    if (img1.size() == img2.size())
    {
        // Assumes the first image contains previously blended image and second is new image to blend
        cv::addWeighted(img1, w1, img2, w2, 0.0, blended_img);
    }

    else
    {
        std::cout << "Images are not same size. Cannot blend." << std::endl;
        std::cout << "Img1 size: " << img1.rows << "x" << img1.cols << ". Img2 size: " << img2.rows <<"x" << img2.cols << std::endl;
    }
}   // end of blend_img


void blend_img(uint8_t *im1, double w1, uint8_t *im2, double w2, int32_t h, int32_t w, uint8_t *bi)
{
    cv::Mat img1 = cv::Mat(h, w, CV_8UC1, im1, w * sizeof(*im1));
    cv::Mat img2 = cv::Mat(h, w, CV_8UC1, im2, w * sizeof(*im2));

    cv::Mat blended_img = cv::Mat(h, w, CV_8UC1, bi, w * sizeof(*bi));

    blend_img(img1, w1, img2, w2, blended_img);

}   // end of blend_img


// ----------------------------------------------------------------------------





// ----------------------------------------------------------------------------


#endif
