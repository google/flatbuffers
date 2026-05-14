#include "native_type_test_impl.h"

#include "native_type_test_generated.h"

namespace flatbuffers {
Geometry::Vector3D Pack(const Native::Vector3D& obj) {
  return Geometry::Vector3D(obj.x, obj.y, obj.z);
}

const Native::Vector3D UnPack(const Geometry::Vector3D& obj) {
  return Native::Vector3D(obj.x(), obj.y(), obj.z());
}

Geometry::Vector3DAlt PackVector3DAlt(const Native::Vector3D& obj) {
  return Geometry::Vector3DAlt(obj.x, obj.y, obj.z);
}

const Native::Vector3D UnPackVector3DAlt(const Geometry::Vector3DAlt& obj) {
  return Native::Vector3D(obj.a(), obj.b(), obj.c());
}
}  // namespace flatbuffers

namespace Geometry {
void Matrix::UnPackTo(
    Native::Matrix* _o,
    const ::flatbuffers::resolver_function_t* _resolver) const {
  (void)_resolver;

  auto _rows = rows();
  if (_rows) {
    _o->rows = _rows;
  }

  auto _columns = columns();
  if (_columns) {
    _o->columns = _columns;
  }

  auto _values = values();
  if (_values) {
    _o->values.resize(_values->size());
    for (::flatbuffers::uoffset_t i = 0; i < _values->size(); i++) {
      _o->values[i] = _values->Get(i);
    }
  }
}

::flatbuffers::Offset<Matrix> Matrix::Pack(
    ::flatbuffers::FlatBufferBuilder& _fbb, const Native::Matrix* _o,
    const ::flatbuffers::rehasher_function_t* _rehasher) {
  (void)_rehasher;

  return CreateMatrix(_fbb, _o->rows, _o->columns,
                      _fbb.CreateVector<float>(_o->values));
}
}  // namespace Geometry
