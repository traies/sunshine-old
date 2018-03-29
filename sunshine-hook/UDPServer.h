#pragma once
#include <thread>
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <boost\array.hpp>
#include <stdint.h>
#include "..\easyloggingpp\easylogging++.h"
#define ARRAY_SIZE	1000

using namespace boost::asio::ip;

class UDPServer
{
public:
	
	UDPServer(int port) : port(port)
	{
		io_service = std::make_unique<boost::asio::io_service>();
		socket = std::make_unique<udp::socket>(*io_service, udp::endpoint(udp::v4(), port));
		socket->async_receive(boost::asio::buffer(sbuf.prepare(ARRAY_SIZE)), boost::bind(
			&UDPServer::Receive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		));
		io_service_thread = std::thread([&] {
			io_service->run();
		});
		io_service_thread.detach();
	}
	~UDPServer() 
	{
		LOG(ERROR) << "UDPServer exited";
	};

private:
	bool stop = false;
	std::thread io_service_thread;
	std::unique_ptr<boost::asio::io_service> io_service;
	std::unique_ptr<udp::socket> socket;
	std::unique_ptr<boost::asio::io_service::work> work;	// While this exists, the service will keep on running.
	boost::asio::streambuf sbuf;
	int port;
	udp::endpoint destination;

	void Receive(const boost::system::error_code& error, std::size_t bytesTransfered);
};

