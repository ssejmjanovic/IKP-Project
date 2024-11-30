#include "ThreadPool.h"
#include <iostream>


ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
	// Kreiranje radnih niti
	for (size_t i = 0; i < numThreads; i++) {
		workers.emplace_back(&ThreadPool::worker, this);
	}
}

ThreadPool::~ThreadPool() {
	// Signalizacija zaustavljanja rada niti
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		stop = true;
	}

	// Obavestavanje svih niti da prestanu sa radom
	condition.notify_all();

	// Cekanje da se sve niti zavrse
	for (std::thread& worker : workers) {
		if (worker.joinable()) {
			worker.join();
		}
	}
}

void ThreadPool::enqueue(std::function<void()> task) {
	// Dodavanje zadatka u red
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		tasks.push(task);
	}

	// Obavestavanje slobodne niti da je zadatak dostupan
	condition.notify_one();
}

void ThreadPool::worker() {
	while (true) {
		std::function<void()> task;

		//Cekanje na zadatak iz reda
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			condition.wait(lock, [this] { return stop || !tasks.empty(); });

			// Ako je signalizovano zaustavljanje, izlazimo iz petlje
			if (stop && tasks.empty()) {
				return;
			}

			// Preuzimanje zadatka sa vrha reda
			task = std::move(tasks.front());
			tasks.pop();
		}

		// Izvrsavanje zadatka
		task();
	}
}