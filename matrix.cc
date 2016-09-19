#include <chrono>
#include <fstream>
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
      if (j)
        cout << " ";
      cout << m[i][j];
    }
    cout << endl;
  }
}

void multCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result,
             int index, vector<bool> &available, int &cont) {

  available[index] = true;
  vector<int> col = getCol(m2, nCol);
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < col.size(); j++) {
      result[i][nCol] += m1[i][j] * col[j];
    }
  }
  available[index] = false;
  cont++;
}

void multConcurrency(const Matrix &m1, const Matrix &m2) {
  if (m1[0].size() != m2.size()) {
    cerr << "It is not possible matrix multiplication!" << endl;
    return;
  }

  queue<int> q;
  int cont = 0;
  Matrix result(m1.size(), Vector(m2[0].size(), 0));
  int n = thread::hardware_concurrency();

  vector<bool> available(n, false);
  vector<thread *> th(n);

  for (int i = 0; i < m1[0].size(); i++)
    q.push(i);
  while (true) {
    for (int i = 0; i < available.size(); i++) {
      if (!available[i] and !q.empty()) {
        int nCol = q.front();
        q.pop();
        th[i] = new thread(multCol, cref(m1), cref(m2), nCol, ref(result), i,
                           ref(available), ref(cont));
      }
    }
    if (cont == m2[0].size())
      break;
  }

  for (int i = 0; i < th.size(); i++) {
    if (th[i]->joinable())
      th[i]->join();
    delete th[i];
  }

  // cout << "Matrix:" << endl;
  // print(result);
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

void diamondConcurrency(const Matrix &m1) {
  queue<int> q;
  Matrix m2 = m1;
  Matrix result;

  int cont = 0;
  int n = thread::hardware_concurrency();

  vector<bool> available(n, false);
  vector<thread *> th(n);

  for (int t = 0; t < m1.size() - 1; t++) {
    for (int i = 0; i < m1[0].size(); i++)
      q.push(i);
    result.assign(m1.size(), Vector(m1[0].size(), 0));

    while (true) {
      for (int i = 0; i < available.size(); i++) {
        if (!available[i] and !q.empty()) {
          int nCol = q.front();
          q.pop();
          th[i] = new thread(diamondCol, cref(m1), cref(m2), nCol, ref(result),
                             i, ref(available), ref(cont));
        }
      }
      if (cont == m1.size()) {
        cont = 0;
        break;
      }
    }

    for (int i = 0; i < th.size(); i++) {
      if (th[i]->joinable())
        th[i]->join();
      delete th[i];
    }

    m2 = result;
  }

  // cout << "Matrix:" << endl;
  // print(result);
}

// Sequential

void multSequential(const Matrix &m1, const Matrix &m2) {
  if (m1[0].size() != m2.size()) {
    cerr << "It is not possible matrix multiplication!" << endl;
    return;
  }

  Matrix result(m1.size(), Vector(m2[0].size()));
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m2[0].size(); j++) {
      for (int k = 0; k < m1[i].size(); k++) {
        result[i][j] += m1[i][k] * m2[k][j];
      }
    }
  }

  // cout << "Matrix" << endl;
  // print(result);
}

void diamondSequential(const Matrix &m1) {
  Matrix m2 = m1;
  int oo = numeric_limits<int>::max();
  Matrix result;

  for (int nodes = 0; nodes < m1.size() - 1; nodes++) {
    result.assign(m1.size(), Vector(m1.size(), oo));
    for (int i = 0; i < m1.size(); i++) {
      for (int j = 0; j < m2[0].size(); j++) {
        for (int k = 0; k < m1[i].size(); k++) {
          result[i][j] = min(result[i][j], m1[i][k] + m2[k][j]);
        }
      }
    }
    m2 = result;
  }

  // cout << "Matrix" << endl;
  // print(result);
}

int main(int argc, char const *argv[]) {
  ifstream dataset;
  int rows, cols;

  dataset.open("files/test40.txt", ios::in);
  dataset >> rows >> cols;
  Matrix m(rows, Vector(cols));

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++)
      dataset >> m[i][j];
  }

  {
    auto start = chrono::high_resolution_clock::now();
    // multSequential(m, m);
    diamondSequential(m);
    auto end = chrono::high_resolution_clock::now();
    auto duration =
        chrono::duration_cast<chrono::nanoseconds>(end - start).count();
    cout << "Time Sequential: " << duration << "ns" << endl;
  }

  {
    auto start = chrono::high_resolution_clock::now();
    // multConcurrency(m, m);
    diamondConcurrency(m);
    auto end = chrono::high_resolution_clock::now();
    auto duration =
        chrono::duration_cast<chrono::nanoseconds>(end - start).count();
    cout << "Time Concurrency: " << duration << "ns" << endl;
  }
  // diamondConcurrency(m);

  return 0;
}
