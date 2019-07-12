#ifndef LPINTERFACE_DATA_OBJECTS_H
#define LPINTERFACE_DATA_OBJECTS_H

#include <vector>

namespace lpint {

class Column {
 private:
  std::vector<double> values_;
  std::vector<size_t> nonzero_indices_;
  std::vector<double> lower_bounds_;
  std::vector<double> upper_bounds_;
  size_t nnz_;  // number of nonzero entries to be added to constraint matrix
};

class Row {};

}  // namespace lpint

class Objective {};

#endif  // LPINTERFACE_DATA_OBJECTS_H