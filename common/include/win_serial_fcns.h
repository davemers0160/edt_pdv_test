#ifndef _WINDOWS_SERIAL_CTRL_H_
#define _WINDOWS_SERIAL_CTRL_H_

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

// https://www.codeguru.com/cpp/i-n/network/serialcommunications/article.php/c5425/Serial-Communication-in-Windows.htm
// https://docs.microsoft.com/en-us/previous-versions/ms810467(v=msdn.10)?redirectedfrom=MSDN

//-----------------------------------------------------------------------------
class serial_port
{

private:
    HANDLE port;
    DCB settings = {0};
    
//-----------------------------------------------------------------------------
    void config(uint32_t baud_rate, uint32_t wait_time, uint32_t data_bits = 8, uint32_t stop_bits = ONESTOPBIT, uint32_t parity = NOPARITY)
    {
        settings.DCBlength = sizeof(settings);

        //After that retrieve the current settings of the serial port using the GetCommState() function.
        bool Status = GetCommState(port, &settings);

        // set the values for Baud rate, Byte size, Number of start/Stop bits etc.
        settings.BaudRate = baud_rate;                              // Setting BaudRate
        settings.ByteSize = data_bits;                              // Setting ByteSize = 8
        settings.StopBits = stop_bits;                              // Setting StopBits = 1
        settings.Parity = parity;                                   // Setting Parity = None

        //settings.fInX = 0;
        //settings.fOutX = 0;
        //settings.XonChar = 0x11;
        //settings.XoffChar = 0x13;
        //settings.XonLim = 200;
        //settings.XoffLim = 200;
        //settings.fRtsControl = RTS_CONTROL_ENABLE;
        //settings.fDtrControl = DTR_CONTROL_ENABLE;
        
        COMMTIMEOUTS timeouts = { 0 };
        timeouts.ReadIntervalTimeout = wait_time*100;               // in milliseconds
        timeouts.ReadTotalTimeoutConstant = wait_time*100;          // in milliseconds
        timeouts.ReadTotalTimeoutMultiplier = wait_time*100;        // in milliseconds
        timeouts.WriteTotalTimeoutConstant = wait_time*100;         // in milliseconds
        timeouts.WriteTotalTimeoutMultiplier = wait_time*100;       // in milliseconds
        SetCommTimeouts (port, &timeouts);
        
        SetCommState(port, &settings);
    }   // end of config

//-----------------------------------------------------------------------------
public:

    serial_port() = default;
    
//-----------------------------------------------------------------------------
    void open_port(std::string named_port, uint32_t baud_rate, uint32_t wait_time, uint32_t data_bits = 8, uint32_t stop_bits = ONESTOPBIT, uint32_t parity = NOPARITY)
    {
        //  Open a handle to the specified com port: expect COMX
        named_port = "\\\\.\\" + named_port;
        port = CreateFile(named_port.c_str(),
              GENERIC_READ | GENERIC_WRITE,
              0,                            //  must be opened with exclusive-access
              NULL,                         //  default security attributes
              OPEN_EXISTING,                //  must use OPEN_EXISTING
              0,                            //  not overlapped I/O
              NULL );                       //  hTemplate must be NULL for comm devices
              
        if(port == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("Error opening port: " + named_port);
            return;
        }

        config(baud_rate, wait_time, data_bits, stop_bits, parity);

    }   // end of open_port


//-----------------------------------------------------------------------------
    uint64_t read_port(std::vector<uint8_t> &read_bufffer, uint64_t bytes_to_read)
    {
        unsigned long num_bytes = 0;                        // Number of bytes read from the port
        unsigned long event_mask = 0;

        read_bufffer.clear();
        read_bufffer.resize(bytes_to_read);
        
        // set the events to be monitored for a communication device: EV_TXTEMPTY|EV_RXCHAR
        //bool Status = SetCommMask(port, EV_RXCHAR);
        
        // wait for the events set by SetCommMask() to happen
        //Status = WaitCommEvent(port, &event_mask, NULL); 
        
        ReadFile(port,                              // Handle of the Serial port
                &read_bufffer[0],                   // Buffer to store the data
                (unsigned long)bytes_to_read,                      // Number of bytes to read in
                &num_bytes,                         // Number of bytes actually read
                NULL);
             
        // if(num_bytes != count)
        // {
            // throw std::runtime_error("Wrong number of bytes received. Expected: " + count.to_string() + ", received: " + num_bytes.to_string());
            // return;
        // }      

		return num_bytes;		
    }   // end of read_port

    uint64_t read_port(std::string &read_bufffer, uint64_t bytes_to_read)
    {
        std::vector<uint8_t> rb(bytes_to_read);
        
        uint64_t num_bytes = read_port(rb, bytes_to_read);

        read_bufffer.assign(rb.begin(), rb.end());
		return num_bytes;		
    }   // end of read_port
  
//-----------------------------------------------------------------------------
    uint64_t write_port(std::vector<char> write_buffer)
    {
        unsigned long bytes_written = 0;                  // Number of bytes written to the port


        bool Status = WriteFile(port,               // Handle to the Serial port
            write_buffer.data(),        // Data to be written to the port
            write_buffer.size(),        // No of bytes to write
            &bytes_written,             // Bytes written
            NULL);

        return bytes_written;
    }   // end of write_port

    uint64_t write_port(std::vector<uint8_t> write_buffer)
    {
        unsigned long bytes_written = 0;                  // Number of bytes written to the port
        
        bool Status = WriteFile(port,               // Handle to the Serial port
                        write_buffer.data(),        // Data to be written to the port
                        write_buffer.size(),        // No of bytes to write
                        &bytes_written,             // Bytes written
                        NULL);

        return bytes_written;
    }   // end of write_port
    
    uint64_t write_port(std::string write_buffer)
    {
        unsigned long bytes_written = 0;                  // Number of bytes written to the port
        
        bool Status = WriteFile(port,               // Handle to the Serial port
                        write_buffer.c_str(),       // Data to be written to the port
                        write_buffer.length(),      // No of bytes to write
                        &bytes_written,             // Bytes written
                        NULL); 
                        
        return bytes_written;
    }   // end of write_port

//-----------------------------------------------------------------------------
    void flush_port()
    {
        Sleep(2);
        PurgeComm(port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    }   // end of flush_port

    inline int64_t bytes_available()
    {
        int64_t bytes_avail = -1;
        COMSTAT cs;
        unsigned long error;

        if (ClearCommError(port, &error, &cs))
            bytes_avail = (int64_t)cs.cbInQue;

        return bytes_avail;
    }   // end of bytes_available

//-----------------------------------------------------------------------------
    void close_port()
    {
        CloseHandle(port);
        //port = NULL;
    }   // end of close_port

};    // end of class
    
#endif  // _WINDOWS_SERIAL_CTRL_H_
