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

// Project Includes
#include "edt_test.h"


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

    // generic config file for a 12-bit monochrome camera
    std::string cfg_file = "../config/generic12cl.cfg";

    // EDT PDV specific variable
    Edtinfo edtinfo;
    Dependent* dd_p;
    uint8_t* image_p;

    // Sierra-Olympic specific variables
    SO::camera vinden;
    uint32_t driver_device_num = 0;
    uint32_t connect_count = 0;
    uint32_t read_timeout = 30000;
    uint32_t write_timeout = 1000;
    uint32_t baud_rate = 115200;
    
#if defined(USE_FTDI)
    uint32_t ftdi_device_count = 0;
    ftdiDeviceDetails driver_details;
    std::vector<ftdiDeviceDetails> ftdi_devices;
    FT_HANDLE ctrl_handle = NULL;
#endif  // USE_FTDI

    // display the information about a specific camera
    std::cout << vinden << std::endl;

    auto t = vinden.lens.set_zoom_index(255).to_vector();

    try
    {
        // test the reading in of a config file
        std::cout << std::endl << "Reading in config file..." << std::endl;
        if ((dd_p = pdv_alloc_dependent()) == NULL)
        {
            std::cout << "alloc_dependent FAILED... exiting!" << std::endl;
            exit(1);
        }
        int result = pdv_readcfg(cfg_file.c_str(), dd_p, &edtinfo);

        // open a specifc board and channel
        std::cout << std::endl << "Opening: " << EDT_INTERFACE << ", Unit: " << EDT_UNIT_0 << ", Channel: " << VINDEN << std::endl;
        PdvDev* pdv_p = pdv_open_channel(EDT_INTERFACE, EDT_UNIT_0, VINDEN);
        if (pdv_p == NULL)
        {
            std::cout << "Failed to connect to a pdv device... exiting!  Press Enter to close" << std::endl;
            std::cin.ignore();
            exit(1);
        }

        // print out the device name
        std::cout << "edt_devname: " << std::string(pdv_p->edt_devname) << std::endl;

        // start the image capture process
        std::cout << std::endl << "Starting the image capture process..." << std::endl;
        pdv_start_image(pdv_p);

        // get the image and place into a unsigned char pointer
        // no clue how the packing is going to for int16_t ot uint16_t images
        std::cout << std::endl << "Grabbing the image..." << std::endl;
        image_p = pdv_wait_image(pdv_p);

        // close the device
        std::cout << std::endl << "Closing the pdv device..." << std::endl;
        result = pdv_close(pdv_p);

        std::cout << "close result: " << result << std::endl;

#if defined(USE_FTDI)

        ftdi_device_count = get_device_list(ftdi_devices);
        if (ftdi_device_count == 0)
        {
            std::cout << "No ftdi devices found... Exiting!" << std::endl;
            std::cin.ignore();
            return -1;
        }

        for (idx = 0; idx < ftdi_devices.size(); ++idx)
        {
            std::cout << ftdi_devices[idx];
        }

        std::cout << "Select Interface Number: ";
        std::getline(std::cin, console_input);
        driver_device_num = stoi(console_input);

        std::cout << std::endl << "Connecting to Controller..." << std::endl;
        ftdi_devices[driver_device_num].baud_rate = baud_rate;
        ftdi_devices[driver_device_num].stop_bits = FT_STOP_BITS_1;

        while ((ctrl_handle == NULL) && (connect_count < 10))
        {
            ctrl_handle = open_com_port(ftdi_devices[driver_device_num], read_timeout, write_timeout);
            ++connect_count;
        }

        if (ctrl_handle == NULL)
        {
            std::cout << "No Controller found... Exiting!" << std::endl;
            std::cin.ignore();
            return -1;
        }

        flush_port(ctrl_handle);

#endif  // USE_FTDI


    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }


    return 0;
    
}   // end of main
