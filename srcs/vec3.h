#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

class vec3 {
    public:
        double e[3];
        vec3() : e{0,0,0} {}
        vec3(double e0, double e1, double e2): e{e0, e1, e2} {}

        double x() const {return e[0];}
        double y() const {return e[1];}
        double z() const {return e[2];}

        vec3 operator-() const {return vec3(-e[0], -e[1], -e[2]);}

        double operator[](int i) const{return e[i];}
        double& operator[](int i){return e[i];}

        vec3& operator+=(const vec3& x){
            e[0]+=x.e[0];
            e[1]+=x.e[1];
            e[2]+=x.e[2];
            return *this;
        }

        vec3& operator*=(const vec3& x){
            e[0]*=x.e[0];
            e[1]*=x.e[1];
            e[2]*=x.e[2];
            return *this;
        }


        vec3& operator/=(const vec3& x){
            return *this*= 1/x;
        }

        double length() const {
            return std::sqrt(length_squared);
        }

        double length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
        }

    // Vector Utility Functions

    inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
        return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
    }

    inline vec3 operator+(const vec3& x, const vec3& y ){
        return vec3(x.e[0] + y.e[0], x.e[1] + y.e[1], x.e[2] + y.e[2]);
    }
        
     inline vec3 operator*(const vec3& x, const vec3& y ){
        return vec3(x.e[0] * y.e[0], x.e[1] * y.e[1], x.e[2] * y.e[2]);
    }
        
     inline vec3 operator-(const vec3& x, const vec3& y ){
        return vec3(x.e[0] - y.e[0], x.e[1] - y.e[1], x.e[2] - y.e[2]);
    }
        
     inline vec3 operator*(const vec3& x, double t){
        return vec3(x.e[0] * t, x.e[1] * t, x.e[2] * t);
    }

inline vec3 operator/(const vec3& v, double t) {
    return (1/t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

#endif
}