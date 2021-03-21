#ifndef _SO_CAM_COMMANDS_H_
#define _SO_CAM_COMMANDS_H_

#include <cstdint>

// include the functions needed for udp comms
#include "udp_network_functions.h"

// include the protocol headers
#include <wind_protocol.h>
#include <fip_protocol.h>

/**
@file
Contains the classes to interface to a camera.
*/

/**
Helper to read 4 bytes into a uint32_t
 */
inline uint32_t read4(const uint8_t* ptr)
{
    return (uint32_t)(ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24));
}

/**
Helper to read 3 bytes into a uint32_t
 */
inline uint32_t read3(const uint8_t* ptr)
{
    return (uint32_t)(ptr[0] | (ptr[1] << 8) | (ptr[2] << 16));
}

/**
Helper to read 2 bytes into a uint16_t
 */
inline uint16_t read2(const uint8_t* ptr)
{
    return (uint16_t)(ptr[0] | (ptr[1] << 8));
}

namespace SO
{

    //-----------------------------------------------------------------------------
    /**
    System Commands

    These are the system command IDs to interact with the camera.  These apply to the Sightline board
    */
    enum system_command {

        SO_ERROR_RESPONSE = 0x00,       /**< Error responce command ID */

        GET_CAMERA_VER = 0x01,          /**< Camera version command ID */

        END_CAMERA_CTRL = 0x02,         /**< End camera control command ID */

        GET_CAMERA_SN = 0x05,           /**< Camera serial number command ID */

        SET_NETWORK_PARAMS = 0x1C       /**< Set Static Network IP Address */

    };

    /**
    Lens Commands

    These are the command IDs to interact with the camera lens
    */
    enum lens_command {

        LENS_VERSION = 0x41,       /**< Lens version command ID */
        LENS_READY = 0x42,       /**< Lens ready command ID */

        SET_ZOOM_INDEX = 0x45,       /**< Set zoom index command ID */
        GET_ZOOM_INDEX = 0x46,       /**< Get zoom index command ID */

        GET_ZOOM_MTR_POS = 0x47,       /**< Get zoom motor position command ID */

        SET_FOCUS_POS = 0x48,       /**< Set focus position command ID */
        GET_FOCUS_POS = 0x49,       /**< get focus position command ID */
        SET_INF_FOCUS = 0x4A,       /**< Set infinite focus command ID */

        START_AUTOFOCUS = 0x4C,       /**< Start autofocus command ID */
        STOP_AUTOFOCUS = 0x4D,       /**< Stop autofocus command ID */

        SET_ZOOM_SPEED = 0x4E,       /**< Set zoom speed command ID */
        GET_ZOOM_SPEED = 0x4F,       /**< Get zoom speed command ID */

        SET_FOCUS_SPEED = 0x50,       /**< Set focus speed command ID */
        GET_FOCUS_SPEED = 0x51,       /**< Get focus speed command ID */

        RUN_CONT_ZOOM = 0x52,       /**< Run continuous zoom command ID */
        RUN_CONT_FOCUS = 0x53,       /**< Run continuous focus command ID */

        SET_LENS_BLUR = 0x54        /**< Set lens blur command ID */
    };

    /**
    Sensor Commands

    These are the command IDs to interact with the camera sensor
    */
    enum sensor_command {
        SENSOR_VERSION = 0x81,       /**< Get sensor version command ID */

        FLAT_FIELD_CORR = 0x82,       /**< Perform flat field correction command ID */

        LOAD_NUC_TABLE = 0x83,       /**< Load NUC table command ID */
        GET_NUC_TABLE = 0x84,       /**< Get loaded NUC table command ID */

        SET_AUTO_FFC_PER = 0x85,       /**< Set auto FFC period command ID */
        GET_AUOT_FFC_PER = 0x86,       /**< Get auto FFC period command ID */

        SET_AUTO_FFC_MODE = 0x87,       /**< Set auto FFC mode command ID */
        GET_AUTO_FFC_MODE = 0x88,       /**< Get auto FFC mode command ID */

