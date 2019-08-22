#ifndef LPINTERFACE_LPUTIL_SOPLEX_H
#define LPINTERFACE_LPUTIL_SOPLEX_H

#include "soplex.h"

namespace lpint {

namespace detail {

const double* begin(const soplex::DVector& v) {
    return v.get_const_ptr();
}

const double* end(const soplex::DVector& v) {
    return v.get_const_ptr() + v.dim();
}

}

}

#endif // LPINTERFACE_LPUTIL_SOPLEX_H