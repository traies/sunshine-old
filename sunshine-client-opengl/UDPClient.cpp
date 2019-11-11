#include "stdafx.h"
#include "UDPClient.h"
#include "..\easyloggingpp\easylogging++.h"

UDPClient::UDPClient(const char* ip, int port)
{
	char portStr[100];
	_itoa_s(port, portStr, 10);
	Init(ip, portStr);
}

UDPClient::~UDPClient()
{

}

bool UDPClient::Init(const char* ip, const char* port)
{
	WSADATA wsaData;

	int res;
	res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res != 0) {
		LOG(ERROR) << "WSAStartup failed: " << res;
		return false;
	}

	struct addrinfo* ptr = nullptr, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	res = getaddrinfo(ip, port, &hints, &addr);
	if (res != 0) {
		LOG(ERROR) << "getaddrinfo failed: " << res;
		WSACleanup();
		return false;
	}

	_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (_socket == INVALID_SOCKET) {
		LOG(ERROR) << "socket failed: " << WSAGetLastError();
		freeaddrinfo(addr);
		WSACleanup();
		return false;
	}

	return true;
}

bool UDPClient::Bind()
{
	int iResult = bind(_socket, addr->ai_addr, addr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		LOG(ERROR) << "Bind failed with error: " << WSAGetLastError();
		freeaddrinfo(addr);
		closesocket(_socket);
		WSACleanup();
		return false;
	}

	return true;
}

int UDPClient::Send(char* buf, int len)
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
		LOG(ERROR) << "Error recv: " << WSAGetLastError();
		return 0;
	}
}

int UDPClient::Receive(char* buf, int len)
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
