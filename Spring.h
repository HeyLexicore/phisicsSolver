#pragma once
#include "Vec3.h"
#include "Point.h"

class Spring{
  private:

    
    double k;
    double restLength;
    double damping;
  public:
    Point* P1;
    Point* P2;

    Spring(Point* p1, Point* p2, double k = 10.0, double restLen = 2.0, double damp = 0.5)
        :  k(k), restLength(restLen), damping(damp), P1(p1), P2(p2) {}

    // Call this every physics update (before or after integrating velocity/position)
    void update() {
        if (!P1 || !P2) return;

        Vec3 delta = P1->pos - P2->pos; 
        double currentLength = delta.length();

        if (currentLength < 0.0001) return;

        double displacement = currentLength - restLength;
        Vec3 forceDirection = delta / currentLength;

        Vec3 springForce = forceDirection * (-k * displacement);

        Vec3 relativeVel = P1->vel - P2->vel;
        double dampingForce = -damping * dot(relativeVel,forceDirection);

        Vec3 totalForce = springForce + (forceDirection * dampingForce);

        P1->applyForce(totalForce);
        P2->applyForce(-totalForce);
    }

};
