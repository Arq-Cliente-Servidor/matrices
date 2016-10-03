#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include "util/SparseMatrix.hpp"

template <typename T> using Arc = std::tuple<size_t, size_t, T>;

inline bool FileExists(const std::string &name) {
  std::ifstream f(name.c_str());
  return f.good();
}

inline void CreateFile(const std::string &name) { std::ofstream outfile(name); }

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: " << argv[0] << " dataset output" << std::endl;
    return 1;
  }

  if (!FileExists(argv[1])) {
    std::cout << "File " << argv[1] << " does not exists." << std::endl;
    return 2;
  }

  std::ifstream dataset_file(argv[1]);

  std::string line;

  size_t num_nodes = 0;
  size_t num_arcs = 0;
  std::vector<Arc<float>> data;

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
      float value;
      stream >> i >> j >> value;
      data.emplace_back(i - 1, j - 1, value);
    }
  }

  dataset_file.close();

  std::cout << "DATASET INFORMATION" << std::endl;
  std::cout << "    Number of nodes: " << num_nodes << std::endl;
  std::cout << "    Number of arcs: " << num_arcs << std::endl;

  std::cout << std::endl;

  std::cout << "Sorting to speed up loading... " << std::flush;
  std::sort(data.begin(), data.end(),
            [](const Arc<float> &t1, const Arc<float> &t2) -> bool {
              if (std::get<0>(t1) < std::get<0>(t2)) {
                return true;
              } else if (std::get<0>(t1) == std::get<0>(t2) &&
                         std::get<1>(t1) < std::get<1>(t2)) {
                return true;
              } else {
                return false;
              };
            });
  std::cout << "Done." << std::endl;

  std::cout << "Loading Matrix... " << std::flush;
  auto start = std::chrono::high_resolution_clock::now();

  SparseMatrix<float> mat(num_nodes, num_nodes);
  for (auto &arc : data) {
    mat.Set(std::get<2>(arc), std::get<0>(arc), std::get<1>(arc));
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << "seconds."
            << std::endl;

  // std::cout << "Storing Matrix... " << std::flush;
  // CreateFile(argv[2]);
  // std::ofstream dump_file(argv[2], std::ofstream::binary);
  // mat.Dump(dump_file);
  // dump_file.close();
  // std::cout << "Done." << std::endl;

  std::cout << "Init mult..." << std::endl;
  start = std::chrono::high_resolution_clock::now();
  SparseMatrix<float> result = mat.MultConcurrent(mat);
  end = std::chrono::high_resolution_clock::now();
  elapsed =
      std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Done, elapsed time: " << elapsed.count() << " seconds."
            << std::endl;

  return 0;
}
