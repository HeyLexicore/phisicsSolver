#pragma once
#include "OBJ_Loader.h"
#include <cmath>


class Vec3{
  public:
    double x,y,z;
    Vec3(): x(0),y(0),z(0) {};
    Vec3(objl::Vertex Vert) : x(Vert.Position.X), y(Vert.Position.Y), z(Vert.Position.Z) {};
    Vec3(double x, double y, double z): x(x),y(y),z(z) {};
    Vec3(double x, double y): x(x),y(y),z(0) {};
    Vec3 operator-() const { return Vec3(-x,-y,-z); };
    
    Vec3 operator+=(const Vec3& v){
      x+=v.x;
      y+=v.y;
      z+=v.z;
      return *this;
    }
    Vec3 operator*=(double t){
      x*=t;
      y*=t;
      z*=t;
      return *this;
    }

    Vec3 operator*=(const Vec3& v){
      x*=v.x;
      y*=v.y;
      z*=v.z;
      return *this;
    }

    Vec3 operator/=(double t){
      return *this *= 1/t;
    }

    double length() const {
      return std::sqrt(x*x+y*y+z*z);
    }
    double length_squared() const {
      return x*x+y*y+z*z;
    }

    bool near_zero() const {
        // Return true if the vector is close to zero in all dimensions.
        auto s = 1e-8;
        return (std::fabs(x) < s) && (std::fabs(y) < s) && (std::fabs(z) < s);
    }
    
  

};

inline Vec3 operator+(const Vec3& u, const Vec3& v){
  return Vec3(u.x+v.x,u.y+v.y,u.z+v.z);
}
inline Vec3 operator-(const Vec3& u, const Vec3& v){
  return Vec3(u.x-v.x,u.y-v.y,u.z-v.z);
}
inline Vec3 operator*(const Vec3& u, const Vec3& v){
  return Vec3(u.x*v.x,u.y*v.y,u.z*v.z);
}

inline Vec3 operator*(const Vec3& v, double t){
  return v * Vec3(t,t,t);
}
inline Vec3 operator*(double t, const Vec3& v){
  return v * Vec3(t,t,t);
}
inline Vec3 operator/(const Vec3& v, double t){
  return v * (1/t);
}



inline double dot(const Vec3& u, const Vec3& v) {
    return u.x * v.x
         + u.y * v.y
         + u.z * v.z;
}

inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

inline Vec3 unit_vector(const Vec3& v) {
    return v / v.length();
}

