#pragma once
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "..\easyloggingpp\easylogging++.h"

#define ARRAY_SIZE	1000

class UDPServer
{
public:
	
	UDPServer(const char * port) : _port(port) {}

	bool Init();
	int Receive(char * buf, int len);
	
	~UDPServer() 
	{
		closesocket(_clientSocket);
		WSACleanup();
		LOG(ERROR) << "UDPServer exited";
	};

private:
	const char * _port;
	SOCKET _clientSocket;
};

