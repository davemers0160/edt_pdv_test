#ifndef _IMAGE_REGISTRATION_H
#define _IMAGE_REGISTRATION_H

#include <cstdint>
#include <limits>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
//#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d.hpp>

const int MAX_FEATURES = 300;
const float GOOD_MATCH_PERCENT = 0.50f;

cv::Mat SE = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

// ----------------------------------------------------------------------------
typedef struct mouse_points
{
    int id;
    std::vector<cv::Point2f> points;
} mouse_points;

template <typename T>
static inline bool max_vector_size(const std::vector<T>& lhs, const std::vector<T>& rhs)
{
    return lhs.size() < rhs.size();
}

// ----------------------------------------------------------------------------
inline void get_rect(std::vector<cv::Point> &p, cv::Rect &r)
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
}

// ----------------------------------------------------------------------------
inline std::vector<cv::Point2f> get_rect_corners(cv::Rect& r)
{
    std::vector<cv::Point2f> p;

    // start at the top left and go counter-clockwise
    p.push_back(cv::Point2f(r.x, r.y));
    p.push_back(cv::Point2f(r.x, r.y+r.height));
    p.push_back(cv::Point2f(r.x+r.width, r.y+r.height));
    p.push_back(cv::Point2f(r.x+r.width, r.y));

    return p;
}

