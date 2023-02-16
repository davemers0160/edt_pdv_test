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

#include "overlay_bounding_box.h"

//#include "homography_test.h"
//#include "img_registration.h"
//#include "homography_class.h"
// 
// ----------------------------------------------------------------------------------------
typedef struct detection_struct
{
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

    char name[256];

    double score;

    unsigned int class_id;

    detection_struct()
    {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
        score = 0.0;
        class_id = 0;
        name[0] = 0;
    }

    detection_struct(unsigned int x_, unsigned int y_, unsigned int w_, unsigned int h_, const char name_[], double s_, unsigned int c_id)
    {
        x = x_;
        y = y_;
        w = w_;
        h = h_;
        score = s_;
        class_id = c_id;
        strcpy(name, name_);
    }

    // custom for this example code
    cv::Rect get_rect()
    {
        return cv::Rect(x, y, w, h);
    }

} detection_struct;

typedef void (*lib_blob_detector)(unsigned int img_w,
    unsigned int img_h,
    uint8_t* img_t,
    double threshold,
    unsigned int* num_dets,
    detection_struct*& dets
);

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    uint32_t idx, jdx;

    char key = 0;

    uint32_t img_w, img_h, img_c;
    double threshold = 30.0;

    std::string lib_filename;

    if (argc < 2)
    {
        std::cout << "Enter the data directory, reference image, all other images" << std::endl;
        std::cin.ignore();
    }


    for (idx = 0; idx < argc; ++idx)
    {
        std::cout << std::string(argv[idx]) << std::endl;
    }

    std::string window_montage = "Montage Image";
    cv::namedWindow(window_montage, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    int bp = 0;

    cv::Mat img;
    std::vector<cv::Mat> img_stack;
    cv::Rect img_rect;

    //std::vector<detection_struct> dets;
    detection_struct* dets;
    uint32_t num_dets = 0;

    std::string img_filename = std::string(argv[1]);

    // load in the library
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#if defined(_DEBUG)
    lib_filename = "../../blob_detector/build/Debug/blob_detector.dll";
#else
    lib_filename = "../../blob_detector/build/Release/blob_detector.dll";
#endif

    HINSTANCE blob_detector_lib = LoadLibrary(lib_filename.c_str());

    if (blob_detector_lib == NULL)
    {
        throw std::runtime_error("error loading library");
    }

    lib_blob_detector blob_detector = (lib_blob_detector)GetProcAddress(blob_detector_lib, "blob_detector");

#else
    lib_filename = "../../fusion_lib/build/libms_fuser.so";
    void* blob_detector_lib = dlopen(lib_filename.c_str(), RTLD_NOW);

    if (blob_detector_lib == NULL)
    {
        throw std::runtime_error("error loading blob_detector_lib library");
    }

    lib_blob_detector blob_detector = (lib_blob_detector)dlsym(blob_detector_lib, "blob_detector");

#endif

    //cv::imreadmulti(img_filename, img_stack, cv::ImreadModes::IMREAD_ANYDEPTH | cv::ImreadModes::IMREAD_GRAYSCALE);
    //int32_t stack_size = img_stack.size();

    cv::VideoCapture cap(img_filename);
    int32_t stack_size = 20;

    // Check if camera opened successfully
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }


    //cv::Mat montage_img;
    //std::vector<cv::Mat> montage_vec(stack_size);

    //-----------------------------------------------------------------------------
    // start up a video writer to save videos
    //cv::VideoWriter writer;
    //int codec = cv::VideoWriter::fourcc('W','M','V','3');                    // select desired codec (must be available at runtime)
    //double fps = 30.0;                                                          // framerate of the created video stream
    //std::string video_filename = "C:/Projects/data/test/test.wmv";              // name of the output video file
    //writer.open(video_filename, codec, fps, cv::Size(6 * ref_img_stack[0].cols, ref_img_stack[0].rows), false);

    //if (!writer.isOpened()) 
    //{
    //    std::cerr << "Could not open the output video file for write\n";
    //    return -1;
    //}
    //-----------------------------------------------------------------------------

    for (idx = 0; idx < stack_size; ++idx)
    {

        //cv::transpose(img_stack[idx], img);

        cap >> img;
        if (img.empty())
            break;

        img_w = img.cols;
        img_h = img.rows;

        uint32_t img_d = img.depth();

        auto tmp = CV_16U;

        if(img_d != CV_8U)
        { 
            img.convertTo(img, CV_8U);
        }

        if (img.channels() > 1)
        {
            cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        }

        blob_detector(img_w, img_h, img.ptr<uint8_t>(0), threshold, &num_dets, dets);

        for (jdx = 0; jdx < num_dets; ++jdx)
        {
            overlay_bounding_box(img, dets[jdx].get_rect(), std::string(dets[jdx].name), cv::Scalar::all(255));
        }

        cv::imshow(window_montage, img);
        //writer.write(montage_img);

        //montage_vec[idx] = montage_img;
        key = cv::waitKey(50);
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

    //writer.release();

    //// try saving a tiff stack
    //std::string save_file = "C:/Projects/data/test/test.tiff";
    //cv::imwrite(save_file, montage_vec);

    //std::cout << "complete" << std::endl;
    //std::cin.ignore();

    //bp = 1;

    cv::destroyAllWindows();

    //std::cout << "Press Enter to close..." << std::endl;

    //std::cin.ignore();
    
    return 0;
}
