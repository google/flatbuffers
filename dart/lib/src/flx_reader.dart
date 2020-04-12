import 'dart:collection';
import 'dart:convert';
import 'dart:typed_data';
import 'flx_types.dart';

/// Main class to read a value out of a FlexBuffer.
class FlxValue {
  final ByteData _buffer;
  final int _offset;
  final BitWidth _parentWidth;
  int _byteWidth;
  ValueType _valueType;
  int _length;

  FlxValue._(this._buffer, this._offset, this._parentWidth, int packedType) {
    _byteWidth = 1 << (packedType & 3);
    _valueType = ValueTypeUtils.fromInt(packedType >> 2);
  }

  /// Use this method to access the root value of a FlexBuffer.
  static FlxValue fromBuffer(ByteBuffer buffer) {
    var len = buffer.lengthInBytes;
    if (len < 3) {
      throw Exception('Buffer needs to be bigger than 3');
    }
    var byteData = ByteData.view(buffer);
    var byteWidth = byteData.getUint8(len - 1);
    var packedType = byteData.getUint8(len - 2);
    var offset = len - byteWidth - 2;
    return FlxValue._(ByteData.view(buffer), offset, BitWidthUtil.fromByteWidth(byteWidth), packedType);
  }

  bool get isNull => _valueType == ValueType.Null;
  bool get isNum => ValueTypeUtils.isNumber(_valueType) || ValueTypeUtils.isIndirectNumber(_valueType);
  bool get isDouble => _valueType == ValueType.Float || _valueType == ValueType.IndirectFloat;
  bool get isInt => isNum && !isDouble;
  bool get isString => _valueType == ValueType.String || _valueType == ValueType.Key;
  bool get isBool => _valueType == ValueType.Bool;
  bool get isBlob => _valueType == ValueType.Blob;
  bool get isVector => ValueTypeUtils.isAVector(_valueType);
  bool get isMap => _valueType == ValueType.Map;

  /// Returns [bool] value or null otherwise.
  bool get boolValue {
    if(_valueType == ValueType.Bool) {
      return _readInt(_offset, _parentWidth) != 0;
    }
    return null;
  }

  /// Returns [int] value or null otherwise.
  int get intValue {
    if (_valueType == ValueType.Int) {
      return _readInt(_offset, _parentWidth);
    }
    if (_valueType == ValueType.UInt) {
      return _readUInt(_offset, _parentWidth);
    }
    if (_valueType == ValueType.IndirectInt) {
      return _readInt(_indirect, BitWidthUtil.fromByteWidth(_byteWidth));
    }
    if (_valueType == ValueType.IndirectUInt) {
      return _readUInt(_indirect, BitWidthUtil.fromByteWidth(_byteWidth));
    }
    return null;
  }

  /// Returns [double] value or null otherwise.
  double get doubleValue {
    if (_valueType == ValueType.Float) {
      return _readFloat(_offset, _parentWidth);
    }
    if (_valueType == ValueType.IndirectFloat) {
      return _readFloat(_indirect, BitWidthUtil.fromByteWidth(_byteWidth));
    }
    return null;
  }

  /// Returns [num] value or null otherwise.
  num get numValue {
    if (isDouble) {
      return doubleValue;
    }
    return intValue;
  }

  /// Returns [String] value or null otherwise.
  String get stringValue {
    if (_valueType == ValueType.String || _valueType == ValueType.Key) {
      return utf8.decode(_buffer.buffer.asUint8List(_indirect, length));
    }
    return null;
  }

  /// Returns [Uint8List] value or null otherwise.
  Uint8List get blobValue {
    if (_valueType == ValueType.Blob) {
      return _buffer.buffer.asUint8List(_indirect, length);
    }
    return null;
  }

  /// Can be used with an [int] or a [String] value for key.
  /// If the underlying value in FlexBuffer is a vector, then use [int] for access.
  /// If the underlying value in FlexBuffer is a map, then use [String] for access.
  /// Returns [FlxValue] value or null. Does not throw out of bounds exception.
  FlxValue operator [](Object key) {
    if (key is int && ValueTypeUtils.isAVector(_valueType)) {
      var index = key;
      if(index >= length) {
        return null;
      }
      var elementOffset = _indirect + index * _byteWidth;
      var flx = FlxValue._(_buffer, elementOffset, BitWidthUtil.fromByteWidth(_byteWidth), 0);
      flx._byteWidth = 1;
      if (ValueTypeUtils.isTypedVector(_valueType)) {
        flx._valueType = ValueTypeUtils.typedVectorElementType(_valueType);
        return flx;
      }
      if(ValueTypeUtils.isFixedTypedVector(_valueType)) {
        flx._valueType = ValueTypeUtils.fixedTypedVectorElementType(_valueType);
        return flx;
      }
      var packedType = _buffer.getUint8(_indirect + length * _byteWidth + index);
      return FlxValue._(_buffer, elementOffset, BitWidthUtil.fromByteWidth(_byteWidth), packedType);
    }
    if (key is String && _valueType == ValueType.Map) {
      var index = _keyIndex(key);
      if (index != null) {
        return _valueForIndex(index);
      }
    }
    return null;
  }

