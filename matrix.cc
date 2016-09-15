#include <iostream>
#include <queue>
#include <thread>
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

void print(Matrix &m) {
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[i].size(); j++) {
      if (j) cout << " ";
      cout << m[i][j];
    }
    cout << endl;
  }
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
  int n = thread::hardware_concurrency();

  vector<bool> available(n, false);
  vector<thread *> th(n, nullptr);

  for (int i = 0; i < m1[0].size(); i++)
    q.push(i);
  while (!q.empty()) {
    for (int i = 0; i < available.size(); i++) {
      if (!available[i] and !q.empty()) {
        int nCol = q.front();
        q.pop();
        th[i] = new thread(multCol, cref(m1), cref(m2), nCol, ref(result), i, ref(available));
      }
    }
  }

  for (int i = 0; i < th.size(); i++) {
    if (th[i] != nullptr) th[i]->join();
    delete th[i];
  }

  cout << "Matrix:" << endl;
  print(result);
}

void diamondCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result,
                int index, vector<bool> &available, int &cont) {

  available[index] = true;
  vector<int> col = getCol(m2, nCol);

  for (int i = 0; i < m1.size(); i++) {
    int mn = numeric_limits<int>::max();
    for (int j = 0; j < m1[i].size(); j++) {
      mn = min(mn, m1[i][j] + col[j]);
    }
    result[i][nCol] = mn;
  }

  cont++;
  available[index] = false;
}

void diamondConcurrency(const Matrix &m1, Matrix &m2) {
  queue<int> q;
  Matrix result;

  int cont = 0;
  int n = thread::hardware_concurrency();

  vector<bool> available(n, false);
  vector<thread *> th(n, nullptr);

  for (int t = 0; t < m1.size() - 1; t++) {
    for (int i = 0; i < m1[0].size(); i++) q.push(i);
    result.assign(m1.size(), Vector(m1[0].size(), 0));

    while (true) {
      for (int i = 0; i < available.size(); i++) {
        if (!available[i] and !q.empty()) {
          int nCol = q.front(); q.pop();
          th[i] = new thread(diamondCol, cref(m1), cref(m2), nCol, ref(result), i, ref(available), ref(cont));
        }
      }
      if (cont == m1.size()) {
        cont = 0;
        break;
      }
    }

    for (int i = 0; i < th.size(); i++) {
      if (th[i] != nullptr) {
        th[i]->join();
      }
      delete th[i];
    }

    m2 = result;
  }

  cout << "Matrix:" << endl;
  print(result);
}

int main(int argc, char const *argv[]) {
  Matrix m1 = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  Matrix m2 = {{2, 0, 1}, {3, 0, 0}, {5, 1, 1}};
  Matrix m3 = {{1, 0, 1}, {1, 2, 1}, {1, 1, 0}};

  multConcurrency(m2, m3);
  // multConcurrency(m2, m2);
  Matrix mc = m1;
  diamondConcurrency(m1, mc);
  // diamondConcurrency(m1, m3);

  // m2 X m3 = [[3 1 2]]
  //            [3 0 3]
  //            [7 3 6]
  //
  // m1 <> m1 (n -1 veces) =
  //            [[3 4 5]]
  //             [6 7 8]
  //             [9 10 11]

  return 0;
}
