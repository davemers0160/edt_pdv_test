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

#include "udp_network_functions.h"

// include for the EDT PCI interface card
#include "edtinc.h"

#if defined(USE_FTDI)
// FTDI helper functions
#include "ftd2xx_functions.h"
#endif	// USE_FTDI

// include lfor the Sierra-Olympic Camera code
#include "sierra_olympic_camera.h"

// OpenCV Includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp> 
#include <sierra_olympic_camera.h>

// ----------------------------------------------------------------------------
constexpr auto EDT_UNIT_0 = 0;                      /* PCI interface for the EDT Crad */
constexpr auto VINDEN = 0;                          /* Channel that the Vinden Camera is connected to */
constexpr auto VENTUS = 0;                          /* Channel that the Ventus Camera is connected to */

// ----------------------------------------------------------------------------
int32_t init_ip_camera(udp_info &udp_camera_info, std::string ip_address, std::string &error_msg)
{

	int32_t result;


	// init the read portion of the UDP socket
	result = init_udp_socket(udp_camera_info, error_msg);

	// init the write portion of the UDP socket
	udp_camera_info.ip_address = ip_address;
	udp_camera_info.read_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());

	udp_camera_info.write_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());
	udp_camera_info.write_addr_obj.sin_port = htons(udp_camera_info.write_port);
	udp_camera_info.write_addr_obj.sin_family = AF_INET;

	return result;

}	// end of init_ip_camera


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
	int32_t focus_position = 0;
	int32_t zoom_index = 0;

	std::string parameter_window = "Parameter Options";
	cv::namedWindow(parameter_window, cv::WINDOW_NORMAL); // Create Window
	cv::resizeWindow(parameter_window, 1000, 20);

	// create focus and zoom trackbars
	cv::createTrackbar("Focus Parameter", parameter_window, &focus_position, 65534, focus_trackbar_callback, &udp_camera_info);
	cv::createTrackbar("Zoom Parameter", parameter_window, &zoom_index, 18, zoom_trackbar_callback, &udp_camera_info);

	//
	//focus_trackbar_callback(0, &focus_position);
	//zoom_trackbar_callback(0, &zoom_position);

}

// ----------------------------------------------------------------------------
#if defined(USE_FTDI)
bool recieve_serial_packet(FT_HANDLE driver, fip_protocol& packet)
{
	bool status = true;
	uint32_t count = 3;

	uint8_t h0=0, h1=0, l0=0, l1=0, length=0;

	std::vector<uint8_t> rx_data;

	// get the first 3 bytes of the fip packet which contain the header and length
	status &= receive_data(driver, h0);
	status &= receive_data(driver, h1);
	status &= receive_data(driver, l0);

	// merge all of the received data into a single vector
	std::vector<uint8_t> p = { h0, h1, l0 };

	// check the length to fit the fip protocol format
	if (l0 > 127)
	{
		status &= receive_data(driver, l1);
		length = (l1 << 7) | (l0 & ~0x80);
		p.push_back(l1);
	}
	else
	{
		length = l0;
	}

	// use the length from the previous packet(s) to read in the remaining data
	status &= receive_data(driver, length, rx_data);

	// finish the merge
	p.insert(p.end(), rx_data.begin(), rx_data.end());

	packet = fip_protocol(p);

	return status;
}

#endif	//USE_FTDI


#endif  // _EDT_TEST_H_
