#ifndef _C10_PROTOCOL_H_
#define _C10_PROTOCOL_H_

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

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
    
    c10_protocol(uint8_t code_) : code(code_)
    {
        length = 0;
        data.clear();
        checksum = calc_checksum();
        checksum_valid = true;
    }
    
    c10_protocol(uint8_t code_, std::vector<uint8_t> data_) : code(code_)
    {
        data = data_;
        length = (uint8_t)data.size();
        checksum = calc_checksum();
        checksum_valid = true;
    }

    
    c10_protocol(uint8_t code_, uint8_t data_) : code(code_)
    {
        data.clear();
        data.push_back(data_);
        
        length = (uint8_t)data.size();
        checksum = calc_checksum();
        checksum_valid = true;
    }
    
    c10_protocol(uint8_t code_, uint16_t data_) : code(code_)
    {
        data.clear();
        data.push_back((data_ >> 8) & 0x00FF);
        data.push_back(data_ & 0x00FF);
        
        length = (uint8_t)data.size();
        checksum = calc_checksum();
        checksum_valid = true;
    }
    
    c10_protocol(std::vector<uint8_t> d)
    {
        int32_t idx = 0;
        int32_t offset = 2;

        data.clear();
               
        if(d.size() >= 3)
        {
            length = d[0];
            code = d[1];
            
            for(idx=0; idx<length; ++idx)
            {
                data.push_back(d[idx + offset]);
            }
            
            checksum = d[idx + offset];
            
            checksum_valid = validate_checksum();
        }
        else
        {
            std::cout << "Error in supplied packet format. File: " << __FILE__ << ", line: " << __LINE__<< std::endl;            
        }
    }
   
    //-----------------------------------------------------------------------------
    inline bool valid_checksum() { return checksum_valid; }

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
