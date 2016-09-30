#pragma once

#include "ThreadPool.hpp"
#include <cassert>
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
      return 0;
    int index = rowPtr[r];
    int endRow = rowPtr[r + 1] - index;
    for (int i = 0; i < endRow; i++) {
      if (colInd[index + i] == c)
        return val[index + i];
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
  void print() {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
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
}

template <typename T>
SparseMatrix<T> multConcurrency(const SparseMatrix<T> &m1,
                                const SparseMatrix<T> &m2) {
  assert(m1.getNumCols() == m2.getNumRows());
  vector<SparseMatrix<T>> results(m1.getNumRows(), {1, m2.getNumCols()});
  SparseMatrix<T> result(m1.getNumRows(), m2.getNumCols());
  {
    thread_pool pool;
    for (int i = 0; i < m1.getNumRows(); i++) {
      auto func = [&m1, &m2, i, &results]() { multCol(m1, m2, i, results); };
      pool.submit(func);
    }
  }

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
  SparseMatrix<T> result(m1.getNumRows(), m2.getNumCols());

  for (int j = 0; j < m1.getNumRows() - 1; j++) {
    vector<SparseMatrix<T>> results(m1.getNumRows(), {1, m2.getNumCols()});
    {
      thread_pool pool;
      for (int i = 0; i < m1.getNumRows(); i++) {
        auto func = [&m1, &m2, i, &results]() {
          diamondCol(m1, m2, i, results);
        };
        pool.submit(func);
      }
    }

    for (int i = 0; i < results.size(); i++) {
      for (int j = 0; j < results[i].getNumCols(); j++) {
        result.set(results[i].get(0, j), i, j);
      }
    }

    m2 = result;
  }
  return result;
}
