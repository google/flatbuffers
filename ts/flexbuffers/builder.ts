flexbuffers.builder = (size = 2048, deduplicateString = true, deduplicateKeys = true, deduplicateKeyVectors = true) => {
    let buffer = new ArrayBuffer(size > 0 ? size : 2048);
    let view = new DataView(buffer);
    const stack = [];
    const stackPointers = [];
    let offset = 0;
    let finished = false;
    const stringLookup = {};
    const keyLookup = {};
    const keyVectorLookup = {};
    const indirectIntLookup = {};
    const indirectUIntLookup = {};
    const indirectFloatLookup = {};
  
    let dedupStrings = deduplicateString;
    let dedupKeys = deduplicateKeys;
    let dedupKeyVectors = deduplicateKeyVectors;
  
    function align(width) {
      const byteWidth = flexbuffers.BitWidthUtil.toByteWidth(width);
      offset += flexbuffers.BitWidthUtil.paddingSize(offset, byteWidth);
      return byteWidth;
    }
  
    function computeOffset(newValueSize) {
      const targetOffset = offset + newValueSize;
      let size = buffer.byteLength;
      const prevSize = size;
      while (size < targetOffset) {
        size <<= 1;
      }
      if (prevSize < size) {
        const prevBuffer = buffer;
        buffer = new ArrayBuffer(size);
        view = new DataView(buffer);
        new Uint8Array(buffer).set(new Uint8Array(prevBuffer), 0);
      }
      return targetOffset;
    }
  
    function pushInt(value, width) {
      if (width === flexbuffers.BitWidth.WIDTH8) {
        view.setInt8(offset, value);
      } else if (width === flexbuffers.BitWidth.WIDTH16) {
        view.setInt16(offset, value, true);
      } else if (width === flexbuffers.BitWidth.WIDTH32) {
        view.setInt32(offset, value, true);
      } else if (width === flexbuffers.BitWidth.WIDTH64) {
        view.setBigInt64(offset, BigInt(value), true);
      } else {
        throw `Unexpected width: ${width} for value: ${value}`;
      }
    }
  
    function pushUInt(value, width) {
      if (width === flexbuffers.BitWidth.WIDTH8) {
        view.setUint8(offset, value);
      } else if (width === flexbuffers.BitWidth.WIDTH16) {
        view.setUint16(offset, value, true);
      } else if (width === flexbuffers.BitWidth.WIDTH32) {
        view.setUint32(offset, value, true);
      } else if (width === flexbuffers.BitWidth.WIDTH64) {
        view.setBigUint64(offset, BigInt(value), true);
      } else {
        throw `Unexpected width: ${width} for value: ${value}`;
      }
    }
  
    function writeInt(value, byteWidth) {
      const newOffset = computeOffset(byteWidth);
      pushInt(value, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
      offset = newOffset;
    }
  
    function writeUInt(value, byteWidth) {
      const newOffset = computeOffset(byteWidth);
      pushUInt(value, flexbuffers.BitWidthUtil.fromByteWidth(byteWidth));
      offset = newOffset;
    }
  
    function writeBlob(arrayBuffer) {
      const length = arrayBuffer.byteLength;
      const bitWidth = flexbuffers.BitWidthUtil.uwidth(length);
      const byteWidth = align(bitWidth);
      writeUInt(length, byteWidth);
      const blobOffset = offset;
      const newOffset = computeOffset(length);
      new Uint8Array(buffer).set(new Uint8Array(arrayBuffer), blobOffset);
      stack.push(offsetStackValue(blobOffset, flexbuffers.ValueType.BLOB, bitWidth));
      offset = newOffset;
    }
  
    function writeString(str) {
      if (dedupStrings && stringLookup.hasOwnProperty(str)) {
        stack.push(stringLookup[str]);
        return;
      }
      const utf8 = toUTF8Array(str);
      const length = utf8.length;
      const bitWidth = flexbuffers.BitWidthUtil.uwidth(length);
      const byteWidth = align(bitWidth);
      writeUInt(length, byteWidth);
      const stringOffset = offset;
      const newOffset = computeOffset(length + 1);
      new Uint8Array(buffer).set(utf8, stringOffset);
      const stackValue = offsetStackValue(stringOffset, flexbuffers.ValueType.STRING, bitWidth);
      stack.push(stackValue);
      if (dedupStrings) {
        stringLookup[str] = stackValue;
      }
      offset = newOffset;
    }
  
    function writeKey(str) {
      if (dedupKeys && keyLookup.hasOwnProperty(str)) {
        stack.push(keyLookup[str]);
        return;
      }
      const utf8 = toUTF8Array(str);
      const length = utf8.length;
      const newOffset = computeOffset(length + 1);
      new Uint8Array(buffer).set(utf8, offset);
      const stackValue = offsetStackValue(offset, flexbuffers.ValueType.KEY, flexbuffers.BitWidth.WIDTH8);
      stack.push(stackValue);
      if (dedupKeys) {
        keyLookup[str] = stackValue;
      }
      offset = newOffset;
    }
  
    function writeStackValue(value, byteWidth) {
      const newOffset = computeOffset(byteWidth);
      if (value.isOffset) {
        const relativeOffset = offset - value.offset;
        if (byteWidth === 8 || BigInt(relativeOffset) < (1n << BigInt(byteWidth * 8))) {
          writeUInt(relativeOffset, byteWidth);
        } else {
          throw `Unexpected size ${byteWidth}. This might be a bug. Please create an issue https://github.com/google/flatbuffers/issues/new`
        }
      } else {
        value.writeToBuffer(byteWidth);
      }
      offset = newOffset;
    }
  
    function integrityCheckOnValueAddition() {
      if (finished) {
        throw "Adding values after finish is prohibited";
      }
      if (stackPointers.length !== 0 && stackPointers[stackPointers.length - 1].isVector === false) {
        if (stack[stack.length - 1].type !== flexbuffers.ValueType.KEY) {
          throw "Adding value to a map before adding a key is prohibited";
        }
      }
    }
  
    function integrityCheckOnKeyAddition() {
      if (finished) {
        throw "Adding values after finish is prohibited";
      }
      if (stackPointers.length === 0 || stackPointers[stackPointers.length - 1].isVector) {
        throw "Adding key before starting a map is prohibited";
      }
    }
  
    function startVector() {
      stackPointers.push({stackPosition: stack.length, isVector: true});
    }
  
    function startMap(presorted = false) {
      stackPointers.push({stackPosition: stack.length, isVector: false, presorted: presorted});
    }
  
    function endVector(stackPointer) {
      const vecLength = stack.length - stackPointer.stackPosition;
      const vec = createVector(stackPointer.stackPosition, vecLength, 1);
      stack.splice(stackPointer.stackPosition, vecLength);
      stack.push(vec);
    }
  
    function endMap(stackPointer) {
      if (!stackPointer.presorted) {
        sort(stackPointer);
      }
      let keyVectorHash = "";
      for (let i = stackPointer.stackPosition; i < stack.length; i += 2) {
        keyVectorHash += `,${stack[i].offset}`;
      }
      const vecLength = (stack.length - stackPointer.stackPosition) >> 1;
  
      if (dedupKeyVectors && !keyVectorLookup.hasOwnProperty(keyVectorHash)) {
        keyVectorLookup[keyVectorHash] = createVector(stackPointer.stackPosition, vecLength, 2);
      }
      const keysStackValue = dedupKeyVectors ? keyVectorLookup[keyVectorHash] : createVector(stackPointer.stackPosition, vecLength, 2);
      const valuesStackValue = createVector(stackPointer.stackPosition + 1, vecLength, 2, keysStackValue);
      stack.splice(stackPointer.stackPosition, vecLength << 1);
      stack.push(valuesStackValue);
    }
  
    function sort(stackPointer) {
      function shouldFlip(v1, v2) {
        if (v1.type !== flexbuffers.ValueType.KEY || v2.type !== flexbuffers.ValueType.KEY) {
          throw `Stack values are not keys ${v1} | ${v2}. Check if you combined [addKey] with add... method calls properly.`
        }
        let c1, c2;
        let index = 0;
        do {
          c1 = view.getUint8(v1.offset + index);
          c2 = view.getUint8(v2.offset + index);
          if (c2 < c1) return true;
          if (c1 < c2) return false;
          index += 1;
        } while (c1 !== 0 && c2 !== 0);
        return false;
      }
  
      function swap(stack, flipIndex, i) {
        if (flipIndex === i) return;
        const k = stack[flipIndex];
        const v = stack[flipIndex + 1];
        stack[flipIndex] = stack[i];
        stack[flipIndex + 1] = stack[i + 1];
        stack[i] = k;
        stack[i + 1] = v;
      }
  
      function selectionSort() {
        for (let i = stackPointer.stackPosition; i < stack.length; i += 2) {
          let flipIndex = i;
          for (let j = i + 2; j < stack.length; j += 2) {
            if (shouldFlip(stack[flipIndex], stack[j])) {
              flipIndex = j;
            }
          }
          if (flipIndex !== i) {
            swap(stack, flipIndex, i);
          }
        }
      }
  
      function smaller(v1, v2) {
        if (v1.type !== flexbuffers.ValueType.KEY || v2.type !== flexbuffers.ValueType.KEY) {
          throw `Stack values are not keys ${v1} | ${v2}. Check if you combined [addKey] with add... method calls properly.`
        }
        if(v1.offset === v2.offset) {
          return false;
        }
        let c1, c2;
        let index = 0;
        do {
          c1 = view.getUint8(v1.offset + index);
          c2 = view.getUint8(v2.offset + index);
          if(c1 < c2) return true;
          if(c2 < c1) return false;
          index += 1;
        } while (c1 !== 0 && c2 !== 0);
        return false;
      }
  
      function quickSort(left, right) {
  
        if (left < right) {
          let mid = left + (((right - left) >> 2)) * 2;
          let pivot = stack[mid],
            left_new = left,
            right_new = right;
  
          do {
            while (smaller(stack[left_new], pivot)) {
              left_new += 2;
            }
            while (smaller(pivot, stack[right_new])) {
              right_new -= 2;
            }
            if (left_new <= right_new) {
              swap(stack, left_new, right_new);
              left_new += 2;
              right_new -= 2;
            }
          } while (left_new <= right_new);
  
          quickSort(left, right_new);
          quickSort(left_new, right);
  
        }
      }
  
      let sorted = true;
      for (let i = stackPointer.stackPosition; i < stack.length - 2; i += 2) {
        if (shouldFlip(stack[i], stack[i + 2])) {
          sorted = false;
          break;
        }
      }
  
      if (!sorted) {
        if (stack.length - stackPointer.stackPosition > 40) {
          quickSort(stackPointer.stackPosition, stack.length - 2);
        } else {
          selectionSort();
        }
      }
    }
  
    function end() {
      if (stackPointers.length < 1) return;
      const pointer = stackPointers.pop();
      if (pointer.isVector) {
        endVector(pointer);
      } else {
        endMap(pointer);
      }
    }
  
    function createVector(start, vecLength, step, keys = null) {
      let bitWidth = flexbuffers.BitWidthUtil.uwidth(vecLength);
      let prefixElements = 1;
      if (keys !== null) {
        const elementWidth = keys.elementWidth(offset, 0);
        if (elementWidth > bitWidth) {
          bitWidth = elementWidth;
        }
        prefixElements += 2;
      }
      let vectorType = flexbuffers.ValueType.KEY;
      let typed = keys === null;
      for (let i = start; i < stack.length; i += step) {
        const elementWidth = stack[i].elementWidth(offset, i + prefixElements);
        if (elementWidth > bitWidth) {
          bitWidth = elementWidth;
        }
        if (i === start) {
          vectorType = stack[i].type;
          typed &= flexbuffers.ValueTypeUtil.isTypedVectorElement(vectorType);
        } else {
          if (vectorType !== stack[i].type) {
            typed = false;
          }
        }
      }
      const byteWidth = align(bitWidth);
      const fix = typed && flexbuffers.ValueTypeUtil.isNumber(vectorType) && vecLength >= 2 && vecLength <= 4;
      if (keys !== null) {
        writeStackValue(keys, byteWidth);
        writeUInt(1 << keys.width, byteWidth);
      }
      if (!fix) {
        writeUInt(vecLength, byteWidth);
      }
      const vecOffset = offset;
      for (let i = start; i < stack.length; i += step) {
        writeStackValue(stack[i], byteWidth);
      }
      if (!typed) {
        for (let i = start; i < stack.length; i += step) {
          writeUInt(stack[i].storedPackedType(), 1);
        }
      }
      if (keys !== null) {
        return offsetStackValue(vecOffset, flexbuffers.ValueType.MAP, bitWidth);
      }
      if (typed) {
        const vType = flexbuffers.ValueTypeUtil.toTypedVector(vectorType, fix ? vecLength : 0);
        return offsetStackValue(vecOffset, vType, bitWidth);
      }
      return offsetStackValue(vecOffset, flexbuffers.ValueType.VECTOR, bitWidth);
    }
  
    function StackValue(type, width, value, _offset) {
      return {
        type: type,
        width: width,
        value: value,
        offset: _offset,
        elementWidth: function (size, index) {
          if (flexbuffers.ValueTypeUtil.isInline(this.type)) return this.width;
          for (let i = 0; i < 4; i++) {
            const width = 1 << i;
            const offsetLoc = size + flexbuffers.BitWidthUtil.paddingSize(size, width) + index * width;
            const offset = offsetLoc - this.offset;
            const bitWidth = flexbuffers.BitWidthUtil.uwidth(offset);
            if (1 << bitWidth === width) {
              return bitWidth;
            }
          }
          throw `Element is unknown. Size: ${size} at index: ${index}. This might be a bug. Please create an issue https://github.com/google/flatbuffers/issues/new`;
        },
        writeToBuffer: function (byteWidth) {
          const newOffset = computeOffset(byteWidth);
          if (this.type === flexbuffers.ValueType.FLOAT) {
            if (this.width === flexbuffers.BitWidth.WIDTH32) {
              view.setFloat32(offset, this.value, true);
            } else {
              view.setFloat64(offset, this.value, true);
            }
          } else if (this.type === flexbuffers.ValueType.INT) {
            const bitWidth = flexbuffers.BitWidthUtil.fromByteWidth(byteWidth);
            pushInt(value, bitWidth);
          } else if (this.type === flexbuffers.ValueType.UINT) {
            const bitWidth = flexbuffers.BitWidthUtil.fromByteWidth(byteWidth);
            pushUInt(value, bitWidth);
          } else if (this.type === flexbuffers.ValueType.NULL) {
            pushInt(0, this.width);
          } else if (this.type === flexbuffers.ValueType.BOOL) {
            pushInt(value ? 1 : 0, this.width);
          } else {
            throw `Unexpected type: ${type}. This might be a bug. Please create an issue https://github.com/google/flatbuffers/issues/new`
          }
          offset = newOffset;
        },
        storedWidth: function (width = flexbuffers.BitWidth.WIDTH8) {
          return flexbuffers.ValueTypeUtil.isInline(this.type) ? Math.max(width, this.width) : this.width;
        },
        storedPackedType: function (width = flexbuffers.BitWidth.WIDTH8) {
          return flexbuffers.ValueTypeUtil.packedType(this.type, this.storedWidth(width));
        },
        isOffset: !flexbuffers.ValueTypeUtil.isInline(type)
      }
    }
  
    function nullStackValue() {
      return StackValue(flexbuffers.ValueType.NULL, flexbuffers.BitWidth.WIDTH8);
    }
  
    function boolStackValue(value) {
      return StackValue(flexbuffers.ValueType.BOOL, flexbuffers.BitWidth.WIDTH8, value);
    }
  
    function intStackValue(value) {
      return StackValue(flexbuffers.ValueType.INT, flexbuffers.BitWidthUtil.iwidth(value), value);
    }
  
    function uintStackValue(value) {
      return StackValue(flexbuffers.ValueType.UINT, flexbuffers.BitWidthUtil.uwidth(value), value);
    }
  
    function floatStackValue(value) {
      return StackValue(flexbuffers.ValueType.FLOAT, flexbuffers.BitWidthUtil.fwidth(value), value);
    }
  
    function offsetStackValue(offset, valueType, bitWidth) {
      return StackValue(valueType, bitWidth, null, offset);
    }
  
    function finishBuffer() {
      if (stack.length !== 1) {
        throw `Stack has to be exactly 1, but it is ${stack.length}. You have to end all started vectors and maps before calling [finish]`;
      }
      const value = stack[0];
      const byteWidth = align(value.elementWidth(offset, 0));
      writeStackValue(value, byteWidth);
      writeUInt(value.storedPackedType(), 1);
      writeUInt(byteWidth, 1);
      finished = true;
    }
  
    return  {
      add: function (value) {
        integrityCheckOnValueAddition();
        if (typeof value === 'undefined') {
          throw "You need to provide a value";
        }
        if (value === null) {
          stack.push(nullStackValue());
        } else if (typeof value === "boolean") {
          stack.push(boolStackValue(value));
        } else if (typeof value === "bigint") {
          stack.push(intStackValue(value));
        } else if (typeof value == 'number') {
          if (Number.isInteger(value)) {
            stack.push(intStackValue(value));
          } else {
            stack.push(floatStackValue(value));
          }
        } else if (ArrayBuffer.isView(value)){
          writeBlob(value.buffer);
        } else if (typeof value === 'string' || value instanceof String) {
          writeString(value);
        } else if (Array.isArray(value)) {
          startVector();
          for (let i = 0; i < value.length; i++) {
            this.add(value[i]);
          }
          end();
        } else if (typeof value === 'object'){
          const properties = Object.getOwnPropertyNames(value).sort();
          startMap(true);
          for (let i = 0; i < properties.length; i++) {
            const key = properties[i];
            this.addKey(key);
            this.add(value[key]);
          }
          end();
        } else {
          throw `Unexpected value input ${value}`;
        }
      },
      finish: function() {
        if (!finished) {
          finishBuffer();
        }
        const result = buffer.slice(0, offset);
        return new Uint8Array(result);
      },
      isFinished: function() {
        return finished;
      },
      addKey: function(key) {
        integrityCheckOnKeyAddition();
        writeKey(key);
      },
      addInt: function(value, indirect = false, deduplicate = false) {
        integrityCheckOnValueAddition();
        if (!indirect) {
          stack.push(intStackValue(value));
          return;
        }
        if (deduplicate && indirectIntLookup.hasOwnProperty(value)) {
          stack.push(indirectIntLookup[value]);
          return;
        }
        const stackValue = intStackValue(value);
        const byteWidth = align(stackValue.width);
        const newOffset = computeOffset(byteWidth);
        const valueOffset = offset;
        stackValue.writeToBuffer(byteWidth);
        const stackOffset = offsetStackValue(valueOffset, flexbuffers.ValueType.INDIRECT_INT, stackValue.width);
        stack.push(stackOffset);
        offset = newOffset;
        if (deduplicate) {
          indirectIntLookup[value] = stackOffset;
        }
      },
      addUInt: function(value, indirect = false, deduplicate = false) {
        integrityCheckOnValueAddition();
        if (!indirect) {
          stack.push(uintStackValue(value));
          return;
        }
        if (deduplicate && indirectUIntLookup.hasOwnProperty(value)) {
          stack.push(indirectUIntLookup[value]);
          return;
        }
        const stackValue = uintStackValue(value);
        const byteWidth = align(stackValue.width);
        const newOffset = computeOffset(byteWidth);
        const valueOffset = offset;
        stackValue.writeToBuffer(byteWidth);
        const stackOffset = offsetStackValue(valueOffset, flexbuffers.ValueType.INDIRECT_UINT, stackValue.width);
        stack.push(stackOffset);
        offset = newOffset;
        if (deduplicate) {
          indirectUIntLookup[value] = stackOffset;
        }
      },
      addFloat: function(value, indirect = false, deduplicate = false) {
        integrityCheckOnValueAddition();
        if (!indirect) {
          stack.push(floatStackValue(value));
          return;
        }
        if (deduplicate && indirectFloatLookup.hasOwnProperty(value)) {
          stack.push(indirectFloatLookup[value]);
          return;
        }
        const stackValue = floatStackValue(value);
        const byteWidth = align(stackValue.width);
        const newOffset = computeOffset(byteWidth);
        const valueOffset = offset;
        stackValue.writeToBuffer(byteWidth);
        const stackOffset = offsetStackValue(valueOffset, flexbuffers.ValueType.INDIRECT_FLOAT, stackValue.width);
        stack.push(stackOffset);
        offset = newOffset;
        if (deduplicate) {
          indirectFloatLookup[value] = stackOffset;
        }
      },
      startVector: function() {
        startVector();
      },
      startMap: function() {
        startMap();
      },
      end: function() {
        end();
      }
    };
  };