#include <algorithm>
#include <armadillo>
#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <tuple>

#include "lib/SparseMatrix.hpp"

template <typename T> using Arc = std::tuple<size_t, size_t, T>;

inline bool FileExists(const std::string &name) {
  std::ifstream f(name.c_str());
  return f.good();
}

inline void CreateFile(const std::string &name) { std::ofstream outfile(name); }

// template <typename T> bool compare(SparseMatrix<T> &a, arma::SpMat<T> &b) {
//   // size_t begin, size_t end) {
//   for (size_t i = 0; i < a.getNumRows(); i++) {
//     for (size_t j = 0; j < a.getNumCols(); j++) {
//       // if (a.get(i, j) != 0)
//       //   cout << a.get(i, j) << endl;
//       if (a.get(i, j) != b(i, j))
//         return false;
//     }
//   }
//   return true;
// }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " dataset" << std::endl;
    return 1;
  }

  if (!FileExists(argv[1])) {
    std::cout << "File " << argv[1] << " does not exists." << std::endl;
    return 2;
  }

  std::ifstream dataset_file(argv[1]);
  std::string line;

  int num_nodes = 0;
  int num_arcs = 0;
  std::set<Arc<double>> data;

  while (std::getline(dataset_file, line)) {
    std::stringstream stream(line);

    char info;
    stream >> info;

    if (info == 'p') {
      std::string temp;
      stream >> temp;
      if (temp == "sp") {
        stream >> num_nodes >> num_arcs;
      }
    } else if (info == 'a') {
      size_t i, j;
      double value;
      stream >> i >> j >> value;
      data.emplace(i - 1, j - 1, value);
    }
  }

  dataset_file.close();
  num_arcs = data.size();
  // arma::umat locations(2, num_arcs);
  // arma::vec vals(num_arcs);
  // size_t cont = 0;

  std::cout << "DATASET INFORMATION" << std::endl;
  std::cout << "    Number of nodes: " << num_nodes << std::endl;
  std::cout << "    Number of arcs: " << num_arcs << std::endl;

  std::cout << std::endl;
  std::cout << std::fixed;

  std::cout << "Done." << std::endl;
  std::cout << "Loading Matrix... " << std::flush;
  auto start = std::chrono::high_resolution_clock::now();

  SparseMatrix<double> mat(num_nodes, num_nodes);
  for (auto &arc : data) {
    mat.set(std::get<2>(arc), std::get<0>(arc), std::get<1>(arc));
    // locations(0, cont) = std::get<0>(arc);
    // locations(1, cont) = std::get<1>(arc);
    // vals(cont) = std::get<2>(arc);
  }

  // arma::SpMat<double> m2(locations, vals, num_nodes, num_nodes);
  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << " seconds."
            << std::endl;

  std::cout << "Init mult..." << std::endl;
  start = std::chrono::high_resolution_clock::now();
  // cout << mat << endl;
  // SparseMatrix<double> result = mat.multConcurrent(mat);
  // cout << result << endl;
  // std::cout << "Rows Zeros: " << mat.countRowsZeros() << std::endl;
  SparseMatrix<double> result = mat.multConcurrent(mat);
  SparseMatrix<double> result2 = mat * mat; //.diamondConcurrent();
  // arma::SpMat<double> result2 = m2 * m2;
  end = std::chrono::high_resolution_clock::now();
  elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << " seconds."
            << std::endl;

  if (result.compare(result)) {
    std::cout << "Buen calculo! :D\n";
  } else {
    std::cout << "Mal calculo! :(\n";
  }

  return 0;
}