// ----------------------------------------------------------------------------
void find_transformation_matrix(cv::Mat& img1, cv::Mat& img2, cv::Mat& h, cv::Mat& img_matches)
{

    cv::Mat tmp1, tmp2;

    //img1.convertTo(tmp1, CV_8UC3, 255);
    //img2.convertTo(tmp2, CV_8UC3, 255);

    tmp1 = img1.clone();
    tmp2 = img2.clone();

    if (tmp1.type() == CV_64FC1)
    {
        tmp1.convertTo(tmp1, CV_8UC1, 255);
    }
    else if (tmp1.type() != CV_8UC3)
    {
        cv::cvtColor(tmp1, tmp1, cv::COLOR_GRAY2RGB);
    }

    if (tmp2.type() == CV_64FC1)
    {
        tmp2.convertTo(tmp2, CV_8UC1, 255);
    }
    else if(tmp2.type() != CV_8UC3)
    {
        cv::cvtColor(tmp2, tmp2, cv::COLOR_GRAY2RGB);
    }

    // Variables to store keypoints and descriptors
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    cv::Ptr<cv::Feature2D> orb = cv::ORB::create(MAX_FEATURES, 1.2, 8, 25, 0, 4, cv::ORB::HARRIS_SCORE, 25);
    orb->detectAndCompute(tmp1, cv::Mat(), kp1, descriptors1);
    orb->detectAndCompute(tmp2, cv::Mat(), kp2, descriptors2);

    // Match features.
    std::vector<cv::DMatch> matches;
    //cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::MatcherType::BRUTEFORCE_SL2); //BRUTEFORCE_SL2 , BRUTEFORCE_HAMMING
    matcher->match(descriptors1, descriptors2, matches, cv::Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches
    const int numGoodMatches = (int)ceil(matches.size() * GOOD_MATCH_PERCENT);
    matches.erase(matches.begin() + numGoodMatches, matches.end());

    // Draw top matches
    cv::drawMatches(tmp1, kp1, tmp2, kp2, matches, img_matches);
    //imwrite("matches.jpg", imMatches);

    // Extract location of good matches
    std::vector<cv::Point2f> points1, points2;

    for (size_t i = 0; i < matches.size(); i++)
    {
        points1.push_back(kp1[matches[i].queryIdx].pt);
        points2.push_back(kp2[matches[i].trainIdx].pt);
    }

    // Find homography
    h = cv::findHomography(points1, points2, cv::RANSAC);

    // Use homography to warp image
    //warpPerspective(img1, im1Reg, h, img2.size());
}

// ----------------------------------------------------------------------------
cv::Mat get_gradient(cv::Mat &src)
{
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;

    int scale = 1;
    int delta = 0;
    int ddepth = CV_64F;

    double min_val, max_val;

    if (src.channels() > 1)
    {
        cv::cvtColor(src, src, cv::COLOR_RGB2GRAY);
    }

    // Calculate the x and y gradients using Sobel operator
    cv::Sobel(src, grad_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);

    cv::minMaxLoc(grad_x, &min_val, &max_val);

    cv::convertScaleAbs(grad_x, abs_grad_x, 255/ max_val);

    cv::Sobel(src, grad_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
    cv::minMaxLoc(grad_y, &min_val, &max_val);

    cv::convertScaleAbs(grad_y, abs_grad_y, 255/ max_val);

    // Combine the two gradients
    cv::Mat grad;
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    return grad;

}

// ----------------------------------------------------------------------------
cv::Rect get_bounding_box(cv::Mat& img, cv::Mat &converted_img,  bool invert)
{

    double min_val, max_val;
    std::vector<std::vector<cv::Point> > img_contours;
    std::vector<cv::Vec4i> img_hr;
    cv::Mat img_pyr, img_grad, img2;
    cv::Rect img_rect;

    cv::minMaxLoc(img, &min_val, &max_val);
    img.convertTo(converted_img, CV_64FC1, 1.0 / (max_val - min_val), -min_val / (max_val - min_val));

    if (invert)
        converted_img = 1.0 - converted_img;

    cv::transpose(converted_img, converted_img);

    converted_img.convertTo(converted_img, CV_8UC1, 255);

    cv::cvtColor(converted_img, img_pyr, cv::COLOR_GRAY2RGB);
    cv::pyrMeanShiftFiltering(img_pyr, img_pyr, 5, 10);

    img_grad = get_gradient(img_pyr);

    cv::threshold(img_grad, img2, 60, 0, cv::THRESH_TOZERO);

    cv::findContours(img2, img_contours, img_hr, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    get_rect(img_contours[0], img_rect);

    return img_rect;
}

// ----------------------------------------------------------------------------
cv::Rect get_bounding_box(cv::Mat& img, cv::Mat& converted_img, double threshold, bool invert)
{

    double min_val, max_val;
    std::vector<std::vector<cv::Point> > img_contours;
    std::vector<cv::Vec4i> img_hr;
    cv::Mat img_pyr, img_grad, img2;
    cv::Rect img_rect;

    cv::minMaxLoc(img, &min_val, &max_val);
    //img.convertTo(converted_img, CV_64FC1, 1.0 / (max_val - min_val), -min_val / (max_val - min_val));
    img.convertTo(converted_img, CV_8UC1, 255.0 / (max_val - min_val), -(255.0*min_val)/ (max_val - min_val));

    if (invert)
        converted_img = 255 - converted_img;

    cv::transpose(converted_img, converted_img);

    cv::threshold(converted_img, img2, threshold, 0, cv::THRESH_TOZERO);

    cv::morphologyEx(img2, img2, cv::MORPH_CLOSE, SE);
    //converted_img.convertTo(img_pyr, CV_8UC1, 255);

    //cv::cvtColor(img_pyr, img_pyr, cv::COLOR_GRAY2RGB);
    //cv::pyrMeanShiftFiltering(img_pyr, img_pyr, 5, 10);

    //img_grad = get_gradient(img_pyr);

    //cv::threshold(img_grad, img2, 60, 0, cv::THRESH_TOZERO);

    cv::findContours(img2, img_contours, img_hr, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (img_contours.size() > 0)
        get_rect(*std::max_element(img_contours.begin(), img_contours.end(), max_vector_size<cv::Point>), img_rect);
        //get_rect(img_contours[0], img_rect);
    else
        img_rect = cv::Rect(0, 0, img.cols >> 1, img.rows >> 1);

    return img_rect;
}

// ----------------------------------------------------------------------------
void generate_checkerboard(uint32_t block_w, uint32_t block_h, uint32_t img_w, uint32_t img_h, cv::Mat& checker_board)
{
    uint32_t idx = 0, jdx = 0;

    cv::Mat white = cv::Mat(block_w, block_h, CV_8UC1, cv::Scalar::all(255));

    checker_board = cv::Mat(img_h + block_h, img_w + block_w, CV_8UC1, cv::Scalar::all(0));

    bool color_row = false;
    bool color_column = false;

    for (idx = 0; idx < img_h; idx += block_h)
    {
        color_row = !color_row;
        color_column = color_row;

        for (jdx = 0; jdx < img_w; jdx += block_w)
        {
            if (!color_column)
                white.copyTo(checker_board(cv::Rect(jdx, idx, block_w, block_h)));

            color_column = !color_column;
        }

    }

    // need to add cropping of image
    cv::Rect roi(0, 0, img_w, img_h);
    checker_board = checker_board(roi);
}

// ----------------------------------------------------------------------------
//void cv_mouse_click(int cb_event, int x, int y, int flags, void* param) 
//{
//    if (cb_event == cv::EVENT_LBUTTONDOWN)
//    {
//        std::vector<cv::Point2f>* point = (std::vector<cv::Point2f>*)param;
//
//        point->push_back(cv::Point2f(x, y));
//
//        std::cout << "Point(" << x << ", " << y << ")" << std::endl;
//    }
//
//}

#endif	// _IMAGE_REGISTRATION_H