  /// Get an iterable if the underlying flexBuffer value is a vector.
  /// Otherwise throws an exception.
  Iterable<FlxValue> get vectorIterable {
    if(isVector == false) {
      throw Exception('Value is not a vector. It is: ${_valueType}');
    }
    return _VectorIterator(this);
  }

  /// Get an iterable for keys if the underlying flexBuffer value is a map.
  /// Otherwise throws an exception.
  Iterable<String> get mapKeyIterable {
    if(isMap == false) {
      throw Exception('Value is not a map. It is: ${_valueType}');
    }
    return _MapKeyIterator(this);
  }

  /// Get an iterable for values if the underlying flexBuffer value is a map.
  /// Otherwise throws an exception.
  Iterable<FlxValue> get mapValueIterable {
    if(isMap == false) {
      throw Exception('Value is not a map. It is: ${_valueType}');
    }
    return _MapValueIterator(this);
  }

  /// Returns the length of the the underlying FlexBuffer value.
  /// If the underlying value is [null] the length is 0.
  /// If the underlying value is a number, or a bool, the length is 1.
  /// If the underlying value is a vector, or map, the length reflects number of elements / element pairs.
  /// If the values is a string or a blob, the length reflects a number of bytes the value occupies (strings are encoded in utf8 format).
  int get length {
    if (_length != null) {
      return _length;
    }
    // needs to be checked before more generic isAVector
    if(ValueTypeUtils.isFixedTypedVector(_valueType)) {
      _length = ValueTypeUtils.fixedTypedVectorElementSize(_valueType);
    } else if(_valueType == ValueType.Blob || ValueTypeUtils.isAVector(_valueType) || _valueType == ValueType.Map){
      _length = _readInt(_indirect - _byteWidth, BitWidthUtil.fromByteWidth(_byteWidth));
    } else if (_valueType == ValueType.Null) {
      _length = 0;
    } else if (_valueType == ValueType.String) {
      var indirect = _indirect;
      var size_byte_width = _byteWidth;
      var size = _readInt(indirect - size_byte_width, BitWidthUtil.fromByteWidth(size_byte_width));
      while (_buffer.getInt8(indirect + size) != 0) {
        size_byte_width <<= 1;
        size = _readInt(indirect - size_byte_width, BitWidthUtil.fromByteWidth(size_byte_width));
      }
      _length = size;
    } else if (_valueType == ValueType.Key) {
      var indirect = _indirect;
      var size = 1;
      while (_buffer.getInt8(indirect + size) != 0) {
        size += 1;
      }
      _length = size;
    } else {
      _length = 1;
    }
    return _length;
  }

  /// Returns a minified JSON representation of the underlying FlexBuffer value.
  /// Primary use should be debugging.
  /// Blob values are represented as base64 encoded string.
  String get json {
    if(_valueType == ValueType.Bool) {
      return boolValue ? 'true' : 'false';
    }
    if (_valueType == ValueType.Null) {
      return 'null';
    }
    if(ValueTypeUtils.isNumber(_valueType)) {
      return jsonEncode(numValue);
    }
    if (_valueType == ValueType.String) {
      return jsonEncode(stringValue);
    }
    if (_valueType == ValueType.Blob) {
      return jsonEncode(base64Encode(blobValue));
    }
    if (ValueTypeUtils.isAVector(_valueType)) {
      var result = StringBuffer();
      result.write('[');
      for (var i = 0; i < length; i++) {
        result.write(this[i].json);
        if (i < length - 1) {
          result.write(',');
        }
      }
      result.write(']');
      return result.toString();
    }
    if (_valueType == ValueType.Map) {
      var result = StringBuffer();
      result.write('{');
      for (var i = 0; i < length; i++) {
        result.write(jsonEncode(_keyForIndex(i)));
        result.write(':');
        result.write(_valueForIndex(i).json);
        if (i < length - 1) {
          result.write(',');
        }
      }
      result.write('}');
      return result.toString();
    }
    return null;
  }

  int get _indirect {
    var step = _readInt(_offset, _parentWidth);
    return _offset - step;
  }

  int _readInt(int offset, BitWidth width) {
    _validateOffset(offset, width);
    if (width == BitWidth.width8) {
      return _buffer.getInt8(offset);
    }
    if (width == BitWidth.width16) {
      return _buffer.getInt16(offset, Endian.little);
    }
    if (width == BitWidth.width32) {
      return _buffer.getInt32(offset, Endian.little);
    }
    return _buffer.getInt64(offset, Endian.little);
  }

