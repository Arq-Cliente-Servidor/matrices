#pragma once

#include <deque>
#include <mutex>

template <typename T, typename Container = std::deque<T>> class SafeQueue {
public:
  using container_type = Container;
  using value_type = typename Container::value_type;
  using size_type = typename Container::size_type;
  using reference = typename Container::reference;
  using const_reference = typename Container::const_reference;

public:
  explicit SafeQueue(const Container &cont) : c(cont) {}

  explicit SafeQueue(Container &&cont = Container()) : c(std::move(cont)) {}

  SafeQueue(const SafeQueue &other) : c(other.c) {}

  template <class Alloc> explicit SafeQueue(const Alloc &alloc) : c(alloc) {}

  template <class Alloc>
  SafeQueue(const Container &cont, const Alloc &alloc) : c(cont, alloc) {}

  template <class Alloc>
  SafeQueue(Container &&cont, const Alloc &alloc) : c(std::move(cont), alloc) {}

  template <class Alloc>
  SafeQueue(const SafeQueue &other, const Alloc &alloc) : c(other.c, alloc) {}

  template <class Alloc>
  SafeQueue(SafeQueue &&other, const Alloc &alloc)
      : c(std::move(other.c), alloc) {}

  ~SafeQueue() { std::lock_guard<std::mutex> lk(mutex_); }

  reference front() { return c.front(); }

  const_reference front() const { return c.front(); }

  reference back() { return c.back(); }

  const_reference back() const { return c.back(); }

  bool empty() const { return c.empty(); }

  size_type size() const { return c.size(); }

  void clear() {
    std::lock_guard<std::mutex> lk(mutex_);
    c.clear();
  }

  void push(const value_type &value) {
    std::lock_guard<std::mutex> lk(mutex_);
    return c.push_back(value);
  }

  void push(value_type &&value) {
    std::lock_guard<std::mutex> lk(mutex_);
    return c.push_back(value);
  };

  template <class... Args> reference emplace(Args &&... args) {
    std::lock_guard<std::mutex> lk(mutex_);
    c.emplace_back(args...);
    return c.back();
  }

  void pop() {
    std::lock_guard<std::mutex> lk(mutex_);
    return c.pop_front();
  }

  bool try_pop(reference value) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (c.empty())
      return false;
    value = std::move(c.front());
    c.pop_front();
    return true;
  }

  void swap(SafeQueue &other) {
    std::lock_guard<std::mutex> lk(mutex_);
    std::lock_guard<std::mutex> lk2(other.mutex_);
    c.swap(other.c);
  }

protected:
  Container c;

private:
  std::mutex mutex_;
};

namespace std {
template <class T, class Container>
void swap(SafeQueue<T, Container> &lhs, SafeQueue<T, Container> &rhs) {
  lhs.swap(rhs);
}
}
