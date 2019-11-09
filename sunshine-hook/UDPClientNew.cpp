#include "stdafx.h"
#include "UDPClientNew.h"
#include <WS2tcpip.h>

#include "..\easyloggingpp\easylogging++.h"


UDPClientNew::UDPClientNew(const char* ip, int port)
{
	char portStr[100];
	_itoa_s(port, portStr, 10);
	Init("127.0.0.1", "2234", ip, portStr);
}

UDPClientNew::~UDPClientNew()
{

}

bool UDPClientNew::Init(const char * fromIp, const char * fromPort, const char* toIp, const char* toPort)
{
	WSADATA wsaData;

	int res;
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		LOG(ERROR) << "WSAStartup failed: " << res;
		return false;
	}

	struct addrinfo* result = nullptr, * ptr = nullptr, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	res = getaddrinfo(fromIp, fromPort, &hints, &result);
	if (res != 0) {
		LOG(ERROR) << "getaddrinfo failed: " << res;
		WSACleanup();
		return false;
	}

	_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (_socket == INVALID_SOCKET) {
		LOG(ERROR) << "socket failed: " << WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}
	freeaddrinfo(result);

	struct addrinfo* to = nullptr;
	res = getaddrinfo(toIp, toPort, &hints, &to);
	if (res != 0) {
		LOG(ERROR) << "getaddrinfo to failed: " << res;
		WSACleanup();
		return false;
	}

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

	int bufferSize;
	while (len > 0) {
		bufferSize = min(1000, len);
		int iResult = send(_socket, buf, bufferSize, 0);
		if (iResult > 0) {
			// OK
			len -= iResult;
			buf += iResult;
			continue;
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
	return len;
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
