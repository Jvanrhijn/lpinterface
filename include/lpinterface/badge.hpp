#ifndef LPINTERFACE_BADGE_H
#define LPINTERFACE_BADGE_H

namespace lpint {

namespace detail {

template <typename T>
class Badge {
  friend T;
  Badge() {}
};

}  // namespace detail

}  // namespace lpint

#endif  // LPINTERFACE_BADGE_H