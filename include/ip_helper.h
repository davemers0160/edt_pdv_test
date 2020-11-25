#ifndef IP_HELPER_FUNCTIONS_H
#define IP_HELPER_FUNCTIONS_H

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library
#pragma comment(lib, "iphlpapi.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#else

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <cstdint>
#include <string>
#include <vector>


// ----------------------------------------------------------------------------------------
/*
void get_ip_address(std::vector<std::string> &data, std::string &error_msg)
{
    int32_t idx;

    // Variables used by GetIpAddrTable 
    PMIB_IPADDRTABLE ip_address_table;
    unsigned long size = 0;
    unsigned long ret_value = 0;
    in_addr ip_address;

    data.clear();
    error_msg = "";

    // Before calling AddIPAddress we use GetIpAddrTable to get an adapter to which we can add the IP.
    ip_address_table = (MIB_IPADDRTABLE *)HeapAlloc(GetProcessHeap(), 0, sizeof(MIB_IPADDRTABLE));

    if (ip_address_table)
    {
        // Make an initial call to GetIpAddrTable to get the necessary size into the dwSize variable
        if (GetIpAddrTable(ip_address_table, &size, 0) ==
            ERROR_INSUFFICIENT_BUFFER) {
            HeapFree(GetProcessHeap(), 0, ip_address_table);
            ip_address_table = (MIB_IPADDRTABLE *)HeapAlloc(GetProcessHeap(), 0, size);
        }

        if (ip_address_table == NULL)
        {
            error_msg = "Memory allocation failed for GetIpAddrTable";
            return;
        }
    }

    // Make a second call to GetIpAddrTable to get the actual data we want
    if ((ret_value = GetIpAddrTable(ip_address_table, &size, 0)) != NO_ERROR)
    {
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, ret_value, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&error_msg, 0, NULL);
        return;
    }

    for (idx = 0; idx < (int)ip_address_table->dwNumEntries; ++idx)
    {
        ip_address.S_un.S_addr = (u_long)ip_address_table->table[idx].dwAddr;
        data.push_back(inet_ntoa(ip_address));
    }

    if (ip_address_table)
    {
        HeapFree(GetProcessHeap(), 0, ip_address_table);
        ip_address_table = NULL;
    }

}   // end of get_ip_address
*/


int32_t get_local_ip(std::string &ip_address, std::string& error_msg)
{
    int32_t result = 0;

    ip_address = "";

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    WSADATA wsaData;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        error_msg = "Winsock startup failed. Error: " + std::to_string(result);
        return 1;
    }

    SOCKET sock;

#else
    int32_t sock;

#endif

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback = {};

    if (sock == -1) 
    {
        std::cout << "Could not initialize socket." << std::endl;
        return 1;
    }

    //std::memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = INADDR_LOOPBACK;   // using loopback ip address
    loopback.sin_port = htons(9);                 // using debug port

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) 
    {

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
        result = shutdown(sock, SD_SEND);
#else
        result = close(sock);
#endif

        std::cout << "Could not connect." << std::endl;
        return 1;
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) 
    {

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
        result = shutdown(sock, SD_SEND);
#else
        result = close(sock);
#endif
        std::cout << "Could not getsockname." << std::endl;
        return 1;
    }

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
    result = shutdown(sock, SD_SEND);
#else
    result = close(sock);
#endif

    char buffer[22];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buffer, INET_ADDRSTRLEN) == 0x0) 
    {
        std::cout << "Could not inet_ntop" << std::endl;
        return 1;
    }

    ip_address = std::string(buffer);

    return result;
}


#endif  // IP_HELPER_FUNCTIONS_H
