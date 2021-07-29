#ifndef _FUSER_HEADER_H
#define _FUSER_HEADER_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/calib3d.hpp>

const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.10f;

// ----------------------------------------------------------------------------
void find_transformation_matrix(cv::Mat& img1, cv::Mat& img2, cv::Mat& h, cv::Mat &img_matches)
{

    // Variables to store keypoints and descriptors
    std::vector<cv::KeyPoint> kp1, kp2;
    cv::Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    cv::Ptr<cv::Feature2D> orb = cv::ORB::create(MAX_FEATURES, 1.1, 6, 25, 0, 4,cv::ORB::HARRIS_SCORE, 25);
    orb->detectAndCompute(img1, cv::Mat(), kp1, descriptors1);
    orb->detectAndCompute(img2, cv::Mat(), kp2, descriptors2);

    // Match features.
    std::vector<cv::DMatch> matches;
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    matcher->match(descriptors1, descriptors2, matches, cv::Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches
    const int numGoodMatches = (int)ceil(matches.size() * GOOD_MATCH_PERCENT);
    matches.erase(matches.begin() + numGoodMatches, matches.end());

    // Draw top matches
    cv::drawMatches(img1, kp1, img2, kp2, matches, img_matches);
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
    int ddepth = CV_32FC1; ;

    // Calculate the x and y gradients using Sobel operator
    cv::Sobel(src, grad_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);
    cv::convertScaleAbs(grad_x, abs_grad_x);

    cv::Sobel(src, grad_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    // Combine the two gradients
    cv::Mat grad;
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    return grad;

}


// ----------------------------------------------------------------------------
void generate_checkerboard(uint32_t block_w, uint32_t block_h, uint32_t img_w, uint32_t img_h, cv::Mat &checker_board)
{
    uint32_t idx = 0, jdx = 0;

    cv::Mat white = cv::Mat(block_w, block_h, CV_8UC1, cv::Scalar::all(255));

    checker_board = cv::Mat(img_w, img_h, CV_8UC1, cv::Scalar::all(0));

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




}
#endif	// _FUSER_HEADER_H
