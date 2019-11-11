#pragma once
#include <WS2tcpip.h>

class UDPClient
{
public:
	UDPClient(const char* ip, int port);
	~UDPClient();

	int Send(char* buf, int len);
	int Receive(char* buf, int len);
	bool Bind();

private:
	bool Init(const char* ip, const char* port);
	SOCKET _socket;
	struct addrinfo* addr;
};

