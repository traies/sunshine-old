#include "stdafx.h"
#include "UDPClient.h"

//UDPClient::UDPClient(const std::string& host, const unsigned int port)
//{
//	io_service = std::make_unique<boost::asio::io_service>();
//	work = std::make_unique<boost::asio::io_service::work>(*io_service);
//	socket = std::make_unique<tcp::socket>(*io_service, tcp::endpoint(tcp::v4(), 0));
//	destination = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port);
//	boost::system::error_code error;
//	while (true) {
//		socket->connect(destination, error);
//		if (error) {
//			LOG(ERROR) << error.message();
//			Sleep(5000);
//		}
//		else {
//			LOG(INFO) << "Connected";
//			break;
//		}
//	}
//	io_service_thread = std::thread(boost::bind(&boost::asio::io_service::run, boost::ref(*io_service)));
//	io_service_thread.detach();
//}
//
//UDPClient::~UDPClient()
//{
//
//}
//
//size_t UDPClient::send(void * msg, int size)
//{
//	try {
//		char* buffer = static_cast<char*>(msg);
//		return socket->send(boost::asio::buffer(msg, size));
//	}
//	catch (std::exception& ex) {
//		LOG(ERROR) << "Error: " << ex.what();
//	}
//
//}