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

// ----------------------------------------------------------------------------
constexpr auto EDT_UNIT_0 = 0;                      /* PCI interface for the EDT Crad */
constexpr auto VINDEN = 0;                          /* Channel that the Vinden Camera is connected to */
constexpr auto VENTUS = 0;                          /* Channel that the Ventus Camera is connected to */

// ----------------------------------------------------------------------------
//int32_t init_ip_camera(std::string camera_ip, uint16_t read_port, SOCKET &read_socket, uint16_t write_port, SOCKET &write_socket)
int32_t init_ip_camera(udp_info &udp_camera_info, std::string ip_address)
{

	int32_t rx_result, tx_result;

	std::string error_msg;

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#else
	results = init_udp_socket(rx_port, rx_socket, error_msg);
#endif	

	// init the read portion of the UDP socket
	rx_result = init_udp_socket(udp_camera_info, error_msg);

	// init the write portion of the UDP socket
	udp_camera_info.ip_address = ip_address;
	udp_camera_info.read_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());

	udp_camera_info.write_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());
	udp_camera_info.write_addr_obj.sin_port = htons(udp_camera_info.write_port);
	udp_camera_info.write_addr_obj.sin_family = AF_INET;


	return 0;


}	// end of init_ip_camera


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