        SET_SHUTTER = 0x89,       /**< Set shutter command ID */

        GET_SENSOR_READY = 0x8A,       /**< Get sensor ready command ID */
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

    const uint16_t STREAM_ON = 1;
    const uint16_t STREAM_OFF = 256;

    //-----------------------------------------------------------------------------
    /** @brief Sierra Olympic Lens Class

    This class build the packets in the to communicate with the camera lens.
    */
    class lens_class {

    public:
        uint16_t fw_maj_rev;
        uint16_t fw_min_rev;
        uint16_t sw_maj_rev;
        uint16_t sw_min_rev;

        uint16_t zoom_index;
        uint16_t zoom_position;
        uint8_t zoom_speed;

        uint16_t focus_position;
        uint8_t focus_speed;

        //-----------------------------------------------------------------------------
        lens_class() = default;

        lens_class(uint16_t fw_maj, uint16_t fw_min, uint16_t sw_maj, uint16_t sw_min) : fw_maj_rev(fw_maj), fw_min_rev(fw_min), sw_maj_rev(sw_maj), sw_min_rev(sw_min)
        {
            zoom_index = 0;
            zoom_position = 0;
            zoom_speed = 0;

            focus_position = 0;
            focus_speed = 0;
        }

        lens_class(std::vector<uint8_t> data)
        {
            fw_maj_rev = data[1] << 8 | data[0];
            fw_min_rev = data[3] << 8 | data[2];
            sw_maj_rev = data[5] << 8 | data[4];
            sw_min_rev = data[7] << 8 | data[6];

            zoom_index = 0;
            zoom_position = 0;
            zoom_speed = 0;

            focus_position = 0;
            focus_speed = 0;
        }

