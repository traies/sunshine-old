#pragma once
class UDPServer
{
public:
	UDPServer(char * port);
	~UDPServer();

	int Send(char* buf, int len);
	int Receive(char* buf, int len);

private:
	SOCKET _socket = 0;
};

