
#include <cstdint>
#include <algorithm>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ms_tracker_main.h"
//#include "image_proc.h"
//#include "camera.h"
//#include "ms_tracker.h"
//#include "target.h"

// additional includes for external libraries
#include "select_roi.h"
#include "ms_tracker_lib.h"

// simple tracker to test out trackers on a single camera

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    
    uint32_t num_cams = 1;

    bool manual_detect = true;

    std::string window_name = "Image";
    cv::Mat img;
    int32_t width = 0;
    int32_t height = 0;
    int32_t channels = 0;

    int32_t rx = 0, ry = 0, rw = 0, rh = 0;

    if (argc < 2)
    {
        std::cout << "Error, no video supplied. Enter the full path of the video file... Exiting" << std::endl;
        std::cin.ignore();
        return - 1;
    }

    std::string mov_filename = std::string(argv[1]);

    // Get video 
    cv::VideoCapture cap(mov_filename);

    // Create camera objects for all cameras

    // Check if camera opened successfully
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file... Exiting" << std::endl;
        std::cout << mov_filename << std::endl;
        std::cin.ignore();
        return -1;
    }


    // tracker type - currently available: MIL, KCF, CSRT
    int32_t tracker_type = tracker_types::MIL;

    // create the tracker
    create_tracker(tracker_type);

    cv::namedWindow(window_name, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);

    target_rect new_target;
    
    // ----------------------------------------------------------------------------

    // get the first frame
    cap >> img;
    if (img.empty())
    {
        std::cout << "Error, video stream or file is empty... Exiting" << std::endl;
        std::cin.ignore();
        return -1;
    }

    width = img.cols;
    height = img.rows;
    channels = img.channels();

    // just in case we need to convert to grayscale
    // if(channels > 1)
    // {
    //     cv::cvtColor(img, img, COLOR_BGR2GRAY);
    //     channels = 1; 
    // }


    if (manual_detect)
    {
        // get a manual detect from the image
        //cv::Rect roi = cv::selectROI("Manual Target Select", img);
        select_roi(img.ptr<uint8_t>(0), width, height, channels, &rx, &ry, &rw, &rh);

        // 
        new_target = target_rect(rx, ry, rw, rh, num2str(time(0), "%08x"));

        // 
        //cv::destroyWindow("Manual Target Select");
    }

    if (!new_target.is_empty())
    {
        //tracker.add_target(new_target);
        //tracker.init(imgs[0], new_target);
    
        init_tracker(img.ptr<uint8_t>(0), height, width, channels, &new_target);

        std::cout << "target:" << std::endl << new_target << std::endl;        
    }

    // ----------------------------------------------------------------------------
    std::cout << "Press q to quit the program." << std::endl;
    
    // ----------------------------------------------------------------------------
    while(1)
    {

        // get the first frame
        cap >> img;
        if (img.empty())
        {
            break;
        }

        // track
        if (tracker_status())
        {
            //tracker.track_object(imgs[idx]);
            target_rect tgt;    // = tracker.get_target();

            update_tracker(img.ptr<uint8_t>(0), height, width, channels, &tgt);
            cv::Rect r(tgt.x, tgt.y, tgt.w, tgt.h);
            cv::rectangle(img, r, cv::Scalar(255), 2, 1);           
        }
        //// detect
        //else
        //{
        //    tracker.find_object(imgs[idx]);
        //    if (tracker.get_num_targets() > 0)
        //    {
        //        auto tgt = tracker.get_targets();
        //        cv::Rect r((*tgt).roi.x, (*tgt).roi.y, (*tgt).roi.w, (*tgt).roi.h);
        //        cv::rectangle(imgs[idx], r, cv::Scalar(255), 2, 1);
        //    }
        //}

        cv::imshow(window_name, img);
    
        // Wait for a keystroke in the window
        int k = cv::waitKey(50);
        if((k == 'q') || (k == 'Q'))
        {
            break;
        }
        else if (k == 't')
        {

            // get a manual detect from the image
            //cv::Rect roi = cv::selectROI(window_name, imgs[idx]);
            //new_target = target(roi.x, roi.y, roi.width, roi.height, num2str(time(0), "%08x"));
            std::string win_name = "Select";
            select_roi(img.ptr<uint8_t>(0), width, height, channels, &rx, &ry, &rw, &rh);
            new_target = target_rect(rx, ry, rw, rh, num2str(time(0), "%08x"));

            if (!new_target.is_empty())
            {
                if (!tracker_status())
                {
                    //tracker = ms_tracker(tracker_type);
                    //tracker.init(imgs[idx], new_target);

                    create_tracker(tracker_type);
                    init_tracker(img.ptr<uint8_t>(0), height, width, channels, &new_target);
                }
                else
                { 
                    //tracker.init(imgs[idx], new_target);
                    init_tracker(img.ptr<uint8_t>(0), height, width, channels, &new_target);

                }

                //tracker.add_target(new_target);

                std::cout << "target:" << std::endl << new_target << std::endl;
            }
        }


    }   // end of imgs for loop
        
    cv::destroyAllWindows();

    std::cout << "Ending..." << std::endl;
    std::cin.ignore();
    
    return 0;
}
