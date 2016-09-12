#include <iostream>
#include <thread>
#include <vector>

using namespace std;

vector<int> multCol(vector<vector<int>> &m, vector<int> &col) {
  vector<int> result(col.size());
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[i].size(); j++) {
      result[i] += m[i][j] * col[j];
    }
  }
  return result;
}

vector<int> getCol(vector<vector<int>> &m, int numCol) {
  vector<int> col;
  for (int i = 0; i < m.size(); i++) {
    col.push_back(m[i][numCol]);
  }
  return col;
}

int main(int argc, char const *argv[]) {
  vector<vector<int>> m1 = {{2, 0, 1}, {3, 0, 0}, {5, 1, 1}};
  vector<vector<int>> m2 = {{1, 0, 1}, {1, 2, 1}, {1, 1, 0}};

  vector<int> col1 = getCol(m2, 0);
  vector<int> col2 = getCol(m2, 1);
  vector<int> col3 = getCol(m2, 2);

  vector<int> mCol1 = multCol(m1, col1);
  vector<int> mCol2 = multCol(m1, col2);
  vector<int> mCol3 = multCol(m1, col3);

  for (int i = 0; i < mCol1.size(); i++) {
    cout << mCol1[i] << " " << mCol2[i] << " " << mCol3[i] << endl;
  }

  return 0;
}
