#include "lib/SparseMatrix.hpp"
#include <cmath>
#include <iostream>

using namespace std;

int main() {
  SparseMatrix<int> m(3, 3);
  m.setData({1, 2, 3, 4, 5, 6, 7, 8, 9});
  // m.setData({0, 2, 0, 2, 0, 6, 0, 6, 0});

  SparseMatrix<int> m2(3, 3);
  m2.setData({2, 3, 8, 5, 6, 11, 8, 9, 14});
  // m2.setData({2, 3, 4, 5, 6, 7, 8, 9, 10});

  // size_t nSize = pow(2, ceil(log2(double(m.getNumRows()))));
  // if (m.getNumRows() != nSize) {
  //   m.resize(nSize, nSize);
  //   m2.resize(nSize, nSize);
  // }
  SparseMatrix<int> r = m.diamondConcurrent(); // m.mult_block_seq(m);
  // SparseMatrix<int> r2 = m.multConcurrent(m);
  cout << r << endl;
  // cout << r << endl;
  return 0;
}
