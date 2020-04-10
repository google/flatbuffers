import 'dart:typed_data';

enum BitWidth {
  width8,
  width16,
  width32,
  width64
}

extension BitWidthUtil on BitWidth {
  int toByteWidth() {
    return 1 << index;
  }
  static BitWidth width(num value) {
    if (value.toInt() == value) {
      var v = value.toInt().abs();
      if (v >> 7 == 0) return BitWidth.width8;
      if (v >> 15 == 0) return BitWidth.width16;
      if (v >> 31 == 0) return BitWidth.width32;
      return BitWidth.width64;
    }
    return value == _toF32(value) ? BitWidth.width32 : BitWidth.width64;
  }
  static BitWidth fromByteWidth(int value) {
    if (value == 1) {
      return BitWidth.width8;
    }
    if (value == 2) {
      return BitWidth.width16;
    }
    if (value == 4) {
      return BitWidth.width32;
    }
    if (value == 8) {
      return BitWidth.width64;
    }
    throw Exception('Unexpected value ${value}');
  }
  static int paddingSize(int bufSize, int scalarSize) {
    return (~bufSize + 1) & (scalarSize - 1);
  }
  static double _toF32(double value) {
    var bdata = ByteData(4);
    bdata.setFloat32(0, value);
    return bdata.getFloat32(0);
  }

  BitWidth max(BitWidth other) {
    if (index < other.index) {
      return other;
    }
    return this;
  }
}

enum ValueType {
  Null, Int, UInt, Float,
  Key, String, IndirectInt, IndirectUInt, IndirectFloat,
  Map, Vector, VectorInt, VectorUInt, VectorFloat, VectorKey, VectorString_DEPRECATED,
  VectorInt2, VectorUInt2, VectorFloat2,
  VectorInt3, VectorUInt3, VectorFloat3,
  VectorInt4, VectorUInt4, VectorFloat4,
  Blob, Bool, VectorBool
}

extension ValueTypeUtils on ValueType {
  int toInt() {
    if (this == ValueType.VectorBool) return 36;
    return index;
  }

  static ValueType fromInt(int value) {
    if (value == 36) return ValueType.VectorBool;
    return ValueType.values[value];
  }

  bool isInline() {
    return this == ValueType.Bool
        || toInt() <= ValueType.Float.toInt();
  }

  bool isNumber() {
    return toInt() >= ValueType.Int.toInt()
        && toInt() <= ValueType.Float.toInt();
  }

  bool isIndirectNumber() {
    return toInt() >= ValueType.IndirectInt.toInt()
        && toInt() <= ValueType.IndirectFloat.toInt();
  }

  bool isTypedVectorElement() {
    return this == ValueType.Bool ||
        (
            toInt() >= ValueType.Int.toInt()
            && toInt() <= ValueType.String.toInt()
        );
  }

  bool isTypedVector() {
    return this == ValueType.VectorBool ||
        (
          toInt() >= ValueType.VectorInt.toInt()
              && toInt() <= ValueType.VectorString_DEPRECATED.toInt()
        );
  }

  bool isFixedTypedVector() {
    return (
            toInt() >= ValueType.VectorInt2.toInt()
                && toInt() <= ValueType.VectorFloat4.toInt()
        );
  }

  bool isAVector() {
    return (
        isTypedVector() || isFixedTypedVector() || this == ValueType.Vector
    );
  }

  ValueType toTypedVector(int length) {
    if (length == 0) {
      return ValueTypeUtils.fromInt(toInt() - ValueType.Int.toInt() + ValueType.VectorInt.toInt());
    }
    if (length == 2) {
      return ValueTypeUtils.fromInt(toInt() - ValueType.Int.toInt() + ValueType.VectorInt2.toInt());
    }
    if (length == 3) {
      return ValueTypeUtils.fromInt(toInt() - ValueType.Int.toInt() + ValueType.VectorInt3.toInt());
    }
    if (length == 4) {
      return ValueTypeUtils.fromInt(toInt() - ValueType.Int.toInt() + ValueType.VectorInt4.toInt());
    }
    throw Exception('unexpected length ' + length.toString());
  }

  ValueType typedVectorElementType() {
    return ValueTypeUtils.fromInt(toInt() - ValueType.VectorInt.toInt() + ValueType.Int.toInt());
  }

  ValueType fixedTypedVectorElementType() {
    return ValueTypeUtils.fromInt((toInt() - ValueType.VectorInt2.toInt()) % 3 + ValueType.Int.toInt());
  }

  int fixedTypedVectorElementSize() {
    return (toInt() - ValueType.VectorInt2.toInt()) ~/ 3 + 2;
  }

  int packedType(BitWidth bitWidth) {
    return bitWidth.index | (toInt() << 2);
  }
}