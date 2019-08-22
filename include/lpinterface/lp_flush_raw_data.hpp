#ifndef LPINTERFACE_LP_FLUSH_RAW_DATA_H
#define LPINTERFACE_LP_FLUSH_RAW_DATA_H

#include <vector>

#include "data_objects.hpp"

namespace lpint {

/**
 * @brief Interface useful for flushing data directly to an LP solver backend.
 * This class provides methods to flush data directly to an LP solver. The
 * given data is moved from, to minimize unnecessary copying.
 *
 * @tparam T Type of data in LP matrix.
 */
template <typename T>
class FlushRawData {
 public:
  /**
   * @brief Add columns to the LP in CSC format.
   *
   * @param values Column values.
   * @param start_indices Starting indices columns in values.
   * @param row_indices Row indices of values in columns
   * @param lb Lower bound of the column constraint.
   * @param ub Upper bound of the column constraint.
   */
  virtual void add_columns(std::vector<T>&& values,
                           std::vector<int>&& start_indices,
                           std::vector<int>&& row_indices, std::vector<T>&& lb,
                           std::vector<T>&& ub) = 0;

  /**
   * @brief Add rows to the LP in CSR format.
   *
   * @param values Row values.
   * @param start_indices Starting indices rows in values.
   * @param row_indices Column indices of values in rows
   * @param lb Lower bound of the row constraint.
   * @param ub Upper bound of the row constraint.
   */
  virtual void add_rows(std::vector<T>&& values,
                        std::vector<int>&& start_indices,
                        std::vector<int>&& col_indices, std::vector<T>&& lb,
                        std::vector<T>&& ub) = 0;

  /**
   * @brief Add variables to the LP.
   *
   * @param objective_values values of objective function vector.
   * @param var_types Type of each variable (real, integer, binary, etc.)
   */
  virtual void add_variables(std::vector<T>&& objective_values,
                             std::vector<VarType>&& var_types) = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_FLUSH_RAW_DATA_H