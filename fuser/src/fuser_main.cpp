
#include <cstdint>
#include <algorithm>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "fuser.h"
//#include "image_proc.h"
//#include "camera.h"
//#include "ms_tracker.h"
//#include "target.h"

// additional includes for external libraries
// #include "select_roi.h"
// #include "ms_tracker_lib.h"

//
// ----------------------------------------------------------------------------
typedef struct ms_image
{
    double* image;

    unsigned int img_w;
    unsigned int img_h;

    bool use_img = true;
    bool invert_img = false;

    double weight = 0.5;

} ms_image;

inline ms_image init_ms_image(double* image, unsigned int img_w, unsigned int img_h, bool use_img, bool invert_img, double weight)
{
    ms_image tmp;
    tmp.image = image;
    tmp.img_w = img_w;
    tmp.img_h = img_h;
    tmp.use_img = use_img;
    tmp.invert_img = invert_img;
    tmp.weight = weight;
    return tmp;
}

// ----------------------------------------------------------------------------
void image_fuser(unsigned int num_images, ms_image* img, double* fused_data64_t, unsigned char* fused_data8_t, unsigned int img_w, unsigned int img_h)
{
    unsigned int idx;

    // assign the fused data pointer to an opencv container
    cv::Mat fused_img = cv::Mat(img_h, img_w, CV_64FC1, fused_data64_t);
    cv::Mat fused_img8_t = cv::Mat(img_h, img_w, CV_8UC1, fused_data8_t);
    cv::Mat tmp_img;

    for (idx = 0; idx < num_images; ++idx)
    {
        if (img[idx].use_img)
        {
            tmp_img = cv::Mat(img[idx].img_h, img[idx].img_w, CV_64FC1, img[idx].image);
            fused_img = fused_img + img[idx].weight * (img[idx].invert_img ? (1.0 - tmp_img) : tmp_img);
        }
    }

    fused_img.convertTo(fused_img8_t, CV_8UC1, 255.0, 0.0);

}   // end of image_fuser


// ----------------------------------------------------------------------------
int main()
{
    uint32_t idx;
    std::vector<bool> use_layer = { true, true, false};
    std::vector<bool> invert_layer = {false, false, false};
    
    std::vector<cv::Mat> layers;
    std::vector<std::string> img_pathes = { "e:/data/lwir_0001a.png","e:/data/mwir_0001b.png" };

    std::vector<double> layer_weight = { 0.5, 0.5, 0.1 };

    std::vector<cv::Point2f> alignment_points1;
    std::vector<cv::Point2f> alignment_points2;

    // create the registration matrix to try and register the two images 
    cv::Mat h;
    cv::Mat tmp_reg;


    std::string window_name1 = "Base Image";
    std::string window_name2 = "Second Image";

    cv::namedWindow(window_name1);// , cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    cv::namedWindow(window_name2);// , cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    cv::namedWindow("fused");

    int bp = 0;

    // read in the image
    layers.clear();
    for (idx = 0; idx < img_pathes.size(); ++idx)
    {
        cv::Mat tmp = cv::imread(img_pathes[idx], cv::ImreadModes::IMREAD_GRAYSCALE);
        tmp.convertTo(tmp, CV_32FC1, 1.0 / 255.0, 0.0);
        layers.push_back(tmp);
    }
    
    
    //cv::Mat fused_img = cv::Mat(layers[0].rows, layers[0].cols, CV_32FC1, cv::Scalar::all(0.0));
    //fused_img = fused_img + layer_weight[0] * (invert_layer[0] ? (1.0 - layers[0]) : layers[0]);

    cv::Mat checker_board, cb1, cb2;
    generate_checkerboard(32, 32, layers[1].cols, layers[1].rows, checker_board);

    /*
    layers.push_back(checker_board);

    // setup the mouse callback to get the points
    cv::setMouseCallback(window_name1, cv_mouse_click, (void*)&alignment_points1);
    cv::setMouseCallback(window_name2, cv_mouse_click, (void*)&alignment_points2);

    char key = 0;

    while (key != 'q')
    {

        cv::imshow(window_name1, layers[0]);
        cv::imshow(window_name2, layers[1]);

        // check to see that the number of points in each image is the same and that there are at least 4 points
        if ((alignment_points1.size() == alignment_points2.size()) && (alignment_points1.size() > 3))
        {
            // Find homography
            h = cv::findHomography(alignment_points2, alignment_points1, cv::RANSAC);
            cv::warpPerspective(layers[1], tmp_reg, h, layers[0].size());
            cv::Mat tmp_fused = fused_img.clone();

            tmp_fused = tmp_fused + layer_weight[1] * (invert_layer[1] ? (1.0 - tmp_reg) : tmp_reg);


            imshow("fused", tmp_fused);
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
    */


    // read in the layer weights
    
    
    // read in the use_layer option
    

    // read in the invert_layer option
    
    

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
    unsigned int img_w = 512, img_h = 512;

    generate_checkerboard(32, 32, img_w, img_h, cb1);
    generate_checkerboard(64, 64, img_w, img_h, cb2);

    cb1.convertTo(cb1, CV_64FC1, 1.0 / 255.0, 0.0);
    cb2.convertTo(cb2, CV_64FC1, 1.0 / 255.0, 0.0);

    cv::Mat fused_img = cv::Mat::zeros(img_h, img_w, CV_64FC1);
    cv::Mat fused_img8_t = cv::Mat::zeros(img_h, img_w, CV_8UC1);

    ms_image tmp1 = init_ms_image(cb1.ptr<double>(0), img_w, img_h, true, false, 0.75);
    ms_image tmp2 = init_ms_image(cb2.ptr<double>(0), img_w, img_h, true, false, 0.25);

    std::vector<ms_image> tmp_ms = { tmp1, tmp2 };

    image_fuser(tmp_ms.size(), tmp_ms.data(), fused_img.ptr<double>(0), fused_img8_t.ptr<uint8_t>(0), img_w, img_h);

    //layers[1] = im1Reg.clone();


    //for (idx = 0; idx < img_pathes.size(); ++idx)
    //{
    //    if (use_layer[idx])
    //        fused_img = fused_img + layer_weight[idx] * (invert_layer[idx] ? (1.0 - layers[idx]) : layers[idx]);

    //}

    //fused_img.convertTo(fused_img, CV_8UC3, 255.0, 0.0);

    //cv::Mat t3 = cv::Mat::zeros(img_h, img_w, CV_8UC3);
    //fused_img.convertTo(fused_img, CV_8UC3, 255.0, 0.0);

    //cv::cvtColor(fused_img*255, fused_img, cv::COLOR_GRAY2RGB);

    // display results
    cv::imshow(window_name1, fused_img);
    cv::waitKey(0);
    
    bp = 1;

    cv::destroyAllWindows();

    std::cout << "Press Enter to close..." << std::endl;

    std::cin.ignore();
    
    return 0;
}
