#define _CRT_SECURE_NO_WARNINGS

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



// Project Includes
#include "fuji_test.h"
#include "fujinon_lens.h"

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
void print_usage(void)
{
    std::cout << "Enter the ip address of the camera to connect:" << std::endl;
    std::cout << "example: cam_test 192.168.0.10" << std::endl;
}

// ----------------------------------------------------------------------------
void init(void)
{

}


// ----------------------------------------------------------------------------
void close_thread()
{

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

    //serial_port sp;
    std::string port_name;
    uint32_t wait_time = 5;
    uint32_t baud_rate = 115200;
    
    std::string error_msg;
    int32_t write_result;
    std::vector<uint8_t> rx_data;

    FLS::fujinon_lens fj;

    //if (argc == 1)
    //{
    //    print_usage();
    //    std::cin.ignore();
    //    return 0;
    //}
    
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    wait_time = 5;
    port_name = "COM4";

#elif defined(__linux__)
    wait_time = 5;
    port_name = "/dev/ttyS0";
    
#endif

    fj = fujinon_lens(port_name, wait_time);


    try
    {

        fj.connect();

        std::cout << fj << std::endl;

        fj.set_focus(5000);
        fj.set_zoom(4000);
        fj.set_iris(60);

        std::cout << "Focus: " << fj.get_focus() << std::endl;
        std::cout << "Zoom: " << fj.get_zoom() << std::endl;
        std::cout << "Iris: " << (uint32_t)fj.get_iris() << std::endl;
        std::cout << "Temp: " << fj.get_temp() << std::endl;
        std::cout << std::endl;

        int bp = 1;
    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }
    
    // close the port
    fj.close();
        
    std::cout << "Press enter to close..." << std::endl;
    std::cin.ignore();

    return 0;
    
}   // end of main
