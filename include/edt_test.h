#ifndef _EDT_TEST_H_
#define _EDT_TEST_H_

// C/C++ Includes
#include <cstdint>
#include <string>
#include <vector>

// include for the EDT PCI interface card
#include "edtinc.h"

// include lfor the Sierra-Olympic Camera code
#include "so_cam_commands.h"

constexpr auto EDT_UNIT_0 = 0;                      /* PCI interface for the EDT Crad */
constexpr auto VINDEN = 0;                          /* Channel that the Vinden Camera is connected to */


#endif  // _EDT_TEST_H_
