#ifndef LPINTERFACE_GENERATORS_H
#define LPINTERFACE_GENERATORS_H

#include "rapidcheck.h"
#include <iostream>

namespace rc {

template <typename T>
inline Gen<std::vector<T>> genUniqueVector(std::size_t size, Gen<T> gen) {
  return gen::suchThat(gen::unique<std::vector<T>>(gen),
        [&](std::vector<T> v) { return v.size() == size; });
}

template <typename T>
Gen<std::vector<T>> genVectorSized(std::size_t size, Gen<std::vector<T>> gen) {
  return gen::suchThat(gen, [&](std::vector<T> v) {
      return v.size() == size;
  });
}

template <>
struct Arbitrary<lpint::Ordering> {
    static Gen<lpint::Ordering> arbitrary() {
        return gen::element(
            lpint::Ordering::EQ,
            lpint::Ordering::GEQ,
            lpint::Ordering::GT,
            lpint::Ordering::LEQ,
            lpint::Ordering::LT
        );
    }
};

template <typename T>
struct Arbitrary<lpint::Constraint<T>> {
    static Gen<lpint::Constraint<T>> arbitrary() {
        return gen::build<lpint::Constraint<T>>(
            gen::set(&lpint::Constraint<T>::value),
            gen::set(&lpint::Constraint<T>::ordering)
        );
    }
};

template <>
struct Arbitrary<lpint::VarType> {
    static Gen<lpint::VarType> arbitrary() {
        return gen::element(
            lpint::VarType::Binary,
            lpint::VarType::Integer,
            lpint::VarType::Real,
            lpint::VarType::SemiInteger,
            lpint::VarType::SemiReal
        );
    }
};

template <typename T>
struct Arbitrary<lpint::Objective<T>> {
    static Gen<lpint::Objective<T>> arbitrary() {
        return gen::build<lpint::Objective<T>>(
            gen::set(&lpint::Objective<T>::values),
            gen::set(&lpint::Objective<T>::variable_types)
        );
    }
};

template <typename T>
struct Arbitrary<lpint::Row<T>> {
    static Gen<lpint::Row<T>> arbitrary() {
    }
};

}

#endif // LPINTERFACE_GENERATORS_H