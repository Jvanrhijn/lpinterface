#ifndef BITMASK_H
#define BITMASK_H

#include <type_traits>

namespace bitmask {

template <class Enum>
struct EnableBitmaskOperators {
    static constexpr bool enable = false;
};

template <class Enum>
typename std::enable_if<EnableBitmaskOperators<Enum>::enable, Enum>::type
constexpr operator|(Enum lhs, Enum rhs) {
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum>(
        static_cast<underlying>(lhs) | static_cast<underlying>(rhs)
    );
}

template <class Enum>
typename std::enable_if<EnableBitmaskOperators<Enum>::enable, Enum>::type
constexpr operator&(Enum lhs, Enum rhs) {
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum>(
        static_cast<underlying>(lhs) & static_cast<underlying>(rhs)
    );
}

} // namespace bitmask

#endif // BITMASK_H