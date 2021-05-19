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
    class lens {

    public:
    
        serial_port sp;
        std::string port_name;
        uint32_t wait_time;
        uint32_t baud_rate;
        
        lens() = default;

        lens(uint8_t maj_rev_, uint8_t min_rev_, uint16_t bn, uint16_t ct) : maj_rev(maj_rev_), min_rev(min_rev_), build_num(bn), camera_type(ct)
        {


        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get the camera serial number.

        This function returns the serial number captured from the camera when the connection is first established.

        @return sn : camera serial number.
        */
        std::string get_sn() { return sn; }

       
        //-----------------------------------------------------------------------------
        /**
        @brief Initialize the camera and get the current parameters.

        This function initializes the UDP connection to the camera for control and gets the relevant parameters of the camera.

        @return int32_t result of the connection attempt.
        */
        int32_t init(std::string ip_address, std::string& error_msg)
        {

            int32_t result;
            int32_t read_result, write_result;
            std::vector<uint8_t> rx_data;
            wind_protocol wind_data;
            fip_protocol fip_data;

            // init the read portion of the UDP socket
            result = init_udp_socket(udp_camera_info, error_msg);

            if (result == 0)
            {
                // init the write portion of the UDP socket
                udp_camera_info.ip_address = ip_address;
                udp_camera_info.read_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());

                udp_camera_info.write_addr_obj.sin_addr.s_addr = inet_addr(udp_camera_info.ip_address.c_str());
                udp_camera_info.write_addr_obj.sin_port = htons(udp_camera_info.write_port);
                udp_camera_info.write_addr_obj.sin_family = AF_INET;

                // get the SLA board version number
                write_result = send_udp_data(udp_camera_info, get_sla_board_version().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                fip_data = fip_protocol(rx_data);

                // get the image size
                write_result = send_udp_data(udp_camera_info, get_sla_image_size().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                fip_data = fip_protocol(rx_data);
                set_image_size(read2(&fip_data.data[2]), read2(&fip_data.data[0]));

                // get the camera wind version number
                write_result = send_udp_data(udp_camera_info, get_version().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                set_version(wind_data);

                // get the camera serial number
                write_result = send_udp_data(udp_camera_info, get_serial_number().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                set_sn(wind_data);

                // get the camera lens version
                write_result = send_udp_data(udp_camera_info, lens.get_version().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                lens.set_version(wind_data);

                // ----------------------------------------------------------------------------
                // get the camera lens zoom index
                write_result = send_udp_data(udp_camera_info, lens.get_zoom_index().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                lens.zoom_index = read2(&wind_data.payload[0]);

                // get the camera lens zoom position
                write_result = send_udp_data(udp_camera_info, lens.get_zoom_position().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                lens.zoom_position = read2(&wind_data.payload[0]);

                // get the camera lens zoom speed
                write_result = send_udp_data(udp_camera_info, lens.get_zoom_speed().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                lens.zoom_speed = (wind_data.payload[0]);

                // get the camera lens focus position
                write_result = send_udp_data(udp_camera_info, lens.get_focus_position().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                lens.focus_position = read2(&wind_data.payload[0]);

                // get the camera lens focus speed
                write_result = send_udp_data(udp_camera_info, lens.get_focus_speed().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                lens.focus_speed = (wind_data.payload[0]);

                // ----------------------------------------------------------------------------
                // get the sensor version number
                write_result = send_udp_data(udp_camera_info, sensor.get_version().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                sensor.set_version(wind_data);

                // get the FFC period
                write_result = send_udp_data(udp_camera_info, sensor.get_auto_ffc_period().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                sensor.ffc_period = read2(&wind_data.payload[0]);

                // get the FFC mode
                write_result = send_udp_data(udp_camera_info, sensor.get_auto_ffc_mode().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                wind_data = wind_protocol(rx_data);
                sensor.ffc_mode = wind_data.payload[0];

                // get the current display parameter settings
                write_result = send_udp_data(udp_camera_info, get_display_parameters().to_vector());
                read_result = receive_udp_data(udp_camera_info, rx_data);
                fip_data = fip_protocol(rx_data);

                // ----------------------------------------------------------------------------
                // remove the zoom
                write_result = send_udp_data(udp_camera_info, set_display_parameters(0x01, 0x4D).to_vector());

            }
            else
            {
                std::cout << "result: " << result << std::endl;
                std::cout << "error msg: " << error_msg << std::endl;
            }

            return result;

        }	// end of init_camera


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
            std::vector<uint8_t> rx_data;

            // set the value
            int32_t result = send_udp_data(udp_camera_info, lens.set_focus_position(value).to_vector());

            return result;
        }   // end of set_focus_position

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens focus position.

        This function sets the lens focus position.

        @param[out] value : position of the focus motor
        @return int32_t result of getting the focus position.

        @sa lens
        */
        int32_t get_focus_position(uint16_t &value)
        {
            std::vector<uint8_t> rx_data;

            // get the value
            int32_t result = send_udp_data(udp_camera_info,lens.get_focus_position().to_vector());
            result = receive_udp_data(udp_camera_info, rx_data);
            wind_protocol wind_data = wind_protocol(rx_data);
            if(wind_data.payload.size() > 0)
            {
                lens.focus_position =  read2(&wind_data.payload[0]);
                value = lens.focus_position;
            }

            return result;
        }   // end of get_focus_position

        //-----------------------------------------------------------------------------
        /**
        @brief Set the zoom index.

        This function sets the lens zoom index.

        @param[in] value : position to set the zoom motor to
        @return int32_t result of setting the zoom index.

        @sa lens
        */
        int32_t set_zoom_index(uint16_t value)
        {
            std::vector<uint8_t> rx_data;

            int32_t result = send_udp_data(udp_camera_info, lens.set_zoom_index(value).to_vector());

            // get the intial value for the zoom index
            result = send_udp_data(udp_camera_info, lens.get_zoom_index().to_vector());

            return result;
        }   // end of set_zoom_index

        //-----------------------------------------------------------------------------
        /**
        @brief Start the len autofocus mode.

        This function starts the lens autofocus mode.

        @param[in] mode : autofocus mode (0 -> non-blocking, 1 -> blocking)
        @return int32_t result of setting the zoom index.

        @sa lens
        */
        int32_t start_auto_focus(uint8_t mode)
        {
            std::vector<uint8_t> rx_data;

            int32_t result = send_udp_data(udp_camera_info, lens.start_autofocus(mode).to_vector());

            return result;
        }   // end of start_auto_focus



        //-----------------------------------------------------------------------------    
        /**
        @brief Close the UDP connection.

        This function closes the UDP connection to the camera.

        @return int32_t result with the status of closing.
        */
        int32_t close()
        {
            std::string error_msg;

            int32_t result = close_connection(udp_camera_info.udp_sock, error_msg);

            return result;
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


    };  // end camera class

}   // end of namespace

#endif  // _FUJINON_CAMERA_H_
