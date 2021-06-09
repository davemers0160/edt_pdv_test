#ifndef _FUJINON_CAMERA_H_
#define _FUJINON_CAMERA_H_

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
#include "win_serial_fcns.h"
#include <windows.h>

#elif defined(__linux__)
#include <sys/ioctl.h>
#include "linux_serial_fcns.h"

#endif

#include <cstdint>
#include <iostream>
#include <vector>

// include the protocol headers
#include <c10_protocol.h>
#include <pelco_d_protocol.h>

/**
@file
Contains the classes to interface to a Fijinon Lens System (FLS).
*/

namespace FLS
{

    //-----------------------------------------------------------------------------
    /**
    Lens Function Codes

    These are the command codes to interact with the lens
    */
    enum class FUNCTION_CODE {

        CONNECT         = 0x01,     /**< Connection request */
        LENS_NAME1      = 0x11,     /**< Request for the first half of the lens name */
        LENS_NAME2      = 0x12,     /**< Request for the second half of the lens name */

        GET_FNUM        = 0x13,     /**< Request for Open F num */
        GET_TELE_END    = 0x14,     /**< Request for Tele-end focal length */
        GET_WIDE_END    = 0x15,     /**< Request for Wide-end focal length */
        GET_MOD         = 0x16,     /**< Request for MOD */

        SET_IRIS_POS    = 0x20,     /**< Set iris position -> 0x0000 - 0xFFFF */
        GET_IRIS_POS    = 0x30,     /**< Get iris position -> 0x0000 - 0xFFFF */
        SET_AUTO_IRIS   = 0x62,     /**< Set auto iris control -> 0x0A - 0x64 */
        GET_AUTO_IRIS   = 0x72,     /**< Get auto iris control -> 0x0A - 0x64 */
           
        SET_ZOOM_POS    = 0x21,     /**< Set zoom position -> 0x0000 - 0xFFFF */
        GET_ZOOM_POS    = 0x31,     /**< get zoom position -> 0x0000 - 0xFFFF */
        SET_ZOOM_SPEED  = 0x26,     /**< Set zoom speed -> 0x0000 - 0xFFFF */
        
        SET_FOCUS_POS   = 0x22,     /**< Set focus position -> 0x0000 - 0xFFFF */
        GET_FOCUS_POS   = 0x32,     /**< get focus position -> 0x0000 - 0xFFFF */
        SET_FOCUS_SPEED = 0x27,     /**< Set focus speed -> 0x0000 - 0xFFFF */
        
        SET_SWITCH_0    = 0x40,     /**< Set switch 0 -> 0xC8 - 0xFF */
        GET_SWITCH_0    = 0x50,     /**< Get switch 0 -> 0xC8 - 0xFF */
        SET_SWITCH_2    = 0x42,     /**< Set switch 2 -> 0x00 = auto, 0x10 = remote */
        GET_SWITCH_2    = 0x52,     /**< Get switch 2 -> 0x00 = auto, 0x10 = remote */
        SET_SWITCH_3    = 0x43,     /**< Set switch 3 -> 0xFE = x2.0, 0xFF = x1.0 */
        GET_SWITCH_3    = 0x53,     /**< Get switch 3 -> 0xFE = x2.0, 0xFF = x1.0 */
        SET_SWITCH_6    = 0x46,     /**< Set switch 6 -> 0x00 = on, 0x10 = off */
        GET_SWITCH_6    = 0x56,     /**< Get switch 6 -> 0x00 = on, 0x10 = off */
        
        SET_AUTOFOCUS   = 0xA5,     /**< Set autofocus control - bit 3-0: 0 = on, 1 = off, bit 4-7: 0-6 */
        GET_AUTOFOCUS   = 0xB5,     /**< Get autofocus control - bit 3-0: 0 = on, 1 = off, bit 4-7: 0-6 */

        SET_VIDEO_DELAY = 0xA8,     /**< Set video delay -> 0x00 - 0x80 */
        GET_VIDEO_DELAY = 0xB8      /**< Get video delay -> 0x00 - 0x80 */
    };

