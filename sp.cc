#include "lib/SparseMatrix.hpp"
#include <cmath>
#include <iostream>

using namespace std;

int main() {
  SparseMatrix<int> m(3, 3);
  m.setData({1, 2, 0, 4, 5, 6, 7, 8, 9});
  // m.setData({0, 2, 0, 2, 0, 6, 0, 6, 0});

  SparseMatrix<int> m2(3, 3);
  m2.setData({2, 3, 8, 5, 6, 11, 8, 9, 14});
  // m2.setData({2, 3, 4, 5, 6, 7, 8, 9, 10});

  // size_t nSize = pow(2, ceil(log2(double(m.getNumRows()))));
  // if (m.getNumRows() != nSize) {
  //   m.resize(nSize, nSize);
  //   m2.resize(nSize, nSize);
  // }
  SparseMatrix<int> r = m * m; // m.mult_block_seq(m);
  SparseMatrix<int> r2 = m.multConcurrent(m);
  cout << r.compare(r2) << endl;
  // cout << r << endl;

  // [2, 3, 1]
  // [5, 6, 4]
  // [8, 9, 7],
  // SparseMatrix<int> r0 = m.partition(0, 0); // m.diamondConcurrent();
  // SparseMatrix<int> r1 = m.partition(0, 2);
  // SparseMatrix<int> r2 = m.partition(2, 0);
  // SparseMatrix<int> r3 = m.partition(2, 2);
  // cout << r0 << endl;
  // cout << r1 << endl;
  // cout << r2 << endl;
  // cout << r3 << endl;
  // SparseMatrix<int> r(4, 4);
  // r0.rebuild(r, 0, 0);
  // r1.rebuild(r, 0, 2);
  // r2.rebuild(r, 2, 0);
  // r3.rebuild(r, 2, 2);
  // auto row = m(0);
  // for (auto it : row) {
  //   cout << it.second << " ";
  // }
  // cout << endl;
  // cout << r << endl;
  // cout << m << endl;
  return 0;
}
