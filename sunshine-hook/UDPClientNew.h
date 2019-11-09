#pragma once


class UDPClientNew
{
public:
	UDPClientNew(const char* ip, int port);
	~UDPClientNew();

	int Send(char* buf, int len);
	int Receive(char* buf, int len);

private:
	bool Init(const char* fromIp, const char* fromPort, const char* toIp, const char* toPort);
	SOCKET _socket;
};

