#ifndef _EDT_TEST_H_
#define _EDT_TEST_H_

// C/C++ Includes
#include <cstdint>
#include <string>
#include <vector>

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
//#include "win_network_fcns.h"
#else
//#include "linux_network_fcns.h"
//typedef int32_t SOCKET;
#endif

// include for the EDT PCI interface card
#include "edtinc.h"

#if defined(USE_FTDI)
// FTDI helper functions
#include "ftd2xx_functions.h"
#endif	// USE_FTDI

// include the functions needed for udp comms
//#include "udp_network_functions.h"

// include lfor the Sierra-Olympic Camera code
#include "sierra_olympic_camera.h"

// OpenCV Includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp> 

// ----------------------------------------------------------------------------
constexpr auto EDT_UNIT_0 = 0;                      /* PCI interface for the EDT Card */
constexpr auto VINDEN = 0;                          /* Channel that the Vinden Camera is connected to */
constexpr auto VENTUS = 1;                          /* Channel that the Ventus Camera is connected to */

// ----------------------------------------------------------------------------
bool init_edt_device(std::string cfg_file, std::string bit_directory, int32_t unit, int32_t channel)
{
    char edt_devname[256] = { 0 };
    char errstr[64];
    char* progname = "initcam";

    EdtDev* edt_p = NULL;
    Edtinfo edt_info;
    Dependent* dd_p;


    // 
    if ((dd_p = pdv_alloc_dependent()) == NULL)
    {
        std::cout << "PDV alloc_dependent failed!" << std::endl;
        return false;
    }

    if (pdv_readcfg(cfg_file.c_str(), dd_p, &edt_info) != 0)
    {
        std::cout << "PDV readcfg failed!" << std::endl;
        free(dd_p);
        return false;
    }

    // open the device
    //unit = edt_parse_unit_channel(unitstr, edt_devname, EDT_INTERFACE, &channel);
    //edt_msg(EDTAPP_MSG_INFO_1, "opening %s unit %d....\n", edt_devname, unit);

    /*
     * IMPORTANT: pdv_initcam is a special case in that it requies a device pointer returned by use
     * edt_open_channel (or edt_open), NOT pdv_open_channel (or etc.). If you port this code to an
     * application that subsequently performs other operations (e.g. image capture) on the device,
     * edt_close should be called after pdv_initcam, then reopen with pdv_open_channel or pdv_open.
     */
    if ((edt_p = edt_open_channel(EDT_INTERFACE, unit, channel)) == NULL)
    {
        std::cout << "error in edt_open_channel: " << std::string(edt_devname) <<  unit << std::endl;
        edt_close(edt_p);
        free(dd_p);
        return false;
    }


    if (pdv_initcam(edt_p, dd_p, unit, &edt_info, cfg_file.c_str(), (char*)bit_directory.c_str(), 0) != 0)
    {
        std::cout << "PDV initcam failed." << std::endl;
        //edt_msg(EDTAPP_MSG_FATAL, "initcam failed. Run with '-V' to see complete debugging output\n");
        edt_close(edt_p);
        free(dd_p);
        return false;
    }

    // close teh edt device
    edt_close(edt_p);

    // free the Dependent pointer
    free(dd_p);

    return true;

}   // end of init_edt_device


void focus_trackbar_callback(int value, void* user_data)
{
	udp_info udp_camera_info = *(udp_info*)user_data;
	uint16_t focus_position = (uint16_t)value;

	int32_t write_result = send_udp_data(udp_camera_info, SO::lens_class().set_focus_position(focus_position).to_vector());

}

void zoom_trackbar_callback(int value, void* user_data)
{
	udp_info udp_camera_info = *(udp_info*)user_data;
	uint16_t zoom_index = (uint16_t)value;

	int32_t write_result = send_udp_data(udp_camera_info, SO::lens_class().set_zoom_index(zoom_index).to_vector());

}

void load_param_gui(udp_info &udp_camera_info)
{
    wind_protocol wind_data;
	int32_t focus_position = 0;
	int32_t zoom_index = 0;
    int32_t write_result, read_result;
    std::vector<uint8_t> rx_data;
    
	std::string parameter_window = "Parameter Options";
	cv::namedWindow(parameter_window, cv::WINDOW_NORMAL); // Create Window
	cv::resizeWindow(parameter_window, 1000, 20);
	
	// get the intial value for the focus position
    write_result = send_udp_data(udp_camera_info, SO::lens_class().get_focus_position().to_vector());
    read_result = receive_udp_data(udp_camera_info, rx_data);
    wind_data = wind_protocol(rx_data);
    focus_position = wind_data.payload[0] | wind_data.payload[1]<<8;

	// get the intial value for the zoom index
    write_result = send_udp_data(udp_camera_info, SO::lens_class().get_zoom_index().to_vector());
    read_result = receive_udp_data(udp_camera_info, rx_data);
    wind_data = wind_protocol(rx_data);
    zoom_index = wind_data.payload[0] | wind_data.payload[1]<<8;
    
	// create focus and zoom trackbars
	cv::createTrackbar("Focus Parameter", parameter_window, &focus_position, 65534, focus_trackbar_callback, &udp_camera_info);
	cv::createTrackbar("Zoom Parameter", parameter_window, &zoom_index, 18, zoom_trackbar_callback, &udp_camera_info);

}

// ----------------------------------------------------------------------------

#endif  // _EDT_TEST_H_
