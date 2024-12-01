#include "pch.h"
#include "ConcreteMessageQueueServiceh.h"

void ConcreteMessageQueueService::SendMessage(const std::string& queueName, const void* message, int messageSize) {
	std::string msg(static_cast<const char*>(message), messageSize);

	// Kreiraj red ako ne postoji
	if (queues.find(queueName) == queues.end()) {
		CreateQueue(queueName);
	}

	queues[queueName]->enqueue(msg);
}

std::shared_ptr<MessageQueue<std::string>> ConcreteMessageQueueService::GetQueue(const std::string& queueName) {
	if (queues.find(queueName) != queues.end()) {
		return queues[queueName];
	}
	return nullptr;
}

void ConcreteMessageQueueService::CreateQueue(const std::string& queueName) {
	queues[queueName] = std::make_shared<MessageQueue<std::string>>();
}