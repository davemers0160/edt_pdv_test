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
    std::cout << "Enter the comm port name that the lens is connected to:" << std::endl;
    std::cout << "example (Windows): COM4" << std::endl << "example (Linux): /dev/ttyS0" << std::endl;
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
    //uint32_t baud_rate = 38400;
    
    std::string error_msg;
    int32_t result;
    std::vector<uint8_t> rx_data;

    FLS::fujinon_lens fj;

    if (argc == 1)
    {
        print_usage();
        std::cin.ignore();
        return 0;
    }
    
    //wait_time = 5;
    port_name = std::string(argv[1]);

//#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
//
//
//#elif defined(__linux__)
//    wait_time = 5;
//    port_name = "/dev/ttyS0";
//    
//#endif

    fj = FLS::fujinon_lens(port_name, wait_time);

    c10_protocol tx((uint8_t)FLS::FUNCTION_CODE::SET_IRIS_POS, (uint16_t)0x8080);
    auto tx_v = tx.to_vector();

    try
    {

        fj.connect();

        std::cout << fj << std::endl;

        result = fj.enable_auto_iris(false);

        result = fj.set_focus_position(0x5400);
        result = fj.set_zoom_position(0x7400);
        result = fj.set_iris_position(0xDF9C);

        result = fj.get_focus_position();
        result = fj.get_zoom_position();
        result = fj.get_zoom_position();

        std::cout << "Focus: " << fj.get_focus() << std::endl;
        std::cout << "Zoom: " << fj.get_zoom() << std::endl;
        std::cout << "Iris: " << fj.get_iris() << std::endl;
        std::cout << std::endl;

        result = fj.set_focus_position(0xCC00);
        result = fj.set_zoom_position(0xA000);
        result = fj.set_iris_position(0x0000);

        result = fj.get_focus_position();
        result = fj.get_zoom_position();
        result = fj.get_iris_position();

        std::cout << "Focus: " << fj.get_focus() << std::endl;
        std::cout << "Zoom: " << fj.get_zoom() << std::endl;
        std::cout << "Iris: " << fj.get_iris() << std::endl;
        std::cout << std::endl;

        result = fj.get_focus_position();
        result = fj.get_zoom_position();
        result = fj.get_iris_position();

        std::cout << "Focus: " << fj.get_focus() << std::endl;
        std::cout << "Zoom: " << fj.get_zoom() << std::endl;
        std::cout << "Iris: " << fj.get_iris() << std::endl;
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
