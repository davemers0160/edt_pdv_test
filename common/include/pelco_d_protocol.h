#ifndef _PELCO_D_PROTOCOL_H_
#define _PELCO_D_PROTOCOL_H_

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
/** @brief Pelco D Protocol Class

This class build a Pelco D packet structure according to the Pelco D Protocol specifications.
https://www.commfront.com/pages/pelco-d-protocol-tutorial
https://www.epiphan.com/userguides/LUMiO12x/Content/UserGuides/PTZ/3-operation/PELCODcommands.htm
https://www.serialporttool.com/sptblog/?p=4830

*/
class pelco_d_protocol
{

public:
    uint8_t address;                    // camera address
    uint8_t command_1;
    uint8_t command_2;
    uint8_t data_1;
    uint8_t data_2;

       
    pelco_d_protocol() = default;
    
    
    pelco_d_protocol(
        uint8_t a_, 
        uint8_t c_1, 
        uint8_t c_2, 
        uint8_t d_1, 
        uint8_t d_2)
        : address(a_), command_1(c_1), command_2(c_2), data_1(d_1), data_2(d_2)
    {
        checksum = calc_checksum();
        checksum_valid = true;
    }
    
    pelco_d_protocol(std::vector<uint8_t> d)
    {
        int32_t idx = 0;
               
        if(d.size() >= 6)
        {
            address     = d[1]; 
            command_1   = d[2];
            command_2   = d[3];
            data_1      = d[4];
            data_2      = d[5];
            checksum    = d[6];
            
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

    This function updates the data bytes with the new value d_1->data_1, d_2->data_2.

    @param[in] d_1 new data_1 value
    @param[in] d_2 new data_2 value
    
    */
    inline void update_data(uint8_t d_1, uint8_t d_2)
    {
        data_1 = d_1;
        data_2 = d_2;

        checksum = calc_checksum();
    }

    /**
    @brief Update data.

    This function updates the data bytes with the new value MSB->data_1, LSB->data_2.

    @param[in] new data value

    */
    inline void update_data(uint16_t value)
    {
        data_1 = ((uint8_t)((value >> 8) & 0x00FF));
        data_2 = ((uint8_t)(value & 0x00FF));
        
        checksum = calc_checksum();
    }
    
    //-----------------------------------------------------------------------------
    /**
    @brief Update command.

    This function updates the command bytes with the new value MSB->command_1, LSB->command_2.

    @param[in] c_1 new command_1 value
    @param[in] c_2 new command_2 value

    */
    inline void update_command(uint8_t c_1, uint8_t c_2)
    {
        command_1 = c_1;
        command_2 = c_2;

        checksum = calc_checksum();
    }

    /**
    @brief Update command.

    This function updates the command bytes with the new value MSB->command_1, LSB->command_2.

    @param[in] new command value

    */
    inline void update_command(uint16_t value)
    {
        command_1 = ((uint8_t)((value >> 8) & 0x00FF));
        command_2 = ((uint8_t)(value & 0x00FF));
        
        checksum = calc_checksum();
    }
    
    //-----------------------------------------------------------------------------
    /**
    @brief to_vector.

    This function converts the pelco_d_protocol class to a uint8_t vector

    @return std::vector<uint8_t> 

    */
    inline std::vector<uint8_t> to_vector(void)
    {
        std::vector<uint8_t> d = {sync, address, command_1, command_2, data_1, data_2, checksum};
        
        return d;
    }

private:

    uint8_t sync = 0xFF;
    uint8_t checksum;
    bool checksum_valid = true;
    
    //-----------------------------------------------------------------------------
    uint8_t calc_checksum(void)
    {
        uint8_t checksum = address + command_1 + command_2 + data_1 + data_2;
        
        return (uint8_t)(checksum % 0x0100);
    }
    
    //-----------------------------------------------------------------------------
    bool validate_checksum(void)
    {
        return (checksum == calc_checksum());
    }

};  // end of pelco_d_protocol class

#endif  // _PELCO_D_PROTOCOL_H_
