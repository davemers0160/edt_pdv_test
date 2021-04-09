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
//#include "cam_ctrl_thread.h"
#include "sierra_olympic_camera.h"

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
void close_thread()
{
    result = so_cam.close();
    std::cout << std::endl << "Closing the UDP connection to the camera..." << std::endl;

    //std::cout << "close result: " << result << std::endl;
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
    
    std::string error_msg;
    
    int32_t camera_num = 0;
    uint8_t mode = 0;
    std::string new_ip_address;
    std::string gateway_address;
    std::string subnet_address;

    SO::network_parameters net_params;
    
    try
    {
    
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
            std::cout << "------------------------------------------------------------------" << std::endl;
            std::cout << "Camera #: " << idx << std::endl;
            std::cout << disc_info[idx] << std::endl;
        }

        std::cout << "------------------------------------------------------------------" << std::endl << std::endl;

        //std::cout << "Discovery Complete!  Press Enter to continue..." << std::endl << std::endl;
        
        std::cout << "Select Camera #: ";
        std::getline(std::cin, console_input);
        camera_num = stoi(console_input);
        
        if(camera_num >= disc_info.size())
        {
            std::cout << "Number selected is out of range of available cameras!  Exiting." << std::endl;
            return 1;
        }

        // set the ip address of the camera to connect to
        camera_ip_address = std::string(disc_info[camera_num].ip_address);

        // initialize the camera with the supplied ip address
        init();

        so_cam.get_network_params(net_params);

        std::cout << net_params << std::endl;

        // get the mode
        std::cout << "Select which mode to put the camera into (0 - DHCP, 1 - Static IP): ";
        std::getline(std::cin, console_input);
        mode = (uint8_t)stoi(console_input);        
        
        switch(mode)
        {
        case 0:
            new_ip_address = camera_ip_address;
            gateway_address = net_params.gateway;
            subnet_address = net_params.subnet;
            break;
            
        case 1:
            std::cout << "Enter the IP Address (192.168.1.25): ";
            std::getline(std::cin, console_input);
            new_ip_address = console_input;

            // get the gateway address
            std::cout << "Enter the Gateway Address (192.168.1.1): ";
            std::getline(std::cin, console_input);
            gateway_address = console_input;

            // get the gateway address
            std::cout << "Enter the Subnet Address (255.255.255.0): ";
            std::getline(std::cin, console_input);
            subnet_address = console_input;

            break;

        default:
            std::cout << "Mode selected is not valid!  Press Enter to close." << std::endl;
            std::cin.ignore(); 
            return 1;
            break;            
        }

        // setting the new mode using default ports
        bool status = so_cam.set_network_params(mode, new_ip_address, gateway_address, subnet_address);

        if(status)
            std::cout << "Setting network parameters successful!" << std::endl;
        else
            std::cout << "Error setting network parameters!" << std::endl;
            
        int bp = 0;

    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }
        
    // close the SO camera thread
    close_thread();

    std::cout << "Press Enter to close." << std::endl;
    std::cin.ignore();

    return 0;
    
}   // end of main
