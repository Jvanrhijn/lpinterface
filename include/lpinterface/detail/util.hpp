#ifndef LPINTERFACE_INCLUDE_UTIL_H
#define LPINTERFACE_INCLUDE_UTIL_H

#include <vector>

namespace lpint {

namespace detail {

/**
 * @brief Takes a vector of numbers and gives them a rank.
 * The output vector will contain a ranking of the input vector.
 * output[i] = j means that list[i] is the jth largest number
 * in the input vector.
 *
 * @tparam T Number type.
 * @param list List of numbers to rank.
 * @return std::vector<T> Ranks of elements of input list.
 */
template <class T>
std::vector<T> rankify(const std::vector<T>& list) {
  std::vector<T> out(list.size(), T(0));
  for (std::size_t i = 0; i < list.size(); i++) {
    for (std::size_t j = 0; j < list.size(); j++) {
      out[i] += list[j] < list[i] ? T(1) : T(0);
    }
  }
  return out;
}

/**
 * @brief Computes the inverse of the given permutation.
 *
 * @param permutation A permutation vector.
 * @return std::vector<std::size_t> The inverse of the given permutation.
 */
inline std::vector<std::size_t> inverse_permutation(
    const std::vector<std::size_t>& permutation) {
  std::vector<std::size_t> inv(permutation.size(), 0);
  for (std::size_t i = 0; i < permutation.size(); i++) {
    inv[permutation[i]] = i;
  }
  return inv;
}

}  // namespace detail

}  // namespace lpint

#endif  // LPINTERFACE_INCLUDE_UTIL_H