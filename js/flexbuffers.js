var flexbuffers = {};

flexbuffers.BitWidth = {
  WIDTH8: 0,
  WIDTH16: 1,
  WIDTH32: 2,
  WIDTH64: 3,
};

flexbuffers.BitWidth.toByteWidth = (bitWidth) => {
  return 1 << bitWidth;
};

flexbuffers.BitWidth.width = (value) => {
  if (Number.isInteger(value)) {
    if (value > 0) {
      const v = value;
      if (v >> 8 === 0) return flexbuffers.BitWidth.WIDTH8;
      if (v >> 16 === 0) return flexbuffers.BitWidth.WIDTH16;
      if (v >> 32 === 0) return flexbuffers.BitWidth.WIDTH32;
      return flexbuffers.BitWidth.WIDTH64;
    } else {
      const v = value.abs(value);
      if (v >> 7 === 0) return flexbuffers.BitWidth.WIDTH8;
      if (v >> 15 === 0) return flexbuffers.BitWidth.WIDTH16;
      if (v >> 31 === 0) return flexbuffers.BitWidth.WIDTH32;
      return flexbuffers.BitWidth.WIDTH64;
    }
  }

  function _toF32(value) {
    const buffer = new ArrayBuffer(4);
    const view = new DataView(buffer);
    view.setFloat32(0, value, true);
    return view.getFloat32(0, true);
  }

  return value === _toF32(value) ? flexbuffers.BitWidth.WIDTH32: flexbuffers.BitWidth.WIDTH64;
};

flexbuffers.BitWidth.fromByteWidth = (value) => {
  if (value === 1) return flexbuffers.BitWidth.WIDTH8;
  if (value === 2) return flexbuffers.BitWidth.WIDTH16;
  if (value === 4) return flexbuffers.BitWidth.WIDTH32;
  if (value === 8) return flexbuffers.BitWidth.WIDTH64;
  throw "Unexpected value " + value;
};

flexbuffers.BitWidth.paddingSize = (bufSize, scalarSize) => {
  return (~bufSize + 1) & (scalarSize - 1);
};

flexbuffers.ValueType = {
  NULL: 0,
  INT: 1,
  UINT: 2,
  FLOAT: 3,
  KEY: 4,
  STRING: 5,
  INDIRECT_INT: 6,
  INDIRECT_UINT: 7,
  INDIRECT_FLOAT: 8,
  MAP: 9,
  VECTOR: 10,
  VECTOR_INT: 11,
  VECTOR_UINT: 12,
  VECTOR_FLOAT: 13,
  VECTOR_KEY: 14,
  VECTOR_STRING_DEPRECATED: 15,
  VECTOR_INT2: 16,
  VECTOR_UINT2: 17,
  VECTOR_FLOAT2: 18,
  VECTOR_INT3: 19,
  VECTOR_UINT3: 20,
  VECTOR_FLOAT3: 21,
  VECTOR_INT4: 22,
  VECTOR_UINT4: 23,
  VECTOR_FLOAT4: 24,
  BLOB: 25,
  BOOL: 26,
  VECTOR_BOOL: 36,
};

flexbuffers.ValueType.isInline = (value) => {
  return value === flexbuffers.ValueType.BOOL
    || value <= flexbuffers.ValueType.FLOAT;
};

flexbuffers.ValueType.isNumber = (value) => {
  return value >= flexbuffers.ValueType.INT
    && value <= flexbuffers.ValueType.FLOAT;
};

flexbuffers.ValueType.isIndirectNumber = (value) => {
  return value >= flexbuffers.ValueType.INDIRECT_INT
    && value <= flexbuffers.ValueType.INDIRECT_FLOAT;
};

flexbuffers.ValueType.isTypedVectorElement = (value) => {
  return value === flexbuffers.ValueType.BOOL
    || (value >= flexbuffers.ValueType.INT
      && value <= flexbuffers.ValueType.STRING);
};

