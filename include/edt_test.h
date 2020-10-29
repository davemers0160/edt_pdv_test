#ifndef _EDT_TEST_H_
#define _EDT_TEST_H_

// C/C++ Includes
#include <cstdint>
#include <string>
#include <vector>

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#else

#endif

// include for the EDT PCI interface card
#include "edtinc.h"

// FTDI helper functions
#include "ftd2xx_functions.h"

// include lfor the Sierra-Olympic Camera code
#include "so_cam_commands.h"

// ----------------------------------------------------------------------------
constexpr auto EDT_UNIT_0 = 0;                      /* PCI interface for the EDT Crad */
constexpr auto VINDEN = 0;                          /* Channel that the Vinden Camera is connected to */
constexpr auto VENTUS = 0;                          /* Channel that the Ventus Camera is connected to */

// ----------------------------------------------------------------------------
bool recieve_serial_packet(FT_HANDLE driver, fip_protocol& packet)
{
	bool status = true;
	uint32_t count = 3;

	uint8_t h0, h1, l0, l1, length;

	std::vector<uint8_t> rx_data;

	// get the first 3 bytes of the fip packet which contain the header and length
	status &= receive_data(driver, h0);
	status &= receive_data(driver, h1);
	status &= receive_data(driver, l0);

	// merge all of the received data into a single vector
	std::vector<uint8_t> p = { h0, h1, l0 };

	// check the length to fit the fip protocol format
	if (l1 > 127)
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







#endif  // _EDT_TEST_H_
