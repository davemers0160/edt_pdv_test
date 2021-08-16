
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
    std::vector<bool> use_layer = { true, true };
    std::vector<bool> invert_layer = {false, false};
    
    std::vector<cv::Mat> layers;
    std::vector<std::string> img_pathes = { "d:/data/lwir_0001a.png","d:/data/mwir_0001a.png" };

    std::vector<double> layer_weight = { 0.5, 0.5 };
    
    int bp = 0;

    // read in the image
    layers.clear();
    for (idx = 0; idx < img_pathes.size(); ++idx)
    {
        cv::Mat tmp = cv::imread(img_pathes[idx], cv::ImreadModes::IMREAD_GRAYSCALE);
        //tmp.convertTo(tmp, CV_32FC1, 1.0/255.0, 0.0);
        layers.push_back(tmp);
    }
    
    cv::Mat checker_board;
    generate_checkerboard(32, 32, 512, 512, checker_board);

    // read in the layer weights
    
    
    // read in the use_layer option
    

    // read in the invert_layer option
    
    
    // create the registration matrix to try and register the two images 
    cv::Mat h;
    cv::Mat img_matches;
    find_transformation_matrix(get_gradient(layers[1]), get_gradient(layers[0]), h, img_matches);

    // apply registration to images
    cv::Mat im1Reg;
    layers[0].convertTo(layers[0], CV_32FC1, 1.0 / 255.0, 0.0);
    layers[1].convertTo(layers[1], CV_32FC1, 1.0 / 255.0, 0.0);
    cv::warpPerspective(layers[1], im1Reg, h, layers[0].size());


    layers[1] = im1Reg.clone();

    // apply scale and inversion to images
    cv::Mat fused_img = cv::Mat(layers[0].rows, layers[0].cols, CV_32FC1, cv::Scalar::all(0.0));
    for (idx = 0; idx < img_pathes.size(); ++idx)
    {
        if (use_layer[idx])
            fused_img = fused_img + layer_weight[idx] * (invert_layer[idx] ? (1.0 - layers[idx]) : layers[idx]);

    }

    // display results
    
    
    bp = 1;

    cv::destroyAllWindows();

    std::cout << "Ending..." << std::endl;
    std::cin.ignore();
    
    return 0;
}
