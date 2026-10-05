#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
    public:
    ray (){}

    ray(const vec3& origin, const vec3& direction): origin(origin), direction(direction){}

    const point3& origin() const {
        return origin;
    
    }

    const vec3& direction() const{
        return direction;

    }

    point3 at( double t) const{
        return origin+direction*t;

    }

    private:
        point3 origin;
        vec3 direction;
}
#endif
