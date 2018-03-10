#include "MessageQueueDispatcher.h"

MessageQueueDispatcher::~MessageQueueDispatcher()
{
}


void MessageQueueDispatcher::SetMessageQueue(const char * mqPath)
{
	mq = std::make_unique<boost::interprocess::message_queue>(boost::interprocess::open_only, mqPath);
}