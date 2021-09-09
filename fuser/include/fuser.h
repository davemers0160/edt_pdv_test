#ifndef _FUSER_HEADER_H
#define _FUSER_HEADER_H

#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

//-----------------------------------------------------------------------------
typedef struct ms_image
{
    double* image;

    unsigned int img_w;
    unsigned int img_h;

    bool use_img = true;
    bool invert_img = false;

    double weight = 0.5;

} ms_image;

//-----------------------------------------------------------------------------
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


#endif	// _FUSER_HEADER_H
