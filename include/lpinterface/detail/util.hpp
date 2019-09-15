#ifndef LPINTERFACE_INCLUDE_UTIL_H
#define LPINTERFACE_INCLUDE_UTIL_H

#include <vector>

namespace lpint {

namespace detail {

template <class T>
std::vector<T> rankify(const std::vector<T>& list) {
  std::vector<T> out(list.size(), 0);
  for (std::size_t i = 0; i < list.size(); i++) {
    for (std::size_t j = 0; j < list.size(); j++) {
        out[i] += list[j] < list[i] ? 1 : 0;
    }
  }
  return out;
}

inline std::vector<std::size_t> inverse_permutation(const std::vector<std::size_t>& permutation) {
  std::vector<std::size_t> inv(permutation.size(), 0);
  for (std::size_t i = 0; i < permutation.size(); i++) {
    inv[permutation[i]] = i;
  }
  return inv;
}

template <class Container>
inline void swap(Container &container, std::size_t i, std::size_t j) {
  std::swap(container[i], container[j]);
}

}

}

#endif // LPINTERFACE_INCLUDE_UTIL_H