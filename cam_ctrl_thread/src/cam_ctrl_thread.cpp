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
#include "cam_ctrl_thread.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
std::string host_ip_address;
int32_t result = 0;
uint16_t read_port = 14002;                 // c2replyPort
uint16_t write_port = 14001;                // listenPort1
uint16_t c2_inbound2 = 14003;               // listenPort2
uint16_t video_port = 15004;

// Sierra-Olympic specific variables
SO::camera so_cam;
std::string camera_ip_address;


// ----------------------------------------------------------------------------
void print_usage(void)
{
    std::cout << "Enter the ip address of the camera to connect:" << std::endl;
    std::cout << "example: cam_test 192.168.0.10" << std::endl;
}

// ----------------------------------------------------------------------------
void init(void)
{
    std::string error_msg;

    // configure the UDP info
    so_cam.udp_camera_info = udp_info(write_port, read_port);

    // initialize camera
    result = so_cam.init_camera(camera_ip_address, error_msg);

    // display the information about a specific camera
    std::cout << so_cam << std::endl;

}


// ----------------------------------------------------------------------------
void update()
{

}


// ----------------------------------------------------------------------------
void close_thread()
{
    std::cout << std::endl << "Closing the UDP connection to the camera..." << std::endl;
    result = so_cam.close();

    std::cout << "close result: " << result << std::endl;
}


// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::string sdate, stime;

    uint64_t idx = 0, jdx = 0;
    std::string console_input;

    typedef std::chrono::duration<double> d_sec;
    auto start_time = std::chrono::system_clock::now();
    auto stop_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<d_sec>(stop_time - start_time);
        
    // IP based comms
    std::string video_cap_address;

    std::string error_msg;
    int32_t write_result;
    std::vector<uint8_t> rx_data;

    // opencv variables to display the video feed
    //std::string window_name = "Video Feed";
    //cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::Mat frame;
    char key = 0;

    if (argc == 1)
    {
        print_usage();
        std::cin.ignore();
        return 0;
    }

    try
    {
    
        camera_ip_address = argv[1];
        //video_cap_address = "udp://" + camera_ip_address + ":" + std::to_string(video_port);

        // the this machines local IP address
        get_local_ip(host_ip_address, error_msg);

        std::cout << "------------------------------------------------------------------" << std::endl;
        std::cout << "host IP address: " << host_ip_address << std::endl;
        std::cout << "------------------------------------------------------------------" << std::endl << std::endl;

        // run the discovery code to find the cameras
        std::vector<SO::discover_info> disc_info;
        so_cam.discover(host_ip_address, disc_info);

        for (idx = 0; idx < disc_info.size(); ++idx)
        {
            std::cout << disc_info[idx] << std::endl;
        }

        std::cout << "Discovery Complete!  Press Enter to continue..." << std::endl << std::endl;

        // initialize the camera with the supplied ip address
        init();

        // load the param gui to control focus and zoom
        load_param_gui(so_cam.udp_camera_info);

        while (key != 'q')
        {

            key = cv::waitKey(1);

        }   // end of while

        int bp = 0;

    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }
    
    cv::destroyAllWindows();
    
    // close the SO camera thread
    close_thread();

    return 0;
    
}   // end of main
