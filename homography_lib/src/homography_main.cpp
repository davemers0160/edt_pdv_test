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
    uint32_t idx;
    std::vector<bool> use_img = { true, true};
    std::vector<bool> invert_img = { false, true};
    std::vector<double> weights = { 0.3, 0.7};
    std::vector<double> scale = { 1.0 / 255.0, 1.0 / 255.0};
    std::vector<bool> scale_img = { true, true };

    int32_t x_position = xy_offset;
    int32_t y_position = xy_offset;
    int32_t scale_position = 50;

    std::string lib_filename;
    
    std::string window_name1 = "Reference Image";
    cv::namedWindow(window_name1, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    std::string window_name2 = "Image";
    cv::namedWindow(window_name2, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    std::string window_name3 = "Fused Image";
    cv::namedWindow(window_name3, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    std::string window_montage = "Montage Image";

    std::vector<cv::Point2f> alignment_points1;
    std::vector<cv::Point2f> alignment_points2;

    // create the h matrix and fill with default value that does no image warping/translation
    //double h_data[] = { 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
    cv::Mat_<double> h(3, 3);// = cv::Mat(3, 3, CV_64FC1);
    h << 1.0, 0.0, 0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0;

    // create trackbars for x, y and scale
    cv::createTrackbar("X", window_name2, &x_position, xy_offset*2, x_trackbar_callback, &h);
    cv::createTrackbar("Y", window_name2, &y_position, xy_offset*2, y_trackbar_callback, &h);
    cv::createTrackbar("Scale", window_name2, &scale_position, 100, scale_trackbar_callback, &h);


    if (argc < 3)
    {
        std::cout << "enter the files names" << std::endl;
        std::cin.ignore();
    }

    // load in the library
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    lib_filename = "../../fusion_lib/build/Release/ms_fuser.dll";
    //HINSTANCE img_fusion_lib = LoadLibrary(lib_filename.c_str());

    //if (img_fusion_lib == NULL)
    //{
    //    throw std::runtime_error("error loading library");
    //}

    //image_fuser lib_image_fuser = (image_fuser)GetProcAddress(img_fusion_lib, "image_fuser");

#else
    lib_filename = "../../fusion_lib/build/libms_fuser.so";
    //void* img_fusion_lib = dlopen(lib_filename.c_str(), RTLD_NOW);

    //if (img_fusion_lib == NULL)
    //{
    //    throw std::runtime_error("error loading library");
    //}

    //image_fuser lib_image_fuser = (image_fuser)dlsym(img_fusion_lib, "image_fuser");

#endif

    int bp = 0;

    cv::Mat fused_img, tmp_img, ref_img, img, ref_img2, img2;
    std::vector<cv::Mat> ref_img_stack;
    std::vector<cv::Mat> img_stack;
    cv::Mat img_matches;
    cv::Rect ref_rect, img_rect;


    std::string ref_img_filename = std::string(argv[1]);
    std::string img_filename = std::string(argv[2]);

    // load in the images
    //cv::Mat ref_img = cv::imread(ref_img_filename, cv::ImreadModes::IMREAD_GRAYSCALE);
    //ref_img.convertTo(ref_img, CV_64FC1, 1.0 / 255.0, 0.0);
    //cv::Mat img = cv::imread(img_filename, cv::ImreadModes::IMREAD_GRAYSCALE);
    //img.convertTo(img, CV_64FC1, 1.0 / 255.0, 0.0);
    // 
    // setup the mouse callback to get the points
    cv::setMouseCallback(window_name1, cv_mouse_click, (void*)&alignment_points1);
    cv::setMouseCallback(window_name2, cv_mouse_click, (void*)&alignment_points2);
    cv::setMouseCallback(window_name3, cv_mouse_measure_distance);

    cv::imreadmulti(ref_img_filename, ref_img_stack, cv::ImreadModes::IMREAD_ANYDEPTH | cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::imreadmulti(img_filename, img_stack, cv::ImreadModes::IMREAD_ANYDEPTH | cv::ImreadModes::IMREAD_GRAYSCALE);
    int32_t stack_size = ref_img_stack.size();
    double min_val, max_val;

    char key = 0;

    int32_t index = (int32_t)floor(stack_size*.75);

    homography ref_h(75);
    homography img_h(20);

    //cv::minMaxLoc(ref_img_stack[index], &min_val, &max_val);
    //ref_img_stack[index].convertTo(ref_img, CV_64FC1, 1.0 / (max_val - min_val), -min_val/ (max_val - min_val));

    //cv::minMaxLoc(img_stack[index], &min_val, &max_val);
    //img_stack[index].convertTo(img, CV_64FC1, 1.0 / (max_val - min_val), -min_val/ (max_val - min_val));

    //if (invert_img[1])
    //    img = 1.0 - img;

    //cv::transpose(ref_img, ref_img);
    //cv::transpose(img, img);

    //cv::Mat ref_img_grad = get_gradient(ref_img);
    //cv::Mat img_grad = get_gradient(img);


    //cv::adaptiveThreshold(ref_img, ref_img, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 5, 0);
    //cv::adaptiveThreshold(img, img, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 5, 0);

    //auto ref_mean = cv::mean(ref_img_grad)[0];
    //auto img_mean = cv::mean(img_grad)[0];

    //cv::threshold(ref_img_grad, ref_img2, 80, 0, cv::THRESH_TOZERO);
    //cv::threshold(img_grad, img2, 60, 0, cv::THRESH_TOZERO);


    //cv::Mat tmp_fused;



    //cv::Mat img_pyr, ref_pyr;
    //ref_img.convertTo(ref_pyr, CV_8UC1, 255);
    //img.convertTo(img_pyr, CV_8UC1, 255);

    //cv::cvtColor(ref_pyr, ref_pyr, cv::COLOR_GRAY2RGB);
    //cv::cvtColor(img_pyr, img_pyr, cv::COLOR_GRAY2RGB);
    //cv::pyrMeanShiftFiltering(ref_pyr, ref_pyr, 5, 10);
    //cv::pyrMeanShiftFiltering(img_pyr, img_pyr, 5, 10);

    //ref_img_grad = get_gradient(ref_pyr);
    //img_grad = get_gradient(img_pyr);

    //cv::threshold(ref_img_grad, ref_img2, 80, 0, cv::THRESH_TOZERO);
    //cv::threshold(img_grad, img2, 60, 0, cv::THRESH_TOZERO);

    //std::vector<std::vector<cv::Point> > ref_contours, img_cont;
    //std::vector<cv::Vec4i> ref_h, img_hr;
    //cv::findContours(ref_img2, ref_contours, ref_h, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    //cv::findContours(img2, img_cont, img_hr, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


    //get_rect(ref_contours[0], ref_rect);
    //get_rect(img_cont[0], img_rect);




    //ref_rect = get_bounding_box(ref_img_stack[0], ref_img, 80, invert_img[0]);
    //img_rect = get_bounding_box(img_stack[0], img, 60, invert_img[1]);

    //// Find homography
    //h = cv::findHomography(get_rect_corners(img_rect), get_rect_corners(ref_rect), cv::RANSAC);
    //

    //cv::Mat ref_draw = cv::Mat::zeros(ref_img.size(), CV_8UC3);
    //for (size_t i = 0; i < ref_contours.size(); i++)
    //{
    //    cv::Scalar color = cv::Scalar(255,255,255);
    //    cv::drawContours(ref_draw, ref_contours, (int)i, color, 1, cv::LINE_8, ref_h, 0);
    //}
    //cv::rectangle(ref_draw, ref_rect, cv::Scalar::all(255), 1, 8, 0);

    //cv::Mat img_draw = cv::Mat::zeros(img.size(), CV_8UC3);
    //for (size_t i = 0; i < img_cont.size(); i++)
    //{
    //    cv::Scalar color = cv::Scalar(255,255,255);
    //    cv::drawContours(img_draw, img_cont, (int)i, color, 1, cv::LINE_8, img_hr, 0);
    //}
    //cv::rectangle(img_draw, img_rect, cv::Scalar::all(255), 1, 8, 0);

    //cv::Mat h2;
    //find_transformation_matrix(ref_draw, img_draw, h, img_matches);

    /*
    while (key != 'q')
    {

        cv::imshow(window_name1, ref_img);
        cv::imshow(window_name2, img);

        // check to see that the number of points in each image is the same and that there are at least 4 points
        if (!homography_complete && (alignment_points1.size() == alignment_points2.size()) && (alignment_points1.size() > 3))
        {

            // Find homography
            h = cv::findHomography(alignment_points2, alignment_points1, cv::RANSAC);
            cv::warpPerspective(img, tmp_img, h, ref_img.size());
            //fused_img = ref_img.clone();

            fused_img = weights[1] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[0]* ref_img;


            cv::imshow(window_name3, fused_img);
            homography_complete = true;
        }

        //find_transformation_matrix(ref_img2, img2, h, img_matches);
        cv::warpPerspective(img, tmp_img, h, ref_img.size());
        fused_img = weights[1] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[0] * ref_img;

        cv::imshow(window_name3, fused_img);

        key = cv::waitKey(20);

        // do stuff with the user input
        switch (key)
        {
        // delete the last pair of points
        case 'd':

            if ((alignment_points1.size() == alignment_points2.size()) && (alignment_points1.size() > 0))
            {
                alignment_points1.pop_back();
                alignment_points2.pop_back();
            }
            else if ((alignment_points1.size() > alignment_points2.size()) && (alignment_points1.size() > 0))
            {
                alignment_points1.pop_back();
            }
            else if ((alignment_points2.size() > alignment_points1.size()) && (alignment_points2.size() > 0))
            {
                alignment_points2.pop_back();
            }

            homography_complete = false;

            break;
        }
    }
    */
    //find_transformation_matrix(get_gradient(layers[1]), get_gradient(layers[0]), h, img_matches);
    //cv::drawMatches(layers[0], alignment_points1, layers[1], alignment_points2, matches, img_matches);

    cv::destroyWindow(window_name1);
    cv::destroyWindow(window_name2);
    cv::destroyWindow(window_name3);

    cv::namedWindow(window_montage, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

    cv::Mat montage_img;
    std::vector<cv::Mat> montage_vec(stack_size);

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
        cv::transpose(img_stack[idx], img);

        ref_h.get_bounding_box(ref_img, ref_img, invert_img[0]);
        img_h.get_bounding_box(img, img, invert_img[1]);


        cv::rectangle(ref_img, ref_h.get_rect(), cv::Scalar::all(255), 1, 8, 0);
        cv::rectangle(img, img_h.get_rect(), cv::Scalar::all(255), 1, 8, 0);


        img_h.calc_homography_matrix(ref_h.get_rect());

        img_h.transform_image(img, tmp_img, ref_img.size());

        cv::hconcat(ref_img, img, montage_img);

        //fused_img = weights[1] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[0] * ref_img;
        fused_img =  weights[0] * ref_img + weights[1] * tmp_img;

        cv::hconcat(montage_img, fused_img, montage_img);

        cv::imshow(window_montage, montage_img);
        //montage_vec.push_back(montage_img);
        montage_vec[idx] = montage_img;
        key = cv::waitKey(0);
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

    std::cout << h << std::endl;

    // write Mat to file
    cv::FileStorage fs("file.yml", cv::FileStorage::WRITE);
    fs << "h_martrix" << h;

    //cv::warpPerspective(img, tmp_img, h, ref_img.size());

    //fused_img = weights[0] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[1] * ref_img;

    // display results
    //cv::imshow(window_name3, fused_img);
    //cv::waitKey(0);

    // try saving a tiff stack
     std::string save_file = "C:/Projects/data/test/test.tiff";
    //cv::imwrite(save_file, montage_vec);

    std::cout << "complete" << std::endl;
    std::cin.ignore();

    bp = 1;

    // close the library
//#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
//    FreeLibrary(img_fusion_lib);
//#else
//    dlclose(img_fusion_lib);
//#endif

    cv::destroyAllWindows();

    //std::cout << "Press Enter to close..." << std::endl;

    //std::cin.ignore();
    
    return 0;
}
