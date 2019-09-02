#ifndef LPINTERFACE_UTIL_CONTAINER_H
#define LPINTERFACE_UTIL_CONTAINER_H

#include <algorithm>
#include <vector>

namespace lpint {

namespace detail {

template <class T, class Container>
inline bool contains_any_of(const std::vector<T>&& elements,
                            const Container& container) {
  for (const auto& element : elements) {
    if (std::find(container.begin(), container.end(), element) !=
        container.end()) {
      return true;
    }
  }
  return false;
}

}  // namespace detail

}  // namespace lpint

#endif  // LPINTERFACE_UTIL_CONTAINER_H