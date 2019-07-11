#ifndef LPINTERFACE_LP_H
#define LPINTERFACE_LP_H

#include <vector>

#include "data_objects.hpp"
#include "errors.hpp"

#include "expected.hpp"

namespace lpint {

class LinearProgram {
 public:
  virtual ~LinearProgram() = default;

  virtual tl::expected<void, LpError> add_columns(
      const std::vector<Column>& columns) = 0;

  virtual tl::expected<void, LpError> add_rows(
      const std::vector<Row>& rows) = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_H