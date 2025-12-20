#ifndef NATIVE_TYPE_TEST_IMPL_H
#define NATIVE_TYPE_TEST_IMPL_H

#include <vector>

namespace Native {
struct Vector3D {
  float x;
  float y;
  float z;

  Vector3D() {
    x = 0;
    y = 0;
    z = 0;
  }
  Vector3D(float _x, float _y, float _z) {
    this->x = _x;
    this->y = _y;
    this->z = _z;
  }

  bool operator==(const Vector3D& other) const {
    return (x == other.x) && (y == other.y) && (z == other.z);
  }
};

struct Matrix {
  int rows;
  int columns;
  std::vector<float> values;

  Matrix() : Matrix(0, 0) {}

  Matrix(int _rows, int _columns) {
    this->rows = _rows;
    this->columns = _columns;
    values.resize(_rows * _columns);
  }

  bool operator==(const Matrix& other) const {
    return (rows == other.rows) && (columns == other.columns) &&
           (values == other.values);
  }
};
}  // namespace Native

namespace Geometry {
struct Vector3D;
struct Vector3DAlt;
}  // namespace Geometry

namespace flatbuffers {
Geometry::Vector3D Pack(const Native::Vector3D& obj);
const Native::Vector3D UnPack(const Geometry::Vector3D& obj);
Geometry::Vector3DAlt PackVector3DAlt(const Native::Vector3D& obj);
const Native::Vector3D UnPackVector3DAlt(const Geometry::Vector3DAlt& obj);
}  // namespace flatbuffers

#endif  // VECTOR3D_PACK_H
