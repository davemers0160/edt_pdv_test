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
#include <errno.h>

typedef int32_t SOCKET;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

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
int32_t winsock_init(std::string &error_msg)
{
	int32_t result = 0;

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	WSADATA wsaData;

	// Initialize Winsock version 2.2
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		error_msg = "Winsock startup failed. Error: " + std::to_string(result);
		return WIN_START_ERR;
	}
#endif

	return result;
}	// end of winsock_init

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

	// fill in the server information 
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

	// create the socket file descriptor 
	if ((!info.udp_sock) || (info.udp_sock == INVALID_SOCKET))
	{
		info.udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else
	{
		error_msg = "Socket already bound.  Need to shutdown.";
		return -1;
	}

	if (info.udp_sock == INVALID_SOCKET)
	{
		error_msg = "Error at socket(): ";
		return -1;
	}

	int recieve_buffer_size = 1024 * 1024;  // MAX_READ_BUFFER_SIZE 
	unsigned long reuse_address = 1;
	setsockopt(info.udp_sock, SOL_SOCKET, SO_RCVBUF, (char*)&recieve_buffer_size, sizeof(recieve_buffer_size));
	setsockopt(info.udp_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_address, sizeof(reuse_address));

	// Associate the address information with the socket using bind.
    // At this point we can receive datagrams on the bound socket.
	result = bind(info.udp_sock, (struct sockaddr*)&info.read_addr_obj, sizeof(struct sockaddr));

	if (result)
	{
		close_connection(info.udp_sock, error_msg);
		info.udp_sock = INVALID_SOCKET;
		error_msg = "failed to bind to port " + std::to_string(info.read_port) + ", " + std::to_string(result);
		return -1;
	}

	return result;
}	// end of bind_udp_server

// ----------------------------------------------------------------------------
int32_t init_udp_socket(udp_info &info, std::string& error_msg)
{

	int32_t result = 0;
	error_msg = "";

//#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
//	WSADATA wsaData;
//
//	// Initialize Winsock version 2.2
//	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
//	if (result != 0) 
//	{
//		error_msg = "Winsock startup failed. Error: " + std::to_string(result);
//		return WIN_START_ERR;
//	}
//#endif
	result = winsock_init(error_msg);

	result = bind_udp_server(info, error_msg);

	return result;
}	// end of init_udp_socket


// ----------------------------------------------------------------------------
inline int32_t send_udp_data(udp_info &info, std::vector<uint8_t> data)
{
	return (int32_t)sendto(info.udp_sock, (char *)data.data(), (int)data.size(), 0, (sockaddr*)&info.write_addr_obj, sizeof(struct sockaddr_in));
}

inline int32_t send_udp_data(udp_info& info, std::string data)
{
	return (int32_t)sendto(info.udp_sock, data.c_str(), (int)data.length(), 0, (sockaddr*)&info.write_addr_obj, sizeof(struct sockaddr_in));;
}

// ----------------------------------------------------------------------------
int32_t receive_udp_data(SOCKET udp_sock, struct sockaddr_in read_addr_obj, std::vector<uint8_t>& data, int32_t length=256)
{
	int32_t result;
	int32_t error = 10060L;		// WIN32 -> WSAETIMEDOUT
	int32_t retry_count = 1;
	int test = 0;

	std::vector<uint8_t> d1(length);

	data.clear();
	
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

	int32_t addr_length = sizeof(read_addr_obj);
	
	// receive up to 256 bytes
	do {
		result = recvfrom(udp_sock, (char*)d1.data(), length, 0, (sockaddr*)&read_addr_obj, &addr_length);
		if (result == -1)
		{

			error = WSAGetLastError();
			if (error != WSAETIMEDOUT) 
			{
				std::cout << "recvfrom error = " << error << std::endl;
			}

		}

	} while (result <= 0 && error == WSAETIMEDOUT && (--retry_count));
#else

	uint32_t addr_length = 0;
	
	// receive up to length number of bytes
	do {
		result = recvfrom(udp_sock, (char*)d1.data(), length, 0, (sockaddr*)&read_addr_obj, &addr_length);
		if (result == -1)
		{
		    std::cout << "recvfrom error: " << strerror(errno) << std::endl;
		}

	} while (result <= 0 && (--retry_count));

#endif

	if (result > 0)
	{
		data.resize(result);
		std::copy(d1.begin(), d1.begin() + result, data.begin());
	}

	return result;
}	// end of receive_udp_data

