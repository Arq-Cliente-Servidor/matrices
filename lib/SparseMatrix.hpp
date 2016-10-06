#pragma once

#include "ThreadPool.hpp"
#include <cassert>
#include <cmath>
#include <vector>

using namespace std;

template <typename T> class SparseMatrix {
private:
  int rows;
  int cols;
  vector<T> val;
  vector<int> colInd;
  vector<int> rowPtr;

public:
  SparseMatrix() : rows(0), cols(0), rowPtr(rows + 1, T(0)) {}
  SparseMatrix(int r, int c) : rows(r), cols(c), rowPtr(r + 1, T(0)) {}
  SparseMatrix(const SparseMatrix<T> &) = default;
  SparseMatrix(SparseMatrix<T> &&) = default;

  SparseMatrix<T> &operator=(const SparseMatrix<T> &other) = default;
  SparseMatrix<T> &operator=(SparseMatrix<T> &&other) = default;

  int getNumCols() const { return cols; }
  int getNumRows() const { return rows; }

  const vector<int> &getColInd() const { return colInd; }
  const vector<int> &getRowPtr() const { return rowPtr; }
  const vector<T> &getVal() const { return val; }

  T get(int r, int c) const {
    if ((r < 0 || c < 0) || (r >= rows || c >= cols) || (r > rowPtr.size() - 1))
      return T(0);
    for (int i = rowPtr[r]; i < rowPtr[r + 1]; i++) {
      if (colInd[i] == c)
        return val[i];
    }
    return T(0);
  }

  // Version añadiendo elementos en desorden
  void set(T value, int r, int c) {
    // Asigna value al elemento en la posición (r,c)
    if ((r > rows || c > cols) || (r < 0 || c < 0)) {
      cout << "row or col invalid "
           << "[" << r << ", " << c << "]" << endl;
      return;
    }
    int init = rowPtr[r];
    int finish = rowPtr[r + 1];
    int index = finish;
    for (int i = init; i < finish; i++) {
      if (c <= colInd[i]) {
        index = i;
        break;
      }
    }
    bool exist_elem = false;
    if (finish - init > 0 && index != rowPtr.back() && colInd[index] == c) {
      exist_elem = true;
    }

    auto valCurr = val.begin() + index;
    auto colIndCurr = colInd.begin() + index;

    if (!exist_elem) {
      if (value != T(0)) {
        val.insert(valCurr, value);
        colInd.insert(colIndCurr, c);
        for (int i = r + 1; i < rowPtr.size(); i++) {
          rowPtr[i] = rowPtr[i] + 1;
        }
      }
    } else {
      if (value == T(0)) {
        val.erase(valCurr);
        colInd.erase(colIndCurr);
        for (int i = r + 1; i < rowPtr.size(); i++) {
          rowPtr[i] = rowPtr[i] - 1;
        }
      } else {
        *valCurr = value;
      }
    }
  }
  // Imprimir la matriz dispersa como una matriz normal
  void print(int rw = 0, int cl = 0) const {
    int r = (rw) ? rw : rows;
    int c = (cl) ? cl : cols;
    for (int i = 0; i < r; i++) {
      for (int j = 0; j < c; j++) {
        if (j)
          cout << " ";
        cout << get(i, j);
      }
      cout << endl;
    }
  }

  SparseMatrix<T> mult(const SparseMatrix<T> &b) {
    assert(cols == b.getNumRows());
    SparseMatrix<T> result(rows, b.getNumCols());

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < b.getNumCols(); j++) {
        T accum = T(0);
        for (int k = rowPtr[i]; k < rowPtr[i + 1]; k++) {
          accum += val[k] * b.get(colInd[k], j);
        }
        if (accum == T(0))
          continue;
        result.set(accum, i, j);
      }
    }
    return result;
  }

  SparseMatrix<T> diamond() const {
    SparseMatrix<T> b(*this);

    assert(cols == b.getNumRows());
    SparseMatrix<T> result(rows, b.getNumCols());

    for (int it = 0; it < rows - 1; it++) {
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < b.getNumCols(); j++) {
          T mn = numeric_limits<T>::max();
          for (int k = rowPtr[i]; k < rowPtr[i + 1]; k++) {
            mn = min(mn, val[k] + b.get(colInd[k], j));
          }
          result.set(mn, i, j);
        }
      }

      b = result;
    }
    return result;
  }

  SparseMatrix<T> diamondSeq(const SparseMatrix<T> &b) const {

    assert(cols == b.getNumRows());
    SparseMatrix<T> result(rows, b.getNumCols());

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < b.getNumCols(); j++) {
        T mn = numeric_limits<T>::max();
        for (int k = rowPtr[i]; k < rowPtr[i + 1]; k++) {
          mn = min(mn, val[k] + b.get(colInd[k], j));
        }
        result.set(mn, i, j);
      }
    }
    return result;
  }

  SparseMatrix<T> partition(int offsetI, int offsetJ) const {
    SparseMatrix<T> p(getNumRows() / 2, getNumCols() / 2);
    for (int i = 0; i < getNumRows() / 2; i++) {
      for (int j = 0; j < getNumCols() / 2; j++) {
        p.set(get(i + offsetI, j + offsetJ), i, j);
      }
    }

    return p;
  }

  void rebuild(SparseMatrix<T> &result, int offsetI, int offsetJ) {
    for (int i = 0; i < getNumRows(); i++) {
      for (int j = 0; j < getNumCols(); j++) {
        result.set(get(i, j), i + offsetI, j + offsetJ);
      }
    }
  }

  SparseMatrix<T> check() const {
    int nSize = pow(2, ceil(log2(double(getNumRows()))));
    if (getNumRows() == nSize)
      return *this;

    SparseMatrix<T> newM(nSize, nSize);
    for (int i = 0; i < getNumRows(); i++) {
      for (int j = 0; j < getNumCols(); j++) {
        newM.set(get(i, j), i, j);
      }
    }
    return newM;
  }

  void compare(const SparseMatrix<T> &b) {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (get(i, j) != b.get(i, j))
          return false;
      }
    }
    return true;
  }

  SparseMatrix<T> multConcurrency(const SparseMatrix<T> &m2) const {

    // Check
    assert(cols == m2.getNumRows());

    thread_pool *pool = new thread_pool();
    vector<SparseMatrix<T>> results(rows, {1, m2.getNumCols()});
    SparseMatrix<T> result(rows, m2.getNumCols());

    auto multCol = [&](int nRow) {
      SparseMatrix<T> &row = results[nRow];
      for (int i = 0; i < m2.getNumCols(); i++) {
        T accum(0);
        for (int j = rowPtr[nRow]; j < rowPtr[nRow + 1]; j++) {
          accum += val[j] * m2.get(colInd[j], i);
        }
        if (accum == T(0))
          continue;
        row.set(accum, 0, i);
      }
    };

    for (int i = 0; i < rows; i++) {
      auto func = [&multCol, i] { multCol(i); };
      pool->submit(func);
    }

    delete pool;

    for (int i = 0; i < results.size(); i++) {
      for (int j = 0; j < results[i].getNumCols(); j++) {
        result.set(results[i].get(0, j), i, j);
      }
    }

    return result;
  }

  SparseMatrix<T> diamondConcurrency() const {
    SparseMatrix<T> m2(*this);
    assert(cols == m2.getNumRows());

    vector<SparseMatrix<T>> results(rows, {1, m2.getNumCols()});
    SparseMatrix<T> result(rows, m2.getNumCols());
    thread_pool *pool;
    // int exp = rows - 1;

    auto diamondCol = [&](int nRow) {
      for (int i = 0; i < cols; i++) {
        T mn = std::numeric_limits<T>::max();
        for (int j = rowPtr[nRow]; j < rowPtr[nRow + 1]; j++) {
          mn = std::min(mn, val[j] + m2.get(colInd[j], i));
        }
        results[nRow].set(mn, 0, i);
      }
    };

    // for (int j = 0; j < rows - 1; j++) {
    for (int j = 0; j < 1; j++) {
      pool = new thread_pool();
      for (int i = 0; i < rows; i++) {
        auto func = [&diamondCol, i] { diamondCol(i); };
        pool->submit(func);
      }

      delete pool;

      for (int i = 0; i < results.size(); i++) {
        for (int j = 0; j < results[i].getNumCols(); j++) {
          result.set(results[i].get(0, j), i, j);
        }
      }

      m2 = result;
    }

    return result;
  }

  SparseMatrix<T> minMatrix(const SparseMatrix<T> &b) const {
    SparseMatrix<T> result(rows, cols);
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        result.set(min(get(i, j), b.get(i, j)), i, j);
      }
    }
    return result;
  }

  SparseMatrix<T> diamond_block_seq(SparseMatrix<T> &m) const {
    if (rows == 2) {
      return diamondSeq(m);
    } else {
      int sizeA = rows / 2;
      int sizeB = m.getNumRows() / 2;

      SparseMatrix<T> a0 = partition(0, 0);
      SparseMatrix<T> a1 = partition(0, sizeA);
      SparseMatrix<T> a2 = partition(sizeA, 0);
      SparseMatrix<T> a3 = partition(sizeA, sizeA);

      SparseMatrix<T> b0 = m.partition(0, 0);
      SparseMatrix<T> b1 = m.partition(0, sizeB);
      SparseMatrix<T> b2 = m.partition(sizeB, 0);
      SparseMatrix<T> b3 = m.partition(sizeB, sizeB);

      SparseMatrix<T> r0 =
          (a0.diamond_block_seq(b0)).minMatrix(a1.diamond_block_seq(b2));
      SparseMatrix<T> r1 =
          (a0.diamond_block_seq(b1)).minMatrix(a1.diamond_block_seq(b3));
      SparseMatrix<T> r2 =
          (a2.diamond_block_seq(b0)).minMatrix(a3.diamond_block_seq(b2));
      SparseMatrix<T> r3 =
          (a2.diamond_block_seq(b1)).minMatrix(a3.diamond_block_seq(b3));

      SparseMatrix<T> result(rows, cols);
      int sizeResult = result.getNumRows() / 2;

      r0.rebuild(result, 0, 0);
      r1.rebuild(result, 0, sizeResult);
      r2.rebuild(result, sizeResult, 0);
      r3.rebuild(result, sizeResult, sizeResult);

      return result;
    }
  }

  SparseMatrix<T> diamond_block_seq_complete() const {
    // this = check();
    SparseMatrix<T> m2(*this);
    SparseMatrix<T> result(rows, cols);
    int exp = rows - 2;

    while (exp) {
      if (exp & 1) {
        result = diamond_block_seq(m2);
      }
      m2 = diamond_block_seq(m2);
      exp >>= 1;
    }

    return result;
  }
};
