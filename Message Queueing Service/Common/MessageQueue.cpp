#include "MessageQueue.h"

void MessageQueue::enqueue(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(message);
    cv.notify_one(); // Obavesti jedan thread da je nova poruka dodata
}

std::string MessageQueue::dequeue() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !queue.empty(); }); // Cekaj dok red nije prazan
    std::string message = queue.front();
    queue.pop();
    return message;
}

bool MessageQueue::isEmpty() {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
}
