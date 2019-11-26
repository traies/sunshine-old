#include "stdafx.h"
#include "UDPClient.h"
#include "..\easyloggingpp\easylogging++.h"

UDPClient::UDPClient(int port)
{
	Init(port);
}

UDPClient::~UDPClient()
{

}

bool UDPClient::Init(int port)
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

	DWORD buffSize = 11728640;
	setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (char*)&buffSize, sizeof(DWORD));

	int enable = 1;
	setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
	
	DWORD timeout = 2000;
	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	struct sockaddr_in from = {};
	from.sin_addr.s_addr = INADDR_ANY;
	from.sin_port = htons(port);
	from.sin_family = AF_INET;

	res = bind(_socket, (SOCKADDR *)&from, sizeof(from));
	if (res == SOCKET_ERROR) {
		LOG(ERROR) << "Bind failed with error: " << WSAGetLastError();
		closesocket(_socket);
		WSACleanup();
		return false;
	}
	return true;
}

bool UDPClient::Listen()
{
	struct sockaddr_in remoteAddr;
	int remoteLen = sizeof(remoteAddr);
	static char buf[1000];
	int res = recvfrom(_socket, buf, 1000, MSG_PEEK, (SOCKADDR*) &remoteAddr, &remoteLen);
	if (res < 0) {
		return false;
	}
	else {
		res = connect(_socket, (SOCKADDR*)&remoteAddr, remoteLen);
		if (res != 0) {
			LOG(ERROR) << "error while connecting...";
			return false;
		}
		return true;
	}
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
