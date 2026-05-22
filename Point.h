
#pragma once
#include "Vec3.h"
#include <vector>

class Point{
  private:
    Vec3 acc;
    Vec3 forceAcc;

    double mass;

    Vec3 force2acc(Vec3 force){
      return force/mass;
    }

  public:
    Vec3 vel;
    Vec3 pos;
    Point();
    Point(Vec3 pos, double mass):  mass(mass), pos(pos) {};
    
    void applyForce(Vec3 force){
      forceAcc += force; 
    }
    void applyImpulse(Vec3 imp){
      vel += imp/mass;
    }

    void tickTime(double dt){
      acc = forceAcc/mass;
      vel += acc*dt;
      pos += vel*dt;

      forceAcc = Vec3();
    }

};
