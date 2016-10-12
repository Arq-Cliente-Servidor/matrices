#pragma once

#include "ThreadPool.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <mutex>
#include <vector>

using namespace std;

template <typename T> class SparseMatrix {
private:
  size_t rows;
  size_t cols;
  vector<map<size_t, T>> vals;

public:
  SparseMatrix() : rows(0), cols(0), vals(0) {}
  SparseMatrix(size_t r, size_t c) : rows(r), cols(c), vals(r) {}
  SparseMatrix(const SparseMatrix<T> &) = default;
  SparseMatrix(SparseMatrix<T> &&) = default;

  SparseMatrix<T> &operator=(const SparseMatrix<T> &other) = default;
  SparseMatrix<T> &operator=(SparseMatrix &&other) = default;

  // getters
  size_t getNumRows() const { return rows; }
  size_t getNumCols() const { return cols; }
  pair<size_t, size_t> size() const { return make_pair(rows, cols); }

  const vector<map<size_t, T>> &getVals() const { return vals; }

  const T get(size_t i, size_t j) const {
    auto &r = vals[i];
    auto v = r.find(j);
    if (v != r.end()) {
      assert(v->second != T(0));
      return v->second;
    }
    return T();
  }

  // get row
  const map<size_t, T> &operator()(size_t r) const { return vals[r]; }

  // setters
  void resize(size_t r, size_t c) {
    rows = r;
    cols = c;
    vals.resize(rows);
  }

  void set(T value, size_t r, size_t c) {
    assert(r >= 0 && r <= rows);
    assert(c >= 0 && c <= cols);
    if (value != T(0)) {
      // cerr << "r " << r << " c " << c << endl;
      vals[r][c] = value;
    } else {
      vals[r].erase(c);
    }
  }

  bool setData(const vector<T> &other) {
    if (other.size() == rows * cols) {
      for (size_t i = 0, r = 0; i < other.size(); i += cols, r++) {
        for (size_t j = i, c = 0; j < i + cols; j++, c++) {
          set(other[j], r, c);
        }
      }
      return true;
    }
    return false;
  }

  // sequential operations
  SparseMatrix<T> operator*(const SparseMatrix<T> &other) {
    assert(cols == other.getNumRows());
    SparseMatrix<T> result(rows, other.getNumCols());

    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < other.getNumCols(); j++) {
        T accum(0);
        for (size_t k = 0; k < cols; k++) {
          if (get(k, j) == T(0) || other.get(i, k) == T(0))
            continue;
          accum += get(k, j) * other.get(i, k);
        }
        if (accum == T(0))
          continue;
        result.set(accum, i, j);
      }
    }

    return result;
  }

  SparseMatrix<T> diamond() {
    SparseMatrix<T> other(*this);
    assert(cols == other.getNumRows());
    SparseMatrix<T> result(rows, other.getNumCols());
    T oo(numeric_limits<T>::max());
    size_t exp = rows - 1;

    auto diamond_once = [&](SparseMatrix<T> m) {
      SparseMatrix<T> result(rows, m.getNumCols());
      for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < other.getNumCols(); j++) {
          T mn = oo;
          for (size_t k = 0; k < cols; k++) {
            if (get(k, j) == T(0) || other.get(i, k) == T(0))
              continue;
            mn = min(mn, get(k, j) + other.get(i, k));
          }
          if (mn == oo || mn == T(0))
            continue;
          result.set(mn, i, j);
        }
      }
      return result;
    };

    // optimization => log2(rows - 1) iterations
    while (exp) {
      if (exp & 1) {
        result = diamond_once(other);
      }
      other = diamond_once(other);
      exp >>= 1;
    }

    return result;
  }

  // concurrent operations
  SparseMatrix<T> multConcurrent(const SparseMatrix<T> &m2) {

    // Check
    assert(cols == m2.getNumRows());

    thread_pool *pool = new thread_pool();
    SparseMatrix<T> result(rows, m2.getNumCols());

    auto multRow = [&](size_t nRow) {
      const auto &row = m2(nRow);
      for (int i = 0; i < rows; i++) {
        T accum(0);

        for (const auto &it : row) {
          T tmp = get(it.first, i);
          if (tmp == T(0) || it.second == T(0))
            continue;
          accum += tmp * it.second;
        }
        if (accum == T(0))
          continue;
        result.set(accum, nRow, i);
      }
    };

    for (size_t i = 0; i < rows; i++) {
      auto func = [&multRow, i] { multRow(i); };
      pool->submit(func);
    }

    delete pool;

    return result;
  }

  SparseMatrix<T> partition(size_t offsetRow, size_t offsetCol) const {
    SparseMatrix<T> sub(rows / 2, cols / 2);
    for (size_t i = 0; i < rows / 2; i++) {
      for (size_t j = 0; j < cols / 2; j++) {
        sub.set(get(i + offsetRow, j + offsetCol), i, j);
      }
    }
    return sub;
  }

  void rebuild(SparseMatrix<T> &result, size_t offsetRow, size_t offsetCol) {
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        result.set(get(i, j), i + offsetRow, j + offsetCol);
      }
    }
  }

  void diamond_block(SparseMatrix<T> &m) const {}

  SparseMatrix<T> diamondConcurrent() const {
    SparseMatrix<T> m2(*this);
    // Check
    assert(cols == m2.getNumRows());
    SparseMatrix<T> result(rows, m2.getNumCols());
    size_t exp = rows - 1;

    auto diamondRow = [&](size_t nRow, const SparseMatrix<T> &m,
                          SparseMatrix<T> &result) {
      auto &row = m(nRow);
      T oo(numeric_limits<T>::max());

      for (size_t i = 0; i < rows; i++) {
        T mn = oo;
        for (const auto &it : row) {
          T tmp = get(it.first, i);
          if (tmp == T(0) || it.second == T(0))
            mn = min(mn, oo);
          else
            mn = min(mn, tmp + it.second);
        }
        if (mn == oo || mn == 0)
          continue;
        result.set(mn, nRow, i);
      }
    };

    auto diamond_once = [&](SparseMatrix<T> m) {
      SparseMatrix<T> result(rows, m.getNumCols());
      thread_pool *pool = new thread_pool();

      for (size_t i = 0; i < rows; i++) {
        auto func = [&diamondRow, i, &m, &result] { diamondRow(i, m, result); };
        pool->submit(func);
      }

      delete pool;
      return result;
    };

    // optimization => 1 + log2(rows - 1) iterations
    // while (exp) {
    //   if (exp & 1) {
    //     result = diamond_once(m2);
    //   }
    //   m2 = diamond_once(m2);
    //   exp >>= 1;
    // }
    result = diamond_once(m2);
    return result;
  }

  void print() {
    cout << "[";
    for (size_t i = 0; i < rows; i++) {
      if (i)
        cout << " ";
      cout << "[";
      for (size_t j = 0; j < cols; j++) {
        if (j)
          cout << ", ";
        cout << get(i, j);
      }
      cout << "]";
      if (i < rows - 1)
        cout << endl;
    }
    cout << "]" << endl;
  }
};

template <typename T>
ostream &operator<<(ostream &os, const SparseMatrix<T> &sp) {
  os << "[";
  for (size_t i = 0; i < sp.getNumRows(); i++) {
    if (i)
      os << " ";
    os << "[";
    for (size_t j = 0; j < sp.getNumCols(); j++) {
      if (j)
        os << ", ";
      os << sp.get(i, j);
    }
    os << "]";
    if (i < sp.getNumRows() - 1)
      os << "\n";
  }
  os << "]\n";
  return os;
}
