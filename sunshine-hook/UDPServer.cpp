#include "stdafx.h"
#include "UDPServer.h"
#include "..\easyloggingpp\easylogging++.h"
#include <cereal\archives\binary.hpp>

void UDPServer::Receive(const boost::system::error_code& error, std::size_t bytesTransfered)
{
	LOG(INFO) << bytesTransfered;
	sbuf.commit(bytesTransfered);
	auto data = sbuf.data();
	std::istream is(&sbuf);
	int64_t val;
	cereal::BinaryInputArchive archive(is);
	archive(val);
	LOG(INFO) << val;
	//LOG(INFO) << s;
	socket->async_receive(boost::asio::buffer(sbuf.prepare(ARRAY_SIZE)), 0, boost::bind(
		&UDPServer::Receive, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred
	));
}
