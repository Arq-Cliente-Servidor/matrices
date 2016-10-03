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
  // Copy Constructor
  SparseMatrix<T> &operator=(const SparseMatrix<T> &other) {
    rows = other.getNumRows();
    cols = other.getNumCols();
    rowPtr = other.getRowPtr();
    val = other.getVal();
    return *this;
  }

  int getNumCols() const { return cols; }

  int getNumRows() const { return rows; }

  vector<int> getColInd() const { return colInd; }

  vector<int> getRowPtr() const { return rowPtr; }

  vector<T> getVal() const { return val; }

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
    int r = (rw)? rw : rows;
    int c = (cl)? cl : cols;
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
        result.set(accum, i, j);
      }
    }
    return result;
  }

  SparseMatrix<T> diamond(const SparseMatrix<T> &b) const {
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

  SparseMatrix<T> check() {
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
};

template <typename T>
void multCol(const SparseMatrix<T> &m1, const SparseMatrix<T> &m2, int nRow,
             vector<SparseMatrix<T>> &results) {

  for (int i = 0; i < m2.getNumCols(); i++) {
    T accum = T(0);
    for (int j = m1.getRowPtr()[i]; j < m1.getRowPtr()[i + 1]; j++) {
      accum += m1.get(nRow, m1.getColInd()[j]) * m2.get(m1.getColInd()[j], i);
    }
    results[nRow].set(accum, 0, i);
  }
  // cout << "Result:" << endl;
  // results[nRow].print();
}

template <typename T>
SparseMatrix<T> multConcurrency(const SparseMatrix<T> &m1,
                                const SparseMatrix<T> &m2) {

  // Check
  assert(m1.getNumCols() == m2.getNumRows());

  thread_pool *pool = new thread_pool();
  vector<SparseMatrix<T>> results(m1.getNumRows(), {1, m2.getNumCols()});
  SparseMatrix<T> result(m1.getNumRows(), m2.getNumCols());

  for (int i = 0; i < m1.getNumRows(); i++) {
    auto func = [&m1, &m2, i, &results]() { multCol(m1, m2, i, results); };
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

template <typename T>
void diamondCol(const SparseMatrix<T> &m1, const SparseMatrix<T> &m2, int nRow,
                vector<SparseMatrix<T>> &results) {

  for (int i = 0; i < m1.getNumCols(); i++) {
    T mn = std::numeric_limits<T>::max();
    for (int j = m1.getRowPtr()[i]; j < m1.getRowPtr()[i + 1]; j++) {
      mn = std::min(mn, m1.get(nRow, m1.getColInd()[j]) +
                            m2.get(m2.getColInd()[j], i));
    }
    results[nRow].set(mn, 0, i);
  }
}

template <typename T>
SparseMatrix<T> diamondConcurrency(const SparseMatrix<T> &m1) {
  SparseMatrix<T> m2(m1);
  assert(m1.getNumCols() == m2.getNumRows());

  vector<SparseMatrix<T>> results(m1.getNumRows(), {1, m2.getNumCols()});
  SparseMatrix<T> result(m1.getNumRows(), m2.getNumCols());
  thread_pool *pool;

  for (int j = 0; j < m1.getNumRows() - 1; j++) {
    pool = new thread_pool();
    for (int i = 0; i < m1.getNumRows(); i++) {
      auto func = [&m1, &m2, i, &results]() { diamondCol(m1, m2, i, results); };
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

template <typename T>
SparseMatrix<T> minMatrix(const SparseMatrix<T> &a,
                          const SparseMatrix<T> &b) {
  SparseMatrix<T> result(a.getNumRows(), a.getNumCols());
  for (int i = 0; i < a.getNumRows(); i++) {
    for (int j = 0; j < a.getNumCols(); j++) {
      result.set(min(a.get(i, j), b.get(i, j)), i, j);
    }
  }
  return result;
}

template <typename T>
SparseMatrix<T> diamond_block_seq(const SparseMatrix<T> &A,
                                  const SparseMatrix<T> &B) {
  if (A.getNumRows() == 2) {
    return A.diamond(B);
  } else {
    int sizeA = A.getNumRows() / 2;
    int sizeB = B.getNumRows() / 2;

    SparseMatrix<T> a0 = A.partition(0, 0);
    SparseMatrix<T> a1 = A.partition(0, sizeA);
    SparseMatrix<T> a2 = A.partition(sizeA, 0);
    SparseMatrix<T> a3 = A.partition(sizeA, sizeA);

    SparseMatrix<T> b0 = B.partition(0, 0);
    SparseMatrix<T> b1 = B.partition(0, sizeB);
    SparseMatrix<T> b2 = B.partition(sizeB, 0);
    SparseMatrix<T> b3 = B.partition(sizeB, sizeB);

    SparseMatrix<T> r0 =
        minMatrix(diamond_block_seq(a0, b0), diamond_block_seq(a1, b2));
    SparseMatrix<T> r1 =
        minMatrix(diamond_block_seq(a0, b1), diamond_block_seq(a1, b3));
    SparseMatrix<T> r2 =
        minMatrix(diamond_block_seq(a2, b0), diamond_block_seq(a3, b2));
    SparseMatrix<T> r3 =
        minMatrix(diamond_block_seq(a2, b1), diamond_block_seq(a3, b3));

    SparseMatrix<T> result(A.getNumRows(), A.getNumCols());
    int sizeResult = result.getNumRows() / 2;

    r0.rebuild(result, 0, 0);
    r1.rebuild(result, 0, sizeResult);
    r2.rebuild(result, sizeResult, 0);
    r3.rebuild(result, sizeResult, sizeResult);

    return result;
  }
}