    //-----------------------------------------------------------------------------
    /**
    Switch 0 Codes for lower 4-bits

    These are the switch 0 codes for the lower 4-bits of the switch 0 input. This code should be OR'd with the SWITCH0_CODE_HIGH value.
    */  
    enum class SWITCH0_LOW
    {
        IR_950      = 0x0C,         /**< n-IR 950nm */
        IR_880      = 0x0D,         /**< n-IR 880nm */
        IR_850      = 0x0E,         /**< n-IR 850nm */
        NO_EFFECT   = 0x0F          /**< no effect */
    };

    /**
    Switch 0 Codes for higher 4-bits

    These are the switch 0 codes for the higher 4-bits of the switch 0 input.  This code should be OR'd with the SWITCH0_CODE_LOW value.
    */
    enum class SWITCH0_HIGH
    {
        FILTER_4    = 0xC0,         /**< Filter type 4 */
        FILTER_3    = 0xD0,         /**< Filter type 3 */
        FILTER_2    = 0xE0,         /**< Filter type 2 */
        FILTER_1    = 0xF0          /**< Filter type 1 */
    };

    //-----------------------------------------------------------------------------
    /**
    Switch 2 Codes

    These are the switch 2 codes
    */
    enum class SWITCH2
    {
        AUTO_IRIS   = 0xCF,         /**< auto iris control */
        REMOTE_IRIS = 0xDF          /**< remote iris control */
    };

    //-----------------------------------------------------------------------------
    /**
    Switch 3 Codes

    These are the switch 3 codes
    */
    enum class SWITCH3
    {
        MAG_X2  = 0xFE,         /**< Magnification = x1.0 */
        MAG_X1  = 0xFF          /**< Magnification = x2.0 */
    };

    //-----------------------------------------------------------------------------
    /**
    Switch 6 Codes

    These are the switch 6 codes
    */
    enum class SWITCH6
    {
        ON      = 0x00,         /**< Stabilizer on */
        OFF     = 0x01          /**< Stabilizer off */
    };


    //-----------------------------------------------------------------------------
    /**
    Auto Focus Switch 0 Codes

    These are the lower 4-bits for the auto focus switch 0 codes
    */
    enum class AF_SWITCH0
    {
        ON              = 0x00,         /**< Auto Focus on */
        OFF             = 0x01,         /**< Auto Focus off */

        //  These are the higher 4-bits for the Auto Focus Switch 0 Search Codes. This code should be OR'd with the ON/OFF 
        FULL            = 0x00,         /**< Full Range Search */
        HALF            = 0x10,         /**< 1/2 Range Search */
        QUARTER         = 0x20,         /**< 1/4 Range Search */
        EIGHTH          = 0x30,         /**< 1/8 Range Search */
        SIXTEENTH       = 0x40,         /**< 1/16 Range Search */
        THIRTYSECOND    = 0x50,         /**< 1/32 Range Search */
        SIXTYFOURTH     = 0x60          /**< 1/64 Range Search */    
    };

    //-----------------------------------------------------------------------------
    /** @brief Fujinon Lens System Class

    This class builds the required packets to communicate with the Fujinon lens.
    */
    class fujinon_lens {

    public:
    
        serial_port sp;
        std::string port_name;
        uint32_t wait_time;

        //-----------------------------------------------------------------------------
        fujinon_lens()
        {
            port_name = "";
            wait_time = 5;
            baud_rate = 38400;
            connected = false;

            focus = 0;
            zoom = 0;
            iris = 0;
        }

        fujinon_lens(std::string pn, uint32_t wt) : port_name(pn), wait_time(wt)
        {
            baud_rate = 38400;
            connected = false;

            focus = 0;
            zoom = 0;
            iris = 0;
        }

        //-----------------------------------------------------------------------------
        uint16_t get_focus() { return focus; }
        uint16_t get_zoom() { return zoom; }
        uint16_t get_iris() { return iris; }
        

        //-----------------------------------------------------------------------------
        uint32_t connect()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint64_t rx_length = 3;
            
            if (port_name.empty())
            {
                std::cout << "port_name is empty" << std::endl;
                return 0;
            }

            // open up the serial port and connect
            sp.open_port(port_name, baud_rate, wait_time);
            connected = true;

