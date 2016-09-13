#include <iostream>
#include <thread>
#include <queue>
#include <vector>

using namespace std;

using Matrix = vector<vector<int>>;
using Vector = vector<int>;


Vector getCol(const Matrix &m, int numCol) {
  Vector col(m.size());
  for (int i = 0; i < m.size(); i++) {
    col[i] = m[i][numCol];
  }
  return col;
}

void diamondCol(const Matrix &m1, const Matrix &m2, int nCol,
                Matrix &result, int index, vector<bool> &available) {

  available[index] = true;
  vector<int> col = getCol(m2, nCol);
  for (int i = 0; i < m1.size(); i++) {
    int mn = numeric_limits<int>::max();
    for (int j = 0; j < m1[i].size(); j++) {
      mn = min(mn, m1[i][j] + col[j]);
    }
    result[i][nCol] = mn;
  }

  available[index] = false;
}

void multCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result,
             int index, vector<bool> &available) {

  available[index] = true;
  vector<int> col = getCol(m2, nCol);
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1[i].size(); j++) {
      result[i][nCol] += m1[i][j] * col[j];
    }
  }
  available[index] = false;
}

void multConcurrency(const Matrix &m1, const Matrix &m2) {
  if (m1[0].size() != m2.size()) {
    cerr << "It is not possible matrix multiplication!" << endl;
    return;
  }

  queue<int> q;
  Matrix result(m1.size(), Vector(m2[0].size(), 0));
  vector<bool> available(4, false);
  vector<thread*> th(4, nullptr);
  for (int i = 0; i < m1.size(); i++) q.push(i);

  while (!q.empty()) {
    if (!available[0] and !q.empty()) {
      int nCol = q.front(); q.pop();
      th[0] = new thread(multCol, cref(m1), cref(m2), nCol, ref(result), 0, ref(available));
    }
    if (!available[1] and !q.empty()) {
      int nCol = q.front(); q.pop();
      th[1] = new thread(multCol, cref(m1), cref(m2), nCol, ref(result), 1, ref(available));
    }
    if (!available[2] and !q.empty()) {
      int nCol = q.front(); q.pop();
      th[2] = new thread(multCol, cref(m1), cref(m2), nCol, ref(result), 2, ref(available));
    }
    if (!available[3] and !q.empty()) {
      int nCol = q.front(); q.pop();
      th[3] = new thread(multCol, cref(m1), cref(m2), nCol, ref(result), 3, ref(available));
    }
  }

  for (int i = 0; i < th.size(); i++) {
    if (th[i] != nullptr) th[i]->join();
  }

  cout << "Matrix:" << endl;
  for (int i = 0; i < result.size(); i++) {
    for (int j = 0; j < result[i].size(); j++) {
      if (j) cout << " ";
      cout << result[i][j];
    }
    cout << endl;
  }
}

void diamondConcurrency(const Matrix &m1, const Matrix &m2) {
  queue<int> q;
  Matrix result(m1.size(), Vector(m1[0].size(), 0));
  vector<bool> available(4, false);
  vector<thread*> th(4, nullptr);
  for (int i = 0; i < m1.size(); i++) q.push(i);

  for (int i = 0; i < 1; i++) {
    while (!q.empty()) {
      if (!available[0] and !q.empty()) {
        int nCol = q.front(); q.pop();
        th[0] = new thread(diamondCol, cref(m1), cref(m2), nCol, ref(result), 0, ref(available));
      }
      if (!available[1] and !q.empty()) {
        int nCol = q.front(); q.pop();
        th[1] = new thread(diamondCol, cref(m1), cref(m2), nCol, ref(result), 1, ref(available));
      }
      if (!available[2] and !q.empty()) {
        int nCol = q.front(); q.pop();
        th[2] = new thread(diamondCol, cref(m1), cref(m2), nCol, ref(result), 2, ref(available));
      }
      if (!available[3] and !q.empty()) {
        int nCol = q.front(); q.pop();
        th[3] = new thread(diamondCol, cref(m1), cref(m2), nCol, ref(result), 3, ref(available));
      }
    }
  }

  for (int i = 0; i < th.size(); i++) {
    if (th[i] != nullptr) th[i]->join();
  }

  cout << "Matrix:" << endl;
  for (int i = 0; i < result.size(); i++) {
    for (int j = 0; j < result[i].size(); j++) {
      if (j) cout << " ";
      cout << result[i][j];
    }
    cout << endl;
  }
  cout << endl;
}

int main(int argc, char const *argv[]) {
  Matrix m1 = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  Matrix m2 = {{2, 0, 1 ,4}, {3, 0, 0, 0}, {5, 1, 1, 1}, {4, 5, 6, 6}};
  Matrix m3 = {{2, 3, 4}, {5, 6, 7}, {8, 9, 10}};

  // multConcurrency(m1, m1);
  // multConcurrency(m2, m2);
  diamondConcurrency(m1, m1);
  diamondConcurrency(m1, m3);

  // m1 X m2 = [[3 1 2]]
  //            [3 0 3]
  //            [7 3 6]
  //
  // m1 ^ m2 = [[1 2 1]]
  //            [1 1 0]
  //            [2 2 1]

  // Multiplicacion
  // Vector col1 = getCol(m1, 0);
  // Vector col2 = getCol(m1, 1);
  // Vector col3 = getCol(m1, 2);
  //
  // Matrix result(m1.size(), Vector(m1[0].size(), 0));
  //
  // thread t1(multCol, cref(m1), cref(m1), 0, ref(result));
  // thread t2(multCol, cref(m1), cref(m1), 1, ref(result));
  // thread t3(multCol, cref(m1), cref(m1), 2, ref(result));
  //
  // t1.join();
  // t2.join();
  // t3.join();
  //
  // cout << "Mult Matrix: " << endl;
  // for (int i = 0; i < result.size(); i++) {
  //   cout << result[i][0] << " " << result[i][1] << " " << result[i][2] << endl;
  // }
  // cout << endl;

  // Operador diamante
  // Vector dCol1(m1.size(), 0);
  // Vector dCol2(m1.size(), 0);
  // Vector dCol3(m1.size(), 0);
  //
  // thread t4(diamondCol, cref(m1), cref(col1), ref(dCol1));
  // thread t5(diamondCol, cref(m1), cref(col2), ref(dCol2));
  // thread t6(diamondCol, cref(m1), cref(col3), ref(dCol3));
  //
  // t4.join();
  // t5.join();
  // t6.join();
  //
  // cout << "Diamond Operator: " << endl;
  // for (int i = 0; i < dCol1.size(); i++) {
  //   cout << dCol1[i] << " " << dCol2[i] << " " << dCol3[i] << endl;
  // }
  // cout << endl;

  return 0;
}
