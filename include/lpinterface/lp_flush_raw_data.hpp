#ifndef LPINTERFACE_LP_FLUSH_RAW_DATA_H
#define LPINTERFACE_LP_FLUSH_RAW_DATA_H

#include <vector>

namespace lpint {

template <typename T>
class FlushRawData {
 public:
  virtual void add_columns(std::vector<T>&& values,
                           std::vector<int>&& start_indices,
                           std::vector<int>&& row_indices,
                           std::vector<Ordering>&& ord,
                           std::vector<T>&& rhs) = 0;
  virtual void add_rows(std::vector<T>&& values,
                        std::vector<int>&& start_indices,
                        std::vector<int>&& col_indices,
                        std::vector<Ordering>&& ord, std::vector<T>&& rhs) = 0;
  virtual void add_variables(std::vector<T>&& objective_values,
                             std::vector<VarType>&& var_types) = 0;
};

}  // namespace lpint

#endif  // LPINTERFACE_LP_FLUSH_RAW_DATA_H