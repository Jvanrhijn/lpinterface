#include "errors.hpp"
#include "expected.hpp"

namespace lpint {

template <typename T, typename E>
using expected = tl::expected<T, E>;

template <typename E>
using unexpected = tl::unexpected<E>;

}  // namespace lpint