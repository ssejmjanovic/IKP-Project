#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>


class ThreadPool {
public:
	ThreadPool(size_t numThreads);
	~ThreadPool();

	template <typename F>
	void enqueue(F&& f);

private:
	void workerThread();

	std::vector<std::thread> threads;
	std::deque<std::function<void()>> tasks;
	std::mutex mutex;
	std::condition_variable condition;
	bool stop = false;

};







#endif // THREADPOOL_H