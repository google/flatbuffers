#include "vector3d_pack.h"

// looks like VS10 does not support std::vector with explicit alignment
// From stackoverflow (https://stackoverflow.com/questions/8456236/how-is-a-vectors-data-aligned):
//
// Visual C++ version 2010 will not work with an std::vector with classes whose alignment are specified.
// The reason is std::vector::resize.
// When compiling, next error appears:
// c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\include\vector(870): error C2719: 
//  '_Val': formal parameter with __declspec(align('8')) won't be aligned [C:\projects\flatbuffers\flattests.vcxproj]
#if !defined(_MSC_VER) || _MSC_VER >= 1700

#include "native_type_test_generated.h"

namespace flatbuffers {
  Geometry::Vector3D Pack(const Native::Vector3D& obj) {
    return Geometry::Vector3D(obj.x, obj.y, obj.z);
  }

  const Native::Vector3D UnPack(const Geometry::Vector3D& obj) {
    return Native::Vector3D(obj.x(), obj.y(), obj.z());
  }
}

#endif
