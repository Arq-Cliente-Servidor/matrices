#pragma once

#include "ThreadPool.hpp"
#include <armadillo>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

template <typename T> class SparseMatrix {
private:
  int rows;
  int cols;
  arma::SpMat<T> sp;

public:
  SparseMatrix() : rows(0), cols(0) {}
  SparseMatrix(int r, int c) : rows(r), cols(c), sp(r, c) {}
  SparseMatrix(const arma::SpMat<T> &m)
      : rows(m.n_rows), cols(m.n_cols), sp(m) {}
  SparseMatrix(const arma::umat &locations, const arma::vec &vals, int r, int c)
      : rows(r), cols(c), sp(locations, vals, r, c) {}

  SparseMatrix(const SparseMatrix<T> &) = default;
  SparseMatrix(SparseMatrix<T> &&) = default;

  SparseMatrix<T> &operator=(const SparseMatrix<T> &other) = default;
  SparseMatrix<T> &operator=(SparseMatrix<T> &&other) = default;

  int getNumCols() const { return cols; }
  int getNumRows() const { return rows; }
  const arma::SpMat<T> &getSP() const { return sp; }

  T get(int r, int c) const { return sp(r, c); }

  arma::SpSubview<T> getRow(int nRow) const { return sp.row(nRow); }

  void set(T value, int r, int c) { sp(r, c) = value; }
  void setData(const std::vector<T> &data) {
    if (data.size() != rows * cols)
      return;
    for (int row = 0; row < rows; row++) {
      for (int col = 0; col < cols; col++) {
        set(data[row * cols + col], row, col);
      }
    }
  }

  SparseMatrix<T> mult(const SparseMatrix<T> &b) const {
    assert(cols == b.getNumRows());
    arma::SpMat<T> m = sp * b.getSP();
    return SparseMatrix<T>(m);
  }

  SparseMatrix<T> diamondSeq() const {
    SparseMatrix<T> b(*this);

    assert(cols == b.getNumRows());
    SparseMatrix<T> result(rows, b.getNumCols());
    int exp = rows - 1;

    auto diamond_it = [&](SparseMatrix<T> m) {
      SparseMatrix<T> result(rows, m.getNumCols());
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < m.getNumCols(); j++) {
          T mn = numeric_limits<T>::max();
          for (int k = 0; k < cols; k++) {
            mn = min(mn, get(i, k) + m.get(k, j));
          }
          if (mn == T(0))
            continue;
          result.set(mn, i, j);
        }
      }
      return result;
    };

    while (exp) {
      if (exp & 1) {
        result = diamond_it(b);
      }
      b = diamond_it(b);
      exp >>= 1;
    }

    return result;
  }

  SparseMatrix<T> multConcurrency(const SparseMatrix<T> &m2) const {

    // Check
    assert(cols == m2.getNumRows());

    thread_pool *pool = new thread_pool();
    vector<arma::SpMat<T>> results(rows, arma::SpMat<T>(1, cols));
    // results.reserve(rows);
    SparseMatrix<T> result(rows, m2.getNumCols());

    auto multCol = [&](int nRow) {
      arma::SpMat<T> r = m2.getRow(nRow);
      auto &rowVal = results[nRow];
      // rowVal = arma::SpMat<T>(1, cols);

      // Solucion solo para matrices cuadradas
      for (auto it = sp.begin(); it != sp.end(); ++it) {
        T accum(0);
        for (auto rw = r.begin(); rw != r.end(); ++rw) {
          accum += *rw * sp(rw.col(), it.col());

          if (accum == T(0))
            continue;
          rowVal(0, it.col()) = accum;
        }
      }
      // cerr << rowVal.row(0) << endl;
    };

    for (int i = 0; i < 1; i++) {
      auto func = [&multCol, i] { multCol(i); };
      pool->submit(func);
    }

    delete pool;

    for (int i = 0; i < results.size(); i++) {
      // cout << results[i] << endl;
      for (auto it = results[i].begin(); it != results[i].end(); ++it) {
        result.set(*it, i, it.col());
      }
      // cout << results[i].n_rows << " " << results[i].n_cols << endl;
      // result.set(results[i](j), i, j);
    }
    // cout << endl;

    return result;
  }

  // TODO partition, rebuild, bloq_concurrent, diamond concurrent

  void print() const {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (j)
          cout << " ";
        cout << get(i, j);
      }
      cout << endl;
    }
  }
};

template <typename T>
arma::SpMat<T> multConcurrency(const arma::SpMat<T> &m1,
                               const arma::SpMat<T> &m2) {

  // Check
  assert(m1.n_cols == m2.n_rows);

  thread_pool *pool = new thread_pool();
  vector<arma::SpMat<T>> results(m1.n_rows, arma::SpMat<T>(1, m1.n_cols));
  // results.reserve(rows);
  arma::SpMat<T> result(m1.n_rows, m2.n_cols);

  auto multCol = [&](int nRow) {
    // arma::SpMat<T> r = m2.row(nRow);
    auto &rowVal = results[nRow];
    // rowVal = arma::SpMat<T>(1, cols);

    // Solucion solo para matrices cuadradas
    for (int i = 0; i < m1.n_rows; i++) {
      T accum(0);
      for (int j = 0; j < m1.n_cols; j++) {
        accum += m1(j, i) * m2(nRow, j);
      }
      if (accum == T(0))
        continue;
      rowVal(0, i) = accum;
    }
    // for (auto it = m1.begin(); it != m1.end(); ++it) {
    //   T accum(0);
    //   for (auto rw = m2.begin_row(nRow); rw != m2.end_row(nRow); ++rw) {
    //     accum += *rw * m1(rw.col(), it.col());
    //
    //     if (accum == T(0))
    //       continue;
    //     rowVal(0, it.col()) = accum;
    //   }
    // }
    // cerr << rowVal.row(0) << endl;
  };

  for (int i = 0; i < m1.n_rows; i++) {
    auto func = [&multCol, i] { multCol(i); };
    pool->submit(func);
  }

  delete pool;

  for (int i = 0; i < results.size(); i++) {
    // cout << results[i] << endl;
    for (auto it = results[i].begin(); it != results[i].end(); ++it) {
      result(i, it.col()) = *it;
    }
    // cout << results[i].n_rows << " " << results[i].n_cols << endl;
    // result.set(results[i](j), i, j);
  }
  // cout << endl;

  return result;
}
