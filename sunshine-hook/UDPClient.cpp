#include "stdafx.h"
//#include "UDPClient.h"
//#include <boost/array.hpp>
//UDPClient::UDPClient(const std::string& host, const unsigned int port)
//{
//	io_service = std::make_unique<boost::asio::io_service>();
//	work = std::make_unique<boost::asio::io_service::work>(*io_service);
//	socket = std::make_unique<tcp::socket>(*io_service, tcp::endpoint(tcp::v4(), 0));
//	destination = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port);
//	boost::system::error_code error;
//	socket->connect(destination, error);
//	if (error) {
//		LOG(ERROR) << error.message();
//	}
//	else {
//		LOG(INFO) << "Connected";
//	}
//	boost::asio::ip::tcp::no_delay option(true);
//	socket->set_option(option);
//	io_service_thread = std::thread(boost::bind(&boost::asio::io_service::run, boost::ref(*io_service)));
//	io_service_thread.detach();
//}
//
//UDPClient::~UDPClient()
//{
//	
//}
//
//void UDPClient::send(void * msg, int size)
//{
//	try {
//		//socket->async_send_to(boost::asio::buffer(msg, size), destination, boost::bind(&UDPClient::WriteHandler,
//		//	this, boost::asio::placeholders::error,
//		//	boost::asio::placeholders::bytes_transferred));
//		char* buffer = static_cast<char*>(msg);
//		size_t sent = socket->send(boost::asio::buffer(buffer, size));
//		//for (int left = 0; left < size;) {
//			//size_t sent = socket->send_to(boost::asio::buffer(buffer + left, min(size - left, 1000)), destination);
//		//	if (sent > 0) {
//		//		left += sent;
//		//	}
//		//	else {
//		//		LOG(INFO) << "Sent: " << sent;
//		//	}
//		//}
//	}
//	catch (std::exception& ex) {
//		LOG(ERROR) << "Error: " << ex.what();
//	}
//
//}
//
//void UDPClient::WriteHandler(
//	const boost::system::error_code& ec,
//	std::size_t bytes_transferred)
//{
//	//	LOG(INFO) << "Bytes sent: " << bytes_transferred;
//}