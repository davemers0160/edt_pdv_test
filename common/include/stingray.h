#ifndef _STINGRAY_PROTOCOL_H_
#define _STINGRAY_PROTOCOL_H_

#include <cstdint>
#include <cmath>
#include <ctime>
#include <vector>


#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
#include "win_serial_fcns.h"
#include <windows.h>

#elif defined(__linux__)
#include <sys/ioctl.h>
#include "linux_serial_fcns.h"

#endif

//-----------------------------------------------------------------------------
const uint16_t focus_min = 0;
const uint16_t focus_max = 10000;
const uint16_t zoom_min = 0;
const uint16_t zoom_max = 10000;
const uint8_t iris_min = 0;
const uint8_t iris_max = 100;


//-----------------------------------------------------------------------------
class stingray_lens
{  
public:

    serial_port sp;
    std::string port_name;
    uint32_t wait_time;
    uint32_t baud_rate;

    //-----------------------------------------------------------------------------
    stingray_lens()
    {
        port_name = "";
        wait_time = 10;
        baud_rate = 115200;
        connected = false;
    }

    stingray_lens(std::string pn, uint32_t br, uint32_t wt) : port_name(pn), baud_rate(br), wait_time(wt)
    {
        connected = false;
    }

    //-----------------------------------------------------------------------------
    uint16_t get_focus() { return focus; }
    uint16_t get_zoom() { return zoom; }
    uint8_t get_iris() { return iris; }


    //-----------------------------------------------------------------------------
    uint32_t connect()
    {
        if (port_name.empty())
        {
            std::cout << "port_name is empty" << std::endl;
            return 0;
        }

        sp.open_port(port_name, baud_rate, wait_time);
        connected = true;

        sleep_ms(50);
        set_verbose_level(1);

        sleep_ms(50);
        get_status();

        return 1;
    }

    void set_verbose_level(uint8_t level)
    {
        uint64_t result;
        std::string read_bufffer;
        std::string msg = "CV" + std::to_string(level) + "\r\n";

        if (connected)
        {
            // flush teh serial port to remove any unwanted data
            sp.flush_port();

            // send the status message to trigger the kens to return status info
            result = sp.write_port(msg);

            // get the returned string from the lens
            //result = sp.read_port(read_bufffer, bytes_to_read);
        }
    }

    //-----------------------------------------------------------------------------
    void get_status()
    {
        // Each line break above has a CRLF sent over the Serial connection.
        // F = XXXX
        // Z = XXXX
        // I = XXX
        // CAX
        // T = XX.XX
        int32_t result;
        std::string rx_msg;
        uint64_t bytes_to_read = 39;
        std::string tx_msg = "STATUS\r\n";
        std::vector<std::string> params;

        // send and receive data
        result = txrx_data(tx_msg, rx_msg, bytes_to_read);

        // check result
        if (result < 0)
        {
            std::cout << "Error getting data.  Most likely not connected!" << std::endl;
        }
        else if (result == 0)
        {
            parse_response(rx_msg, params);

            // parse the data
            focus = str2int<uint16_t>(params[0].substr(2, std::string::npos));      // Expecting "F=X\r\n", X ranges from 1-5 digits 
            zoom = str2int<uint16_t>(params[1].substr(2, std::string::npos));       // Expecting "Z=nX\r\n", X ranges from 1-5 digits 
            iris = str2int<uint8_t>(params[2].substr(2, std::string::npos));        // Expecting "I=nX\r\n", X ranges from 1-3 digits 
            at_mode = str2int<uint8_t>(params[3].substr(2, std::string::npos));     // Expecting "CAX\r\n"
            temp = str2float(params[4].substr(2, std::string::npos));               // Expecting "T=XX.XX\r\n" 
        }
        else
        {
            std::cout << error_codes[result] << std::endl;
        }

    }   // end of get_status

    //-----------------------------------------------------------------------------
    float get_temp() 
    { 
        int32_t result;
        std::string rx_msg;
        uint64_t bytes_to_read = 7;
        std::vector<std::string> params;

        std::string tx_msg = "T\r\n";

        // send and receive data
        result = txrx_data(tx_msg, rx_msg, bytes_to_read);

        // check result
        if (result < 0)
        {
            std::cout << "Error getting data.  Most likely not connected!" << std::endl;
        }
        else if (result == 0)
        {
            // parse the data.  Expecting "T=XX.XX\r\n", but what we actually get is "XX.XX\r\n"
            parse_response(rx_msg, params);
            temp = str2float(params[0].substr(0, std::string::npos));         //(uint16_t)std::stoi(params[0].substr(2, std::string::npos));
        }
        else
        {
            std::cout << error_codes[result] << std::endl;
        }

        return temp; 
    }   // end of get_temp

