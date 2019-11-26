#pragma once

class TCPClient
{
public:
	TCPClient();
	~TCPClient();
	bool Listen(const char * port);
	bool Connect(const char* ip, const char* port);
	int Send(char* buf, int len);
	int Receive(char* buf, int len);
private:
	SOCKET _socket = 0;
};


