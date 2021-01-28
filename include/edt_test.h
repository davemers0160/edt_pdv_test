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
//int32_t init_ip_camera(udp_info &udp_camera_info, std::string ip_address, std::string &error_msg)
//{
//
//	int32_t result;
//
//
//	// init the read portion of the UDP socket
//	result = init_udp_socket(udp_camera_info, error_msg);
//
//	// init the write portion of the UDP socket
//	udp_camera_info.ip_address = ip_address;
//	udp_camera_info.read_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());
//
//	udp_camera_info.write_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());
//	udp_camera_info.write_addr_obj.sin_port = htons(udp_camera_info.write_port);
//	udp_camera_info.write_addr_obj.sin_family = AF_INET;
//
//	return result;
//
//}	// end of init_ip_camera


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
