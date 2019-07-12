#ifndef LPINTERFACE_LP_H
#define LPINTERFACE_LP_H

#include <vector>

#include "data_objects.hpp"
#include "errors.hpp"

#include "common.hpp"

namespace lpint {

class LinearProgram {
 public:
  virtual ~LinearProgram() = default;

  /**
   * @brief Add columns to the LP
   *
   */
  virtual expected<void, LpError> add_columns(
      const std::vector<Column>& columns) = 0;

  /**
   * @brief Add rows to the LP
   *
   */
  virtual expected<void, LpError> add_rows(const std::vector<Row>& rows) = 0;

  /**
   * @brief Set the objective function to be used
   *
   */
  virtual expected<void, LpError> set_objective(const Objective& objective) = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_H