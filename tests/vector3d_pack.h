#ifndef VECTOR3D_PACK_H
#define VECTOR3D_PACK_H

#include "vector3d.h"

namespace Geometry { 
  struct Vector3D;
}

namespace flatbuffers {
  Geometry::Vector3D Pack(const Native::Vector3D& obj);
  const Native::Vector3D UnPack(const Geometry::Vector3D& obj);
}

#endif // VECTOR3D_PACK_H
