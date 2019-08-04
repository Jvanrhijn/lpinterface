#ifndef LPINTERFACE_COMMON_H
#define ifndef LPINTERFACE_COMMON_H

std::size_t vectorize_indices(const std::size_t i, const std::size_t j,
                              const std::size_t stride) {
  return j + i * stride;
}

template <typename T>
std::vector<std::vector<T>> vector_to_matrix(const std::vector<T>& v) {
  const std::size_t nsquared = v.size();
  const std::size_t n = std::round(std::sqrt(nsquared));
  std::vector<std::vector<T>> m(n, std::vector<T>(n, 0));
  for (auto i = 0; i < n; i++) {
    for (auto j = 0; j < n; j++) {
      m[i][j] = v[vectorize_indices(i, j, n)];
    }
  }
  return m;
}

template <typename T>
void print_matrix(const std::vector<std::vector<T>>& m) {
  const auto n = m.size();
  for (auto i = 0; i < n; i++) {
    for (auto j = 0; j < n; j++) {
      std::cout << m[i][j] << " ";
    }
    std::cout << "\n";
  }
}

template <typename T>
void print_vector(const std::vector<T>& v) {
  if (v.size() == 0) {
    std::cout << "[]";
    return;
  }
  std::cout << "[";
  for (const auto& el : v) {
    std::cout << el << ", ";
  }
  std::cout << "\b\b]";
}

#endif // LPINTERFACE_COMMON_H
