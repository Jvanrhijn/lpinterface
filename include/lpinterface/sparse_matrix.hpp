#ifndef LPINTERFACE_SPARSE_MATRIX_H
#define LPINTERFACE_SPARSE_MATRIX_H

namespace lpint {

/// Type of sparse matrices supported.
enum class SparseMatrixType {
  RowWise, /// CSR encoding.
  ColumnWise, /// CSC encoding.
};

// matrix entry is templated over T, with T restricted to
// arithmetic types i.e. numbers
/**
 * @brief Matrix entry type, for use in sparse matrix.
 * The matrix entry base class is specialized as Row and Column,
 * for the two sparse matrix representations. The class
 * thus represents a row or column vector with mostly zero
 * entries. Its operator[] is overloaded to provide
 * access as if it is a dense vector. Access using operator[]
 * is O(n) in time, with n the number of nonzero entries.
 */
template <typename T>
class MatrixEntry {
  static_assert(std::is_arithmetic<T>::value,
                "MatrixEntry<T> requires T to be arithmetic");

 private:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

 public:
  using Index = int;
  using SizeType = std::size_t;

  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using difference_type = int;
  using pointer = value_type*;
  using reference = value_type&;

  MatrixEntry() = default;
  MatrixEntry(MatrixEntry<T>&&) = default;
  MatrixEntry(const MatrixEntry<T>&) = delete;
  MatrixEntry<T>& operator=(MatrixEntry<T>&&) = default;

  MatrixEntry(const std::vector<T>& values, const std::vector<Index>& indices)
      : values_(values), nonzero_indices_(indices) {
        check_entry_valid();
      }
  virtual ~MatrixEntry() = default;


  /**
   * @brief Indexing operator; can be used identically to a dense vector.
   * Will perform bounds checking ifndef NDEBUG.
   *
   * @param index Index of element to retrieve.
   * @return T Element at index.
   */
  T operator[](const SizeType index) const {
    auto index_in_data =
        std::find(nonzero_indices_.begin(), nonzero_indices_.end(), index);
    if (index_in_data == nonzero_indices_.end()) {
      return T();
    } else {
#if NDEBUG
      return values_[static_cast<SizeType>(index_in_data -
                                           nonzero_indices_.begin())];
#else
      return values_.at(
          static_cast<SizeType>(index_in_data - nonzero_indices_.begin()));
#endif
    }
  }

  //! Returns the smallest entry of this matrix element.
  T lower_bound() const {
    return std::min_element(values_.begin(), values_.end());
  }

  //! Returns the largest entry of this matrix element.
  T upper_bound() const {
    return std::max_element(values_.begin(), values_.end());
  }

  //! Return the number of nonzero entries in the matrix entry.
  typename std::vector<T>::size_type num_nonzero() const {
    return values_.size();
  }

  //! Get a const reference to the underlying value array.
  const std::vector<T>& values() const { return values_; }

  //! Get a reference to the underlying value array.
  std::vector<T>& values() { return values_; }

  //! Get a const reference to the underlying nonzero indices.
  const std::vector<Index>& nonzero_indices() const { return nonzero_indices_; }

  //! Get a reference to the underlying nonzero indices.
  std::vector<Index>& nonzero_indices() { return nonzero_indices_; }

  //! Obtain an iterator to the begin of the ests/CMakeFiles/unit_tests.dir/all]
  //! Error 2
  iterator begin() { return values_.begin(); }

  //! Obtain a const  iterator to the begin of the underlying value array.
  const_iterator begin() const { return values_.begin(); }

  //! Obtain an iterator to the end of the underlying value array.
  iterator end() { return values_.end(); }

  //! Obtain a const  iterator to the end of the underlying value array.
  const_iterator end() const { return values_.end(); }

 protected:
  std::vector<T> values_;
  std::vector<Index> nonzero_indices_;  // indices of nonzero entries

  void check_entry_valid() {
    // matrix entries are invalid if there are
    // duplicate nonzero indices present
    std::set<Index> index_set(nonzero_indices_.begin(), nonzero_indices_.end());
    if (nonzero_indices_.size() != index_set.size()) {
      throw InvalidMatrixEntryException();
    }
  }

};

template <typename T>
class Column : public MatrixEntry<T> {
 public:
  using Index = typename MatrixEntry<T>::Index;
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  Column(Column<T>&&) = default;
  Column(const std::vector<T>& values, const std::vector<Index>& indices)
      : MatrixEntry<T>(values, indices) {}
  Column(MatrixEntry<T>&& m) : MatrixEntry<T>(std::move(m)) {}
  Column() = default;
  Column<T>& operator=(Column<T>&&) = default; 

// TODO: find a more elegant way to do this
#if TESTING
  friend class rc::Arbitrary<Row<T>>;
#endif
};

template <typename T>
class Row : public MatrixEntry<T> {
 public:
  using Index = typename MatrixEntry<T>::Index;
  using SizeType = typename MatrixEntry<T>::SizeType;

 public:
  Row(Row<T>&&) = default;
  Row(const std::vector<T>& values, const std::vector<Index>& indices)
      : MatrixEntry<T>(values, indices) {}

  Row(MatrixEntry<T>&& m) : MatrixEntry<T>(std::move(m)) {}
  Row() = default;
  Row<T>& operator=(Row<T>&&) = default;

// TODO: find a more elegant way to do this
#if TESTING
  friend class rc::Arbitrary<Row<T>>;
#endif
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
  SparseMatrix(SparseMatrix<T>&&) = default;
  SparseMatrix(const SparseMatrix<T>&) = delete;

  SparseMatrix() : type_(SparseMatrixType::RowWise) {}

  SparseMatrix(SparseMatrixType mtype) : type_(mtype) {}

  SparseMatrix(std::vector<Row<T>>&& rows) : type_(SparseMatrixType::RowWise) {
    add_rows(std::forward<decltype(rows)>(rows));
  }

  SparseMatrix(std::vector<Column<T>>&& columns)
      : type_(SparseMatrixType::ColumnWise) {
    add_columns(std::forward<decltype(columns)>(columns));
  }

  SparseMatrix(const std::initializer_list<Column<T>>&& columns)
      : type_(SparseMatrixType::ColumnWise) {
    add_columns(std::forward<decltype(columns)>(columns));
  }

  SparseMatrix(const std::initializer_list<Row<T>>&& rows)
      : type_(SparseMatrixType::RowWise) {
    add_rows(std::forward<decltype(rows)>(rows));
  }

  SparseMatrix<T>& operator=(SparseMatrix<T>&&) = default;

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

}

#endif // LPINTERFACE_SPARSE_MATRIX_H