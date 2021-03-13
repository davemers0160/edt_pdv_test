
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
int main()
{
    
    uint32_t num_cams = 1;

    bool manual_detect = false;

    // opencv specific variables needed to run the examples 
    //samples::addSamplesDataSearchPath("../../data/19Nov20");
    //std::string ir_image_path = samples::findFile("InfraredImage2.tiff");
    //std::string vis_image_path = samples::findFile("VisibleImage2.tiff");

    cv::samples::addSamplesDataSearchPath("../../../../data/movie_images");
    std::string image_path = cv::samples::findFile("test.tiff");

    // these are the images that would normally become from a separate thread
    std::vector<cv::Mat> imgs;

    std::string window_name = "Image";

    // Get video 
    bool valid_imgs = get_video(image_path, imgs);

    int32_t width = 0;
    int32_t height = 0;

    // Create camera objects for all cameras
    //Camera cam("Camera Feed", imgs[0].rows, imgs[0].cols);

    // tracker class
    int32_t tracker_type = tracker_types::MIL;
    //ms_tracker tracker(tracker_type);
    create_tracker(tracker_type);

    cv::namedWindow(window_name, cv::WINDOW_NORMAL);

    target_rect new_target;
    
    // ----------------------------------------------------------------------------
    if (valid_imgs)
    {
        width = imgs[0].cols;
        height = imgs[0].rows;

        if (manual_detect)
        {
            // get a manual detect from the image
            cv::Rect roi = cv::selectROI("Manual Target Select", imgs[0]);

            // 
            new_target = target_rect(roi.x, roi.y, roi.width, roi.height, num2str(time(0), "%08x"));

            // 
            cv::destroyWindow("Manual Target Select");
        }

        if (!new_target.is_empty())
        {
            //tracker.add_target(new_target);
            //tracker.init(imgs[0], new_target);
        
            init_tracker(imgs[0].ptr<uint8_t>(0), height, width, 1, &new_target);

            std::cout << "target:" << std::endl << new_target << std::endl;        
        }
    }

    // ----------------------------------------------------------------------------
    std::cout << "Press q to quit the program." << std::endl;
    
    // ----------------------------------------------------------------------------
    for(uint64_t idx=1; idx< imgs.size(); ++idx)
    {

        // track
        if (tracker_status())
        {
            //tracker.track_object(imgs[idx]);
            target_rect tgt;    // = tracker.get_target();

            update_tracker(imgs[idx].ptr<uint8_t>(0), height, width, 1, &tgt);
            cv::Rect r(tgt.x, tgt.y, tgt.w, tgt.h);
            cv::rectangle(imgs[idx], r, cv::Scalar(255), 2, 1);           
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

        cv::imshow(window_name, imgs[idx]);
    
        // Wait for a keystroke in the window
        int k = cv::waitKey(50);
        if((k == 'q') || (k == 'Q'))
        {
            break;
        }
        else if (k == 't')
        {
            int rx = 0, ry = 0, rw = 0, rh = 0;

            // get a manual detect from the image
            //cv::Rect roi = cv::selectROI(window_name, imgs[idx]);
            //new_target = target(roi.x, roi.y, roi.width, roi.height, num2str(time(0), "%08x"));
            std::string win_name = "Select";
            select_roi((char*)win_name.c_str(), imgs[idx].ptr<uint8_t>(0), width, height, 1, &rx, &ry, &rw, &rh);
            new_target = target_rect(rx, ry, rw, rh, num2str(time(0), "%08x"));

            if (!new_target.is_empty())
            {
                if (!tracker_status())
                {
                    //tracker = ms_tracker(tracker_type);
                    //tracker.init(imgs[idx], new_target);

                    create_tracker(tracker_type);
                    init_tracker(imgs[idx].ptr<uint8_t>(0), height, width, 1, &new_target);
                }
                else
                { 
                    //tracker.init(imgs[idx], new_target);
                    init_tracker(imgs[idx].ptr<uint8_t>(0), height, width, 1, &new_target);

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