  int _readUInt(int offset, BitWidth width) {
    _validateOffset(offset, width);
    if (width == BitWidth.width8) {
      return _buffer.getUint8(offset);
    }
    if (width == BitWidth.width16) {
      return _buffer.getUint16(offset, Endian.little);
    }
    if (width == BitWidth.width32) {
      return _buffer.getUint32(offset, Endian.little);
    }
    return _buffer.getUint64(offset, Endian.little);
  }

  double _readFloat(int offset, BitWidth width) {
    _validateOffset(offset, width);
    if (width.index < BitWidth.width32.index) {
      throw Exception('Bad width: ${width}');
    }

    if (width == BitWidth.width32) {
      return _buffer.getFloat32(offset, Endian.little);
    }

    return _buffer.getFloat64(offset, Endian.little);
  }

  void _validateOffset(int offset, BitWidth width) {
    if (_offset < 0 || _buffer.lengthInBytes <= offset + width.index || offset & (BitWidthUtil.toByteWidth(width) - 1) != 0) {
      throw Exception('Bad offset');
    }
  }

  int _keyIndex(String key) {
    var input = utf8.encode(key);
    var keysVectorOffset = _indirect - _byteWidth * 3;
    var indirectOffset = keysVectorOffset - _readInt(keysVectorOffset, BitWidthUtil.fromByteWidth(_byteWidth));
    var byteWidth = _readInt(keysVectorOffset + _byteWidth, BitWidthUtil.fromByteWidth(_byteWidth));
    var low = 0;
    var high = length - 1;
    while(low <= high) {
      var mid = (high + low) >> 1;
      var dif = _difKeys(input, mid, indirectOffset, byteWidth);
      if (dif == 0) return mid;
      if (dif < 0) {
        high = mid - 1;
      } else {
        low = mid + 1;
      }
    }
    return null;
  }

  int _difKeys(List<int> input, int index, int indirect_offset, int byteWidth) {
    var keyOffset = indirect_offset + index * byteWidth;
    var keyIndirectOffset = keyOffset - _readInt(keyOffset, BitWidthUtil.fromByteWidth(byteWidth));
    for(var i = 0; i < input.length; i++) {
      var dif = input[i] - _buffer.getUint8(keyIndirectOffset + i);
      if (dif != 0) {
        return dif;
      }
    }
    return (_buffer.getUint8(keyIndirectOffset + input.length) == 0) ? 0 : -1;
  }

  FlxValue _valueForIndex(int index) {
    var indirect = _indirect;
    var elemOffset = indirect + index * _byteWidth;
    var packedType = _buffer.getUint8(indirect + length * _byteWidth + index);
    return FlxValue._(_buffer, elemOffset, BitWidthUtil.fromByteWidth(_byteWidth), packedType);
  }

  String _keyForIndex(int index) {
    var keysVectorOffset = _indirect - _byteWidth * 3;
    var indirectOffset = keysVectorOffset - _readInt(keysVectorOffset, BitWidthUtil.fromByteWidth(_byteWidth));
    var byteWidth = _readInt(keysVectorOffset + _byteWidth, BitWidthUtil.fromByteWidth(_byteWidth));
    var keyOffset = indirectOffset + index * byteWidth;
    var keyIndirectOffset = keyOffset - _readInt(keyOffset, BitWidthUtil.fromByteWidth(byteWidth));
    var length = 0;
    while (_buffer.getUint8(keyIndirectOffset + length) != 0) {
      length += 1;
    }
    return utf8.decode(_buffer.buffer.asUint8List(keyIndirectOffset, length));
  }

}

class _VectorIterator with IterableMixin<FlxValue> implements Iterator<FlxValue> {
  final FlxValue _vector;
  int index;

  _VectorIterator(this._vector) {
    index = -1;
  }

  @override
  FlxValue get current => _vector[index];

  @override
  bool moveNext() {
    index++;
    return index < _vector.length;
  }

  @override
  Iterator<FlxValue> get iterator => this;
}

class _MapKeyIterator with IterableMixin<String> implements Iterator<String> {
  final FlxValue _map;
  int index;

  _MapKeyIterator(this._map) {
    index = -1;
  }

  @override
  String get current => _map._keyForIndex(index);

  @override
  bool moveNext() {
    index++;
    return index < _map.length;
  }

  @override
  Iterator<String> get iterator => this;
}

class _MapValueIterator with IterableMixin<FlxValue> implements Iterator<FlxValue> {
  final FlxValue _map;
  int index;

  _MapValueIterator(this._map) {
    index = -1;
  }

  @override
  FlxValue get current => _map._valueForIndex(index);

  @override
  bool moveNext() {
    index++;
    return index < _map.length;
  }

  @override
  Iterator<FlxValue> get iterator => this;
}
