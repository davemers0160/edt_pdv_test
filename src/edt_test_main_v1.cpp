#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#else

#endif

// C/C++ includes
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

#include "ip_helper.h"

// Project Includes
#include "edt_test.h"

// OpenCV Includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp> 
#include <opencv2/videoio.hpp>

void print_usage(void)
{
    std::cout << "Enter the ip address of the camera to connect:" << std::endl;
    std::cout << "example: cam_test 192.168.0.10" << std::endl;
}

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::string sdate, stime;

    uint64_t idx = 0, jdx = 0;
    int32_t result = 0;
    std::string console_input;

    typedef std::chrono::duration<double> d_sec;
    auto start_time = std::chrono::system_clock::now();
    auto stop_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<d_sec>(stop_time - start_time);

    // EDT PDV specific variable
    //Edtinfo edtinfo;
    //Dependent* dd_p;
    PdvDev* pdv_p;
    uint8_t* image_p;
    int32_t edt_width, edt_height, edt_depth;
    char* edt_cameratype;
    int32_t fifo_buffers = 4;
    int32_t timeouts = 0, last_timeouts = 0;
    bool recovering_timeout = false;
    std::string bit_directory;
    std::string cfg_file;

    // Sierra-Olympic specific variables
    SO::camera vinden;
    wind_protocol wind_data;
    uint32_t driver_device_num = 0;
    uint32_t connect_count = 0;
    uint32_t read_timeout = 30000;
    uint32_t write_timeout = 1000;
    uint32_t baud_rate = 115200;
    
    // IP based comms
    std::string camera_ip_address;
    std::string video_cap_address;      // = "udp://camera_ip_address:video_port";
    uint16_t read_port = 14002;
    uint16_t write_port = 14001;
    uint16_t video_port = 15004;
    //udp_info udp_camera_info(write_port, read_port);
    std::string error_msg;
    int32_t read_result, write_result;
    std::string host_ip_address;
    std::vector<uint8_t> rx_data;

    // opencv variables to display the video feed
    std::string window_name = "Video Feed";
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    char key = 0;

    if (argc == 1)
    {
        print_usage();
        std::cin.ignore();
        return 0;
    }

    // generic config file for a 14-bit monochrome camera
    cfg_file = "../config/generic14cl.cfg";

    // directory where the bit file is located from the config file
    bit_directory = "/opt/edtpdv/";

    camera_ip_address = argv[1];
    video_cap_address = "udp://" + camera_ip_address + ":" + std::to_string(video_port);

    vinden.udp_camera_info = udp_info(write_port, read_port);

    // the this machines local IP address
    get_local_ip(host_ip_address, error_msg);

    std::cout << "------------------------------------------------------------------" << std::endl;
    std::cout << "host IP address: " << host_ip_address << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl << std::endl;

    try
    {
        // initialize the EDT device
        result = init_edt_device(cfg_file, bit_directory, EDT_UNIT_0, VINDEN);

        if (result < 0)
        {
            return -1;
        }


        // open a specifc board and channel
        std::cout << std::endl << "Opening: " << EDT_INTERFACE << ", Unit: " << EDT_UNIT_0 << ", Channel: " << VINDEN << std::endl;
        pdv_p = pdv_open_channel(EDT_INTERFACE, EDT_UNIT_0, VINDEN);
        if (pdv_p == NULL)
        {
            std::cout << "Failed to connect to a PDV device!  Press Enter to close" << std::endl;
            std::cin.ignore();
            exit(1);
        }

        // print out the device name
        std::cout << "------------------------------------------------------------------" << std::endl;
        std::cout << "edt_devname: " << std::string(pdv_p->edt_devname) << std::endl;

        // flush the FIFO buffer
        pdv_flush_fifo(pdv_p);

        // get image size and name for display, save, printfs, etc.
        edt_width = pdv_get_width(pdv_p);
        edt_height = pdv_get_height(pdv_p);
        edt_depth = pdv_get_depth(pdv_p);
        edt_cameratype = pdv_get_cameratype(pdv_p);

        std::cout << "------------------------------------------------------------------" << std::endl;
        std::cout << "Image (h x w x d): " << edt_height << " x " << edt_width << " x " << edt_depth << std::endl;
        std::cout << "Camera Type: " << edt_cameratype << std::endl;
        std::cout << "------------------------------------------------------------------" << std::endl;

        // allocate four buffers for optimal pdv ring buffer pipeline (reduce if memory is at a premium)
        pdv_multibuf(pdv_p, fifo_buffers);
        //printf(" %d image%s from '%s'\nwidth %d height %d depth %d\n", loops, loops == 1 ? "" : "s", edt_cameratype, edt_width, edt_height, edt_depth);


        result = vinden.init_camera(camera_ip_address, error_msg);
        
        //if(result < 0)
        //{
        //    std::cout << "result: " << result << std::endl;
        //    std::cout << "error msg: " << error_msg << std::endl;
        //}
        
        load_param_gui(vinden.udp_camera_info);

        // display the information about a specific camera
        std::cout << vinden << std::endl;

#ifdef USE_UDP_VIDEO
        // set the video output parameters
        write_result = send_udp_data(vinden.udp_camera_info, vinden.set_ethernet_display_parameter(inet_addr(host_ip_address.c_str()), video_port).to_vector());

        // Turn on video streaming over ethernet
        write_result = send_udp_data(vinden.udp_camera_info, vinden.config_streaming_control(SO::STREAM_ON).to_vector());

        cv::VideoCapture cap(video_cap_address, cv::CAP_FFMPEG);

        if (!cap.isOpened())
        {
            key = 'q';
        }
#else
        std::cout << std::endl << "Starting the cameralink image capture process..." << std::endl;

        // prestart the first image or images outside the loop to get the pipeline going. 
        // Start multiple images unless force_single set in config file, since some cameras 
        // (e.g. ones that need a gap between images or that take a serial command to start 
        // every image) don't tolerate queueing of multiple images
        if (pdv_p->dd_p->force_single)
        {
            pdv_start_image(pdv_p);
        }
        else
        {
            pdv_start_images(pdv_p, fifo_buffers);
        }

#endif

        while (key != 'q')
        {

#ifdef USE_UDP_VIDEO

            cap >> frame;
#else

            // get the image and immediately start the next one (if not the last time through the 
            // loop). Processing (saving to a file in this case) can then occur in parallel with 
            // the next acquisition
            image_p = pdv_wait_image(pdv_p);
            pdv_start_image(pdv_p);

            timeouts = pdv_timeouts(pdv_p);

            // check for timeouts or data overruns -- timeouts occur when data is lost, camera isn't 
            // hooked up, etc, and application programs should always check for them. 
            if (timeouts > last_timeouts)
            {
                // pdv_timeout_cleanup helps recover gracefully after a timeout, particularly if multiple buffers were prestarted
                pdv_timeout_restart(pdv_p, TRUE);
                last_timeouts = timeouts;
                recovering_timeout = true;
                printf("\ntimeout....\n");
            }
            else if (recovering_timeout)
            {
                pdv_timeout_restart(pdv_p, TRUE);
                recovering_timeout = false;
                printf("\nrestarted....\n");
            }

            frame = cv::Mat(edt_height, edt_width, CV_8UC1, image_p);

#endif

            cv::imshow(window_name, frame);

            key = cv::waitKey(1);
        }

        cv::destroyAllWindows();

        int bp = 0;


    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }

    write_result = send_udp_data(vinden.udp_camera_info, vinden.config_streaming_control(SO::STREAM_OFF).to_vector());

    std::cout << std::endl << "Closing the UDP connection to the camera..." << std::endl;
    result = vinden.close();

    // close the device
    std::cout << std::endl << "Closing the pdv device..." << std::endl;
    result = pdv_close(pdv_p);

    std::cout << "close result: " << result << std::endl;

    return 0;
    
}   // end of main