// ----------------------------------------------------------------------------
inline int32_t receive_udp_data(udp_info& info, std::vector<uint8_t>& data, int32_t length = 256)
{
	int32_t result = receive_udp_data(info.udp_sock, info.read_addr_obj, data, length);

	return result;

}	// end of receive_udp_data

// ----------------------------------------------------------------------------
int32_t init_udp_broadcast(SOCKET &sock,
	struct sockaddr_in &sock_addr,
	std::string host_ip_address,
	std::string broadcast_address,
	uint16_t broadcast_port,
	std::string &error_msg,
	uint32_t recv_timeout_ms = 2000,
	uint32_t send_timeout_ms = 1000
)
{
	int32_t result = 0;
	int32_t broadcast = 1;

	// initialize socket for listening and/or sending braodcast messages
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		error_msg = "Error configuring socket.  Socket Invalid";
		return -1;
	}

	// initialize the sockaddr_in struct
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(broadcast_port);
	sock_addr.sin_addr.s_addr = inet_addr(host_ip_address.c_str());

	// bind the socket
	result = bind(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	if (result)
	{
		close_connection(sock, error_msg);
		sock = INVALID_SOCKET;
		error_msg = "Failed to bind to port " + std::to_string(broadcast_port) + ", result: " + std::to_string(result);
		return -1;
	}

	// switch over to the broadcast address
	sock_addr.sin_addr.s_addr = inet_addr(broadcast_address.c_str());

	// set the socket options
	result = 0;
	result |= setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast));
	if (result != 0)
	{
		error_msg += "Error setting SO_BROADCAST: " + std::string(strerror(errno)) + ". result: " + std::to_string(result) + ".\n";
	}

	result |= setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&broadcast, sizeof(broadcast));
	if (result != 0)
	{
		error_msg = "Error setting SO_REUSEADDR: " + std::string(strerror(errno)) + ". result: " + std::to_string(result) + ".\n";
	}

	result |= setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeout_ms, sizeof(recv_timeout_ms));
	if (result != 0)
	{
		error_msg = "Error setting SO_RCVTIMEO: " + std::string(strerror(errno)) + ". result: " + std::to_string(result) + ".\n";
	}

	result |= setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&send_timeout_ms, sizeof(send_timeout_ms));
	if (result != 0)
	{
		error_msg = "Error setting SO_SNDTIMEO: " + std::string(strerror(errno)) + ". result: " + std::to_string(result) + ".\n ";
	}

	return result;
}	// end of init_udp_broadcast

int32_t init_udp_broadcast(udp_info& info,
	std::string host_ip_address,
	std::string broadcast_address,
	uint16_t broadcast_port,
	std::string& error_msg,
	uint32_t recv_timeout_ms = 2000,
	uint32_t send_timeout_ms = 1000
)
{
	return init_udp_broadcast(info.udp_sock, info.read_addr_obj, host_ip_address, broadcast_address, broadcast_port, error_msg, recv_timeout_ms, send_timeout_ms);
}	// end of init_udp_broadcast

// ----------------------------------------------------------------------------
int32_t receive_broadcast_response(SOCKET& sock,
	struct sockaddr_in& sock_addr,
	char *data,
	uint32_t length,
	int32_t &bytes_received,
	std::string& error_msg
)
{
	int32_t result = 0;
	int32_t num_found = 0;

	fd_set read_fds;

	FD_ZERO(&read_fds);
	FD_SET(sock, &read_fds);

	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	num_found = select(0, &read_fds, 0, 0, &timeout);
	if (num_found <= 0)
	{
		error_msg = "Error with select: Nothing found.\n";
		return -1;
	}

	// got something
	result = FD_ISSET(sock, &read_fds);

	if (result) 
	{
		FD_CLR(sock, &read_fds);
		bytes_received = recvfrom(sock, data, length, 0, 0, 0);
	}
	else
	{
		error_msg = "Error with FD_ISSET: " + std::to_string(result) + ".\n ";
	}

	return num_found;
	
}	// end of receive_broadcast_response

// ----------------------------------------------------------------------------

#endif  // _UDP_NETWORK_FUNCTIONS_H
