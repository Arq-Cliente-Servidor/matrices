#include "lib/SparseMatrix.hpp"
#include "lib/ThreadPool.hpp"
#include <chrono>
#include <fstream>

int main() {
  std::ifstream dataset;
  int rows, cols;

  std::cout << "Loading Matrix... " << std::flush;
  auto start = std::chrono::high_resolution_clock::now();

  std::cout << std::endl;
  std::cout << std::fixed;

  dataset.open("files/test1.txt", std::ios::in);
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

  // SparseMatrix<int> result(3, 3);
  // SparseMatrix<int> result = diamondConcurrency(m);
  // SparseMatrix<int> ch = checkSM(m);
  // SparseMatrix<int> result = diamond_block_seqSM(ch, ch);
  // result.print();

  start = std::chrono::high_resolution_clock::now();
  SparseMatrix<int> m2 = m.check();
  SparseMatrix<int> result = m.diamond_block_seq_complete();
  // SparseMatrix<int> result2 = m2.diamond_block_seq(result);

  // result.print(m.getNumRows(), m.getNumCols());
  end = std::chrono::high_resolution_clock::now();
  elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << " seconds."
            << std::endl;

  result.print(m.getNumRows(), m.getNumCols());
  // result2.print(m.getNumRows(), m.getNumCols());

  // m4 * m4
  // 30 36 42
  // 66 81 96
  // 102 126 150

  // m <> m = {{2, 3, 4, 5}
  //           {6, 7, 8, 9},
  //          {10, 11, 12, 13},
  //          {14, 15, 16, 17}}

  return 0;
}
