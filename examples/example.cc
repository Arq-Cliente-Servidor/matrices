#include <iostream>
#include <thread>
#include <utility>
#include <vector>

using namespace std;

using Matrix = vector<vector<int>>;
using Vector = vector<int>;

void print(Matrix &r) {
  for (size_t i = 0; i < r.size(); i++) {
    for (size_t j = 0; j < r.size(); j++) {
      cout << r[j][i] << " ";
    }
    cout << endl;
  }
}

void mult(const Matrix &m, int col, Vector &r) {
  for (int i = 0; i < m.size(); i++) {
    int mn = numeric_limits<int>::max();
    for (int j = 0; j < m[0].size(); j++) {
      mn = min(mn, m[i][j] + m[j][col]);
    }
    r[i] = mn;
  }
}

int main() {
  Matrix m = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  Vector v = {5, 7, 9};
  Matrix r = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
  thread t0 = thread(mult, cref(m), 0, ref(r[0]));
  thread t1 = thread(mult, cref(m), 1, ref(r[1]));
  thread t2 = thread(mult, cref(m), 2, ref(r[2]));
  // mult(m, 1, r[1]);
  // mult(m, 2, r[2]);
  t0.join();
  t1.join();
  t2.join();
  print(r);
  return 0;
}