    //-----------------------------------------------------------------------------
    void set_focus(uint16_t value)
    {
        int32_t result;
        std::string rx_msg;
        uint64_t bytes_to_read = 4 + count_digits(value); //10;
        std::vector<std::string> params;

        value = check_limits(value, focus_min, focus_max);

        std::string tx_msg = "F=" + std::to_string(value) + "\r\n";

        // send and receive data
        result = txrx_data(tx_msg, rx_msg, bytes_to_read);

        // check result
        if (result < 0)
        {
            std::cout << "Error getting data.  Most likely not connected!" << std::endl;
        }
        else if (result == 0)
        {
            // parse the data. Expecting "F=X\r\n", X ranges from 1-5 digits 
            parse_response(rx_msg, params);
            focus = str2int<uint16_t>(params[0].substr(2, std::string::npos));         //(uint16_t)std::stoi(params[0].substr(2, std::string::npos));
        }
        else
        {
            std::cout << error_codes[result] << std::endl;
        }

    }   // end of set_focus

    //-----------------------------------------------------------------------------
    void set_zoom(uint16_t value)
    {
        int32_t result;
        std::string rx_msg;
        uint64_t bytes_to_read = 4 + count_digits(value);   //10;
        std::vector<std::string> params;

        value = check_limits(value, zoom_min, zoom_max);

        std::string tx_msg = "Z=" + std::to_string(value) + "\r\n";

        // send and receive data
        result = txrx_data(tx_msg, rx_msg, bytes_to_read);

        // check result
        if (result < 0)
        {
            std::cout << "Error getting data.  Most likely not connected!" << std::endl;
        }
        else if (result == 0)
        {
            // parse the data. Expecting "Z=X\r\n", X ranges from 1-5 digits 
            parse_response(rx_msg, params);
            zoom = str2int<uint16_t>(params[0].substr(2, std::string::npos));   // (uint16_t)std::stoi(params[0].substr(2, std::string::npos));
        }
        else
        {
            std::cout << error_codes[result] << std::endl;
        }

    }   // end of set_zoom

    //-----------------------------------------------------------------------------
    void set_iris(uint8_t value)
    {
        int32_t result;
        std::string rx_msg;
        uint64_t bytes_to_read = 4 + count_digits(value);  //8;
        std::vector<std::string> params;

        value = check_limits(value, iris_min, iris_max);

        std::string tx_msg = "I=" + std::to_string(value) + "\r\n";

        // send and receive data
        result = txrx_data(tx_msg, rx_msg, bytes_to_read);

        // check result
        if (result < 0)
        {
            std::cout << "Error getting data.  Most likely not connected!" << std::endl;
        }
        else if (result == 0)
        {
            // parse the data. Expecting "I=X\r\n", X ranges from 1-3 digits 
            parse_response(rx_msg, params);
            iris = str2int<uint8_t>(params[0].substr(2, std::string::npos));   // (uint8_t)std::stoi(params[0].substr(2, std::string::npos));
        }
        else
        {
            std::cout << error_codes[result] << std::endl;
        }

    }   // end of set_iris

    //-----------------------------------------------------------------------------
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
        const stingray_lens& item
        )
    {
        out << "stringray_lens:" << std::endl;
        out << "  port name:   " << item.port_name << std::endl;
        out << "  baud rate:   " << item.baud_rate << std::endl;
        out << "  wait time:   " << item.wait_time << std::endl;
        out << "  focus step:  " << item.focus << std::endl;
        out << "  zoom step:   " << item.zoom << std::endl;
        out << "  iris step:   " << (uint32_t)item.iris << std::endl;
        out << "  temperature: " << item.temp << std::endl;
        out << "  athermal:    " << (uint32_t)item.at_mode << std::endl;

        return out;
    }   // end of operator<<