        //-----------------------------------------------------------------------------
        void set_version(wind_protocol ver)
        {
            if (ver.payload.size() >= 8)
            {
                fw_maj_rev = read2(&ver.payload[0]);
                fw_min_rev = read2(&ver.payload[2]);
                sw_maj_rev = read2(&ver.payload[4]);
                sw_min_rev = read2(&ver.payload[6]);
            }
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get the lens version.

        This function builds a fip protocol packet to retrieve the camera lens version.

        @return fip_protocol that contains the structure to retrieve the lens version.

        @note The returned data will contain the FW Major Revision, FW Minor Revision, SW Major Version, SW Minor Version.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_version(void)
        {
            wind_protocol lens_packet(LENS_VERSION);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get lens ready.

        This function builds a fip protocol packet to determine if the camera lens is ready.

        @return fip_protocol that contains the structure to retrieve the lens status.

        @note The returned status will be 0 when the lens is busy and unable to accept new commands, and while autofocus is running.  Otherwise the statua will be set to 1.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol ready(void)
        {
            wind_protocol lens_packet(LENS_READY);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set zoom index.

        This function builds a fip protocol packet to set the camera to go to the specified zoom index.

        @param[in] value Zoom Index : (0-Max Zoom Index)
        @return fip_protocol that contains the structure to set the zoom index.

        @note The command will return immediately, even while the lens is moving to the sent index.
        Focal Length    Max Zoom Index
          - 15-75           17
          - 25-150          26
          - 40-300          18

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_zoom_index(uint16_t value)
        {
            wind_protocol lens_packet(SET_ZOOM_INDEX);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get zoom index.

        This function builds a fip protocol packet to get the zoom motor position that the lens is currently poisitioned at.

        @return fip_protocol that contains the structure to get the zoom index.

        @note The returned packet will contain the zoom index of the camera lens.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_zoom_index(void)
        {
            wind_protocol lens_packet(GET_ZOOM_INDEX);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get zoom position.

        This function builds a fip protocol packet to get the zoom motor position that the lens is currently poisitioned at.

        @return fip_protocol that contains the structure to get the zoom position.

        @note The returned packet will contain the zoom position of the camera lens.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_zoom_position(void)
        {
            wind_protocol lens_packet(GET_ZOOM_MTR_POS);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set focus position.

        This function builds a fip protocol packet to set the camera to the specified motor position. The command will return immediately, even while the lens is moving to the send position.

        @param[in] value Position : (0-65534)
        @return fip_protocol that contains the structure to set the focus position.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_focus_position(uint16_t value)
        {
            wind_protocol lens_packet(SET_FOCUS_POS);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get focus position.

        This function builds a fip protocol packet to get the focus position of the camera lens.

        @return fip_protocol that contains the structure to get the focus position.

        @note The returned packet will contain the focus position of the camera lens.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_focus_position(void)
        {
            wind_protocol lens_packet(GET_FOCUS_POS);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set infinity focus.

        This function builds a fip protocol packet to set the focus motor to the len's calibrated infinity focus position, which corresponds to the focus position 32767.

        @return fip_protocol that contains the structure to set the lens to the infinity focus.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_infinity_focus(void)
        {
            wind_protocol lens_packet(SET_INF_FOCUS);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Start autofocus.

        This function builds a fip protocol packet to start the camera autofocus.

          - In non-blocking mode, the command will return a response before autofocus has completed. The autofocus can be queried on whether it is still running with the "Get Lens Ready" command and can be stopped with the "Abort Autofocus" command; all other commands will return an error.

          - In blocking mode the Autofocus command to only return once autofocus is complete.

        @param[in] value 0 - Non Blocking, 1 - Blocking
        @return fip_protocol that contains the structure to start the autofocus.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol start_autofocus(uint8_t value)
        {
            wind_protocol lens_packet(START_AUTOFOCUS);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Stop autofocus.

        This function builds a fip protocol packet to stop the camera autofocus.  Returns when autofocus is successfully aborted.

        @return fip_protocol that contains the structure to stop the autofocus.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol stop_autofocus(void)
        {
            wind_protocol lens_packet(STOP_AUTOFOCUS);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set zoom speed.

        This function builds a fip protocol packet to set the lens zoom speed.

        @param[in] value Speed : 0 is the default manufacturer speed of the lens. 1 is the slowest speed the lens will zoom at, and 100 is the fastest.
        @return fip_protocol that contains the structure to set the zoom speed.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_zoom_speed(uint8_t value)
        {
            wind_protocol lens_packet(GET_ZOOM_INDEX);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get zoom speed.

        This function builds a fip protocol packet to get the lens zoom speed.

        @return fip_protocol that contains the structure to set the focus position.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_zoom_speed(void)
        {
            wind_protocol lens_packet(GET_ZOOM_INDEX);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set focus speed.

        This function builds a fip protocol packet to set the lens focus speed.  This speed will effect the speed and performance of autofocus.

        @param[in] value Speed : 0 is the default manufacturer speed of the lens. 1 is the slowest speed the lens will zoom at, and 100 is the fastest.
        @return fip_protocol that contains the structure to set the focus speed.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_focus_speed(uint8_t value)
        {
            wind_protocol lens_packet(GET_ZOOM_INDEX);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get focus speed.

        This function builds a fip protocol packet to Get the lens focus speed.

        @return fip_protocol that contains the structure to get the focus speed.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_focus_speed(void)
        {
            wind_protocol lens_packet(GET_ZOOM_INDEX);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Run continuous zoom.

        This function builds a fip protocol packet to start zooming in either the narrow or wide direction at the set zoom speed, or stops a zoom in progress.

        The zoom may stop at a position that is not directly related to a zoom index position, in this case the zoom index will return the closest zoom index in the wide direction.

        @param[in] value Mode : 0 - Stop, 1 - Zoom Narrow, 2- Zoom Wide
        @return fip_protocol that contains the structure to set the zoom mode.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol run_continuous_zoom(uint8_t value)
        {
            wind_protocol lens_packet(RUN_CONT_ZOOM);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Run continuous focus.

        This function builds a fip protocol packet to start focusing in either the the far or near direction at the set focus speed, or stops a focus in progress.

        @param[in] value Mode : 0 - Stop, 1 - Focus Far, 2- Focus Near
        @return fip_protocol that contains the structure to set the focus mode.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol run_continuous_focus(uint8_t value)
        {
            wind_protocol lens_packet(RUN_CONT_FOCUS);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set lens blur.

        This function builds a fip protocol packet to blur the lens so a flat field correction can be performed. A lens blur is automatically performed when the flat field correction command is issued and blur lens is set to 1.

        @param[in] value Mode : 0 - Unblur lens, 1 - Blur lens
        @return fip_protocol that contains the structure to set the blur mode.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_lens_blur(uint8_t value)
        {
            wind_protocol lens_packet(SET_LENS_BLUR);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        inline friend std::ostream& operator<< (
            std::ostream& out,
            const lens_class& item
            )
        {
            out << "  Firmware Version: " << (uint32_t)item.fw_maj_rev << "." << (uint32_t)item.fw_min_rev;
            //out << "  Software Version: " << (uint32_t)item.sw_maj_rev << "." << (uint32_t)item.sw_min_rev << std::endl;
            out << "." << (uint32_t)item.sw_maj_rev << "." << (uint32_t)item.sw_min_rev << std::endl;
            out << "  Zoom Index:       " << (uint32_t)item.zoom_index << std::endl;
            out << "  Zoom Position:    " << (uint32_t)item.zoom_position << std::endl;
            out << "  Zoom Speed:       " << (uint32_t)item.zoom_speed << std::endl;
            out << "  Focus Position:   " << (uint32_t)item.focus_position << std::endl;
            out << "  Focus Speed:      " << (uint32_t)item.focus_speed << std::endl;
            return out;
        }

    private:
        uint8_t port = 12;

    };  // end lens_class


    //-----------------------------------------------------------------------------
    /** @brief Sierra Olympic Sensor Class

    This class build the packets in the to communicate with the camera sensor.
    */
    class sensor_class {

    public:
        uint16_t fw_maj_rev;
        uint16_t fw_min_rev;
        uint16_t sw_maj_rev;
        uint16_t sw_min_rev;

        uint16_t ffc_period;
        uint8_t ffc_mode;

        sensor_class() = default;

        sensor_class(uint16_t fw_maj, uint16_t fw_min, uint16_t sw_maj, uint16_t sw_min) : fw_maj_rev(fw_maj), fw_min_rev(fw_min), sw_maj_rev(sw_maj), sw_min_rev(sw_min)
        {
            ffc_period = 0;
            ffc_mode = 0;
        }

        sensor_class(std::vector<uint8_t> data)
        {
            fw_maj_rev = data[1] << 8 | data[0];
            fw_min_rev = data[3] << 8 | data[2];
            sw_maj_rev = data[5] << 8 | data[4];
            sw_min_rev = data[7] << 8 | data[6];

            ffc_period = 0;
            ffc_mode = 0;
        }

        //-----------------------------------------------------------------------------
        void set_version(wind_protocol ver)
        {
            if (ver.payload.size() >= 8)
            {
                fw_maj_rev = read2(&ver.payload[0]);
                fw_min_rev = read2(&ver.payload[2]);
                sw_maj_rev = read2(&ver.payload[4]);
                sw_min_rev = read2(&ver.payload[6]);
            }
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get the sensor version.

        This function builds a fip protocol packet to retrieve the camera sensor version.

        @return fip_protocol that contains the structure to retrieve the sensor version.

        @note The returned data will contain the FW Major Revision, FW Minor Revision, SW Major Version, SW Minor Version.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_version(void)
        {
            wind_protocol lens_packet(SENSOR_VERSION);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Perform Flat Field Correction.

        This function builds a fip protocol packet to perform a Flat Field Correction.

        The camera will perform a flat field correction based on the settings from the loaded NUC table.

        The with lens blur FFC will block until the camera has started unblurring the lens. Without lens blur allows the user to call non-blocking calls, or to use a different flat field to perform an FFC.

        @param[in] value Type : 0 - with shutter close (default), 1 - with shutter open
        @return fip_protocol that contains the structure to set the FFC mode.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol perform_ffc(uint8_t value)
        {
            wind_protocol lens_packet(FLAT_FIELD_CORR);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set Auto FFC Period.

        This function builds a fip protocol packet to set the auto FFC period.

        @param[in] value Period : 10-1800 (in seconds) : 0 - off
        @return fip_protocol that contains the structure to set the FFC period.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_auto_ffc_period(uint8_t value)
        {
            wind_protocol lens_packet(SET_AUTO_FFC_PER);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get Auto FFC Period.

        This function builds a fip protocol packet to get the auto FFC period.

        @return fip_protocol that contains the structure to get the FFC period.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_auto_ffc_period(void)
        {
            wind_protocol lens_packet(GET_AUOT_FFC_PER);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set Auto FFC Mode.

        This function builds a fip protocol packet to set whether the auto FFC will be performed with or without the shutter closing.

        @param[in] value Mode : 0 - with shutter close, 1 - without shutter close
        @return fip_protocol that contains the structure to set the FFC mode.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_auto_ffc_mode(uint8_t value)
        {
            wind_protocol lens_packet(SET_AUTO_FFC_MODE);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get Auto FFC mode.

        This function builds a fip protocol packet to get the auto FFC period.

        @return fip_protocol that contains the structure to get the FFC mode.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_auto_ffc_mode(void)
        {
            wind_protocol lens_packet(GET_AUTO_FFC_MODE);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set shutter.

        This function builds a fip protocol packet to set the shutter open or closed.

        Sending this command will turn off automatic NUC. To re-enable NUC, set the NUC period to the desired value.

        @param[in] value shutterPosition : 0 - close, 1 - open
        @return fip_protocol that contains the structure to set the shutter position.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol set_shutter(uint8_t value)
        {
            wind_protocol lens_packet(SET_SHUTTER);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------   
        /**
        @brief Get sensor ready.

        This function builds a fip protocol packet to get the status of the sensor.

        @return fip_protocol that contains the structure to get the status of the sensor.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol ready(void)
        {
            wind_protocol lens_packet(GET_SENSOR_READY);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Load NUC Table.

        This function builds a fip protocol packet to load the corresponding NUC table from the sensor.

        @param[in] value Table Number : (0-3)
        @return fip_protocol that contains the structure to load a NUC table.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol load_nuc_table(uint8_t value)
        {
            wind_protocol lens_packet(LOAD_NUC_TABLE);
            lens_packet.update_payload(value);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Get NUC Table.

        This function builds a fip protocol packet to get the currently loaded NUC table from the sensor.

        @return fip_protocol that contains the structure to get the currently loaded NUC table.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_nuc_table(void)
        {
            wind_protocol lens_packet(GET_NUC_TABLE);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }

        //-----------------------------------------------------------------------------    
        inline friend std::ostream& operator<< (
            std::ostream& out,
            const sensor_class& item
            )
        {
            out << "  Firmware Version: " << (uint32_t)item.fw_maj_rev << "." << (uint32_t)item.fw_min_rev;
            out << "." << (uint32_t)item.sw_maj_rev << "." << (uint32_t)item.sw_min_rev << std::endl;
            //out << "  Software Version: " << (uint32_t)item.sw_maj_rev << "." << (uint32_t)item.sw_min_rev << std::endl;
            out << "  FFC Period:       " << (uint32_t)item.ffc_period << std::endl;
            out << "  FFC Mode:         " << (uint32_t)item.ffc_mode << std::endl;
            return out;
        }

    private:
        uint8_t port = 12;

    };  // end sensor class

    //-----------------------------------------------------------------------------
    /** @brief Sierra Olympic Camera Class

    This class build the packets in the to communicate with the camera.
    */
    class camera {

    public:
        uint8_t maj_rev;
        uint8_t min_rev;
        uint16_t build_num;
        uint16_t camera_type;

        lens_class lens;
        sensor_class sensor;

        uint16_t height;
        uint16_t width;

        udp_info udp_camera_info;

        camera() = default;

        camera(uint8_t maj_rev_, uint8_t min_rev_, uint16_t bn, uint16_t ct) : maj_rev(maj_rev_), min_rev(min_rev_), build_num(bn), camera_type(ct)
        {
            lens = lens_class();
            sensor = sensor_class();

            height = 0;
            width = 0;

        }

        //-----------------------------------------------------------------------------
        void set_image_size(uint16_t h, uint16_t w)
        {
            height = h;
            width = w;
        }

        //-----------------------------------------------------------------------------
        void set_version(uint8_t maj_rev_, uint8_t min_rev_, uint16_t bn, uint16_t ct)
        {
            maj_rev = maj_rev_;
            min_rev = min_rev_;
            build_num = bn;
            camera_type = ct;
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set the camera wind version.

        This function takes a wind_protocol from the get_version function to set the wind protocol version variables in the class.

        @param[in] ver : a wind_protocol packet from the get_version function.

        @sa wind_protocol, get_version
        */
        void set_version(wind_protocol ver)
        {
            if (ver.payload.size() >= 6)
            {
                maj_rev = ver.payload[0];
                min_rev = ver.payload[1];
                build_num = read2(&ver.payload[2]);
                camera_type = read2(&ver.payload[4]);
            }
        }   // end of set_version

        //-----------------------------------------------------------------------------
        /**
        @brief Get the SLA board version.

        This function builds a fip protocol packet to get the SLA board version.

        @return fip_protocol that contains the request to get the SLA board version.

        @note A fip_protocol packet is returned with the SLA board version.

        @sa fip_protocol
        */
        fip_protocol get_sla_board_version(void)
        {
            // build a fip packet
            return fip_protocol(0x28, { 0 });
        }   // end of get_sla_board_version

        //-----------------------------------------------------------------------------
        /**
        @brief Get the image size from the camera.

        This function builds a fip protocol packet to get the image size from the SLA board.

        @return fip_protocol that contains the request to get the camera image size.

        @note A fip_protocol packet is returned with the camera image size.

        @sa fip_protocol
        */
        fip_protocol get_sla_image_size(void)
        {
            // build a fip packet
            return fip_protocol(0x31, { 0 });
        }   // end of get_sla_image_size

        //-----------------------------------------------------------------------------
        /**
        @brief Set the camera serial number.

        This function takes a wind_protocol from the get_serial_number function to set the serial number variable in the class.

        @param[in] ser_num : a wind_protocol packet from the get_serial_number function.

        @sa wind_protocol, get_serial_number
        */
        void set_sn(wind_protocol ser_num)
        {
            sn = "";

            for(uint32_t idx = 0; idx<ser_num.payload.size(); ++idx)
            {
                char c = (char)(ser_num.payload[idx]);
                sn = sn + c;
            }
        }   // end of set_sn

        //-----------------------------------------------------------------------------
        /**
        @brief Get the camera serial number.

        This function builds a fip protocol packet to get the camera serial number.

        @return fip_protocol that contains the structure to get the cmaera serial number.

        @note A wind_protocol packet is returned with the serial number.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_serial_number(void)
        {
            wind_protocol lens_packet(GET_CAMERA_SN);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }   // end of get_serial_number
       
        //-----------------------------------------------------------------------------
        /**
        @brief Initialize the camera and get the current parameters.

        This function initializes the UDP connection to the camera for control and gets the relevant parameters of the camera.

        @return int32_t result of the connection attempt.
        */
        int32_t init_camera(std::string ip_address, std::string& error_msg)
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
        @brief Discover SLA Board.

        This function sends out a UDP broadcast message to determine what devices are attached and what those
        ip addresses are.

        @return ip address of the discovered SLA boards.
        */
        uint32_t discover(void)
        {
            uint32_t ip_address = 0;
            std::string sl_discover = "SLDISCOVER";
            std::vector<uint8_t> rx_data;
            std::string error_msg;

            udp_info udp_discover("255.255.255.255", 51000, 51000);
            int32_t result = init_udp_socket(udp_discover, error_msg);

            std::vector<uint8_t> sld(sl_discover.begin(), sl_discover.end());

            result = send_udp_data(udp_discover, sld);
            result = receive_udp_data(udp_discover, rx_data);

            uint32_t ID = read4(rx_data.data());
            uint32_t length = read4(rx_data.data() + 4);
            uint16_t minor = read2(rx_data.data() + 4);
            uint16_t major = read2(rx_data.data() + 2);
            uint16_t type = read2(rx_data.data() + 2);
            uint16_t hw_type = read2(rx_data.data() + 2);
            uint32_t mac = read4(rx_data.data() + 2);
            ip_address = read4(rx_data.data() + 4);
            uint32_t net_mask = read4(rx_data.data() + 4);
            uint16_t port = read2(rx_data.data() + 38);

            close_connection(udp_discover.udp_sock, error_msg);
            return ip_address;
        }

        //-----------------------------------------------------------------------------
        /**
        @brief Set the network parameters on the SLA board.

        This function builds a fip protocol packet to set the network parameters for the SLA board.

        @param[in] mode : the ip assignement mode: 0 = Use DHCP, 1 = Use specified Static IP address
        @param[in] ip_address : ip address to set teh SLA board to; use inet_addr to convert string address to 32-bits
        @param[in] gateway : ip gateway
        @param[in] subnet : ip subnet; default = 255.255.255.0

        @note No return data is expected.
        */
        void set_network_params(uint8_t mode, uint32_t ip_address, uint32_t gateway, uint32_t subnet = 0xFFFFFF00)
        {
            fip_protocol fp = fip_protocol(SET_NETWORK_PARAMS, { 0x01 });
            fp.add_data(ip_address);        // ip_address
            fp.add_data(subnet);            // subnet
            fp.add_data(gateway);           // gateway
            fp.add_data((uint16_t)0x00);    // c2replyPort
            fp.add_data((uint32_t)0x00);    // reserved
            fp.add_data((uint16_t)0x00);    // listenPort
            fp.add_data((uint16_t)0x00);    // listenPort2
            fp.add_data((uint8_t)0x00);     // hostName.len

            int32_t write_result = send_udp_data(udp_camera_info, fp.to_vector());

        }   // end of set_network_params

        //-----------------------------------------------------------------------------
        /**
        @brief Set the Ethernet Display parameter on the SLA board.

        This function builds a fip protocol packet to set the ethernet display parameter.

        @param[in] ip_address : ip address of the receiving device in 32-bit notation use inet_addr to convert string address to 32-bits
        @param[in] port : port of the receiving device
        @return fip_protocol that contains the structure to set the ethernet display parameter.

        @note Return data will be a fip_protocol packet that contains the Ethernet display parameters.

        @sa fip_protocol
        */
        fip_protocol set_ethernet_display_parameter(uint32_t ip_address, uint16_t port = 15004)
        {
            // build a fip packet: 0x51 0xAC 0x0B 0x29 0x03 [0x0A 0x7F 0x01 0x0C] [0x9C 0x3A] [0x30 0x00] 0x72
            fip_protocol fp = fip_protocol(0x29, { 0x03 });
            fp.add_data(ip_address);
            fp.add_data(port);
            fp.add_data((uint16_t)(0x03));

            return fp;
        }   // end of set_ethernet_display_parameter

        //-----------------------------------------------------------------------------
        /**
        @brief Get the display parameters from the SLA board.

        This function builds a fip protocol packet to get the display parameters from the SLA board.

        @return fip_protocol that contains the structure to get the display parameters.

        @note Return data will be a fip_protocol packet that contains the display parameters.

        @sa fip_protocol
        */
        fip_protocol get_display_parameters(void)
        {
            fip_protocol fp = fip_protocol(0x28, { 0x3A, 0x00 });

            return fp;
        }   // end of get_display_parameters

        //-----------------------------------------------------------------------------
        /**
        @brief Set the display parameters on the SLA board.

        This function builds a fip protocol packet to set the display parameters on the SLA board.

        @param[in] color: (0 - 41, 127)
        @param[in] zoom: (0 - 255)
        @return fip_protocol that contains the structure to set the display parameters.

        @note No return data is expected.

        @sa fip_protocol
        */
        fip_protocol set_display_parameters(uint8_t color, uint8_t zoom)
        {
            fip_protocol fp = fip_protocol(0x16);
            fp.add_data((uint16_t)0x00);            // rotationDegrees
            fp.add_data((uint16_t)0x00);            // rotationLimit
            fp.add_data((uint8_t)0x00);             // decayRate
            fp.add_data(color);                     // falseColorZTT
            fp.add_data(zoom);                      // zoom
            fp.add_data((uint16_t)0x00);            // panCol
            fp.add_data((uint16_t)0x00);            // tiltRow
            fp.add_data((uint8_t)0x00);             // cameraIndex

            return fp;
        }   // end of set_display_parameters

        //-----------------------------------------------------------------------------
        /**
        @brief Configure the streaming control on the SLA board.

        This function builds a fip protocol packet to configure the streaming control and set how the video streams from the SLA board.

        @param[in] value Table Number : (1 -> turn on streaming,  256 -> turn streaming off)
        @return fip_protocol that contains the structure to set the ethernet display parameter.

        @note No return data is expected.

        @sa fip_protocol
        */
        fip_protocol config_streaming_control(uint16_t value)
        {
            // build a fip packet: 0x51 0xAC 0x04 0x90 [0x01 0x00] 0x47
            fip_protocol fp = fip_protocol(0x90);
            fp.add_data(value);
            return fp;
        }   // end of config_streaming_control

        //-----------------------------------------------------------------------------
        /**
        @brief Get the camera version.

        This function builds a fip protocol packet to retrieve the camera version.

        @return fip_protocol that contains the structure to retrieve the camera version.

        @note The returned data will be a wind_protocol packet containing the Major Revision, Minor Revision, Build Number, Camera Type.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol get_version(void)
        {
            wind_protocol lens_packet(GET_CAMERA_VER);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }   // end of get_version

        //-----------------------------------------------------------------------------
        /**
        @brief Set the lens focus position.

        This function sets the lens focus position.

        @return int32_t result of setting the focus position.

        @sa lens
        */
        int32_t set_focus_position(uint16_t value)
        {
            int32_t result = send_udp_data(udp_camera_info, lens.set_focus_position(value).to_vector());
            return result;
        }   // end of set_focus_position

        //-----------------------------------------------------------------------------
        /**
        @brief Set the zoom index.

        This function sets the lens zoom index.

        @return int32_t result of setting the zoom index.

        @sa lens
        */
        int32_t set_zoom_index(uint16_t value)
        {
            int32_t result = send_udp_data(udp_camera_info, lens.set_zoom_index(value).to_vector());
            return result;
        }   // end of set_zoom_index

        //-----------------------------------------------------------------------------
        /**
        @brief End Camera Control.

        This function builds a fip protocol packet to end the application controlling the camera. The only way to restart the app is to power cycle the camera.

        @return fip_protocol that contains the structure to end the camera app.

        @sa fip_protocol, wind_protocol
        */
        fip_protocol end_cam_control(void)
        {
            wind_protocol lens_packet(END_CAMERA_CTRL);

            // build a fip packet
            return fip_protocol(0x3D, port, lens_packet.to_vector());
        }   // end of end_cam_control

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

        uint8_t port = 12;

        std::string sn;

    };  // end camera class

}   // end of namespace

#endif  // _SO_CAM_COMMANDS_H_
