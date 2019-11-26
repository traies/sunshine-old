#include "stdafx.h"
#include "UDPClientNew.h"
#include <WS2tcpip.h>

#include "..\easyloggingpp\easylogging++.h"


UDPClientNew::UDPClientNew(const char* ip, int port)
{
	char portStr[100];
	_itoa_s(port, portStr, 10);
	Init(ip, portStr);
}

UDPClientNew::~UDPClientNew()
{

}

bool UDPClientNew::Init(const char* toIp, const char* toPort)
{
	WSADATA wsaData;

	int res;
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		LOG(ERROR) << "WSAStartup failed: " << res;
		return false;
	}

	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_socket == INVALID_SOCKET) {
		LOG(ERROR) << "socket failed: " << WSAGetLastError();
		WSACleanup();
		return false;
	}

	struct sockaddr_in from = {};
	from.sin_family = AF_INET;
	from.sin_addr.s_addr = INADDR_ANY;
	bind(_socket, (sockaddr*) &from, sizeof(from));

	struct addrinfo* to = nullptr, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	res = getaddrinfo(toIp, toPort, &hints, &to);
	if (res != 0) {
		LOG(ERROR) << "getaddrinfo to failed: " << res;
		WSACleanup();
		return false;
	}
	
	DWORD buffSize = 104857600;
	setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, (char*)&buffSize, sizeof(DWORD));
	res = connect(_socket, to->ai_addr, to->ai_addrlen);
	if (res != 0) {
		LOG(ERROR) << "connect failed: " << WSAGetLastError();
		WSACleanup();
		return false;
	}
	return true;
}

int UDPClientNew::Send(char* buf, int len)
{
	if (!_socket) {
		LOG(ERROR) << "No socket to send";
		return 0;
	}

	int iResult = send(_socket, buf, len, 0);
	if (iResult > 0) {
		// OK
		return iResult;
	}
	else if (iResult == 0) {
		// Closing nicely
		LOG(INFO) << "Closing socket...";
		return 0;
	}
	else {
		// Closing with errors.
		LOG(ERROR) << "Error sending: " << WSAGetLastError();
		return 0;
	}
}

int UDPClientNew::Receive(char* buf, int len)
{
	if (!_socket) {
		LOG(ERROR) << "No socket to send";
		return 0;
	}
	int iResult = recv(_socket, buf, len, 0);
	if (iResult > 0) {
		// OK
		return iResult;
	}
	else if (iResult == 0) {
		// Closing nicely
		LOG(INFO) << "Closing socket...";
		return 0;
	}
	else {
		// Closing with errors.
		LOG(ERROR) << "Error recv: " << WSAGetLastError();
		return 0;
	}
}
