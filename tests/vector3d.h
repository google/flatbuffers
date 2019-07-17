#ifndef VECTOR3D_H
#define VECTOR3D_H

namespace Native {
  struct Vector3D {
    double x;
    double y;
    double z;

    Vector3D() { x = 0; y = 0; z = 0; };
    Vector3D(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }
  };
}

#endif // VECTOR3D_H
