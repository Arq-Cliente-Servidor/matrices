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
    } // else {
    // vals[r].erase(c);
    // }
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

  size_t countRowsZeros() {
    size_t count = 0;
    for (size_t i = 0; i < rows; i++) {
      count += (vals[i].size() == 0);
    }
    return count;
  }

  // sequential operations
  SparseMatrix<T> operator*(const SparseMatrix<T> &other) {
    assert(cols == other.getNumRows());
    SparseMatrix<T> result(rows, other.getNumCols());

    for (size_t i = 0; i < rows; i++) {
      const auto &row = vals[i];
      for (const auto &j : row) {
        const auto &othRow = other(j.first);
        for (const auto &k : othRow) {
          result.set(result.get(i, k.first) + (k.second * j.second), i,
                     k.first);
        }
      }
    }

    return result;
  }

  bool compare(SparseMatrix<T> &m2) const {
    for (size_t i = 0; i < rows; i++) {
      const auto &r = m2(i);
      for (const auto &it : r) {
        if (it.second != get(i, it.first))
          return false;
      }
    }
    return true;
  }

  SparseMatrix<T> diamond() {
    SparseMatrix<T> other(*this);
    assert(cols == other.getNumRows());
    SparseMatrix<T> result(rows, other.getNumCols());
    size_t exp = rows - 1;

    auto diamond_once = [&](const SparseMatrix<T> &m) {
      SparseMatrix<T> result(rows, m.getNumCols());
      T oo(numeric_limits<T>::max());
      for (size_t i = 0; i < rows; i++) {
        const auto &row = vals[i];
        for (const auto &j : row) {
          const auto &othRow = m(j.first);
          for (const auto &k : othRow) {
            // if (!ok) {
            //   result.set(k.second + j.second, i, k.first);
            //   ok = true;
            // }
            if (result.get(i, k.first) == T(0)) {
              result.set(min(k.second + j.second), i, k.first);
            } else {
              result.set(min(result.get(i, k.first), k.second + j.second), i,
                         k.first);
            }
          }
        }
      }
      return result;
    };

    // optimization => log2(rows - 1) iterations
    while (exp) {
      if (exp & 1) {
        result = diamond_once(other);
        exp = (exp - 1) >> 1;
      } else {
        other = diamond_once(other);
        exp >>= 1;
      }
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
      // get row
      const auto &row = vals[nRow];
      for (const auto &j : row) {
        const auto &othRow = m2(j.first);
        for (const auto &it : othRow) {
          result.set(result.get(nRow, it.first) + (it.second * j.second), nRow,
                     it.first);
        }
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

  SparseMatrix<T> diamondSeq(const SparseMatrix<T> &b) const {
    assert(cols == b.getNumRows());
    SparseMatrix<T> result(rows, b.getNumCols());
    T oo(numeric_limits<T>::max());

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < b.getNumCols(); j++) {
        T mn = oo;
        for (int k = 0; k < cols; k++) {
          if (get(i, k) == T(0) || b.get(k, j) == T(0))
            continue;
          // else if (get(i, k) != T(0) && && b.get(k, j) == T(0))
          mn = min(mn, get(i, k) + b.get(k, j));
        }

        if (mn != oo)
          result.set(mn, i, j);
      }
    }
    return result;
  }

  SparseMatrix<T> minMatrix(const SparseMatrix<T> &b) const {
    SparseMatrix<T> result(rows, cols);
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        T weight1 = get(i, j);
        T weight2 = b.get(i, j);
        if (weight1 == T(0) || weight2 == T(0)) {
          if (weight1 != T(0)) {
            result.set(weight1, i, j);
          } else if (weight2 != T(0)) {
            result.set(weight2, i, j);
          }
        } else {
          result.set(min(weight1, weight2), i, j);
        }
      }
    }
    return result;
  }

  SparseMatrix<T> diamond_block_seq(SparseMatrix<T> &m) const {
    if (rows == 2) {
      // cout << m << endl;
      return diamondSeq(m);
    } else {
      size_t sizeA = rows / 2;
      size_t sizeB = m.getNumRows() / 2;

      SparseMatrix<T> a0 = partition(0, 0);
      SparseMatrix<T> a1 = partition(0, sizeA);
      SparseMatrix<T> a2 = partition(sizeA, 0);
      SparseMatrix<T> a3 = partition(sizeA, sizeA);

      SparseMatrix<T> b0 = m.partition(0, 0);
      SparseMatrix<T> b1 = m.partition(0, sizeB);
      SparseMatrix<T> b2 = m.partition(sizeB, 0);
      SparseMatrix<T> b3 = m.partition(sizeB, sizeB);

      SparseMatrix<T> r0 =
          a0.diamond_block_seq(b0).minMatrix(a1.diamond_block_seq(b2));
      SparseMatrix<T> r1 =
          a0.diamond_block_seq(b1).minMatrix(a1.diamond_block_seq(b3));
      SparseMatrix<T> r2 =
          a2.diamond_block_seq(b0).minMatrix(a3.diamond_block_seq(b2));
      SparseMatrix<T> r3 =
          a2.diamond_block_seq(b1).minMatrix(a3.diamond_block_seq(b3));

      SparseMatrix<T> result(rows, cols);
      size_t sizeResult = result.getNumRows() / 2;

      r0.rebuild(result, 0, 0);
      r1.rebuild(result, 0, sizeResult);
      r2.rebuild(result, sizeResult, 0);
      r3.rebuild(result, sizeResult, sizeResult);

      return result;
    }
  }

  SparseMatrix<T> operator+(const SparseMatrix<T> &b) const {
    SparseMatrix<T> c(rows, cols);
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < cols; j++) {
        c.set(get(i, j) + b.get(i, j), i, j);
      }
    }

    return c;
  }

  SparseMatrix<T> multMatrix(const SparseMatrix<T> &m2) {
    SparseMatrix<T> result(rows, m2.getNumCols());
    for (size_t i = 0; i < rows; i++) {
      for (size_t j = 0; j < m2.getNumCols(); j++) {
        T accum(0);
        for (size_t k = 0; k < cols; k++) {
          if (get(i, k) == T(0) || m2.get(k, j) == T(0))
            continue;
          accum += get(i, k) * m2.get(k, j);
        }
        result.set(accum, i, j);
      }
    }

    return result;
  }

  SparseMatrix<T> mult_block_seq(const SparseMatrix<T> &m) {
    if (rows == 2) {
      return multMatrix(m);
    } else {
      size_t sizeA = rows / 2;
      size_t sizeB = m.getNumRows() / 2;

      SparseMatrix<T> a0 = partition(0, 0);
      SparseMatrix<T> a1 = partition(0, sizeA);
      SparseMatrix<T> a2 = partition(sizeA, 0);
      SparseMatrix<T> a3 = partition(sizeA, sizeA);

      SparseMatrix<T> b0 = m.partition(0, 0);
      SparseMatrix<T> b1 = m.partition(0, sizeB);
      SparseMatrix<T> b2 = m.partition(sizeB, 0);
      SparseMatrix<T> b3 = m.partition(sizeB, sizeB);

      SparseMatrix<T> r0 = a0.mult_block_seq(b0) + a1.mult_block_seq(b2);
      SparseMatrix<T> r1 = a0.mult_block_seq(b1) + a1.mult_block_seq(b3);
      SparseMatrix<T> r2 = a2.mult_block_seq(b0) + a3.mult_block_seq(b2);
      SparseMatrix<T> r3 = a2.mult_block_seq(b1) + a3.mult_block_seq(b3);

      SparseMatrix<T> result(rows, cols);
      size_t sizeResult = result.getNumRows() / 2;

      r0.rebuild(result, 0, 0);
      r1.rebuild(result, 0, sizeResult);
      r2.rebuild(result, sizeResult, 0);
      r3.rebuild(result, sizeResult, sizeResult);

      return result;
    }
  }

  SparseMatrix<T> diamondConcurrent() const {
    SparseMatrix<T> m2(*this);
    // Check
    assert(cols == m2.getNumRows());
    SparseMatrix<T> result(rows, m2.getNumCols());
    size_t exp = rows - 1;

    auto diamondRow = [&](size_t nRow, const SparseMatrix<T> &m,
                          SparseMatrix<T> &result) {
      auto &row = vals[nRow];
      T oo(numeric_limits<T>::max());

      for (const auto &i : row) {
        const auto &othRow = m(i.first);
        for (const auto &it : othRow) {
          if (result.get(nRow, it.first) == 0) {
            result.set(it.second + i.second, nRow, it.first);
          }
          result.set(min(result.get(nRow, it.first), it.second + i.second),
                     nRow, it.first);
        }
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
    while (exp) {
      if (exp & 1) {
        result = diamond_once(m2);
      }
      m2 = diamond_once(m2);
      exp >>= 1;
    }
    // result = diamond_once(m2);
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