private:
    bool connected = false;
    uint16_t focus;
    uint16_t zoom;
    uint8_t iris;
    uint8_t at_mode;
    float temp;


    std::vector<std::string> error_codes = { " ", " ",
        "ERROR02 Stepper driver error (filter wheel)",
        "ERROR03 Invalid command or numerical value",
        "ERROR04 Iris motor timed out",
        "ERROR05 SPI communication error",
        "ERROR06 Relative bump out of bounds, will travel to extreme and stop",
        "ERROR07 I2C communication error (zoom and focus)",
        "ERROR08 DC motor driver error",
        "ERROR09 Zoom motor timeout",
        "ERROR10 Iris timeout",
        "ERROR11 Focus Motor defect",
        "ERROR12 Zoom motor defect",
        "ERROR13 Iris motor defect"
    };
    
    //-----------------------------------------------------------------------------
    template <typename T>
    inline T check_limits(T value, T lower, T upper)
    {
        return std::max(std::min(value, upper), lower);

    }   // end of check_limits

    //-----------------------------------------------------------------------------
    uint32_t get_error_code(std::string error_msg)
    {
        uint32_t code = 0;

        if (error_msg.compare(0, 5, "ERROR") == 0)
        {
            try
            {
                code = std::stoi(error_msg.substr(5, 2));
            }
            catch (...)
            {
                code = 0;
            }
        }

        return code;
    }   // end of get_error_code

    //-----------------------------------------------------------------------------
    inline int32_t txrx_data(std::string tx_msg, std::string &rx_msg, uint64_t bytes_to_read)
    {
        int32_t result = -1;
        if (connected)
        {
            // flush teh serial port to remove any unwanted data
            sp.flush_port();

            // send the status message to trigger the kens to return status info
            result = (int32_t)sp.write_port(tx_msg);

            // get the returned string from the lens
            result = (int32_t)sp.read_port(rx_msg, bytes_to_read);

            // check the error code returned
            result = (int32_t)get_error_code(rx_msg);
        }

        return result;
    }   // end of poll_lens

    //-----------------------------------------------------------------------------
    void parse_response(std::string response, std::vector<std::string>& params)
    {
        int32_t start = 0, end = 0;
        const std::string CRLF = "\r\n";
        params.clear();

        while (end != std::string::npos)
        {
            end = response.find(CRLF.c_str(), start, 2);
            if (end != std::string::npos)
            {
                params.push_back(response.substr(start, end - start));
                start = end + 2;
            }
            else if (start < response.length())
            {
                params.push_back(response.substr(start, response.length() - start));
            }
        }

    }   // end of parse_response

    //-----------------------------------------------------------------------------
    // create a sleep function that can be used in both Windows and Linux
    void sleep_ms(uint32_t value)
    {

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
        Sleep(value);
#else
        const timespec delay[] = { 0, (uint32_t)(value * 1000000) };
        nanosleep(delay, NULL);
#endif

    }   // end of sleep_ms


    //-----------------------------------------------------------------------------
    template<typename T>
    uint16_t count_digits(T n)
    {
        uint16_t count = 0;
        while (n != 0)
        {
            n = n / 10;
            ++count;
        }
        return count;
    }   // end of count_digits

    //-----------------------------------------------------------------------------
    template<typename T>
    T str2int(std::string str)
    {
        size_t index = 0;
        T num = 0;
        try {
            num = (T)std::stoi(str, &index);
        }
        catch (std::exception& e)
        {
            num = 0;
            if (index == 0)
            {
                std::cout << "error converting string '" << str << "' to number" << std::endl;
            }
            else
            {
                std::cout << "error converting string '" << str << "' to number: " << e.what() << std::endl;
            }
        }
        
        return num;
    }   // end of str2int

    //-----------------------------------------------------------------------------
    float str2float(std::string str)
    {
        size_t index = 0;
        float num = 0.0;
        try {
            num = std::stof(str, &index);
        }
        catch (std::exception& e)
        {
            num = 0.0;
            if (index == 0)
            {
                
                std::cout << "error converting string '" << str << "' to number." << std::endl;
            }
            else
            {
                std::cout << "error converting string '" << str << "' to number: " << e.what() << std::endl;
            }
        }

        return num;
    }   // end of str2float

};  // end of stringray_lens

//-----------------------------------------------------------------------------
#endif  // _STINGRAY_PROTOCOL_H_
