#pragma once

#include <cassert>
#include <istream>
#include <map>
#include <ostream>
#include <vector>

#include "ThreadPool.hpp"

template <typename T>
class SparseMatrix {
public:
    SparseMatrix() : num_rows_(0), num_cols_(0), rowPtr(num_rows_ + 1, T(0)) {}

    SparseMatrix(size_t r, size_t c)
          : num_rows_(r), num_cols_(c), rowPtr(num_rows_ + 1, T(0)) {}

    SparseMatrix(std::istream& is)
          : num_rows_(0), num_cols_(0), rowPtr(num_rows_ + 1, T(0)) {
        // Retreive the identificator
        std::string id("SparseMatrix");
        is.read(&id[0], id.size());

        // If the id is not valid this contructor behaves as the default
        // contructor
        if (id != "SparseMatrix") return;

        // Temp variable to store lvalue references
        size_t size;

        // Load the matrix size
        is.read(reinterpret_cast<char*>(&num_rows_), sizeof(size_t));
        is.read(reinterpret_cast<char*>(&num_cols_), sizeof(size_t));

        // Load the vals
        size = vals.size();
        is.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        vals.resize(size);
        is.read(reinterpret_cast<char*>(vals.data()), size * sizeof(T));

        // Load the colInd
        size = colInd.size();
        is.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        colInd.resize(size);
        is.read(reinterpret_cast<char*>(colInd.data()), size * sizeof(size_t));

        // Load the rowPtr
        size = rowPtr.size();
        is.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        rowPtr.resize(size);
        is.read(reinterpret_cast<char*>(rowPtr.data()), size * sizeof(size_t));
    }

    SparseMatrix(const SparseMatrix<T>&) = default;

    SparseMatrix(SparseMatrix<T>&&) = default;

    SparseMatrix<T>& operator=(const SparseMatrix<T>& other) = default;

    SparseMatrix<T>& operator=(SparseMatrix<T>&& other) = default;

    size_t NumRows() const {
        return num_rows_;
    }

    size_t NumCols() const {
        return num_cols_;
    }

    T Get(size_t r, size_t c) const {
        if (rowPtr.back() == 0 || r >= num_rows_ || c >= num_cols_) return T(0);

        for (size_t i = rowPtr[r]; i < rowPtr[r + 1]; i++) {
            if (colInd[i] == c) return vals[i];
        }

        return T(0);
    }

    void Set(const T& v, size_t r, size_t c) {
        // Find the index where the value should be stored
        size_t index = rowPtr[r + 1];
        for (size_t i = rowPtr[r]; i < rowPtr[r + 1]; i++) {
            if (c <= colInd[i]) {
                index = i;
                break;
            }
        }

        // Check if the element already exist in the row
        bool item_found = (rowPtr[r + 1] - rowPtr[r] > 0 &&
                           index != rowPtr.back() && colInd[index] == c);

        // Iterators to the element to insert, replace or remove
        auto vals_it = vals.begin() + index;
        auto colInd_it = colInd.begin() + index;

        if (item_found) {
            // If the value is 0 remove the index from the SparseMatrix
            // else change the existing value
            if (v == T(0)) {
                vals.erase(vals_it);
                colInd.erase(colInd_it);
                for (size_t i = r + 1; i < rowPtr.size(); i++) rowPtr[i]--;
            } else {
                *vals_it = v;
            }
        } else {
            // Add a non-zero value to the SparseMatrix
            if (v != T(0)) {
                vals.insert(vals_it, v);
                colInd.insert(colInd_it, c);
                for (size_t i = r + 1; i < rowPtr.size(); i++) rowPtr[i]++;
            }
        }
    }

    void SetData(const std::vector<T>& data) {
        if (data.size() != NumRows() * NumCols()) return;
        for (size_t row = 0; row < NumRows(); row++) {
            for (size_t col = 0; col < NumCols(); col++) {
                Set(data[row * NumCols() + col], row, col);
            }
        }
    }

