
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

//#include "edtinc.h"
//
//#include "so_cam_commands.h"

#include "edt_test.h"

// ----------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::string sdate, stime;

    uint64_t idx=0, jdx=0;

    typedef std::chrono::duration<double> d_sec;
    auto start_time = std::chrono::system_clock::now();
    auto stop_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<d_sec>(stop_time - start_time);
    
    std::string cfg_file = "../config/generic12cl.cfg";

    Edtinfo edtinfo;

    Dependent *dd_p;


    so_camera vinden;

    std::cout << vinden << std::endl;

    std::cout << vinden.lens << std::endl;

    std::cout << vinden.sensor << std::endl;


    auto t = vinden.lens.set_zoom_index(255).to_array();


    // test the reading in of a config file
    if ((dd_p = pdv_alloc_dependent()) == NULL)
    {
        std::cout << "alloc_dependent FAILED... exiting!" << std::endl;
        exit(1);
    }
    int result = pdv_readcfg(cfg_file.c_str(), dd_p, &edtinfo);
    
    
    PdvDev* pdv_p = pdv_open_channel(EDT_INTERFACE, EDT_UNIT_0, VINDEN);
    if (pdv_p == NULL)
    {
        std::cout << "Failed to connect... exiting!" << std::endl;
        exit(1);
    }

    std::cout << "edt_devname: " << std::string(pdv_p->edt_devname) << std::endl;
    
    // close the device
    result = pdv_close(pdv_p);
    
    std::cout << "close: " << result << std::endl;
   
    return 0;
    
}   // end of main