flexbuffers.ValueType.isTypedVector = (value) => {
  return value === flexbuffers.ValueType.VECTOR_BOOL
    || (value >= flexbuffers.ValueType.VECTOR_INT
      && value <= flexbuffers.ValueType.VECTOR_STRING_DEPRECATED);
};

flexbuffers.ValueType.isFixedTypedVector = (value) => {
  return value >= flexbuffers.ValueType.VECTOR_INT2
    && value <= flexbuffers.ValueType.VECTOR_FLOAT4;
};

flexbuffers.ValueType.isAVector = (value) => {
  return flexbuffers.ValueType.isTypedVector(value)
    || flexbuffers.ValueType.isFixedTypedVector(value)
    || value === flexbuffers.ValueType.VECTOR;
};

flexbuffers.ValueType.toTypedVector = (valueType, length) => {
  if (length === 0) return valueType - flexbuffers.ValueType.INT + flexbuffers.ValueType.VECTOR_INT;
  if (length === 2) return valueType - flexbuffers.ValueType.INT + flexbuffers.ValueType.VECTOR_INT2;
  if (length === 3) return valueType - flexbuffers.ValueType.INT + flexbuffers.ValueType.VECTOR_INT3;
  if (length === 4) return valueType - flexbuffers.ValueType.INT + flexbuffers.ValueType.VECTOR_INT4;
  throw "Unexpected length " + length;
};

flexbuffers.ValueType.typedVectorElementType = (valueType) => {
  return valueType - flexbuffers.ValueType.VECTOR_INT + flexbuffers.ValueType.INT;
};

flexbuffers.ValueType.fixedTypedVectorElementType = (valueType) => {
  return ((valueType - flexbuffers.ValueType.VECTOR_INT2) % 3) + flexbuffers.ValueType.INT;
};

flexbuffers.ValueType.fixedTypedVectorElementSize = (valueType) => {
  // The x / y >> 0 trick is to have an int division. Suppose to be faster than Math.floor()
  return (((valueType - flexbuffers.ValueType.VECTOR_INT2) / 3) >> 0)  + 2;
};

flexbuffers.ValueType.packedType = (valueType, bitWidth) => {
  return bitWidth | (valueType << 2);
};

