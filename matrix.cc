#include <iostream>
#include <thread>
#include <vector>

using namespace std;

using Matrix = vector<vector<int>>;
using Vector = vector<int>;

void multCol(const Matrix &m, const Vector &col, Vector &result) {
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[i].size(); j++) {
      result[i] += m[i][j] * col[j];
    }
  }
}

void diamondCol(const Matrix &m, const Vector &col, Vector &result) {
  for (int i = 0; i < m.size(); i++) {
    int mn = numeric_limits<int>::max();
    for (int j = 0; j < m[i].size(); j++) {
      mn = min(mn, m[i][j] + col[j]);
    }
    result[i] = mn;
  }
}

Vector getCol(Matrix &m, int numCol) {
  Vector col(m.size());
  for (int i = 0; i < m.size(); i++) {
    col[i] = m[i][numCol];
  }
  return col;
}

int main(int argc, char const *argv[]) {
  Matrix m1 = {{2, 0, 1}, {3, 0, 0}, {5, 1, 1}};
  Matrix m2 = {{1, 0, 1}, {1, 2, 1}, {1, 1, 0}};
  // m1 X m2 = [[3 1 2]]
  //            [3 0 3]
  //            [7 3 6]
  //
  // m1 ^ m2 = [[1 2 1]]
  //            [1 1 0]
  //            [2 2 1]

  // Multiplicacion
  Vector col1 = getCol(m2, 0);
  Vector col2 = getCol(m2, 1);
  Vector col3 = getCol(m2, 2);

  Vector mCol1(m1.size(), 0);
  Vector mCol2(m1.size(), 0);
  Vector mCol3(m1.size(), 0);

  thread t1(multCol, cref(m1), cref(col1), ref(mCol1));
  thread t2(multCol, cref(m1), cref(col2), ref(mCol2));
  thread t3(multCol, cref(m1), cref(col3), ref(mCol3));

  t1.join();
  t2.join();
  t3.join();

  cout << "Mult Matrix: " << endl;
  for (int i = 0; i < mCol1.size(); i++) {
    cout << mCol1[i] << " " << mCol2[i] << " " << mCol3[i] << endl;
  }
  cout << endl;

  // Operador diamante
  Vector dCol1(m1.size(), 0);
  Vector dCol2(m1.size(), 0);
  Vector dCol3(m1.size(), 0);

  thread t4(diamondCol, cref(m1), cref(col1), ref(dCol1));
  thread t5(diamondCol, cref(m1), cref(col2), ref(dCol2));
  thread t6(diamondCol, cref(m1), cref(col3), ref(dCol3));

  t4.join();
  t5.join();
  t6.join();

  cout << "Diamond Operator: " << endl;
  for (int i = 0; i < dCol1.size(); i++) {
    cout << dCol1[i] << " " << dCol2[i] << " " << dCol3[i] << endl;
  }
  cout << endl;

  return 0;
}
