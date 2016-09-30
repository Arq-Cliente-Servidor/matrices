#include "lib/SparseMatrix.hpp"
#include "lib/ThreadPool.hpp"
#include <chrono>
#include <cmath>

using Matrix = std::vector<std::vector<int>>;
using Vector = std::vector<int>;

void print(const Matrix &m) {
  std::cerr << "------------------------\n";
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[0].size(); j++) {
      if (j)
        std::cout << " ";
      std::cout << m[i][j];
    }
    std::cout << std::endl;
  }
  std::cerr << "------------------------\n";
}

// void diamondCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result)
// {
//   // std::cerr << "start " << nCol << std::endl;
//   // std::vector<int> col = getCol(m2, nCol);
//   for (int i = 0; i < m1.size(); i++) {
//     int mn = std::numeric_limits<int>::max();
//     for (int j = 0; j < m1[i].size(); j++) {
//       mn = std::min(mn, m1[i][j] + m2[j][nCol]);
//     }
//     result[i][nCol] = mn;
//   }
//   // print(result);
//   // std::cerr << "dcol\n";
// }

Matrix partition(const Matrix &m, int offsetI, int offsetJ) {
  Matrix p(m.size() / 2, Vector(m.size() / 2, 0));
  for (int i = 0; i < m.size() / 2; i++) {
    for (int j = 0; j < m.size() / 2; j++) {
      p[i][j] = m[i + offsetI][j + offsetJ];
    }
  }
  return p;
}

Matrix check(Matrix &m) {
  int nSize = pow(2, ceil(log2(double(m.size()))));
  if (m.size() == nSize)
    return m;
  m.resize(nSize);
  for (int i = 0; i < nSize; i++) {
    m[i].resize(nSize);
  }
  return m;
}

void rebuild(Matrix &result, int offsetI, int offsetJ, const Matrix &m) {
  for (int i = 0; i < m.size(); i++) {
    for (int j = 0; j < m[i].size(); j++) {
      result[i + offsetI][j + offsetJ] = m[i][j];
    }
  }
}

Matrix addMatrix(const Matrix &a, const Matrix &b) {
  Matrix c(a.size(), Vector(a.size(), 0));
  for (int i = 0; i < a.size(); i++) {
    for (int j = 0; j < a[i].size(); j++) {
      c[i][j] = a[i][j] + b[i][j];
    }
  }

  return c;
}

// Secuencial
Matrix multMatrix(const Matrix &m1, const Matrix &m2) {
  Matrix result(m1.size(), Vector(m1.size(), 0));
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1.size(); j++) {
      for (int k = 0; k < m1.size(); k++) {
        result[i][j] += m1[i][k] * m2[k][j];
      }
    }
  }

  return result;
}

void multCol(const Matrix &m1, const Matrix &m2, int nCol, Matrix &result) {
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1.size(); j++) {
      result[i][nCol] += m1[i][j] * m2[j][nCol];
    }
  }
}

Matrix multConcurrency(const Matrix &m1, const Matrix &m2) {
  Matrix result(m1.size(), Vector(m1[0].size(), 0));
  {
    thread_pool pool;
    for (int i = 0; i < m1.size(); i++) {
      auto w = [&m1, &m2, &result, i]() { multCol(m1, m2, i, result); };
      pool.submit(w);
    }
  }
  // print(result);
  return result;
}

Matrix block_seq(const Matrix &A, const Matrix &B) {
  if (A.size() == 2) {
    return multMatrix(A, B);
  } else {
    int sizeA = A.size() / 2;
    int sizeB = B.size() / 2;

    Matrix a0 = partition(A, 0, 0);
    Matrix a1 = partition(A, 0, sizeA);
    Matrix a2 = partition(A, sizeA, 0);
    Matrix a3 = partition(A, sizeA, sizeA);

    Matrix b0 = partition(B, 0, 0);
    Matrix b1 = partition(B, 0, sizeB);
    Matrix b2 = partition(B, sizeB, 0);
    Matrix b3 = partition(B, sizeB, sizeB);

    Matrix r0 = addMatrix(block_seq(a0, b0), block_seq(a1, b2));
    Matrix r1 = addMatrix(block_seq(a0, b1), block_seq(a1, b3));
    Matrix r2 = addMatrix(block_seq(a2, b0), block_seq(a3, b2));
    Matrix r3 = addMatrix(block_seq(a2, b1), block_seq(a3, b3));

    Matrix result(A.size(), Vector(A.size(), 0));
    int sizeResult = result.size() / 2;

    rebuild(result, 0, 0, r0);
    rebuild(result, 0, sizeResult, r1);
    rebuild(result, sizeResult, 0, r2);
    rebuild(result, sizeResult, sizeResult, r3);

    return result;
  }
}

Matrix diamondMatrix(const Matrix &m1, const Matrix &m2) {
  Matrix result(m1.size(), Vector(m1.size(), 0));
  for (int i = 0; i < m1.size(); i++) {
    for (int j = 0; j < m1.size(); j++) {
      int mn = std::numeric_limits<int>::max();
      for (int k = 0; k < m1.size(); k++) {
        mn = std::min(mn, m1[i][k] + m2[k][j]);
      }
      result[i][j] = mn;
    }
  }

  return result;
}

Matrix minMatrix(const Matrix &A, const Matrix &B) {
  Matrix C(A.size(), Vector(A.size(), std::numeric_limits<int>::max()));
  for (int i = 0; i < A.size(); i++) {
    for (int j = 0; j < A[i].size(); j++) {
      C[i][j] = std::min(A[i][j], B[i][j]);
    }
  }
  return C;
}

