#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#else

#endif

// C/C++ includes
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>

#include "ip_helper.h"

// Project Includes
#include "edt_test.h"

// OpenCV Includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp> 
#include <opencv2/videoio.hpp>

void print_usage(void)
{
    std::cout << "Enter the ip address of the camera to connect:" << std::endl;
    std::cout << "example: cam_test 192.168.0.10" << std::endl;
}

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::string sdate, stime;

    uint64_t idx = 0, jdx = 0;
    std::string console_input;

    typedef std::chrono::duration<double> d_sec;
    auto start_time = std::chrono::system_clock::now();
    auto stop_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<d_sec>(stop_time - start_time);

    // generic config file for a 12-bit monochrome camera
    std::string cfg_file = "../config/generic12cl.cfg";

    // EDT PDV specific variable
    Edtinfo edtinfo;
    Dependent* dd_p;
    uint8_t* image_p;

    // Sierra-Olympic specific variables
    SO::camera vinden;
    wind_protocol wind_data;
    uint32_t driver_device_num = 0;
    uint32_t connect_count = 0;
    uint32_t read_timeout = 30000;
    uint32_t write_timeout = 1000;
    uint32_t baud_rate = 115200;
    
    // IP based comms
    int32_t result = 0;
    std::string camera_ip_address;
    std::string video_cap_address;      // = "udp://camera_ip_address:video_port";
    uint16_t read_port = 14002;
    uint16_t write_port = 14001;
    uint16_t video_port = 15004;
    udp_info udp_camera_info(write_port, read_port);
    std::string error_msg;
    int32_t read_result, write_result;
    std::string host_ip_address;
    std::vector<uint8_t> rx_data;

    // opencv variables to display the video feed
    std::string window_name = "Video Feed";
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::Mat frame;

    if (argc == 1)
    {
        print_usage();
        std::cin.ignore();
        return 0;
    }

    camera_ip_address = argv[1];
    video_cap_address = "udp://" + camera_ip_address + ":" + std::to_string(video_port);

    try
    {
        /*
        // test the reading in of a config file
        std::cout << std::endl << "Reading in config file..." << std::endl;
        if ((dd_p = pdv_alloc_dependent()) == NULL)
        {
            std::cout << "alloc_dependent FAILED... exiting!" << std::endl;
            exit(1);
        }
        int result = pdv_readcfg(cfg_file.c_str(), dd_p, &edtinfo);

        // open a specifc board and channel
        std::cout << std::endl << "Opening: " << EDT_INTERFACE << ", Unit: " << EDT_UNIT_0 << ", Channel: " << VINDEN << std::endl;
        PdvDev* pdv_p = pdv_open_channel(EDT_INTERFACE, EDT_UNIT_0, VINDEN);
        if (pdv_p == NULL)
        {
            std::cout << "Failed to connect to a pdv device... exiting!  Press Enter to close" << std::endl;
            std::cin.ignore();
            exit(1);
        }

        // print out the device name
        std::cout << "edt_devname: " << std::string(pdv_p->edt_devname) << std::endl;

        // start the image capture process
        std::cout << std::endl << "Starting the image capture process..." << std::endl;
        pdv_start_image(pdv_p);

        // get the image and place into a unsigned char pointer
        // no clue how the packing is going to for int16_t ot uint16_t images
        std::cout << std::endl << "Grabbing the image..." << std::endl;
        image_p = pdv_wait_image(pdv_p);

        // close the device
        std::cout << std::endl << "Closing the pdv device..." << std::endl;
        result = pdv_close(pdv_p);

        std::cout << "close result: " << result << std::endl;
        */

        get_local_ip(host_ip_address, error_msg);

        std::cout << "------------------------------------------------------------------" << std::endl;
        std::cout << "host IP address: " << host_ip_address << std::endl;
        std::cout << "------------------------------------------------------------------" << std::endl << std::endl;

        result = init_ip_camera(udp_camera_info, camera_ip_address, error_msg);
        
        if(result < 0)
        {
            std::cout << "result: " << result << std::endl;
            std::cout << "error msg: " << error_msg << std::endl;
        }
        
        write_result = send_udp_data(udp_camera_info, vinden.get_sla_board_version().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        fip_protocol sla_board_version = fip_protocol(rx_data);
        //std::cout << sla_board_version << std::endl;

        write_result = send_udp_data(udp_camera_info, vinden.get_sla_image_size().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        fip_protocol sla_image_size = fip_protocol(rx_data);
        vinden.set_image_size(read2(&sla_image_size.data[2]), read2(&sla_image_size.data[0]));

        std::cout << "Image size (h x w): " << vinden.height << " x " << vinden.width << std::endl;

        // get the camera wind version number
        write_result = send_udp_data(udp_camera_info, vinden.get_version().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.set_version(wind_data);

        // get the camera serial number
        write_result = send_udp_data(udp_camera_info, vinden.get_serial_number().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.set_sn(wind_data);

        // get the camera lens version
        write_result = send_udp_data(udp_camera_info, vinden.lens.get_version().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.lens.set_version(wind_data);

        // ----------------------------------------------------------------------------
        // get the camera lens zoom index
        write_result = send_udp_data(udp_camera_info, vinden.lens.get_zoom_index().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.lens.zoom_index = read2(&wind_data.payload[0]);

        // get the camera lens zoom position
        write_result = send_udp_data(udp_camera_info, vinden.lens.get_zoom_position().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.lens.zoom_position = read2(&wind_data.payload[0]);

        // get the camera lens zoom speed
        write_result = send_udp_data(udp_camera_info, vinden.lens.get_zoom_speed().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.lens.zoom_speed = (wind_data.payload[0]);

        // get the camera lens focus position
        write_result = send_udp_data(udp_camera_info, vinden.lens.get_focus_position().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.lens.focus_position = read2(&wind_data.payload[0]);

        // get the camera lens focus speed
        write_result = send_udp_data(udp_camera_info, vinden.lens.get_focus_speed().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.lens.focus_speed = (wind_data.payload[0]);

        // ----------------------------------------------------------------------------
        // get the sensor version number
        write_result = send_udp_data(udp_camera_info, vinden.sensor.get_version().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.sensor.set_version(wind_data);

        // get the FFC period
        write_result = send_udp_data(udp_camera_info, vinden.sensor.get_auto_ffc_period().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.sensor.ffc_period = read2(&wind_data.payload[0]);

        // get the FFC mode
        write_result = send_udp_data(udp_camera_info, vinden.sensor.get_auto_ffc_mode().to_vector());
        read_result = receive_udp_data(udp_camera_info, rx_data);
        wind_data = wind_protocol(rx_data);
        vinden.sensor.ffc_mode = wind_data.payload[0];

        // display the information about a specific camera
        std::cout << vinden << std::endl;

        // set the video output parameters
        write_result = send_udp_data(udp_camera_info, vinden.set_ethernet_display_parameter(inet_addr(host_ip_address.c_str()), video_port).to_vector());

        // Turn on video streaming over ethernet
        write_result = send_udp_data(udp_camera_info, vinden.config_streaming_control(SO::STREAM_ON).to_vector());

        char key = 0;

        cv::VideoCapture cap(video_cap_address, cv::CAP_FFMPEG);

        if (!cap.isOpened())
        {
            key = 'q';
        }

        while (key != 'q')
        {
            cap >> frame;
            cv::imshow(window_name, frame);

            key = cv::waitKey(5);
        }

        cv::destroyAllWindows();

        int bp = 0;


    }
    catch (std::exception& e)
    {
        std::cout << "error: " << e.what() << std::endl;
    }

    write_result = send_udp_data(udp_camera_info, vinden.config_streaming_control(SO::STREAM_OFF).to_vector());

    result = close_connection(udp_camera_info.udp_sock, error_msg);

    return 0;
    
}   // end of main
