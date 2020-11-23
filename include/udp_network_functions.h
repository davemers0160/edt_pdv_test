#ifndef _UDP_NETWORK_FUNCTIONS_H
#define _UDP_NETWORK_FUNCTIONS_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

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

typedef int32_t SOCKET;

#endif

typedef struct udp_info
{
	std::string ip_address;
	uint16_t write_port;
	uint16_t read_port;

	struct sockaddr_in write_addr_obj;
	struct sockaddr_in read_addr_obj;

	SOCKET udp_sock;

	udp_info() = default;

	udp_info(uint16_t wp, uint16_t rp) : write_port(wp), read_port(rp)
	{
		ip_address = "";

		write_addr_obj = sockaddr_in();
		read_addr_obj = sockaddr_in();

		udp_sock = SOCKET(0);
	}

	udp_info(std::string ipa, uint16_t wp, uint16_t rp) : ip_address(ipa), write_port(wp), read_port(rp)
	{
		write_addr_obj = sockaddr_in();
		read_addr_obj = sockaddr_in();

		udp_sock = SOCKET(0);
	}

} udp_info;

enum socket_errors {
	SUCCESS = 0,
	WIN_START_ERR = 1,
	GET_ADDRESS_ERROR = 2,
	SOCKET_CREATION_ERR = 3,
	SOCKET_BIND_ERR = 4,
	CONNECTION_ERROR = 5,
	SEND_ERROR = 6,
	READ_FAILED = 7,
	CLOSE_FAILED = 10
};

// ----------------------------------------------------------------------------
int32_t close_connection(SOCKET& s, std::string& error_msg)
{
	int32_t result = 0;

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	result = shutdown(s, SD_SEND);
#else
	result = close(s);
#endif

	if (result == SOCKET_ERROR)
	{
		error_msg = "Closing socket failed with error: (" + std::to_string(result) + ")";
		return CLOSE_FAILED;
	}

	return SUCCESS;

}   // end of close_connection


// ----------------------------------------------------------------------------
int32_t bind_udp_server(udp_info &info, std::string &error_msg)
{
	int32_t result = SUCCESS;

	info.read_addr_obj.sin_family = AF_INET;
	info.read_addr_obj.sin_port = htons(info.read_port);

	if (info.ip_address.length() == 0)
	{
		info.read_addr_obj.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		info.read_addr_obj.sin_addr.s_addr = inet_addr(info.ip_address.c_str());
	}

	if ((!info.udp_sock) || (info.udp_sock == INVALID_SOCKET))
	{
		info.udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else
	{
		std::cout << "Socket already bound.  Need to shutdown." << std::endl;
		return -1;
	}

	if (info.udp_sock == INVALID_SOCKET)
	{
		std::cout << "Error at socket(): " << std::endl;
		return -1;
	}

	int rcvBuffSizeOption = 1024 * 1024;  // see MAX_READ_BUFFER_SIZE - jsarao
	ULONG reuseAddr = 1;
	setsockopt(info.udp_sock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuffSizeOption, sizeof(rcvBuffSizeOption));
	setsockopt(info.udp_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr));

	result = bind(info.udp_sock, (struct sockaddr*)&info.read_addr_obj, sizeof(struct sockaddr));

	if (result)
	{
		close_connection(info.udp_sock, error_msg);
		info.udp_sock = INVALID_SOCKET;
		std::cout << "SLSockServerBind failed to bind to port " << info.read_port << ", " << result << std::endl;
		return -1;
	}

	return result;
}	// end of bind_udp_server

// ----------------------------------------------------------------------------
int32_t init_udp_socket(udp_info &info, std::string& error_msg)
{

	int32_t result = 0;
	error_msg = "";

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	WSADATA wsaData;

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) 
	{
		error_msg = "Winsock startup failed. Error: " + std::to_string(result);
		return WIN_START_ERR;
	}

#else


#endif

	result = bind_udp_server(info, error_msg);

	return result;
}	// end of init_udp_socket


// ----------------------------------------------------------------------------
int32_t send_udp_data(udp_info &info, std::vector<uint8_t> data)
{
	int32_t result = 0;

	result = sendto(info.udp_sock, (char *)data.data(), data.size(), 0, (sockaddr*)&info.write_addr_obj, sizeof(struct sockaddr_in));

	return result;
}

int32_t receive_udp_data(udp_info &info, std::vector<uint8_t> &data)
{
	int32_t result;
	int32_t error = WSAETIMEDOUT;
	int32_t retry_count = 1;
	int test = 0;

	std::vector<uint8_t> d1(256);

	int32_t addr_length = sizeof(info.read_addr_obj);

	data.clear();

	// receive up to 256 bytes
	do {
		result = recvfrom(info.udp_sock, (char *)d1.data(),256, 0, (sockaddr*)&info.read_addr_obj, &addr_length);
		if (result == -1) 
		{
			error = WSAGetLastError();
			if (error != WSAETIMEDOUT) {
				//test = 1;
				//SLATrace("recvfrom error = %d\n", error);
			}
		}
	} while (result <= 0 && error == WSAETIMEDOUT && (--retry_count));

	if (result > 0)
	{
		data.resize(result);
		std::copy(d1.begin(), d1.begin() + result, data.begin());
	}

	return result;
}	// end of receive_udp_data


#endif  // _UDP_NETWORK_FUNCTIONS_H
