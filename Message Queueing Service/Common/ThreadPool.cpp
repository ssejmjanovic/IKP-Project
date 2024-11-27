#include "ThreadPool.h"
#include <iostream>
#include <functional>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

ThreadPool::ThreadPool(size_t numThreads) {
	for (size_t i = 0; i < numThreads; ++i) {
		threads.push_back(std::thread([this]() {
			this->workerThread();
			}));
	}
}

ThreadPool::~ThreadPool() {
	{
		std::unique_lock<std::mutex> lock(mutex);
		stop = true;
	}

	condition.notify_all();

	for (std::thread& t : threads) {
		if (t.joinable()) {
			t.join();
		}
	}
}

template <typename F>
void ThreadPool::enqueue(F&& f) {
	{
		std::unique_lock<std::mutex> lock(mutex);
		tasks.push_back(std::function<void()>(std::forward<F>(f)));
	}

	condition.notify_one();
}

void ThreadPool::workerThread() {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mutex);
			condition.wait(lock, [this]() {return !tasks.empty() || stop; });

			if (stop && tasks.empty()) {
				return;
			}

			task = std::move(tasks.front());
			tasks.pop_front();
		}
		task();
	}
}