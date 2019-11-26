#pragma once
#include <WS2tcpip.h>

class UDPClient
{
public:
	UDPClient(int port);
	~UDPClient();

	int Send(char* buf, int len);
	int Receive(char* buf, int len);
	bool Listen();
private:
	bool Init(int port);
	SOCKET _socket;
};

