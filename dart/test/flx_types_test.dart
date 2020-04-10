import 'package:flat_buffers/src/flx_types.dart';
import 'package:test/test.dart';

void main() {
  test('is inline', () {
    expect(ValueType.Bool.isInline(), isTrue);
    expect(ValueType.Int.isInline(), isTrue);
    expect(ValueType.UInt.isInline(), isTrue);
    expect(ValueType.Float.isInline(), isTrue);
    expect(ValueType.Null.isInline(), isTrue);
    expect(ValueType.String.isInline(), isFalse);
  });
  test('is type vector element', () {
    expect(ValueType.Bool.isTypedVectorElement(), isTrue);
    expect(ValueType.Int.isTypedVectorElement(), isTrue);
    expect(ValueType.UInt.isTypedVectorElement(), isTrue);
    expect(ValueType.Float.isTypedVectorElement(), isTrue);
    expect(ValueType.Key.isTypedVectorElement(), isTrue);
    expect(ValueType.String.isTypedVectorElement(), isTrue);

    expect(ValueType.Null.isTypedVectorElement(), isFalse);
    expect(ValueType.Blob.isTypedVectorElement(), isFalse);
  });
  test('is typed vector', () {
    expect(ValueType.VectorInt.isTypedVector(), isTrue);
    expect(ValueType.VectorUInt.isTypedVector(), isTrue);
    expect(ValueType.VectorFloat.isTypedVector(), isTrue);
    expect(ValueType.VectorBool.isTypedVector(), isTrue);
    expect(ValueType.VectorKey.isTypedVector(), isTrue);
    expect(ValueType.VectorString_DEPRECATED.isTypedVector(), isTrue);

    expect(ValueType.Vector.isTypedVector(), isFalse);
    expect(ValueType.Map.isTypedVector(), isFalse);
    expect(ValueType.Bool.isTypedVector(), isFalse);
    expect(ValueType.VectorInt2.isTypedVector(), isFalse);
  });
  test('is fixed typed vector', () {
    expect(ValueType.VectorInt2.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorInt3.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorInt4.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorUInt2.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorUInt3.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorUInt4.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorFloat2.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorFloat3.isFixedTypedVector(), isTrue);
    expect(ValueType.VectorFloat4.isFixedTypedVector(), isTrue);

    expect(ValueType.VectorInt.isFixedTypedVector(), isFalse);
  });
  test('to typed vector', () {
    expect(ValueType.Int.toTypedVector(0), equals(ValueType.VectorInt));
    expect(ValueType.UInt.toTypedVector(0), equals(ValueType.VectorUInt));
    expect(ValueType.Bool.toTypedVector(0), equals(ValueType.VectorBool));
    expect(ValueType.Float.toTypedVector(0), equals(ValueType.VectorFloat));
    expect(ValueType.Key.toTypedVector(0), equals(ValueType.VectorKey));
    expect(ValueType.String.toTypedVector(0), equals(ValueType.VectorString_DEPRECATED));

    expect(ValueType.Int.toTypedVector(2), equals(ValueType.VectorInt2));
    expect(ValueType.UInt.toTypedVector(2), equals(ValueType.VectorUInt2));
    expect(ValueType.Float.toTypedVector(2), equals(ValueType.VectorFloat2));

    expect(ValueType.Int.toTypedVector(3), equals(ValueType.VectorInt3));
    expect(ValueType.UInt.toTypedVector(3), equals(ValueType.VectorUInt3));
    expect(ValueType.Float.toTypedVector(3), equals(ValueType.VectorFloat3));

    expect(ValueType.Int.toTypedVector(4), equals(ValueType.VectorInt4));
    expect(ValueType.UInt.toTypedVector(4), equals(ValueType.VectorUInt4));
    expect(ValueType.Float.toTypedVector(4), equals(ValueType.VectorFloat4));
  });
  test('typed vector element type', () {
    expect(ValueType.VectorInt.typedVectorElementType(), equals(ValueType.Int));
    expect(ValueType.VectorUInt.typedVectorElementType(), equals(ValueType.UInt));
    expect(ValueType.VectorFloat.typedVectorElementType(), equals(ValueType.Float));
    expect(ValueType.VectorString_DEPRECATED.typedVectorElementType(), equals(ValueType.String));
    expect(ValueType.VectorKey.typedVectorElementType(), equals(ValueType.Key));
    expect(ValueType.VectorBool.typedVectorElementType(), equals(ValueType.Bool));
  });
  test('fixed typed vector element type', () {
    expect(ValueType.VectorInt2.fixedTypedVectorElementType(), equals(ValueType.Int));
    expect(ValueType.VectorInt3.fixedTypedVectorElementType(), equals(ValueType.Int));
    expect(ValueType.VectorInt4.fixedTypedVectorElementType(), equals(ValueType.Int));

    expect(ValueType.VectorUInt2.fixedTypedVectorElementType(), equals(ValueType.UInt));
    expect(ValueType.VectorUInt3.fixedTypedVectorElementType(), equals(ValueType.UInt));
    expect(ValueType.VectorUInt4.fixedTypedVectorElementType(), equals(ValueType.UInt));

    expect(ValueType.VectorFloat2.fixedTypedVectorElementType(), equals(ValueType.Float));
    expect(ValueType.VectorFloat3.fixedTypedVectorElementType(), equals(ValueType.Float));
    expect(ValueType.VectorFloat4.fixedTypedVectorElementType(), equals(ValueType.Float));
  });
  test('fixed typed vector element size', () {
    expect(ValueType.VectorInt2.fixedTypedVectorElementSize(), equals(2));
    expect(ValueType.VectorInt3.fixedTypedVectorElementSize(), equals(3));
    expect(ValueType.VectorInt4.fixedTypedVectorElementSize(), equals(4));

    expect(ValueType.VectorUInt2.fixedTypedVectorElementSize(), equals(2));
    expect(ValueType.VectorUInt3.fixedTypedVectorElementSize(), equals(3));
    expect(ValueType.VectorUInt4.fixedTypedVectorElementSize(), equals(4));

    expect(ValueType.VectorFloat2.fixedTypedVectorElementSize(), equals(2));
    expect(ValueType.VectorFloat3.fixedTypedVectorElementSize(), equals(3));
    expect(ValueType.VectorFloat4.fixedTypedVectorElementSize(), equals(4));
  });
  test('packed type', () {
    expect(ValueType.Null.packedType(BitWidth.width8), equals(0));
    expect(ValueType.Null.packedType(BitWidth.width16), equals(1));
    expect(ValueType.Null.packedType(BitWidth.width32), equals(2));
    expect(ValueType.Null.packedType(BitWidth.width64), equals(3));

    expect(ValueType.Int.packedType(BitWidth.width8), equals(4));
    expect(ValueType.Int.packedType(BitWidth.width16), equals(5));
    expect(ValueType.Int.packedType(BitWidth.width32), equals(6));
    expect(ValueType.Int.packedType(BitWidth.width64), equals(7));
  });
  test('bit width', () {
    expect(BitWidthUtil.width(0), BitWidth.width8);
    expect(BitWidthUtil.width(-20), BitWidth.width8);
    expect(BitWidthUtil.width(127), BitWidth.width8);
    expect(BitWidthUtil.width(128), BitWidth.width16);
    expect(BitWidthUtil.width(128123), BitWidth.width32);
    expect(BitWidthUtil.width(12812324534), BitWidth.width64);
    expect(BitWidthUtil.width(-127), BitWidth.width8);
    expect(BitWidthUtil.width(-128), BitWidth.width16);
    expect(BitWidthUtil.width(-12812324534), BitWidth.width64);
    expect(BitWidthUtil.width(-0.1), BitWidth.width64);
    expect(BitWidthUtil.width(0.25), BitWidth.width32);
  });
  test('padding size', () {
    expect(BitWidthUtil.paddingSize(10, 8), 6);
    expect(BitWidthUtil.paddingSize(10, 4), 2);
    expect(BitWidthUtil.paddingSize(15, 4), 1);
    expect(BitWidthUtil.paddingSize(15, 2), 1);
    expect(BitWidthUtil.paddingSize(15, 1), 0);
    expect(BitWidthUtil.paddingSize(16, 8), 0);
    expect(BitWidthUtil.paddingSize(17, 8), 7);
  });
}
