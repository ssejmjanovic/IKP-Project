#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

class MessageQueue {
private:
    std::queue<std::string> queue; // Red poruka
    std::mutex mutex;              // Zastita pristupa redu
    std::condition_variable cv;    // Za sinhronizaciju

public:
    // Dodavanje poruke u red
    void enqueue(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(message);
        cv.notify_one(); // Obavesti radne threade da je nova poruka dodata
    }

    // Uzimanje poruke iz reda
    std::string dequeue() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty(); }); // Ceka dok red nije prazan
        std::string message = queue.front();
        queue.pop();
        return message;
    }

    // Provera da li je red prazan
    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
};

#endif // MESSAGEQUEUE_H

