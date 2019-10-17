#include "stdafx.h"
#include "UDPServer.h"
#include "..\easyloggingpp\easylogging++.h"

bool UDPServer::Init()
{
	// https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-server
	WSADATA wsaData;

	// Initialize Winsock
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		LOG(ERROR) << "WSAStartup failed: " << iResult;
		return false;
	}

	struct addrinfo* result = nullptr, * ptr = nullptr, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(nullptr, _port, &hints, &result);
	if (iResult != 0) {
		LOG(ERROR) << "getaddrinfo failed: " << iResult;
		return false;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		LOG(ERROR) << "Error at socket(): " << WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	iResult = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		LOG(ERROR) << "Bind failed with error: " << WSAGetLastError();
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		LOG(ERROR) << "Listen failed with error: " << WSAGetLastError();
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}

	_clientSocket = INVALID_SOCKET;
	_clientSocket = accept(ListenSocket, (sockaddr *) NULL, (int) NULL);
	if (_clientSocket == INVALID_SOCKET) {
		LOG(ERROR) << "Accept failed with error: " << WSAGetLastError();
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}
	closesocket(ListenSocket);
	return true;
}

int UDPServer::Receive(char * buf, int len)
{
	int iResult = recv(_clientSocket, buf, len, 0);
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