            // send the connection request to the lens
            c10_protocol tx((uint8_t)FUNCTION_CODE::CONNECT);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::CONNECT))
            {
                c10_protocol rx(rx_data);

                // check  for a valid return message
                if (rx.valid_checksum() == true)
                {
                    // get the name of the lens
                    result = get_name();

                    result = get_focus_position();
                    result = get_zoom_position();
                    result = get_iris_position();
                }
            }

            return result;
        }   // end of connect
       
        //-----------------------------------------------------------------------------
        int32_t get_name() 
        { 
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint64_t rx_length = 18;
            c10_protocol rx;

            c10_protocol tx((uint8_t)FUNCTION_CODE::LENS_NAME1);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            if ((rx_data.size() > 2) && (rx_data[1] == (uint8_t)FUNCTION_CODE::LENS_NAME1))
            {
                rx = c10_protocol(rx_data);
                name = std::string(rx.data.begin(), rx.data.end());
                result = 1;
            }
            else
            {
                name = "";
                result = 0;
            }

            // check the size of the returned data
            if (rx.length == 15)
            {
                // read in the second half of the name
                tx = c10_protocol((uint8_t)FUNCTION_CODE::LENS_NAME2);
                result = txrx_data(tx.to_vector(), rx_data, rx_length);
                        
                if ((rx_data.size() > 2) && (rx_data[1] == (uint8_t)FUNCTION_CODE::LENS_NAME2))
                {
                    rx = c10_protocol(rx_data);
                    name += std::string(rx.data.begin(), rx.data.end());
                    result = 1;
                }
            }

            return result; 
        }
        
        
        //-----------------------------------------------------------------------------
        /**
        @brief Set the lens focus position.

        This function sets the lens focus position.

        @param[in] value : position to set the focus motor to
        @return int32_t result of setting the focus position.

        @sa lens
        */
        int32_t set_focus_position(uint16_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 3;

            c10_protocol tx((uint8_t)FUNCTION_CODE::SET_FOCUS_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::SET_FOCUS_POS))
            {
                result = get_focus_position();
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of set_focus_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens focus position.

        This function gets the lens focus position.

        @param[out] value : position of the focus motor
        @return int32_t result of getting the focus position.

        @sa c10_protocol
        */
        int32_t get_focus_position()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 5;

            c10_protocol tx((uint8_t)FUNCTION_CODE::GET_FOCUS_POS);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::GET_FOCUS_POS))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    focus = to_uint16(rx.data);
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_focus_position

        //-----------------------------------------------------------------------------
        /**
        @brief Set the zoom position.

        This function sets the lens zoom position.

        @param[in] value : position to set the zoom motor to
        @return int32_t result of setting the zoom position.

        @sa c10_protocol
        */
        int32_t set_zoom_position(uint16_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 3;

            c10_protocol tx((uint8_t)FUNCTION_CODE::SET_ZOOM_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::SET_ZOOM_POS))
            {
                result = get_zoom_position();
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of set_zoom_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens zoom position.

        This function gets the lens zoom position.

        @param[out] value : position of the zoom motor
        @return int32_t result of getting the zoom position.

        @sa c10_protocol
        */
        int32_t get_zoom_position()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 5;

            c10_protocol tx((uint8_t)FUNCTION_CODE::GET_ZOOM_POS);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::GET_ZOOM_POS))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    zoom = to_uint16(rx.data);
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_zoom_position
        
        //-----------------------------------------------------------------------------
        /**
        @brief Set the iris position.

        This function sets the lens iris position.

        @param[in] value : position to set the iris motor to.
        @return int32_t result of setting the iris position.

        @sa c10_protocol
        */
        int32_t set_iris_position(uint16_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 3;

            c10_protocol tx((uint8_t)FUNCTION_CODE::SET_IRIS_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::SET_IRIS_POS))
            {
                result = get_iris_position();
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of set_iris_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens iris position.

        This function gets the lens iris position.

        @return int32_t result of getting the iris position.

        @sa c10_protocol
        */
        int32_t get_iris_position()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 5;

            c10_protocol tx((uint8_t)FUNCTION_CODE::GET_IRIS_POS);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::GET_IRIS_POS))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    iris = to_uint16(rx.data);
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_iris_position
        
        //-----------------------------------------------------------------------------
        /**
        @brief Set the auto iris parameter.

        This function sets the lens auto iris parameter.

        @param[in] value : value to set the auto iris parameter to.
        @return int32_t result of setting the auto iris parameter.

        @sa c10_protocol
        */
        int32_t set_auto_iris_param(uint8_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 3;

            // make sure that the input value is within the valid range for the lens
            value = max(min(value, 0x64), 0x0A);

            c10_protocol tx((uint8_t)FUNCTION_CODE::SET_AUTO_IRIS, value);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::SET_AUTO_IRIS))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of set_auto_iris_param

        //-----------------------------------------------------------------------------
        /**
        @brief Get the auto iris parameter.

        This function gets the lens auto iris parameter.

        @param[out] value : auto iris parameter.
        @return int32_t result of getting the auto iris parameter.

        @sa c10_protocol
        */
        int32_t get_auto_iris_param(uint8_t &value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 4;

            c10_protocol tx((uint8_t)FUNCTION_CODE::GET_AUTO_IRIS);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::GET_AUTO_IRIS))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    value = rx.data[0];
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_auto_iris_param

        //-----------------------------------------------------------------------------
        /**
        @brief Set the switch X value.

        This function sets the lens switch X value.

        @param[in] sw : the switch number to set.
        @param[in] value : the value to set.
        @return int32_t result of setting the switch X value.

        @sa c10_protocol
        */
        int32_t set_switch(uint8_t sw, uint8_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 3;
            c10_protocol tx;
            uint8_t code = 0;

            switch (sw)
            {
            case 0:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_0;
                break;

            case 2:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_2;
                break;

            case 3:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_3;
                break;

            case 6:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_6;
                break;
            }

            tx = c10_protocol(code, value);

            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == code))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of set_switch

        //-----------------------------------------------------------------------------
        /**
        @brief Get the  switch X value from the lens.

        This function gets the switch X value from the lens.

        @param[in] sw : the switch number to get.
        @param[out] value : switch X value
        @return int32_t result of getting the switch X value.

        @sa c10_protocol
        */
        int32_t get_switch_0(uint8_t sw, uint8_t &value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 4;
            uint8_t code = 0;

            switch (sw)
            {
            case 0:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_0;
                break;

            case 2:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_2;
                break;

            case 3:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_3;
                break;

            case 6:
                code = (uint8_t)FUNCTION_CODE::SET_SWITCH_6;
                break;
            }

            c10_protocol tx(code);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == code))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    value = rx.data[0];
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_switch
        
        //-----------------------------------------------------------------------------
        /**
        @brief Enable/Disable the auto iris.

        This function enables/disables the auto iris functionality.

        @param[in] value : value to set the iris: true -> enable, false -> disable.
        @return int32_t result of setting the auto iris parameter.

        @sa c10_protocol
        */
        int32_t enable_auto_iris(bool value)
        {
            uint8_t v = (uint8_t)(value ? SWITCH2::AUTO_IRIS : SWITCH2::REMOTE_IRIS);
            int32_t result = set_switch(2, v);
            return result;
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set the video delay parameter.

        This function sets the video delay for the lens.

        @param[in] value : value to set the video delay for the lens.
        @return int32_t result of setting the video delay parameter.

        @sa c10_protocol
        */
        int32_t set_video_delay(uint8_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 3;

            // make sure that the input value is within the valid range for the lens
            value = max(min(value, 0x80), 0x00);

            c10_protocol tx((uint8_t)FUNCTION_CODE::SET_VIDEO_DELAY, value);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::SET_VIDEO_DELAY))
            {
                result = 1;
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of set_video_delay


        //-----------------------------------------------------------------------------
        /**
        @brief Get the video delay parameter.

        This function gets the lens video delay parameter.

        @param[out] value : video delay parameter.
        @return int32_t result of getting the video delay parameter.

        @sa c10_protocol
        */
        int32_t get_video_delay(uint8_t& value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 4;

            c10_protocol tx((uint8_t)FUNCTION_CODE::GET_VIDEO_DELAY);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::GET_VIDEO_DELAY))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    value = rx.data[0];
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_video_delay

        //-----------------------------------------------------------------------------
        /**
        @brief Get the narrow end focal length of the lens.

        This function gets the lens narrow end focal length.

        @param[out] value : narrow end focal length.
        @return int32_t result of getting the narrow end focal length.

        @sa c10_protocol
        */
        int32_t get_narrow_focal_length(float& value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            uint8_t rx_length = 5;

            c10_protocol tx((uint8_t)FUNCTION_CODE::GET_TELE_END);
            result = txrx_data(tx.to_vector(), rx_data, rx_length);

            // check for the right number of received bytes and that the returned code is as expected
            if ((result >= rx_length) && (rx_data[1] == (uint8_t)FUNCTION_CODE::GET_TELE_END))
            {
                c10_protocol rx(rx_data);

                // checksum validation
                if (rx.valid_checksum() == true)
                {
                    value = to_float(rx.data);
                    result = 1;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                result = 0;
            }

            return result;
        }   // end of get_narrow_focal_length


        //-----------------------------------------------------------------------------    
        /**
        @brief Close the serial port connection.

        This function closes the serial port connection to the lens.
        */
        void close()
        {
            if (connected)
            {
                sp.close_port();
                connected = false;
            }
        }   // end of close

        //-----------------------------------------------------------------------------    
        inline friend std::ostream& operator<< (
            std::ostream& out,
            const fujinon_lens& item
            )
        {
            out << "Lens:" << std::endl;
            out << "  name:       " << item.name << std::endl;
            out << "  port name:  " << item.port_name << std::endl;
            out << "  baud rate:  " << item.baud_rate << std::endl;
            out << "  wait time:  " << item.wait_time << std::endl;
            out << "  focus step: " << item.focus << std::endl;
            out << "  zoom step:  " << item.zoom << std::endl;
            out << "  iris step:  " << item.iris << std::endl;
            return out;
        }

    private:

        std::string name;
        
        bool connected = false;
        uint16_t focus;
        uint16_t zoom;
        uint16_t iris;
        
        uint32_t baud_rate = 38400;
        
        //-----------------------------------------------------------------------------
        inline int32_t txrx_data(std::vector<uint8_t> tx_data, std::vector<uint8_t> &rx_data, uint64_t bytes_to_read)
        {
            int32_t result = -1;
            
            if (connected)
            {
                rx_data.clear();
                
                // flush teh serial port to remove any unwanted data
                sp.flush_port();

                // send the status message to trigger the kens to return status info
                result = (int32_t)sp.write_port(tx_data);

                // get the returned string from the lens
                result = (int32_t)sp.read_port(rx_data, bytes_to_read);

                // check the error code returned
                //result = (int32_t)get_error_code(rx_msg);
            }

            return result;
        }   // end of txrx_data
        
        //-----------------------------------------------------------------------------
        inline uint16_t to_uint16(std::vector<uint8_t> d)
        {
            uint16_t v;

            if(d.size() > 1)
            {
                v = d[0] << 8 | d[1];
            }
            
            return v;
        }   // end of to_uint16

        //-----------------------------------------------------------------------------
        inline float to_float(std::vector<uint8_t> value)
        {

            float mant = 0.0;
            int8_t expo = 0;
            if (value.size() > 1)
            {
                mant = (float)((value[0] & 0x0F) << 8 | value[1]);

                //expo = (int8_t)(((int16_t)value[0]) >> 4);
                expo = (int8_t)(value[0] >> 4);
                expo = (int8_t)((expo & 0x08) ? expo | 0xF0 : expo);
            }
            return (float)(mant * std::powf(10.0, expo));
        }

        //-----------------------------------------------------------------------------
        inline float to_float(uint16_t value)
        {
            float mant = (float)(value & 0x0FFF);

            int8_t expo = (int8_t)(value >> 12);
            expo = (int8_t)((expo & 0x08) ? expo | 0xF0 : expo);

            return (float)(mant * std::powf(10.0, expo));
        }


    };  // end camera class

}   // end of namespace

#endif  // _FUJINON_CAMERA_H_
