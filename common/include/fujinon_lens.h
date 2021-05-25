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
    enum FUNCTION_CODES {

        CONNECT         = 0x01,     /**< Connection request */
        LENS_NAME1      = 0x11,     /**< Request for the first half of the lens name */
        LENS_NAME2      = 0x12,     /**< Request for the second half of the lens name */

        OPEN_FNUM       = 0x13,     /**< Request for Open F num */
        FL_TELE_END     = 0x14,     /**< Request for Tele-end focal length */
        FL_WIDE_END     = 0x15,     /**< Request for Wide-end focal length */
        MOD             = 0x16,     /**< Request for MOD */

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
    Error Codes

    These are the string error codes returned by the error response packet
    */
    const std::vector<std::string> error_codes = {
        "None",
        "Wrong command ID",
        "Wrond data size",
        "Argument out of range",
        "Wrong checksum",
        "Receive buffer full",
        "Communication timeout",
        "Boot up error",
        "Error while writing",
        "Error while reading",
        "Lens serial 1 commincation issue",
        "Sensor serial 2 commincation issue",
        "Command not implemented",
        "Telemtry error",
        "Undefined camera model",
        "Autofocus running"
    };

    const uint8_t STREAM_ON = 1;
    const uint8_t STREAM_OFF = 256;


    //-----------------------------------------------------------------------------
    /** @brief Fujinon Lens System Class

    This class build the packets in the to communicate with the camera.
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
        }

        fujinon_lens(std::string pn, uint32_t wt) : port_name(pn), wait_time(wt)
        {
            connected = false;
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
            uint64_t data_length = 3;
            
            if (port_name.empty())
            {
                std::cout << "port_name is empty" << std::endl;
                return 0;
            }

            // open up the serial port and connect
            sp.open_port(port_name, baud_rate, wait_time);
            connected = true;

            // send the connection request to the lens
            c10_protocol tx(FUNCTION_CODES::CONNECT);
            result = txrx_data(tx.to_vector(), rx_data, data_length);
            c10_protocol rx(rx_data);
            
            // check  for a valid return message
            if(rx.checksum_valid == true)
            {
                // get the name of the lens
                data_length = 15;
                
                tx = c10_protocol(FUNCTION_CODES::LENS_NAME1);
                result = txrx_data(tx.to_vector(), rx_data, data_length);
                rx = c10_protocol(rx_data);
                
                name = std::string(rx.data->begin(), rx.data->end());
                
                // check the size of the returned data
                if(rx.length == 15)
                {
                    // read in the second half of the name
                    c10_protocol tx(FUNCTION_CODES::LENS_NAME2);           
                    result = txrx_data(tx.to_vector(), rx_data, data_length);
                    rx = c10_protocol(rx_data);
                    
                    name += std::string(rx.data->begin(), rx.data->end());                   
                }
            }

            return 1;
        }   // end of connect
       
        //-----------------------------------------------------------------------------
        std::string get_name() { return name; }
        
        
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
            
            c10_protocol tx(FUNCTION_CODES::SET_FOCUS_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, 3);

            if(rx_data[1] == FUNCTION_CODES::SET_FOCUS_POS)
            {
                get_focus_position();
                result = 1;
            }

            return result;
        }   // end of set_focus_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens focus position.

        This function sets the lens focus position.

        @param[out] value : position of the focus motor
        @return int32_t result of getting the focus position.

        @sa c10_protocol
        */
        int32_t get_focus_position()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            
            c10_protocol tx(FUNCTION_CODES::GET_FOCUS_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, 5);
            c10_protocol rx(rx_data);

            if(rx.code == FUNCTION_CODES::GET_FOCUS_POS)
            {
                focus = to_uint16(rx.data);
                result = 1;
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
            
            c10_protocol tx(FUNCTION_CODES::SET_ZOOM_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, 3);

            if(rx_data[1] == FUNCTION_CODES::SET_ZOOM_POS)
            {
                get_zoom_position();
                result = 1;
            }

            return result;
        }   // end of set_zoom_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens zoom position.

        This function sets the lens zoom position.

        @param[out] value : position of the zoom motor
        @return int32_t result of getting the zoom position.

        @sa c10_protocol
        */
        int32_t get_zoom_position()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            
            c10_protocol tx(FUNCTION_CODES::GET_ZOOM_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, 5);
            c10_protocol rx(rx_data);

            if(rx.code == FUNCTION_CODES::GET_ZOOM_POS)
            {
                zoom = to_uint16(rx.data);
                result = 1;
            }

            return result;
        }   // end of get_zoom_position
        
        //-----------------------------------------------------------------------------
        /**
        @brief Set the iris position.

        This function sets the lens iris position.

        @param[in] value : position to set the iris motor to
        @return int32_t result of setting the iris position.

        @sa c10_protocol
        */
        int32_t set_iris_position(uint16_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            
            c10_protocol tx(FUNCTION_CODES::SET_IRIS_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, 3);

            if(rx_data[1] == FUNCTION_CODES::SET_IRIS_POS)
            {
                get_iris_position();
                result = 1;
            }

            return result;
        }   // end of set_iris_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens iris position.

        This function sets the lens iris position.

        @param[out] value : position of the iris motor
        @return int32_t result of getting the iris position.

        @sa c10_protocol
        */
        int32_t get_iris_position()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            
            c10_protocol tx(FUNCTION_CODES::GET_IRIS_POS, value);
            result = txrx_data(tx.to_vector(), rx_data, 5);
            c10_protocol rx(rx_data);

            if(rx.code == FUNCTION_CODES::GET_IRIS_POS)
            {
                iris = to_uint16(rx.data);
                result = 1;
            }

            return result;
        }   // end of get_iris_position
        
        
        //-----------------------------------------------------------------------------
        /**
        @brief Set the switch 0 value.

        This function sets the lens switch 0 value.

        @param[in] value : switch 0 value to set.
        @return int32_t result of setting the switch 0 value.

        @sa c10_protocol
        */
        int32_t set_switch_0(uint8_t value)
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            
            c10_protocol tx(FUNCTION_CODES::SET_SWITCH_0, value);
            result = txrx_data(tx.to_vector(), rx_data, 3);

            if(rx_data[1] == FUNCTION_CODES::SET_SWITCH_0)
            {
                result = 1;
            }

            return result;
        }   // end of set_switch_0

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens switch 0 value.

        This function sets the lens switch 0 value.

        @param[out] value : switch 0 value
        @return int32_t result of getting the switch 0 value.

        @sa c10_protocol
        */
        int32_t get_switch_0()
        {
            int32_t result = 0;
            std::vector<uint8_t> rx_data;
            
            c10_protocol tx(FUNCTION_CODES::GET_SWITCH_0, value);
            result = txrx_data(tx.to_vector(), rx_data, 4);
            c10_protocol rx(rx_data);

            if(rx.code == FUNCTION_CODES::GET_SWITCH_0)
            {
                result = 1;
            }

            return result;
        }   // end of get_switch_0
        
        //-----------------------------------------------------------------------------    
        /**
        @brief Close the connection.

        This function closes the serial port connection to the lens.

        @return int32_t result with the status of closing.
        */
        int32_t close()
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
            const camera& item
            )
        {
            out << "Camera:" << std::endl;
            out << "  Serial Number:    " << item.sn << std::endl;
            out << "  Firmware Version: " << (uint32_t)item.maj_rev << "." << (uint32_t)item.min_rev << "." << (uint32_t)item.build_num << "." << (uint32_t)item.camera_type << std::endl;
            out << "  Image size (h x w): " << item.height << " x " << item.width << std::endl;
            out << "Lens:" << std::endl;
            out << item.lens;
            out << "Sensor:" << std::endl;
            out << item.sensor << std::endl;
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

            if(d.size > 1)
            {
                v = d[0] << 8 | d[1];
            }
            
            return v;
        }   // end of to_uint16

        //-----------------------------------------------------------------------------
        inline float to_float(uint16_t value)
        {
            float v = (float)(v & 0x0FFF);
            int8_t expo = (int8_t)((int16_t)v >> 11);
            return (float) v * (10^expo);
        }

    };  // end camera class

}   // end of namespace

#endif  // _FUJINON_CAMERA_H_