flexbuffers.read = (buffer) => {

  function validateOffset(dataView, offset, width) {
    if (dataView.byteLength <= offset + width || offset & (flexbuffers.BitWidth.toByteWidth(width) - 1) !== 0) {
      throw "Bad offset: " + offset + ", width: " + width;
    }
  }

  function readInt(dataView, offset, width) {
    validateOffset(dataView, offset, width);
    if (width === flexbuffers.BitWidth.WIDTH8) {
      return dataView.getInt8(offset);
    }
    if (width === flexbuffers.BitWidth.WIDTH16) {
      return dataView.getInt16(offset, true);
    }
    if (width === flexbuffers.BitWidth.WIDTH32) {
      return dataView.getInt32(offset, true);
    }
    return dataView.getBigInt64(offset, true);
  }

  function readUInt(dataView, offset, width) {
    validateOffset(dataView, offset, width);
    if (width === flexbuffers.BitWidth.WIDTH8) {
      return dataView.getUint8(offset);
    }
    if (width === flexbuffers.BitWidth.WIDTH16) {
      return dataView.getUint16(offset, true);
    }
    if (width === flexbuffers.BitWidth.WIDTH32) {
      return dataView.getUint32(offset, true);
    }
    return dataView.getBigUint64(offset, true);
  }

  function readFloat(dataView, offset, width) {
    validateOffset(dataView, offset, width);
    if (width < flexbuffers.BitWidth.WIDTH32) {
      throw "Bad width: " + width;
    }
    if (width === flexbuffers.BitWidth.WIDTH32) {
      return dataView.getFloat32(offset, true);
    }
    return dataView.getFloat64(offset, true);
  }

  function indirect(dataView, offset, width) {
    const step = readInt(dataView, offset, width);
    return offset - step;
  }

  function keyIndex(key, dataView, offset, parentWidth, byteWidth, length) {
    const input = toUTF8Array(key);
    const keysVectorOffset = indirect(dataView, offset, parentWidth) - byteWidth * 3;
    const bitWidth = flexbuffers.BitWidth.fromByteWidth(byteWidth);
    const indirectOffset = keysVectorOffset - readInt(dataView, keysVectorOffset, bitWidth);
    const _byteWidth = readInt(dataView, keysVectorOffset + byteWidth, bitWidth);
    let low = 0;
    let high = length - 1;
    while (low <= high) {
      const mid = (high + low) >> 1;
      const dif = diffKeys(input, mid, dataView, indirectOffset, _byteWidth);
      if (dif === 0) return mid;
      if (dif < 0) {
        high = mid - 1;
      } else {
        low = mid + 1;
      }
    }
    return null;
  }

  function diffKeys(input, index, dataView, offset, width) {
    const keyOffset = offset + index * width;
    const keyIndirectOffset = keyOffset - readInt(dataView, keyOffset, flexbuffers.BitWidth.fromByteWidth(width));
    for (let i = 0; i < input.length; i++) {
      const dif = input[i] - dataView.getUint8(keyIndirectOffset + i);
      if (dif !== 0) {
        return dif;
      }
    }
    return dataView.getUint8(keyIndirectOffset + input.length) === 0 ? 0 : -1;
  }

  function valueForIndexWithKey(index, key, dataView, offset, parentWidth, byteWidth, length, path) {
    const _indirect = indirect(dataView, offset, parentWidth);
    const elementOffset = _indirect + index * byteWidth;
    const packedType = dataView.getUint8(_indirect + length * byteWidth + index);
    return constructor(dataView, elementOffset, flexbuffers.BitWidth.fromByteWidth(byteWidth), packedType, `${path}/${key}`)
  }

  function keyForIndex(index, dataView, offset, parentWidth, byteWidth) {
    const keysVectorOffset = indirect(dataView, offset, parentWidth) - byteWidth * 3;
    const bitWidth = flexbuffers.BitWidth.fromByteWidth(byteWidth)
    const indirectOffset = keysVectorOffset - readInt(dataView, keysVectorOffset, bitWidth);
    const _byteWidth = readInt(dataView, keysVectorOffset + byteWidth, bitWidth);
    const keyOffset = indirectOffset + index * _byteWidth;
    const keyIndirectOffset = keyOffset - readInt(dataView, keyOffset, flexbuffers.BitWidth.fromByteWidth(_byteWidth));
    var length = 0;
    while (dataView.getUint8(keyIndirectOffset + length) !== 0) {
      length++;
    }
    return fromUTF8Array(new Uint8Array(dataView.buffer, keyIndirectOffset, length));
  }

  function constructor(dataView, offset, parentWidth, packedType, path) {
    const byteWidth = 1 << (packedType & 3);
    const valueType = packedType >> 2;
    let length = -1;
    return {
      isNull: function() { return valueType === flexbuffers.ValueType.NULL; },
      isNumber: function() { return flexbuffers.ValueType.isNumber(valueType) || flexbuffers.ValueType.isIndirectNumber(valueType); },
      isFloat: function() { return flexbuffers.ValueType.FLOAT === valueType || flexbuffers.ValueType.INDIRECT_FLOAT === valueType; },
      isInt: function() { return this.isNumber() && !this.isFloat(); },
      isString: function() { return flexbuffers.ValueType.STRING === valueType || flexbuffers.ValueType.KEY === valueType; },
      isBool: function() { return flexbuffers.ValueType.BOOL === valueType; },
      isBlob: function() { return flexbuffers.ValueType.BLOB === valueType; },
      isVector: function() { return flexbuffers.ValueType.isAVector(valueType); },
      isMap: function() { return flexbuffers.ValueType.MAP === valueType; },

      boolValue: function() {
        if (this.isBool()) {
          return readInt(dataView, offset, parentWidth) > 0;
        }
        return null;
      },

      intValue: function() {
        if (valueType === flexbuffers.ValueType.INT) {
          return readInt(dataView, offset, parentWidth);
        }
        if (valueType === flexbuffers.ValueType.UINT) {
          return readUInt(dataView, offset, parentWidth);
        }
        if (valueType === flexbuffers.ValueType.INDIRECT_INT) {
          return readInt(dataView, indirect(dataView, offset, parentWidth), flexbuffers.BitWidth.fromByteWidth(byteWidth));
        }
        if (valueType === flexbuffers.ValueType.INDIRECT_UINT) {
          return readUInt(dataView, indirect(dataView, offset, parentWidth), flexbuffers.BitWidth.fromByteWidth(byteWidth));
        }
        return null;
      },

      floatValue: function() {
        if (valueType === flexbuffers.ValueType.FLOAT) {
          return readFloat(dataView, offset, parentWidth);
        }
        if (valueType === flexbuffers.ValueType.INDIRECT_FLOAT) {
          return readFloat(dataView, indirect(dataView, offset, parentWidth), flexbuffers.BitWidth.fromByteWidth(byteWidth));
        }
        return null;
      },

      numericValue: function() { return this.floatValue() || this.intValue()},

      stringValue: function() {
        if (valueType === flexbuffers.ValueType.STRING || valueType === flexbuffers.ValueType.KEY) {
          const begin = indirect(dataView, offset, parentWidth);
          return fromUTF8Array(new Uint8Array(dataView.buffer, begin, this.length()));
        }
        return null;
      },

      blobValue: function() {
        if (this.isBlob()) {
          const begin = indirect(dataView, offset, parentWidth);
          return new Uint8Array(dataView.buffer, begin, this.length());
        }
        return null;
      },

      get: function(key) {
        const length = this.length();
        if (Number.isInteger(key) && flexbuffers.ValueType.isAVector(valueType)) {
          if (key >= length || key < 0) {
            throw `Key: [${key}] is ot applicable on ${path} of ${valueType} length: ${length}`;
          }
          const _indirect = indirect(dataView, offset, parentWidth);
          const elementOffset = _indirect + key * byteWidth;
          let _packedType = dataView.getUint8(_indirect + length * byteWidth + key);
          if (flexbuffers.ValueType.isTypedVector(valueType)) {
            const _valueType = flexbuffers.ValueType.typedVectorElementType(valueType);
            _packedType = flexbuffers.ValueType.packedType(_valueType, flexbuffers.BitWidth.WIDTH8);
          } else if (flexbuffers.ValueType.isFixedTypedVector(valueType)) {
            const _valueType = flexbuffers.ValueType.fixedTypedVectorElementType(valueType);
            _packedType = flexbuffers.ValueType.packedType(_valueType, flexbuffers.BitWidth.WIDTH8);
          }
          return constructor(dataView, elementOffset, flexbuffers.BitWidth.fromByteWidth(byteWidth), _packedType, `${path}[${key}]`);
        }
        if (typeof key === 'string') {
          const index = keyIndex(key, dataView, offset, parentWidth, byteWidth, length);
          if (index !== null) {
            return valueForIndexWithKey(index, key, dataView, offset, parentWidth, byteWidth, length, path)
          }
        }
        throw `Key [${key}] is not applicable on ${path} of ${valueType}`;
      },

      length: function() {
        let size;
        if (length > -1) {
          return length;
        }
        if (flexbuffers.ValueType.isFixedTypedVector(valueType)) {
          length = flexbuffers.ValueType.fixedTypedVectorElementSize(valueType);
        } else if (valueType === flexbuffers.ValueType.BLOB
          || valueType === flexbuffers.ValueType.MAP
          || flexbuffers.ValueType.isAVector(valueType)) {
          length = readInt(dataView, indirect(dataView, offset, parentWidth) - byteWidth, flexbuffers.BitWidth.fromByteWidth(byteWidth))
        } else if (valueType === flexbuffers.ValueType.NULL) {
          length = 0;
        } else if (valueType === flexbuffers.ValueType.STRING) {
          const _indirect = indirect(dataView, offset, parentWidth);
          let sizeByteWidth = byteWidth;
          size = readInt(dataView, _indirect - sizeByteWidth, flexbuffers.BitWidth.fromByteWidth(byteWidth));
          while (dataView.getInt8(_indirect + size) !== 0) {
            sizeByteWidth <<= 1;
            size = readInt(dataView, _indirect - sizeByteWidth, flexbuffers.BitWidth.fromByteWidth(byteWidth));
          }
          length = size;
        } else if (valueType === flexbuffers.ValueType.KEY) {
          const _indirect = indirect(dataView, offset, parentWidth);
          size = 1;
          while (dataView.getInt8(_indirect + size) !== 0) {
            size++;
          }
          length = size;
        } else {
          length = 1;
        }
        return length;
      },

      toObject: function() {
        const length = this.length();
        if (this.isVector()) {
          let result = [];
          for (let i = 0; i < length; i++) {
            result.push(this.get(i).toObject());
          }
          return result;
        }
        if (this.isMap()) {
          let result = {};
          for (let i = 0; i < length; i++) {
            let key = keyForIndex(i, dataView, offset, parentWidth, byteWidth);
            result[key] = valueForIndexWithKey(i, key, dataView, offset, parentWidth, byteWidth, length, path).toObject();
          }
          return result;
        }
        if (this.isNull()) {
          return null;
        }
        if (this.isBool()) {
          return this.boolValue();
        }
        return this.numericValue() || this.blobValue() || this.stringValue();
      }
    }
  }

  const len = buffer.byteLength;

  if (len < 3) {
    throw "Buffer needs to be bigger than 3";
  }

  const dataView = new DataView(buffer);
  const byteWidth = dataView.getUint8(len - 1);
  const packedType = dataView.getUint8(len - 2);
  const parentWidth = flexbuffers.BitWidth.fromByteWidth(byteWidth);
  const offset = len - byteWidth - 2;

  return constructor(dataView, offset, parentWidth, packedType, "/")
};

