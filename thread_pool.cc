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
    // std::cerr << "destructing joiner\n";
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

void bijk(const Matrix &A, const Matrix &B, Matrix &C) {
  int i, j, k, kk, jj;
  double sum;
  int n = A.size();
  int en = n;
  int bsize = n / 2;
  // bsize * (n / bsize); /* Amount that fits evenly into blocks */

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      C[i][j] = 0.0;

  for (kk = 0; kk < en; kk += bsize) {
    for (jj = 0; jj < en; jj += bsize) {
      for (i = 0; i < n; i++) {
        for (j = jj; j < jj + bsize; j++) {
          sum = C[i][j];
          for (k = kk; k < kk + bsize; k++) {
            sum += A[i][k] * B[k][j];
          }
          C[i][j] = sum;
        }
      }
    }
  }

  print(C);
}

Matrix partition(const Matrix &m, int offsetI, int offsetJ) {
  Matrix p(m.size() / 2, Vector(m.size() / 2, 0));
  for (int i = 0; i < m.size() / 2; i++) {
    for (int j = 0; j < m.size() / 2; j++) {
      p[i][j] = m[i + offsetI][j + offsetJ];
    }
  }
  return p;
}

void rebuild(Matrix &result, int offsetI, int offsetJ, const Matrix &m) {
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[i].size(); j++) {
      result[i + offsetI][j + offsetJ] = m[i][j];
    }
  }
}

Matrix addMatrix(const Matrix &a, const Matrix &b) {
  Matrix c(a.size(), Vector(a.size(), 0));
  for (int i = 0; i < a.size(); i++) {
    for (int j = 0; j < a[i].size(); j++) {
      c[i][j] = a[i][j] + b[i][j];
    }
  }

  return c;
}

// Secuencial
Matrix multMatrix(const Matrix &m1, const Matrix &m2) {
  Matrix result(m1.size(), Vector(m1.size(), 0));
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1.size(); j++) {
      for (int k = 0; k < m1.size(); k++) {
        result[i][j] += m1[i][k] * m2[k][j];
      }
    }
  }

  return result;
}

void multCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result) {
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1.size(); j++) {
      result[i][nCol] += m1[i][j] * m2[j][nCol];
    }
  }
}

Matrix multConcurrency(const Matrix &m1, const Matrix &m2) {
  Matrix result(m1.size(), Vector(m1[0].size(), 0));
  {
    thread_pool pool;
    for (int i = 0; i < m1.size(); i++) {
      auto w = [&m1, &m2, &result, i]() { multCol(m1, m2, i, result); };
      pool.submit(w);
    }
  }
  // print(result);
  return result;
}

Matrix block_seq(const Matrix &A, const Matrix &B) {
  if (A.size() == 2) {
    return multMatrix(A, B);
  } else {
    int sizeA = A.size() / 2;
    int sizeB = B.size() / 2;

    Matrix a0 = partition(A, 0, 0);
    Matrix a1 = partition(A, 0, sizeA);
    Matrix a2 = partition(A, sizeA, 0);
    Matrix a3 = partition(A, sizeA, sizeA);

    Matrix b0 = partition(B, 0, 0);
    Matrix b1 = partition(B, 0, sizeB);
    Matrix b2 = partition(B, sizeB, 0);
    Matrix b3 = partition(B, sizeB, sizeB);

    Matrix r0 = addMatrix(block_seq(a0, b0), block_seq(a1, b2));
    Matrix r1 = addMatrix(block_seq(a0, b1), block_seq(a1, b3));
    Matrix r2 = addMatrix(block_seq(a2, b0), block_seq(a3, b2));
    Matrix r3 = addMatrix(block_seq(a2, b1), block_seq(a3, b3));

    Matrix result(A.size(), Vector(A.size(), 0));
    int sizeResult = result.size() / 2;

    rebuild(result, 0, 0, r0);
    rebuild(result, 0, sizeResult, r1);
    rebuild(result, sizeResult, 0, r2);
    rebuild(result, sizeResult, sizeResult, r3);

    return result;
  }
}

Matrix diamondMatrix(const Matrix &m1, const Matrix &m2) {
  Matrix result(m1.size(), Vector(m1.size(), 0));
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1.size(); j++) {
      int mn = std::numeric_limits<int>::max();
      for (int k = 0; k < m1.size(); k++) {
        mn = std::min(mn, m1[i][k] + m2[k][j]);
      }
      result[i][j] = mn;
    }
  }

  return result;
}

