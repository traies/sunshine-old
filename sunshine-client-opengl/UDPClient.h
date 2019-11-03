#pragma once
//#include <boost\asio.hpp>
//#include "..\easyloggingpp\easylogging++.h"
//#include <boost\bind.hpp>
//#include <thread>
//using namespace boost::asio::ip;
//
//class UDPClient
//{
//public:
//	UDPClient() {};
//	UDPClient(const std::string& host, const unsigned int port);
//	~UDPClient();
//	void Stop();
//	size_t send(void * msg, int size);
//
//private:
//	bool stop = false;
//	std::thread io_service_thread;
//	std::unique_ptr<boost::asio::io_service> io_service;
//	std::unique_ptr<tcp::socket> socket;
//	std::unique_ptr<boost::asio::io_service::work> work;	// While this exists, the service will keep on running.
//	tcp::endpoint destination;
//
//};
//