flexbuffers.toObject = (buffer) => {
  return flexbuffers.read(buffer).toObject();
};

function fromUTF8Array(data) { // array of bytes
  var str = '',
    i;

  for (i = 0; i < data.length; i++) {
    var value = data[i];

    if (value < 0x80) {
      str += String.fromCharCode(value);
    } else if (value > 0xBF && value < 0xE0) {
      str += String.fromCharCode((value & 0x1F) << 6 | data[i + 1] & 0x3F);
      i += 1;
    } else if (value > 0xDF && value < 0xF0) {
      str += String.fromCharCode((value & 0x0F) << 12 | (data[i + 1] & 0x3F) << 6 | data[i + 2] & 0x3F);
      i += 2;
    } else {
      // surrogate pair
      var charCode = ((value & 0x07) << 18 | (data[i + 1] & 0x3F) << 12 | (data[i + 2] & 0x3F) << 6 | data[i + 3] & 0x3F) - 0x010000;

      str += String.fromCharCode(charCode >> 10 | 0xD800, charCode & 0x03FF | 0xDC00);
      i += 3;
    }
  }

  return str;
}

function toUTF8Array(str) {
  var utf8 = [];
  for (var i=0; i < str.length; i++) {
    var charcode = str.charCodeAt(i);
    if (charcode < 0x80) utf8.push(charcode);
    else if (charcode < 0x800) {
      utf8.push(0xc0 | (charcode >> 6),
        0x80 | (charcode & 0x3f));
    }
    else if (charcode < 0xd800 || charcode >= 0xe000) {
      utf8.push(0xe0 | (charcode >> 12),
        0x80 | ((charcode>>6) & 0x3f),
        0x80 | (charcode & 0x3f));
    }
    // surrogate pair
    else {
      i++;
      charcode = (((charcode&0x3ff)<<10)|(str.charCodeAt(i)&0x3ff)) + 0x010000;
      utf8.push(0xf0 | (charcode >>18),
        0x80 | ((charcode>>12) & 0x3f),
        0x80 | ((charcode>>6) & 0x3f),
        0x80 | (charcode & 0x3f));
    }
  }
  return utf8;
}

// Exports for Node.js and RequireJS
this.flexbuffers = flexbuffers;
