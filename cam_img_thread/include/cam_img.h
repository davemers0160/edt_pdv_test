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
void init();
void update();
void close_edt();


// ----------------------------------------------------------------------------

#endif  // _EDT_TEST_H_
