#ifndef NATIVE_TYPE_TEST_IMPL_H
#define NATIVE_TYPE_TEST_IMPL_H

namespace Native {
  struct Vector3D {
    double x;
    double y;
    double z;

    Vector3D() { x = 0; y = 0; z = 0; };
    Vector3D(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }
  };
}

namespace Geometry { 
  struct Vector3D;
}

namespace flatbuffers {
  Geometry::Vector3D Pack(const Native::Vector3D &obj);
  const Native::Vector3D UnPack(const Geometry::Vector3D &obj);
}

#endif // VECTOR3D_PACK_H
