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

#include "homography_lib.h"
#include "img_registration.h"
//#include "ms_image_struct.h"
#include "homography_class.h"

std::atomic<bool> homography_complete = false;
cv::Point2f initial_point, final_point;

const int32_t xy_offset = 128;

// ----------------------------------------------------------------------------
void cv_mouse_click(int cb_event, int x, int y, int flags, void* param)
{
    if (cb_event == cv::EVENT_LBUTTONDOWN)
    {
        std::vector<cv::Point2f>* point = (std::vector<cv::Point2f>*)param;

        point->push_back(cv::Point2f(x, y));

        std::cout << "Point(" << x << ", " << y << ")" << std::endl;
    }
    homography_complete = false;
}

// ----------------------------------------------------------------------------
void cv_mouse_measure_distance(int cb_event, int x, int y, int flags, void* param)
{
    if (cb_event == cv::EVENT_LBUTTONDOWN)
    {
        //std::vector<cv::Point2f>* point = (std::vector<cv::Point2f>*)param;

        initial_point = cv::Point2f(x, y);
        //point->push_back(cv::Point2f(x, y));

        //std::cout << "Point(" << x << ", " << y << ")" << std::endl;
    }
    else if (cb_event == cv::EVENT_LBUTTONUP)
    {
        final_point = cv::Point2f(x, y);
        std::cout << "distance: x = " << (final_point.x - initial_point.x) << ", y = " << (final_point.y - initial_point.y)  << std::endl;
    }
}

// ----------------------------------------------------------------------------
void x_trackbar_callback(int value, void* user_data)
{
    cv::Mat h = *(cv::Mat*)user_data;

    h.at<double>(0, 2) = (double)(value - xy_offset);
}

// ----------------------------------------------------------------------------
void y_trackbar_callback(int value, void* user_data)
{
    cv::Mat h = *(cv::Mat*)user_data;

    h.at<double>(1, 2) = (double)(value - xy_offset);
}

// ----------------------------------------------------------------------------
void scale_trackbar_callback(int value, void* user_data)
{
    cv::Mat h = *(cv::Mat*)user_data;

    double scale = (value) / 50.0;

    h.at<double>(0, 0) = scale;
    h.at<double>(1, 1) = scale;

}


// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    uint32_t idx, jdx;
    //std::vector<bool> use_img = { true, true};
    std::vector<bool> invert_img = { false, false, false, true, true };
    std::vector<double> weights = { 0.2, 0.2, 0.2, 0.2, 0.2};
    //std::vector<double> scale = { 1.0 / 255.0, 1.0 / 255.0};
    //std::vector<bool> scale_img = { true, true };
    double min_val, max_val;

    char key = 0;

    uint32_t ref_threshold = 75;
    std::vector<uint32_t> img_threshold = { 25, 75, 20, 25 };

    int32_t x_position = xy_offset;
    int32_t y_position = xy_offset;
    int32_t scale_position = 50;

    std::string lib_filename;
    
    //std::string window_name1 = "Reference Image";
    //cv::namedWindow(window_name1, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    //std::string window_name2 = "Image";
    //cv::namedWindow(window_name2, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    //std::string window_name3 = "Fused Image";
    //cv::namedWindow(window_name3, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    std::string window_montage = "Montage Image";

    //std::vector<cv::Point2f> alignment_points1;
    //std::vector<cv::Point2f> alignment_points2;

    // create the h matrix and fill with default value that does no image warping/translation
    cv::Mat_<double> h(3, 3);// = cv::Mat(3, 3, CV_64FC1);
    h << 1.0, 0.0, 0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0;


    if (argc < 4)
    {
        std::cout << "Enter the data directory, reference image, all other images" << std::endl;
        std::cin.ignore();
    }

    int bp = 0;

    cv::Mat fused_img, tmp_img, ref_img, img, ref_img2, img2;
    std::vector<cv::Mat> ref_img_stack;
    //std::vector<std::vector<cv::Mat>> img_stack;
    cv::Mat img_matches;
    cv::Rect ref_rect, img_rect;

    uint32_t num_images = argc - 3;

    std::string data_directory = std::string(argv[1]);
    std::string ref_img_filename = std::string(argv[2]);

    cv::imreadmulti(data_directory + ref_img_filename, ref_img_stack, cv::ImreadModes::IMREAD_ANYDEPTH | cv::ImreadModes::IMREAD_GRAYSCALE);
    int32_t stack_size = ref_img_stack.size();

    homography ref_h(75);

    std::vector<std::vector<cv::Mat>> img_stack(num_images);
    std::vector<homography> img_h(num_images);

    for (idx = 0; idx < num_images; ++idx)
    {
        cv::imreadmulti(data_directory + std::string(argv[3+idx]), img_stack[idx], cv::ImreadModes::IMREAD_ANYDEPTH | cv::ImreadModes::IMREAD_GRAYSCALE);
        img_h[idx].threshold = img_threshold[idx];
    }

    cv::namedWindow(window_montage, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

    cv::Mat montage_img;
    std::vector<cv::Mat> montage_vec(stack_size);

    cv::VideoWriter writer;
    int codec = cv::VideoWriter::fourcc('W','M','V','3');                    // select desired codec (must be available at runtime)
    double fps = 30.0;                                                          // framerate of the created video stream
    std::string video_filename = "C:/Projects/data/test/test.wmv";              // name of the output video file
    writer.open(video_filename, codec, fps, cv::Size(6 * ref_img_stack[0].cols, ref_img_stack[0].rows), false);

    if (!writer.isOpened()) 
    {
        std::cerr << "Could not open the output video file for write\n";
        return -1;
    }

    for (idx = 0; idx < stack_size; ++idx)
    {
        /*
        ref_rect = get_bounding_box(ref_img_stack[idx], ref_img, 80, invert_img[0]);
        img_rect = get_bounding_box2(img_stack[idx], img, 25, invert_img[1]);

        cv::rectangle(ref_img, ref_rect, cv::Scalar::all(255), 1, 8, 0);
        cv::rectangle(img, img_rect, cv::Scalar::all(255), 1, 8, 0);

        // Find homography
        h = cv::findHomography(get_rect_corners(img_rect), get_rect_corners(ref_rect), cv::RANSAC);

        //cv::minMaxLoc(ref_img_stack[idx], &min_val, &max_val);
        //ref_img_stack[idx].convertTo(ref_img, CV_64FC1, 1.0 / (max_val - min_val), -min_val / (max_val - min_val));

        //cv::minMaxLoc(img_stack[idx], &min_val, &max_val);
        //img_stack[idx].convertTo(img, CV_64FC1, 1.0 / (max_val - min_val), -min_val / (max_val - min_val));

        //cv::transpose(ref_img, ref_img);
        //cv::transpose(img, img);

        cv::warpPerspective(img, tmp_img, h, ref_img.size());
        */

        cv::transpose(ref_img_stack[idx], ref_img);
        ref_h.get_bounding_box(ref_img, ref_img, invert_img[0]);
        fused_img = weights[0] * ref_img;

        cv::rectangle(ref_img, ref_h.get_rect(), cv::Scalar::all(255), 1, 8, 0);
        montage_img = ref_img.clone();

        for (jdx = 0; jdx < num_images; ++jdx)
        {
            cv::transpose(img_stack[jdx][idx], img);

            img_h[jdx].get_bounding_box(img, img, invert_img[1]);

            img_h[jdx].calc_homography_matrix(ref_h.get_rect());

            img_h[jdx].transform_image(img, tmp_img, ref_img.size());

            fused_img = fused_img + weights[1+jdx] * tmp_img;
            
            cv::rectangle(img, img_h[jdx].get_rect(), cv::Scalar::all(255), 1, 8, 0);
            cv::hconcat(montage_img, img, montage_img);

        }

        //cv::hconcat(montage_img, img, montage_img);

        //fused_img = weights[1] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[0] * ref_img;
        //fused_img =  weights[0] * ref_img + weights[1] * tmp_img;

        cv::hconcat(montage_img, fused_img, montage_img);

        cv::imshow(window_montage, montage_img);
        writer.write(montage_img);
        //montage_vec.push_back(montage_img);
        montage_vec[idx] = montage_img;
        key = cv::waitKey(1);
        std::cout << "index: " << idx << std::endl;

        switch (key)
        {
        case 'n':
            std::cout << "index: " << idx << std::endl;
            break;
        case 'q':
            idx = stack_size;
            break;
        }
    }

    //std::cout << img_h.get_homography_matrix() << std::endl;

    // write Mat to file
    //cv::FileStorage fs("file.yml", cv::FileStorage::WRITE);
    //fs << "h_martrix" << img_h.get_homography_matrix();

    //cv::warpPerspective(img, tmp_img, h, ref_img.size());

    //fused_img = weights[0] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[1] * ref_img;

    // display results
    //cv::imshow(window_name3, fused_img);
    //cv::waitKey(0);

    writer.release();

    // try saving a tiff stack
    std::string save_file = "C:/Projects/data/test/test.tiff";
    cv::imwrite(save_file, montage_vec);

    std::cout << "complete" << std::endl;
    std::cin.ignore();

    bp = 1;

    cv::destroyAllWindows();

    //std::cout << "Press Enter to close..." << std::endl;

    //std::cin.ignore();
    
    return 0;
}