    SparseMatrix<T> Mult(const SparseMatrix<T>& b) const {
        // http://gauss.cs.ucsb.edu/~aydin/csb2009.pdf
        assert(NumCols() == b.NumRows());

        SparseMatrix<T> result(NumRows(), b.NumCols());

        for (size_t row = 0; row < NumRows(); row++) {
            for (size_t col = 0; col < b.NumCols(); col++) {
                T temp(0);
                for (size_t ind = rowPtr[row]; ind < rowPtr[row + 1]; ind++) {
                    temp += vals[ind] * b.Get(colInd[ind], col);
                }
                result.Set(temp, row, col);
            }
        }

        return result;
    }

    SparseMatrix<T> MultConcurrent(const SparseMatrix<T>& b) const {
        // http://gauss.cs.ucsb.edu/~aydin/csb2009.pdf
        assert(NumCols() == b.NumRows());

        // Create a ThreadPool to execute concurrent tasks
        ThreadPool* pool = new ThreadPool();

        // A vector to store each result of the rows
        std::vector<SparseMatrix<T>> results(NumRows(), {1, b.NumCols()});

        // This lambda function is in charge of multiply by each row
        auto mult_row = [&](size_t row) {
            SparseMatrix<T>& r = results[row];
            for (size_t col = 0; col < b.NumCols(); col++) {
                T temp(0);
                for (size_t ind = rowPtr[row]; ind < rowPtr[row + 1]; ind++) {
                    temp += vals[ind] * b.Get(colInd[ind], col);
                }
                if (temp == 0) break;
                r.Set(temp, 0, col);
            }
        };

        // Register each task in the ThreadPool
        for (size_t row = 0; row < NumRows(); row++) {
            pool->Submit([&mult_row, row] { mult_row(row); });
        }

        // Wait all the task to be completed and delete the ThreadPool
        delete pool;

        // The resulting matrix
        SparseMatrix<T> result(NumRows(), b.NumCols());

        for (size_t row = 0; row < results.size(); row++) {
            for (size_t col = 0; col < results[row].NumCols(); col++) {
                result.Set(results[row].Get(0, col), row, col);
            }
        }

        return result;
    }

    void Dump(std::ostream& os) {
        // Add the identificator
        std::string id("SparseMatrix");
        os.write(id.data(), id.size());

        // Temp variable to store lvalue references
        size_t size;

        // Save the matrix size
        os.write(reinterpret_cast<const char*>(&num_rows_), sizeof(size_t));
        os.write(reinterpret_cast<const char*>(&num_cols_), sizeof(size_t));

        // Save the vals
        size = vals.size();
        os.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        os.write(reinterpret_cast<const char*>(vals.data()), size * sizeof(T));

        // Save the colInd
        size = colInd.size();
        os.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        os.write(reinterpret_cast<const char*>(colInd.data()),
                 size * sizeof(size_t));

        // Save the rowPtr
        size = rowPtr.size();
        os.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        os.write(reinterpret_cast<const char*>(rowPtr.data()),
                 size * sizeof(size_t));
    }

    template <typename T2>
    friend bool operator==(const SparseMatrix<T2>& m1,
                           const SparseMatrix<T2>& m2);

private:
    size_t num_rows_;
    size_t num_cols_;
    std::vector<T> vals;
    std::vector<size_t> colInd;
    std::vector<size_t> rowPtr;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const SparseMatrix<T>& m) {
    os << "[";
    for (size_t j = 0; j < m.NumRows(); j++) {
        if (j != 0) os << " ";
        os << "[";
        for (size_t i = 0; i < m.NumCols(); i++) {
            os << m.Get(j, i);
            if (i != m.NumCols() - 1) os << ", ";
        }
        os << "]";
        if (j != m.NumRows() - 1) os << "\n";
    }
    os << "]";
    return os;
}

template <typename T>
bool operator==(const SparseMatrix<T>& m1, const SparseMatrix<T>& m2) {
    return (m1.NumCols() == m2.NumCols() && m1.NumRows() == m2.NumRows() &&
            m1.vals == m2.vals && m1.colInd == m2.colInd &&
            m1.rowPtr == m2.rowPtr);
}
