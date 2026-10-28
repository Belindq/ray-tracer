#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

using std::make_shared;
using std::shared_ptr;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

// Common headers. These keep their own dependencies to avoid include cycles.
#include "colour.h"
#include "ray.h"
#include "vec3.h"

#endif
