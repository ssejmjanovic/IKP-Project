#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

// Klasa koja implementira red za poruke sa sinhronizacijom za visestruke thread-ove
class MessageQueue {
public:
    // Dodaje poruku u red
    void enqueue(const std::string& message);

    // Uklanja i vraca poruku iz reda (blokira dok red nije prazan)
    std::string dequeue();

    // Proverava da li je red prazan
    bool isEmpty();

private:
    std::queue<std::string> queue;             // Interna struktura za cuvanje poruka
    std::mutex mutex;                          // Mutex za sinhronizaciju pristupa redu
    std::condition_variable cv;               // Condition variable za upravljanje cekanjem
};

#endif // MESSAGE_QUEUE_H
