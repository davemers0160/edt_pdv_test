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

//#include "ip_helper.h"

// Project Includes
#include "cam_img.h"

// OpenCV Includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp> 
#include <opencv2/videoio.hpp>


// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
int32_t result = 0;
std::string bit_directory;
std::string cfg_file;

// EDT PDV specific variable
int32_t edt_unit;
int32_t edt_channel;
int32_t edt_width, edt_height, edt_depth;
PdvDev* pdv_p;
int32_t timeouts = 0;
int32_t last_timeouts = 0;
bool recovering_timeout = false;
uint8_t* image_p;

// ----------------------------------------------------------------------------
void print_usage(void)
{
    std::cout << "Enter the ip address of the camera to connect:" << std::endl;
    std::cout << "example: cam_test 192.168.0.10" << std::endl;
}


// ----------------------------------------------------------------------------
void init()
{
    int32_t fifo_buffers = 8; 
    
    char edt_devname[256] = { 0 };

    EdtDev* edt_p = NULL;
    Edtinfo edt_info;
    Dependent* dd_p = NULL;

    // open a specifc board and channel
    std::cout << std::endl << "Opening: " << EDT_INTERFACE << ", Unit: " << edt_unit << ", Channel: " << edt_channel << std::endl;

    // allocate memory for the Dependent structure
    if ((dd_p = pdv_alloc_dependent()) == NULL)
    {
        std::cout << "PDV alloc_dependent failed!" << std::endl;
        result = -1;
        return;
    }

    // read in the config file
    if (pdv_readcfg(cfg_file.c_str(), dd_p, &edt_info) != 0)
    {
        std::cout << "PDV readcfg failed!" << std::endl;
        free(dd_p);
        result = -1;
        return;
    }

    // IMPORTANT: pdv_initcam is a special case in that it requies a device pointer returned by use
    // edt_open_channel (or edt_open), NOT pdv_open_channel (or etc.). If you port this code to an
    // application that subsequently performs other operations (e.g. image capture) on the device,
    // edt_close should be called after pdv_initcam, then reopen with pdv_open_channel or pdv_open.
    if ((edt_p = edt_open_channel(EDT_INTERFACE, edt_unit, edt_channel)) == NULL)
    {
        std::cout << "error in edt_open_channel: " << std::string(edt_devname) << edt_unit << std::endl;
        edt_close(edt_p);
        free(dd_p);
        result = -1;
        return;
    }

    // init the edt device
    if (pdv_initcam(edt_p, dd_p, edt_unit, &edt_info, cfg_file.c_str(), (char*)bit_directory.c_str(), 0) != 0)
    {
        std::cout << "PDV initcam failed." << std::endl;
        //edt_msg(EDTAPP_MSG_FATAL, "initcam failed. Run with '-V' to see complete debugging output\n");
        edt_close(edt_p);
        free(dd_p);
        result = -1;
        result;
    }

    // close teh edt device
    edt_close(edt_p);

    // free the Dependent pointer
    free(dd_p);

    // open the pdv channel
    pdv_p = pdv_open_channel(EDT_INTERFACE, edt_unit, edt_channel);
    if (pdv_p == NULL)
    {
        std::cout << "Failed to connect to a PDV device!  Press Enter to close" << std::endl;
        //std::cin.ignore();
        result = -1;
        return;
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
    //edt_cameratype = pdv_get_cameratype(pdv_p);

    //buffer_size = edt_width * edt_height * sizeof(uint16_t);

    std::cout << "------------------------------------------------------------------" << std::endl;
    std::cout << "Image (h x w x d): " << edt_height << " x " << edt_width << " x " << edt_depth << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl;

    // allocate N buffers for optimal pdv ring buffer pipeline (reduce if memory is at a premium)
    pdv_multibuf(pdv_p, fifo_buffers);

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

    std::cout << std::endl << "Camera Initialization Complete!" << std::endl;




}   // end of init


// ----------------------------------------------------------------------------
void update()
{
   
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
        std::cout << "timeout...." << std::endl;
    }
    else if (recovering_timeout)
    {
        pdv_timeout_restart(pdv_p, TRUE);
        recovering_timeout = false;
        std::cout << "restarted...." << std::endl;
    }

}   // end of update


// ----------------------------------------------------------------------------
void close_edt()
{
    // close the device
    std::cout << std::endl << "Closing the pdv device..." << std::endl;
    int32_t result = pdv_close(pdv_p);

    std::cout << "close result: " << result << std::endl;
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

    int32_t cam_type;

    // opencv variables to display the video feed
    std::string window_name = "Video Feed";
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    char key = 0;
    //bool record = false;
    //std::vector<cv::Mat> multiframes(300);
    //uint32_t index;

    //if (argc == 1)
    //{
    //    print_usage();
    //    std::cin.ignore();
    //    return 0;
    //}

    std::cout << "Enter the EDT Board Number [0-2]: ";
    std::getline(std::cin, console_input);
    edt_unit = std::stoi(console_input);

    std::cout << "Enter the EDT Port Number [0-1]: ";
    std::getline(std::cin, console_input);
    edt_channel = std::stoi(console_input);

    std::cout << "Enter the Camera Type [0 - Ventus, 1 - Vinden]: ";
    std::getline(std::cin, console_input);
    cam_type = std::stoi(console_input);

    // generic config file for a monochrome camera
    switch (cam_type)
    {
    case 0:
        cfg_file = "../../common/config/ventus_cl.cfg";
        break;
    case 1:
        cfg_file = "../../common/config/vinden_cl.cfg";
        break;
    default:
        cfg_file = "../../common/config/vinden_cl.cfg";
        break;
    }

    // directory where the bit file is located from the config file
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    bit_directory = "C:/EDT/pdv/camera_config/bitfiles";
#else
    bit_directory = "/opt/EDTpdv/camera_config/bitfiles";
#endif

    try
    {
     
        init();

        while (key != 'q')
        {

            update();

            frame = cv::Mat(edt_height, edt_width, CV_16UC1, image_p);

            cv::normalize(frame, frame, 0, 255, cv::NORM_MINMAX, CV_8UC1);

            cv::imshow(window_name, frame);

            key = cv::waitKey(1);


        }   // end of while

        cv::destroyAllWindows();

        int bp = 0;


    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }


    close_edt();

    return 0;
    
}   // end of main
