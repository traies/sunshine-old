#include "stdafx.h"
#include "UDPClient.h"

UDPClient::UDPClient(const std::string& host, const unsigned int port)
{
	io_service = std::make_unique<boost::asio::io_service>();
	work = std::make_unique<boost::asio::io_service::work>(*io_service);
	socket = std::make_unique<udp::socket>(*io_service, udp::endpoint(udp::v4(), 0));
	destination = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(host), port);
	io_service_thread = std::thread(boost::bind(&boost::asio::io_service::run, boost::ref(*io_service)));
	io_service_thread.detach();
}

UDPClient::~UDPClient()
{
	
}

void UDPClient::send(void * msg, int size)
{
	try {
		socket->async_send_to(boost::asio::buffer(msg, size), destination, boost::bind(&UDPClient::WriteHandler,
			this, boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
	catch (std::exception& ex) {
		LOG(ERROR) << "Error: " << ex.what();
	}

}

void UDPClient::WriteHandler(
	const boost::system::error_code& ec,
	std::size_t bytes_transferred)
{
	//	LOG(INFO) << "Bytes sent: " << bytes_transferred;
}