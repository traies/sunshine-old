#pragma once
#include "..\easyloggingpp\easylogging++.h"
#include <boost\interprocess\ipc\message_queue.hpp>
#include <mutex>

class MessageQueueDispatcher : public el::LogDispatchCallback
{
public:
	void SetMessageQueue(const char * mqPath);
	~MessageQueueDispatcher();
protected:
	void handle(const el::LogDispatchData* data) override
	{
		try {
			if (mq == nullptr) {
				//	handle was called before message queue was set.
				ExitProcess(1);
			}
			auto m_data = data->logMessage()->logger()->logBuilder()->build(data->logMessage(),
				data->dispatchAction() == el::base::DispatchAction::NormalLog);
			mq->send(m_data.data(), m_data.size(), 0);
		}
		catch (...) {

		}
	};
private:
	std::unique_ptr<boost::interprocess::message_queue> mq;
};

