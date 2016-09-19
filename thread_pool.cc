#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class join_threads {
  std::vector<std::thread> &threads;

public:
  explicit join_threads(std::vector<std::thread> &threads_)
      : threads(threads_) {}
  ~join_threads() {
    std::cerr << "destructing joiner\n";
    for (unsigned long i = 0; i < threads.size(); ++i) {
      if (threads[i].joinable())
        threads[i].join();
    }
  }
};

template <typename T> class threadsafe_queue {
private:
  mutable std::mutex mut;
  std::queue<T> data_queue;
  std::condition_variable data_cond;

public:
  threadsafe_queue() {}
  void push(T data) {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(std::move(data));
    data_cond.notify_one();
  }
  void wait_and_pop(T &value) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    value = std::move(data_queue.front());
    data_queue.pop();
  }
  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));

    data_queue.pop();
    return res;
  }
  bool try_pop(T &value) {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
      return false;
    value = std::move(data_queue.front());
    data_queue.pop();
    return true;
  }
  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
      return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
    data_queue.pop();
    return res;
  }
  bool empty() const {
    std::lock_guard<std::mutex> lk(mut);
    return data_queue.empty();
  }
};

class thread_pool {
  std::atomic_bool done;
  threadsafe_queue<std::function<void()>> work_queue;
  std::vector<std::thread> threads;
  join_threads *joiner;
  void worker_thread() {
    while (!done && !work_queue.empty()) {
      std::function<void()> task;
      if (work_queue.try_pop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }

public:
  thread_pool() : done(false), joiner(new join_threads(threads)) {
    // joiner(new join_threads(threads));
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        threads.push_back(std::thread(&thread_pool::worker_thread, this));
      }
    } catch (...) {
      done = true;
      throw;
    }
  }
  ~thread_pool() {
    joiner->~join_threads();
    done = true;
    // std::string s("Destructing pool ");
    // s += std::to_string(work_queue.empty());
    // s += '\n';
    // std::cerr << s;
  }
  template <typename FunctionType> void submit(FunctionType f) {
    work_queue.push(std::function<void()>(f));
    //    std::cerr << std::this_thread::get_id() << std::endl;
  }
};

using Matrix = std::vector<std::vector<int>>;
using Vector = std::vector<int>;

void print(const Matrix &m) {
  std::cerr << "------------------------\n";
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[0].size(); j++) {
      if (j)
        std::cout << " ";
      std::cout << m[i][j];
    }
    std::cout << std::endl;
  }
  std::cerr << "------------------------\n";
}

Vector getCol(const Matrix &m, int numCol) {
  Vector col(m.size());
  for (int i = 0; i < m.size(); i++) {
    col[i] = m[i][numCol];
  }
  return col;
}

void diamondCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result) {
  // std::cerr << "start " << nCol << std::endl;
  // std::vector<int> col = getCol(m2, nCol);
  for (int i = 0; i < m1.size(); i++) {
    int mn = std::numeric_limits<int>::max();
    for (int j = 0; j < m1[i].size(); j++) {
      mn = std::min(mn, m1[i][j] + m2[j][nCol]);
    }
    result[i][nCol] = mn;
  }
  // print(result);
  // std::cerr << "dcol\n";
}

int main() {
  std::ifstream dataset;
  int rows, cols;

  dataset.open("files/test1.txt", std::ios::in);
  dataset >> rows >> cols;
  Matrix m(rows, Vector(cols));
  Matrix result(m.size(), Vector(m[0].size(), 0));

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++)
      dataset >> m[i][j];
  }
  //  Matrix m0(m);

  // print(m);
  // for (int j = 0; j < 40; j++) {
  //   result = Matrix(m.size(), Vector(m[0].size(), 0));
  //
  {
    thread_pool pool;
    for (int i = 0; i < 3; i++) {
      auto w = [&m, &result, i]() { diamondCol(m, m, i, result); };
      pool.submit(w);
    }
  }
  //   m = result;
  // }
  print(result);

  return 0;
}
