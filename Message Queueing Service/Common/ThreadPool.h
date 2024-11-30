#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>


class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    // Dodavanje zadatka u red
    void enqueue(std::function<void()> task);

private:
    // Radne niti
    std::vector<std::thread> workers;

    // Red zadataka
    std::queue<std::function<void()>> tasks;

    // Sinhronizacija
    std::mutex queueMutex;
    std::condition_variable condition;

    // Kontrola rada niti
    std::atomic<bool> stop;

    // Funkcija koja pokrece niti i obradjuje zadatke
    void worker();
};







#endif // THREADPOOL_H