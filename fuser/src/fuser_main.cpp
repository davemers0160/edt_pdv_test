
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
    
    cv::Mat fused_img = cv::Mat(layers[0].rows, layers[0].cols, CV_32FC1, cv::Scalar::all(0.0));
    fused_img = fused_img + layer_weight[0] * (invert_layer[0] ? (1.0 - layers[0]) : layers[0]);

    cv::Mat checker_board;
    generate_checkerboard(32, 32, layers[1].cols, layers[1].rows, checker_board);

    layers.push_back(checker_board);

    // setup the mouse callback to get the points
    cv::setMouseCallback(window_name1, cv_mouse_click, (void*)&alignment_points1);
    cv::setMouseCallback(window_name2, cv_mouse_click, (void*)&alignment_points2);

    char key = 0;

    while (key != 'q')
    {

        cv::imshow(window_name1, layers[0]);
        cv::imshow(window_name2, layers[1]);

        //if (alignment_points1.size() > 0)
        //{
        //    for (idx = 0; idx < alignment_points1.size(); ++idx)
        //    {
        //        std::cout << "Point1[" << idx << "]: " << alignment_points1[idx].x << ", " << alignment_points1[idx].y << std::endl;
        //    }
        //}

        //if (alignment_points2.size() > 0)
        //{
        //    for (idx = 0; idx < alignment_points2.size(); ++idx)
        //    {
        //        std::cout << "Point2[" << idx << "]: " << alignment_points2[idx].x << ", " << alignment_points2[idx].y << std::endl;
        //    }
        //}

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

        switch (key)
        {
        case 'd':
            alignment_points1.pop_back();
            alignment_points2.pop_back();
            break;






        }
    }



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

    for (idx = 1; idx < layers.size(); ++idx)
    {
        layers[idx].convertTo(layers[idx], CV_32FC1, 1.0 / 255.0, 0.0);
        cv::warpPerspective(layers[idx], tmp_reg, h, layers[0].size());
        layers[idx] = tmp_reg.clone();

        if (use_layer[idx])
            fused_img = fused_img + layer_weight[idx] * (invert_layer[idx] ? (1.0 - layers[idx]) : layers[idx]);

    }

    //layers[1] = im1Reg.clone();


    //for (idx = 0; idx < img_pathes.size(); ++idx)
    //{
    //    if (use_layer[idx])
    //        fused_img = fused_img + layer_weight[idx] * (invert_layer[idx] ? (1.0 - layers[idx]) : layers[idx]);

    //}

    // display results
    cv::imshow(window_name1, fused_img);
    cv::waitKey(0);
    
    bp = 1;

    cv::destroyAllWindows();

    std::cout << "Press Enter to close..." << std::endl;

    std::cin.ignore();
    
    return 0;
}