Matrix diamond_block_seq(const Matrix &A, const Matrix &B) {
  if (A.size() == 2) {
    return diamondMatrix(A, B);
  } else {
    int sizeA = A.size() / 2;
    int sizeB = B.size() / 2;

    Matrix a0 = partition(A, 0, 0);
    Matrix a1 = partition(A, 0, sizeA);
    Matrix a2 = partition(A, sizeA, 0);
    Matrix a3 = partition(A, sizeA, sizeA);

    Matrix b0 = partition(B, 0, 0);
    Matrix b1 = partition(B, 0, sizeB);
    Matrix b2 = partition(B, sizeB, 0);
    Matrix b3 = partition(B, sizeB, sizeB);

    Matrix r0 = minMatrix(diamond_block_seq(a0, b0), diamond_block_seq(a1, b2));
    Matrix r1 = minMatrix(diamond_block_seq(a0, b1), diamond_block_seq(a1, b3));
    Matrix r2 = minMatrix(diamond_block_seq(a2, b0), diamond_block_seq(a3, b2));
    Matrix r3 = minMatrix(diamond_block_seq(a2, b1), diamond_block_seq(a3, b3));

    Matrix result(A.size(), Vector(A.size(), 0));
    int sizeResult = result.size() / 2;

    rebuild(result, 0, 0, r0);
    rebuild(result, 0, sizeResult, r1);
    rebuild(result, sizeResult, 0, r2);
    rebuild(result, sizeResult, sizeResult, r3);

    return result;
  }
}

// void diamondConcurrency(const Matrix &m1) {
//   Matrix result(m1.size(), Vector(m1[0].size(), 0));
//   Matrix m2(m1);
//   for (int j = 0; j < m1.size() - 1; j++) {
//     {
//       thread_pool pool;
//       for (int i = 0; i < m1.size(); i++) {
//         auto w = [&m1, &m2, &result, i]() { diamondCol(m1, m2, i, result); };
//         pool.submit(w);
//       }
//     }
//     m2 = result;
//     print(result);
//     std::cout << std::endl;
//   }
//   print(result);
// }

int main() {
  std::ifstream dataset;
  int rows, cols;

  std::cout << "Loading Matrix... " << std::flush;
  auto start = std::chrono::high_resolution_clock::now();

  std::cout << std::endl;

  dataset.open("files/test300.txt", std::ios::in);
  dataset >> rows >> cols;

  std::cout << "DATASET INFORMATION" << std::endl;
  std::cout << "    Number of nodes: " << rows << std::endl;
  std::cout << "    Number of arcs: " << cols << std::endl;
  SparseMatrix<int> m(rows, cols);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      int tmp;
      dataset >> tmp;
      m.set(tmp, i, j);
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << " seconds."
            << std::endl;

  // SparseMatrix<int> m(3, 3);
  // m.set(1, 0, 0);
  // m.set(2, 0, 1);
  // m.set(3, 0, 2);
  // m.set(4, 1, 0);
  // m.set(5, 1, 1);
  // m.set(6, 1, 2);
  // m.set(7, 2, 0);
  // m.set(8, 2, 1);
  // m.set(9, 2, 2);
  // m.set(10, 2, 1);
  // m.set(11, 2, 2);
  // m.set(12, 2, 3);
  // m.set(13, 3, 0);
  // m.set(14, 3, 1);
  // m.set(15, 3, 2);
  // m.set(16, 3, 3);
  // SparseMatrix<int> result(3, 3);
  // diamondCol(m, m2, 0, result);
  // SparseMatrix<int> result = multConcurrency(m, m);
  //

  start = std::chrono::high_resolution_clock::now();
  SparseMatrix<int> result = multConcurrency(m, m);
  end = std::chrono::high_resolution_clock::now();
  elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << " seconds."
            << std::endl;

  // result.print();

  // Matrix m1 = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15,
  // 16}};
  // Matrix result2(4, Vector(4, 0));
  // diamondCol(m1, m1, 0, result2);
  // print(result2);
  // print(diamond_block_seq(m, m));

  // Matrix m4 = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  // Matrix result3 = block_seq(check(m4), check(m4));
  // print(result3);

  // m4 * m4
  // 30 36 42 0
  // 66 81 96 0
  // 102 126 150

  // m <> m = {{2, 3, 4, 5}
  //           {6, 7, 8, 9},
  //          {10, 11, 12, 13},
  //          {14, 15, 16, 17}}

  // print(m);
  // auto start = std::chrono::high_resolution_clock::now();
  // Matrix result = block_seq(m, m);
  // print(result);
  // diamondConcurrency(m);
  // multConcurrency(m, m);
  // auto end = std::chrono::high_resolution_clock::now();
  // auto duration =
  //     std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
  //         .count();
  // std::cout << "Time Concurrency: " << duration << "ms" << std::endl;
  // print(result);

  // Matrix B = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15,
  // 16}};
  // Matrix AA = {{1, 2}, {3, 4}};
  // Matrix BB = {{1, 2}, {3, 4}};
  // Matrix C(A.size(), Vector(A.size(), 0));
  // bijk(A, A, C);
  // int n = A.size() / 2;
  // Matrix p1 = partition(A, 0, 0);
  // Matrix p2 = partition(A, 0, n);
  // Matrix p3 = partition(A, n, 0);
  // Matrix p4 = partition(A, n, n);
  //
  // rebuild(C, 0, 0, p1);
  // rebuild(C, 0, n, p2);
  // rebuild(C, n, 0, p3);
  // rebuild(C, n, n, p4);

  //
  // print(p1);
  // std::cout << std::endl;
  // print(p2);
  // std::cout << std::endl;
  // print(p3);
  // std::cout << std::endl;
  // print(p4);
  // std::cout << std::endl;
  // Matrix C = block_seq(A, B);
  // print(C);

  return 0;
}
