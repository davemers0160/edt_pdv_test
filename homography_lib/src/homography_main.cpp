#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstdint>
#include <algorithm>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include "homography_lib.h"
//#include "ms_image_struct.h"



// ----------------------------------------------------------------------------
void cv_mouse_click(int cb_event, int x, int y, int flags, void* param)
{
    if (cb_event == cv::EVENT_LBUTTONDOWN)
    {
        std::vector<cv::Point2f>* point = (std::vector<cv::Point2f>*)param;

        point->push_back(cv::Point2f(x, y));

        std::cout << "Point(" << x << ", " << y << ")" << std::endl;
    }

}








// ----------------------------------------------------------------------------
int main()
{
    uint32_t idx;
    std::vector<bool> use_img = { true, true};
    std::vector<bool> invert_img = { false, false};
    std::vector<double> weights = { 0.5, 0.5};
    std::vector<double> scale = { 1.0 / 255.0, 1.0 / 255.0};
    std::vector<bool> scale_img = { true, true };


    std::string lib_filename;

    cv::Mat cb1, cb2, cb3;
    std::vector<cv::Mat> cb(3);
    std::vector<std::string> win_names(3);
    std::vector<image_struct> tmp_ms(3);
    
    std::string window_name1 = "Reference Image";
    cv::namedWindow(window_name1);// , cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    std::string window_name2 = "Image";
    cv::namedWindow(window_name2);// , cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    std::string window_name3 = "Fused Image";
    cv::namedWindow(window_name3);// , cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

    std::vector<cv::Point2f> alignment_points1;
    std::vector<cv::Point2f> alignment_points2;

    // create the registration matrix to try and register the two images 
    cv::Mat h;
    cv::Mat tmp_reg;

    // load in the library
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    lib_filename = "../../fusion_lib/build/Release/ms_fuser.dll";
    HINSTANCE img_fusion_lib = LoadLibrary(lib_filename.c_str());

    if (img_fusion_lib == NULL)
    {
        throw std::runtime_error("error loading library");
    }

    //image_fuser lib_image_fuser = (image_fuser)GetProcAddress(img_fusion_lib, "image_fuser");

#else
    lib_filename = "../../fusion_lib/build/libms_fuser.so";
    void* img_fusion_lib = dlopen(lib_filename.c_str(), RTLD_NOW);

    if (img_fusion_lib == NULL)
    {
        throw std::runtime_error("error loading library");
    }

    image_fuser lib_image_fuser = (image_fuser)dlsym(img_fusion_lib, "image_fuser");

#endif

    int bp = 0;


    // load in the images
    cv::Mat ref_img = cv::imread("", cv::ImreadModes::IMREAD_GRAYSCALE);
    ref_img.convertTo(ref_img, CV_64FC1, 1.0 / 255.0, 0.0);
    cv::Mat img = cv::imread("", cv::ImreadModes::IMREAD_GRAYSCALE);
    img.convertTo(img, CV_64FC1, 1.0 / 255.0, 0.0);

    unsigned int img_w = 512, img_h = 512;
    
    cv::Mat fused_img;

    // setup the mouse callback to get the points
    cv::setMouseCallback(window_name1, cv_mouse_click, (void*)&alignment_points1);
    cv::setMouseCallback(window_name2, cv_mouse_click, (void*)&alignment_points2);

    //cv::Mat tmp_fused;
    char key = 0;

    while (key != 'q')
    {

        cv::imshow(window_name1, ref_img);
        cv::imshow(window_name2, cb[1]);

        // check to see that the number of points in each image is the same and that there are at least 4 points
        if ((alignment_points1.size() == alignment_points2.size()) && (alignment_points1.size() > 3))
        {
            cv::Mat tmp_img = img.clone();

            // Find homography
            h = cv::findHomography(alignment_points2, alignment_points1, cv::RANSAC);
            cv::warpPerspective(ref_img, tmp_img, h, ref_img.size());
            //fused_img = ref_img.clone();

            fused_img = weights[0] * (invert_img[1] ? (1.0 - tmp_img) : tmp_img) + weights[1]* ref_img;


            cv::imshow(window_name3, fused_img);
        }

        key = cv::waitKey(20);

        // do stuff with the user input
        switch (key)
        {
        // delete the last pair of points
        case 'd':

            if (alignment_points1.size() == alignment_points2.size())
            {
                alignment_points1.pop_back();
                alignment_points2.pop_back();
            }
            else if (alignment_points1.size() > alignment_points2.size())
            {
                alignment_points1.pop_back();
            }
            else
            {
                alignment_points2.pop_back();
            }

            break;

        }
    }
    

    //find_transformation_matrix(get_gradient(layers[1]), get_gradient(layers[0]), h, img_matches);

    //cv::drawMatches(layers[0], alignment_points1, layers[1], alignment_points2, matches, img_matches);

    // Find homography
    //h = cv::findHomography(alignment_points2, alignment_points1, cv::RANSAC);

    // apply registration to images
    //std::vector<cv::Mat> register_img;
    //layers[0].convertTo(layers[0], CV_32FC1, 1.0 / 255.0, 0.0);
    // apply scale and inversion to images
    /*cv::Mat fused_img = cv::Mat(layers[0].rows, layers[0].cols, CV_32FC1, cv::Scalar::all(0.0));*/
    //fused_img = fused_img + layer_weight[0] * (invert_layer[0] ? (1.0 - layers[0]) : layers[0]);
/*
    for (idx = 1; idx < layers.size(); ++idx)
    {
        layers[idx].convertTo(layers[idx], CV_32FC1, 1.0 / 255.0, 0.0);
        cv::warpPerspective(layers[idx], tmp_reg, h, layers[0].size());
        layers[idx] = tmp_reg.clone();

        if (use_layer[idx])
            fused_img = fused_img + layer_weight[idx] * (invert_layer[idx] ? (1.0 - layers[idx]) : layers[idx]);

    }
*/


    for (idx=0; idx<cb.size(); ++idx)
    {
        cb[idx].convertTo(cb[idx], CV_64FC1, 1.0, 0.0);
        win_names[idx] = "checkerboard " + std::to_string(idx);
        
        cv::namedWindow(win_names[idx]);
        cv::imshow(win_names[idx], cb[idx]);
        cv::waitKey(10);
        
        /*tmp_ms[idx] = init_ms_image(cb[idx].ptr<double>(0), img_w, img_h, use_img[idx], invert_img[idx], weights[idx], scale[idx], scale_img[idx]);*/
    }
    
    //img_w = img_h = 200;

    // create the containers for the fused images
    //cv::Mat fused_img = cv::Mat::zeros(img_h, img_w, CV_64FC1);
    //cv::Mat fused_img8_t = cv::Mat::zeros(img_h, img_w, CV_8UC1);

    // run the image fuser lib
    //lib_image_fuser(tmp_ms.size(), tmp_ms.data(), fused_img.ptr<double>(0), fused_img8_t.ptr<uint8_t>(0), img_w, img_h);

    bp = 2;

    //image_fuser2(tmp_ms.size(), tmp_ms.data(), fused_img.ptr<double>(0), fused_img8_t.ptr<uint8_t>(0), img_w, img_h);

    // display results
    cv::imshow(window_name1, fused_img);
    cv::waitKey(0);
    
    bp = 1;

    // close the library
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    FreeLibrary(img_fusion_lib);
#else
    dlclose(img_fusion_lib);
#endif

    cv::destroyAllWindows();

    //std::cout << "Press Enter to close..." << std::endl;

    //std::cin.ignore();
    
    return 0;
}
