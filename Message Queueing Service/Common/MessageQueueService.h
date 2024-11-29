#ifndef MESSAGE_QUEUE_SERVICE_H
#define MESSAGE_QUEUE_SERVICE_H

#include <string>

class MessageQueueService {
public:
	virtual ~MessageQueueService() = default;

	// Omogucava slanje poruka na red odredjen imenom queueName
	virtual void SendMessage(const std::string& queueName, const void* message, int messageSize) = 0;
};


#endif // MESSAGE_QUEUE_SERVICE_H