#pragma once

// #include "ThreadSafeQueue.hpp"
#include "SafeQueue.hpp"
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class thread_pool {
private:
  std::atomic_bool done;
  // threadsafe_queue<std::function<void()>> work_queue;
  SafeQueue<std::function<void()>> work_queue;
  std::vector<std::thread> threads;

  void worker_thread() {
    while (!done || !work_queue.empty()) {
      std::function<void()> task;
      if (work_queue.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }

public:
  thread_pool() : done(false) {
    // joiner(new join_threads(threads));
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads.emplace_back(std::thread(&thread_pool::worker_thread, this));
      }
    } catch (...) {
      done = true;
      throw;
    }
  }
  ~thread_pool() {
    // joiner->~join_threads();
    done = true;
    for (auto &thread : threads) {
      if (thread.joinable())
        thread.join();
    }
    // std::string s("Destructing pool ");
    // s += std::to_string(work_queue.empty());
    // s += '\n';
    // std::cerr << s;
  }

  std::vector<std::thread::id> getThreadIds() const {
    std::vector<std::thread::id> ids;
    for (auto &thread : threads) {
      ids.emplace_back(thread.get_id());
    }
    return ids;
  }

  template <typename FunctionType> void submit(FunctionType f) {
    // work_queue.push(std::function<void()>(f));
    work_queue.emplace(f);
    //    std::cerr << std::this_thread::get_id() << std::endl;
  }
};
