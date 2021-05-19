#ifndef _C10_PROTOCOL_H_
#define _C10_PROTOCOL_H_

#include <cstdint>


//-----------------------------------------------------------------------------
enum C10_FUNCTION_CODES {

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
/** @brief C10 Protocol Class

This class build a C10 packet structure according to the FujiFilm C10 Protocol specifications Manual v2.16.
*/
class c10_protocol
{

public:
    uint8_t code;                     // function code for the packet
    uint8_t length;                   // number of bytes in the data
    
    std::vector<uint8_t> data;
    
    c10_protocol() = default;
    
    c10_protocol(uint8_t id_) : id(id_)
    {
        size = 0;
        data.clear();
        checksum = calc_checksum();
        checksum_valid = true;
    }
    
    c10_protocol(uint8_t code_, std::vector<uint8_t> data_) : code(id_)
    {
        data = data_;
        length = (uint8_t)data.size();
        checksum = calc_checksum();
        checksum_valid = true;
    }
    
    c10_protocol(std::vector<uint8_t> d)
    {
        int32_t idx = 0;
               
        if(d.size() >= 3)
        {
            length = d[0];
            code = d[1];
            
            for(idx=0; idx<length; ++idx)
            {
                data.push_back(d[idx+3]);
            }
            
            checksum = d[idx+3];
            
            checksum_valid = validate_checksum();
        }
        else
        {
            std::cout << "Error in supplied packet format. File: " << __FILE__ << ", line: " << __LINE__<< std::endl;            
        }
    }
   
    //-----------------------------------------------------------------------------
    /**
    @brief Update data.

    This function adds a single byte to the packet data.

    @param[in] value to be added to the data

    */
    inline void update_data(uint8_t value)
    {
        data.push_back((uint8_t)(value & 0x00FF));

        length = (uint8_t)data.size();
        checksum = calc_checksum();
    }

    /**
    @brief Update data.

    This function adds a 16-bit integer to the packet data in MSB ... LSB order.

    @param[in] value to be added to the data

    */
    inline void update_data(uint16_t value)
    {
        data.push_back((uint8_t)((value >> 8) & 0x00FF));
        data.push_back((uint8_t)(value & 0x00FF));
        
        length = (uint8_t)data.size();
        checksum = calc_checksum();
    }
    
    /**
    @brief Update data.

    This function adds a 32-bit integer to the packet data in MSB ... LSB order.

    @param[in] value to be added to the data

    */
    inline void update_data(uint32_t value)
    {
        data.push_back((uint8_t)((value >> 24) & 0x00FF));
        data.push_back((uint8_t)((value >> 16) & 0x00FF));
        data.push_back((uint8_t)((value >> 8) & 0x00FF));
        data.push_back((uint8_t)(value & 0x00FF));
        
        length = (uint8_t)data.size();
        checksum = calc_checksum();
    }    
    
    //-----------------------------------------------------------------------------
    /**
    @brief to_vector.

    This function converts the wind_protocol class to a uint8_t vector

    @return std::vector<uint8_t> 

    */
    std::vector<uint8_t> to_vector(void)
    {
        std::vector<uint8_t> d = {length, code};

        d.insert(d.end(), data.begin(), data.end());

        d.push_back(checksum);
        
        return d;
    }

private:

    uint8_t checksum;
    bool checksum_valid = true;
    
    //-----------------------------------------------------------------------------
    uint8_t calc_checksum(void)
    {
        uint8_t checksum = length;
        
        checksum += code;
        
        for(uint8_t idx = 0; idx < data.size(); ++idx)
        {
            checksum += data[idx];
        }
        
        return (uint8_t)(0x100 - checksum);
    }
    
    //-----------------------------------------------------------------------------
    bool validate_checksum(void)
    {
        return (checksum == calc_checksum());
    }

};  // end of C10_protocol class

#endif  // _C10_PROTOCOL_H_
