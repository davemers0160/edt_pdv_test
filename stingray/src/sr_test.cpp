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
#include "sr_test.h"
#include "stingray.h"

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

    stingray_lens sr;

    //if (argc == 1)
    //{
    //    print_usage();
    //    std::cin.ignore();
    //    return 0;
    //}
    
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    wait_time = 10;
    port_name = "COM8";

#elif defined(__linux__)
    wait_time = 5;
    port_name = "/dev/ttyS0";
    
#endif

    sr = stingray_lens(port_name, baud_rate, wait_time);

    try
    {

        std::cout << sr << std::endl;


        // test of finding the CRLF in a string
        std::string status = "F=0\r\nZ=1000\r\nI=100\r\nCA0\r\nT=23.30\r\n";
        std::string err = "ERROR3\r\n";
        std::vector<std::string> params;

        parse_response(status, params);

        // parse the data
        uint16_t focus = (uint16_t)std::stoi(params[0].substr(2, std::string::npos));
        uint16_t zoom = (uint16_t)std::stoi(params[1].substr(2, std::string::npos));
        uint8_t iris = (uint8_t)std::stoi(params[2].substr(2, std::string::npos));
        uint8_t athermal_mode = (uint8_t)std::stoi(params[3].substr(2, std::string::npos));
        float temp = std::stof(params[4].substr(2, std::string::npos));


        parse_response(err, params);

        //sr.connect();

        int bp = 1;
    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }
    
    // close the port
    sr.close();
        
    std::cout << "Press enter to close..." << std::endl;
    std::cin.ignore();

    return 0;
    
}   // end of main
