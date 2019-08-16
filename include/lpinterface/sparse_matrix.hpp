#ifndef LPINTERFACE_SPARSE_MATRIX_H
#define LPINTERFACE_SPARSE_MATRIX_H

#include <algorithm>
#include <set>
#include <type_traits>
#include <vector>

#include "data_objects.hpp"
#include "errors.hpp"

namespace lpint {

/// Type of sparse matrices supported.
enum class SparseMatrixType {
  RowWise,     /// CSR encoding.
  ColumnWise,  /// CSC encoding.
};

/**
 * @brief Represents a sparse matrix.
 * This class represents either a row or column oriented
 * sparse matrix. In order to access matrix elements, use
 * operator(), i.e. `double x = matrix(3, 5);`. Access
 * is O(n) in time.
 */
template <typename T>
class SparseMatrix {
 private:
  using iterator = typename std::vector<MatrixEntry<T>>::iterator;
  using const_iterator = typename std::vector<MatrixEntry<T>>::const_iterator;

 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::vector<T>;
  using difference_type = int;
  using pointer = value_type*;
  using reference = value_type&;

  //! Type of the nonzero matrix entry indices.
  using Index = typename MatrixEntry<T>::Index;
  //! Size type used to index into the sparse matrix.
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  SparseMatrix() : type_(SparseMatrixType::RowWise) {}

  SparseMatrix(SparseMatrix<T>&&) = default;
  SparseMatrix(const SparseMatrix<T>&) = delete;
  SparseMatrix& operator=(const SparseMatrix<T>&) = delete;
  SparseMatrix& operator=(SparseMatrix<T>&&) = default;
  ~SparseMatrix() = default;

  explicit SparseMatrix(SparseMatrixType mtype) : type_(mtype) {}

  SparseMatrix(std::vector<Row<T>>&& rows) : type_(SparseMatrixType::RowWise) {
    add_rows(std::forward<decltype(rows)>(rows));
  }

  explicit SparseMatrix(std::vector<Column<T>>&& columns)
      : type_(SparseMatrixType::ColumnWise) {
    add_columns(std::forward<decltype(columns)>(columns));
  }

  explicit SparseMatrix(const std::initializer_list<Column<T>>&& columns)
      : type_(SparseMatrixType::ColumnWise) {
    add_columns(std::forward<decltype(columns)>(columns));
  }

  explicit SparseMatrix(const std::initializer_list<Row<T>>&& rows)
      : type_(SparseMatrixType::RowWise) {
    add_rows(std::forward<decltype(rows)>(rows));
  }

  //! Return an iterator to the begin of the underlying vector of entries
  iterator begin() { return entries_.begin(); }
  //! Return an iterator to the end of the underlying vector of entries
  iterator end() { return entries_.end(); }

  //! Return a const iterator to the begin of the underlying vector of entries
  const_iterator begin() const { return entries_.begin(); }
  //! Return a const iterator to the end of the underlying vector of entries
  const_iterator end() const { return entries_.end(); }

  /**
   * @brief Return the number of matrix entries.
   * If type() == SparseMatrixType::RowWise, returns the number of rows.
   * If type() == SparseMatrixType::ColumnWise, returns the number of columns.
   *
   * @return std::size_t Number of matrix entries.
   */
  typename std::vector<MatrixEntry<T>>::size_type num_entries() const {
    return entries_.size();
  }

  const std::vector<MatrixEntry<T>>& entries() const { return entries_; }

  /**
   * @brief Add columns to the sparse matrix.
   *
   * @param columns Vector of columns to add. Ownership of the data is
   * transferred to the matrix.
   */
  void add_columns(std::vector<Column<T>>&& columns) {
    if (type_ != SparseMatrixType::ColumnWise) {
      throw MatrixTypeException();
    } else {
      for (auto& entry : columns) {
        entries_.emplace_back(std::move(entry));
      }
    }
  }

  /**
   * @brief Add rows to the sparse matrix.
   *
   * @param rows Vector of rows to add. Ownership of the data is transferred to
   * the matrix.
   */
  void add_rows(std::vector<Row<T>>&& rows) {
    if (type_ != SparseMatrixType::RowWise) {
      throw MatrixTypeException();
    } else {
      for (auto& entry : rows) {
        entries_.emplace_back(std::move(entry));
      }
    }
  }

  /**
   * @brief Matrix indexing operator. Performs bounds checking ifndef NDEBUG.
   *
   * @param i Row index of element to access.
   * @param j Column index of element to access.
   * @return T Element at matrix position A_{ij}.
   */
  T operator()(const SizeType i, const SizeType j) const {
#ifdef NDEBUG
    if (type_ == SparseMatrixType::ColumnWise) {
      return entries_[j][i];
    } else {
      return entries_[i][j];
    }
#else
    if (type_ == SparseMatrixType::ColumnWise) {
      return entries_.at(j)[i];
    } else {
      return entries_.at(i)[j];
    }
#endif
  }

  SparseMatrixType type() const { return type_; }

 private:
  void add_entries(const std::vector<MatrixEntry<T>>& entries) {
    for (const auto& entry : entries) {
      check_entry_valid(entry);
      entries_.emplace_back(entry);
    }
  }

  SparseMatrixType type_;
  std::vector<std::size_t> begin_indices_;
  std::vector<MatrixEntry<T>> entries_;
};

}  // namespace lpint

#endif  // LPINTERFACE_SPARSE_MATRIX_H