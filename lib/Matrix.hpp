#pragma once

#include <vector>

template <typename T>
class Matrix {
private:
  std::vector<std::vector<T>> elements;
  int rows, cols;

public:
  Matrix() : rows(0), cols(0), elements(0) {}

  Matrix(std::vector<std::vector<T>> m) {
    elements = m;
    rows = elements.size();
    cols = elements[0].size();
  }

  const int &getNumRows() {
    return rows;
  }

  const int &getNumCols() {
    return cols;
  }

  const int &getValue(int i, int j) {
    return elements[i][j];
  }
};

template <typename T>
std::ostream& operator << (std::ostream &out, Matrix<T> &m) {
  out << "[";
  for (int i = 0; i < m.getNumRows(); i++) {
    out << "[";
    for (int j = 0; j < m.getNumCols(); j++) {
      if (j) out << ",";
      out << m.getValue(i, j);
    }
    out << "]";
  }
  out << "]";
  return out;
}
