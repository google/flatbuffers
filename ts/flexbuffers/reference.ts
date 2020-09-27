flexbuffers.toReference = (buffer) => {

    // Add to readInt, readUInt, readFloat in order to check for offset bugs
    function validateOffset(dataView, offset, width) {
      if (dataView.byteLength <= offset + width || offset & (flexbuffers.BitWidthUtil.toByteWidth(width) - 1) !== 0) {
        throw "Bad offset: " + offset + ", width: " + width;
      }
    }
  
    function readInt(dataView, offset, width) {
      if (width < 2) {
        if (width < 1) {
          return dataView.getInt8(offset);
        } else {
          return dataView.getInt16(offset, true);
        }
      } else {
        if (width < 3) {
          return dataView.getInt32(offset, true)
        } else {
          if (dataView.setBigInt64 === undefined) {
            return {
              low: dataView.getInt32(offset, true),
              high: dataView.getInt32(offset + 4, true)
            }
          }
          return dataView.getBigInt64(offset, true)
        }
      }
    }
  
    function readUInt(dataView, offset, width) {
      if (width < 2) {
        if (width < 1) {
          return dataView.getUint8(offset);
        } else {
          return dataView.getUint16(offset, true);
        }
      } else {
        if (width < 3) {
          return dataView.getUint32(offset, true)
        } else {
          if (dataView.getBigUint64 === undefined) {
            return {
              low: dataView.getUint32(offset, true),
              high: dataView.getUint32(offset + 4, true)
            }
          }
          return dataView.getBigUint64(offset, true)
        }
      }
    }
  
    function readFloat(dataView, offset, width) {
      if (width < 2 /*flexbuffers.BitWidth.WIDTH32*/) {
        throw "Bad width: " + width;
      }
      if (width === 2 /*flexbuffers.BitWidth.WIDTH32*/) {
        return dataView.getFloat32(offset, true);
      }
      return dataView.getFloat64(offset, true);
    }
  
    function indirect(dataView, offset, width) {
      const step = readUInt(dataView, offset, width);
      return offset - step;
    }
  
    function keyIndex(key, dataView, offset, parentWidth, byteWidth, length) {
      const input = toUTF8Array(key);
      const keysVectorOffset = indirect(dataView, offset, parentWidth) - byteWidth * 3;
      const bitWidth = flexbuffers.BitWidthUtil.fromByteWidth(byteWidth);
      const indirectOffset = keysVectorOffset - readUInt(dataView, keysVectorOffset, bitWidth);
      const _byteWidth = readUInt(dataView, keysVectorOffset + byteWidth, bitWidth);
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
      const keyIndirectOffset = keyOffset - readUInt(dataView, keyOffset, flexbuffers.BitWidthUtil.fromByteWidth(width));
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
      return Reference(dataView, elementOffset, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth), packedType, `${path}/${key}`)
    }
  
    function keyForIndex(index, dataView, offset, parentWidth, byteWidth) {
      const keysVectorOffset = indirect(dataView, offset, parentWidth) - byteWidth * 3;
      const bitWidth = flexbuffers.BitWidthUtil.fromByteWidth(byteWidth);
      const indirectOffset = keysVectorOffset - readUInt(dataView, keysVectorOffset, bitWidth);
      const _byteWidth = readUInt(dataView, keysVectorOffset + byteWidth, bitWidth);
      const keyOffset = indirectOffset + index * _byteWidth;
      const keyIndirectOffset = keyOffset - readUInt(dataView, keyOffset, flexbuffers.BitWidthUtil.fromByteWidth(_byteWidth));
      let length = 0;
      while (dataView.getUint8(keyIndirectOffset + length) !== 0) {
        length++;
      }
      return fromUTF8Array(new Uint8Array(dataView.buffer, keyIndirectOffset, length));
    }
  
    function Reference(dataView, offset, parentWidth, packedType, path) {
      const byteWidth = 1 << (packedType & 3);
      const valueType = packedType >> 2;
      let length = -1;
      return {
        isNull: function() { return valueType === flexbuffers.ValueType.NULL; },
        isNumber: function() { return flexbuffers.ValueTypeUtil.isNumber(valueType) || flexbuffers.ValueTypeUtil.isIndirectNumber(valueType); },
        isFloat: function() { return flexbuffers.ValueType.FLOAT === valueType || flexbuffers.ValueType.INDIRECT_FLOAT === valueType; },
        isInt: function() { return this.isNumber() && !this.isFloat(); },
        isString: function() { return flexbuffers.ValueType.STRING === valueType || flexbuffers.ValueType.KEY === valueType; },
        isBool: function() { return flexbuffers.ValueType.BOOL === valueType; },
        isBlob: function() { return flexbuffers.ValueType.BLOB === valueType; },
        isVector: function() { return flexbuffers.ValueTypeUtil.isAVector(valueType); },
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
            return readInt(dataView, indirect(dataView, offset, parentWidth), flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
          }
          if (valueType === flexbuffers.ValueType.INDIRECT_UINT) {
            return readUInt(dataView, indirect(dataView, offset, parentWidth), flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
          }
          return null;
        },
  
        floatValue: function() {
          if (valueType === flexbuffers.ValueType.FLOAT) {
            return readFloat(dataView, offset, parentWidth);
          }
          if (valueType === flexbuffers.ValueType.INDIRECT_FLOAT) {
            return readFloat(dataView, indirect(dataView, offset, parentWidth), flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
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
          if (Number.isInteger(key) && flexbuffers.ValueTypeUtil.isAVector(valueType)) {
            if (key >= length || key < 0) {
              throw `Key: [${key}] is not applicable on ${path} of ${valueType} length: ${length}`;
            }
            const _indirect = indirect(dataView, offset, parentWidth);
            const elementOffset = _indirect + key * byteWidth;
            let _packedType = dataView.getUint8(_indirect + length * byteWidth + key);
            if (flexbuffers.ValueTypeUtil.isTypedVector(valueType)) {
              const _valueType = flexbuffers.ValueTypeUtil.typedVectorElementType(valueType);
              _packedType = flexbuffers.ValueTypeUtil.packedType(_valueType, flexbuffers.BitWidth.WIDTH8);
            } else if (flexbuffers.ValueTypeUtil.isFixedTypedVector(valueType)) {
              const _valueType = flexbuffers.ValueTypeUtil.fixedTypedVectorElementType(valueType);
              _packedType = flexbuffers.ValueTypeUtil.packedType(_valueType, flexbuffers.BitWidth.WIDTH8);
            }
            return Reference(dataView, elementOffset, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth), _packedType, `${path}[${key}]`);
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
          if (flexbuffers.ValueTypeUtil.isFixedTypedVector(valueType)) {
            length = flexbuffers.ValueTypeUtil.fixedTypedVectorElementSize(valueType);
          } else if (valueType === flexbuffers.ValueType.BLOB
            || valueType === flexbuffers.ValueType.MAP
            || flexbuffers.ValueTypeUtil.isAVector(valueType)) {
            length = readUInt(dataView, indirect(dataView, offset, parentWidth) - byteWidth, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth))
          } else if (valueType === flexbuffers.ValueType.NULL) {
            length = 0;
          } else if (valueType === flexbuffers.ValueType.STRING) {
            const _indirect = indirect(dataView, offset, parentWidth);
            let sizeByteWidth = byteWidth;
            size = readUInt(dataView, _indirect - sizeByteWidth, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
            while (dataView.getInt8(_indirect + size) !== 0) {
              sizeByteWidth <<= 1;
              size = readUInt(dataView, _indirect - sizeByteWidth, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
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
          if (this.isNumber()) {
            return this.numericValue();
          }
          return this.blobValue() || this.stringValue();
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
    const parentWidth = flexbuffers.BitWidthUtil.fromByteWidth(byteWidth);
    const offset = len - byteWidth - 2;
  
    return Reference(dataView, offset, parentWidth, packedType, "/")
  };