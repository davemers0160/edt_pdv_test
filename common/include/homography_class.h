#ifndef HOMOGRAPHY_CLASS_H
#define HOMOGRAPHY_CLASS_H

#include <cstdint>
#include <limits>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>


// ----------------------------------------------------------------------------
class homography
{

public:

    double threshold;
        
    homography(double t_) : threshold(t_) {}

    // ----------------------------------------------------------------------------
    cv::Rect get_rect(void) { return previous_rect; }

    // ----------------------------------------------------------------------------
    inline cv::Rect get_bounding_box(cv::Mat& img, cv::Mat& converted_img, bool invert)
    {
        uint32_t idx;
        double min_val, max_val;
        std::vector<std::vector<cv::Point> > img_contours;
        std::vector<cv::Vec4i> img_hr;
        cv::Mat img_pyr, img_grad, img2, img_blur, img_blur_abs;
        cv::Rect img_rect;
        double alpha = 0.4;

        cv::minMaxLoc(img, &min_val, &max_val);
        //img.convertTo(converted_img, CV_64FC1, 1.0 / (max_val - min_val), -min_val / (max_val - min_val));
        img.convertTo(converted_img, CV_8UC1, 255.0 / (max_val - min_val), -(255.0 * min_val) / (max_val - min_val));

        if (invert)
            converted_img = 255 - converted_img;

        //cv::transpose(converted_img, converted_img);

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
            get_rect(img_contours[0], img_rect);
        }
        else if (img_contours.size() > 1)
        {
            // TODO: find the rect with the highest IOU, if IOU doesn't meet a certain threshold then use previous_rect
            // TODO: figure out how to dampen a rapid change in rect size, look at IOU and scaling to slowly move towards the current rect
            // TODO: look at exponential weighted moving average or some other FIFO like thing with weights.  3-tap FIR
            get_rect(*std::max_element(img_contours.begin(), img_contours.end(), max_vector_size<cv::Point>), img_rect);

            for (idx = 0; idx < img_contours.size(); ++idx)
            {

            }
        }
        else
        {
            img_rect = previous_rect;
        }

        // do some bounding box conditioning  
        // New average = old average * (n-1)/n + new value /n
        sma_width = floor(img_rect.width * alpha + (1.0 - alpha) * sma_width);
        sma_height = floor(img_rect.height * alpha + (1.0 - alpha) * sma_height);

        img_rect.width = sma_width;
        img_rect.height = sma_height;

        previous_rect = img_rect;

        return img_rect;

    }   // end of get_bounding_box

    inline void calc_homography_matrix(cv::Rect &ref_rect)
    {
        // Find homography
        h = cv::findHomography(get_rect_corners(previous_rect), get_rect_corners(ref_rect), cv::RANSAC);

    }   // end of calc_homography_matrix


    inline void transform_image(cv::Mat &src, cv::Mat &dst, cv::Size &ref_img_size)
    {
        cv::warpPerspective(src, dst, h, ref_img_size);
        
    }   // end of transform_image

private:

    cv::Rect previous_rect;
    cv::Rect current_rect;

    cv::Mat SE3_rect = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::Mat SE5_rect = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

    cv::Mat h;

    double sma_length = 3.0;
    double sma_width = 0.0;
    double sma_height = 0.0;


    // ----------------------------------------------------------------------------
    template <typename T>
    static inline bool max_vector_size(const std::vector<T>& lhs, const std::vector<T>& rhs)
    {
        return lhs.size() < rhs.size();
    }   // end of max_vector_size


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
    }   // end of get_rect

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
    }   // end of get_rect_corners

    // ----------------------------------------------------------------------------
    inline double calc_iou(cv::Rect &r1, cv::Rect &r2)
    {

        int64_t intersection = (r1 & r2).area();
        int64_t rect_union = (r1 | r2).area() + 1e-12;

        return ((rect_union == 0) ? 0.0 : intersection / (double)rect_union);

    }   // end of calc_iou

};

#endif  // HOMOGRAPHY_CLASS_H
