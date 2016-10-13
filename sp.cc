#include "lib/SparseMatrix.hpp"
#include <cmath>
#include <iostream>

using namespace std;

int main() {
  SparseMatrix<int> m(3, 3);
  m.setData({1, 2, 3, 4, 5, 6, 7, 8, 9});
  size_t nSize = pow(2, ceil(log2(double(m.getNumRows()))));
  if (m.getNumRows() != nSize) {
    m.resize(nSize, nSize);
  }
  SparseMatrix<int> r = m.mult_block_seq(m);
  cout << m * m << endl;
  cout << r << endl;

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
