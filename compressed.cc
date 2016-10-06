#include "lib/ThreadPool.hpp"
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;
using namespace boost::numeric::ublas;

template <typename T> class SparseMatrix {
private:
  int rows;
  int cols;
  compressed_matrix<T> cm;

public:
  SparseMatrix() : rows(0), cols(0) {}
  SparseMatrix(int r, int c) : rows(r), cols(c), cm(r, c) {}

  SparseMatrix(const SparseMatrix<T> &) = default;
  SparseMatrix(SparseMatrix<T> &&) = default;

  SparseMatrix<T> &operator=(const SparseMatrix<T> &other) = default;
  SparseMatrix<T> &operator=(SparseMatrix<T> &&other) = default;

  int getNumCols() const { return cols; }
  int getNumRows() const { return rows; }
  const unbounded_array<T> &getValues() const { return cm.value_data(); }

  T get(int r, int c) const { return cm(r, c); }
  void set(T value, int r, int c) { cm(r, c) = value; }

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

// begin1 -> referencia a las filas
// begin2 -> referencia a las columnas

int main() {
  SparseMatrix<int> m(3, 3);
  // compressed_matrix<double> m(3, 3);
  for (unsigned i = 0; i < m.getNumRows(); ++i)
    for (unsigned j = 0; j < m.getNumCols(); ++j)
      // if (i & 1)
      m.set(j + 1, i, j);

  m.print();
  // unbounded_array<double> a = m.value_data();
  // for (size_t i = 0; i < a.size(); ++i)
  //   cout << a[i] << ' ';
  // cout << '\n';

  // compressed_matrix<double> ans(prod(m, m));
  //
  // print(ans);
  return 0;
}