Matrix minMatrix(const Matrix &A, const Matrix &B) {
  Matrix C(A.size(), Vector(A.size(), std::numeric_limits<int>::max()));
  for (int i = 0; i < A.size(); i++) {
    for (int j = 0; j < A[i].size(); j++) {
      C[i][j] = std::min(A[i][j], B[i][j]);
    }
  }
  return C;
}

Matrix diamond_block_seq(const Matrix &A, const Matrix &B) {
  if (A.size() == 2) {
    return diamondMatrix(A, B);
  } else {
    int sizeA = A.size() / 2;
    int sizeB = B.size() / 2;

    Matrix a0 = partition(A, 0, 0);
    Matrix a1 = partition(A, 0, sizeA);
    Matrix a2 = partition(A, sizeA, 0);
    Matrix a3 = partition(A, sizeA, sizeA);

    Matrix b0 = partition(B, 0, 0);
    Matrix b1 = partition(B, 0, sizeB);
    Matrix b2 = partition(B, sizeB, 0);
    Matrix b3 = partition(B, sizeB, sizeB);

    Matrix r0 = minMatrix(diamond_block_seq(a0, b0), diamond_block_seq(a1, b2));
    Matrix r1 = minMatrix(diamond_block_seq(a0, b1), diamond_block_seq(a1, b3));
    Matrix r2 = minMatrix(diamond_block_seq(a2, b0), diamond_block_seq(a3, b2));
    Matrix r3 = minMatrix(diamond_block_seq(a2, b1), diamond_block_seq(a3, b3));

    Matrix result(A.size(), Vector(A.size(), 0));
    int sizeResult = result.size() / 2;

    rebuild(result, 0, 0, r0);
    rebuild(result, 0, sizeResult, r1);
    rebuild(result, sizeResult, 0, r2);
    rebuild(result, sizeResult, sizeResult, r3);

    return result;
  }
}

void diamondConcurrency(const Matrix &m1) {
  Matrix result(m1.size(), Vector(m1[0].size(), 0));
  Matrix m2(m1);
  for (int j = 0; j < m1.size() - 1; j++) {
    {
      thread_pool pool;
      for (int i = 0; i < m1.size(); i++) {
        auto w = [&m1, &m2, &result, i]() { diamondCol(m1, m2, i, result); };
        pool.submit(w);
      }
    }
    m2 = result;
    print(result);
    std::cout << std::endl;
  }
  print(result);
}

int main() {
  std::ifstream dataset;
  // int rows, cols;
  //
  // dataset.open("files/test1024.txt", std::ios::in);
  // dataset >> rows >> cols;
  // Matrix m(rows, Vector(cols));
  //
  // for (int i = 0; i < rows; i++) {
  //   for (int j = 0; j < cols; j++)
  //     dataset >> m[i][j];
  // }
  Matrix m = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  print(diamond_block_seq(m, m));

  // m <> m = {{2, 3, 4, 5}
  //           {6, 7, 8, 9},
  //          {10, 11, 12, 13},
  //          {14, 15, 16, 17}}

  // print(m);
  // auto start = std::chrono::high_resolution_clock::now();
  // Matrix result = block_seq(m, m);
  // print(result);
  // diamondConcurrency(m);
  // multConcurrency(m, m);
  // auto end = std::chrono::high_resolution_clock::now();
  // auto duration =
  //     std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
  //         .count();
  // std::cout << "Time Concurrency: " << duration << "ms" << std::endl;
  // print(result);

  // Matrix B = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}};
  // Matrix AA = {{1, 2}, {3, 4}};
  // Matrix BB = {{1, 2}, {3, 4}};
  // Matrix C(A.size(), Vector(A.size(), 0));
  // bijk(A, A, C);
  // int n = A.size() / 2;
  // Matrix p1 = partition(A, 0, 0);
  // Matrix p2 = partition(A, 0, n);
  // Matrix p3 = partition(A, n, 0);
  // Matrix p4 = partition(A, n, n);
  //
  // rebuild(C, 0, 0, p1);
  // rebuild(C, 0, n, p2);
  // rebuild(C, n, 0, p3);
  // rebuild(C, n, n, p4);

  //
  // print(p1);
  // std::cout << std::endl;
  // print(p2);
  // std::cout << std::endl;
  // print(p3);
  // std::cout << std::endl;
  // print(p4);
  // std::cout << std::endl;
  // Matrix C = block_seq(A, B);
  // print(C);

  return 0;
}
