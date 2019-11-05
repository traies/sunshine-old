#include "stdafx.h"

#include "TCPClient.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "..\easyloggingpp\easylogging++.h"
TCPClient::TCPClient() 
{

}

TCPClient::~TCPClient() {
	if (_socket) {
		closesocket(_socket);
	}
	WSACleanup();
}

bool TCPClient::Listen(const char * port)
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

	iResult = getaddrinfo(nullptr, port, &hints, &result);
	if (iResult != 0) {
		LOG(ERROR) << "getaddrinfo failed: " << iResult;
		WSACleanup();
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

	_socket = INVALID_SOCKET;
	_socket = accept(ListenSocket, (sockaddr*)NULL, (int)NULL);
	if (_socket == INVALID_SOCKET) {
		LOG(ERROR) << "Accept failed with error: " << WSAGetLastError();
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}
	closesocket(ListenSocket);
	return true;
}

bool TCPClient::Connect(const char* ip, const char* port)
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
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ip, port, &hints, &result);
	if (iResult != 0) {
		LOG(ERROR) << "getaddrinfo failed: " << iResult;
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

	while (true) {
		iResult = connect(_socket, result->ai_addr, result->ai_addrlen);
		if (iResult != 0) {
			LOG(ERROR) << "connect failed: " << iResult;
			//freeaddrinfo(result);
			//WSACleanup();
			//return false;
			Sleep(1000);
		}
		else {
			break;
		}
	}
	freeaddrinfo(result);
	LOG(INFO) << "Connected";
	return true;
}

int TCPClient::Send(char* buf, int len)
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

int TCPClient::Receive(char* buf, int len)
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
