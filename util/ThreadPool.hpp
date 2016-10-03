#pragma once

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

#include "SafeQueue.hpp"

class ThreadPool {
public:
    ThreadPool() : running_(true) {
        unsigned int thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned int i = 0; i < thread_count; i++) {
                threads_.emplace_back(&ThreadPool::WorkerThread, this);
            }
        } catch (...) {
            running_ = false;
            throw;
        }
    }

    ~ThreadPool() {
        running_ = false;
        for (auto& thread : threads_) {
            if (thread.joinable()) thread.join();
        }
    }

    std::vector<std::thread::id> GetThreadIds() const {
        std::vector<std::thread::id> ids;
        for (auto& thread : threads_) {
            ids.emplace_back(thread.get_id());
        }
        return ids;
    }

    template <typename FunctionType>
    void Submit(FunctionType f) {
        work_queue_.emplace(f);
    }

private:
    void WorkerThread() {
        while (running_ || !work_queue_.empty()) {
            std::function<void()> task;
            if (work_queue_.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

private:
    std::atomic_bool running_;
    std::vector<std::thread> threads_;
    SafeQueue<std::function<void()>> work_queue_;
};
