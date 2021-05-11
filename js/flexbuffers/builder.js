"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Builder = void 0;
var bit_width_1 = require("./bit-width");
var bit_width_util_1 = require("./bit-width-util");
var flexbuffers_util_1 = require("./flexbuffers-util");
var value_type_1 = require("./value-type");
var value_type_util_1 = require("./value-type-util");
var stack_value_1 = require("./stack-value");
var Builder = /** @class */ (function () {
    function Builder(size, dedupStrings, dedupKeys, dedupKeyVectors) {
        if (size === void 0) { size = 2048; }
        if (dedupStrings === void 0) { dedupStrings = true; }
        if (dedupKeys === void 0) { dedupKeys = true; }
        if (dedupKeyVectors === void 0) { dedupKeyVectors = true; }
        this.dedupStrings = dedupStrings;
        this.dedupKeys = dedupKeys;
        this.dedupKeyVectors = dedupKeyVectors;
        this.stack = [];
        this.stackPointers = [];
        this.offset = 0;
        this.finished = false;
        this.stringLookup = {};
        this.keyLookup = {};
        this.keyVectorLookup = {};
        this.indirectIntLookup = {};
        this.indirectUIntLookup = {};
        this.indirectFloatLookup = {};
        this.buffer = new ArrayBuffer(size > 0 ? size : 2048);
        this.view = new DataView(this.buffer);
    }
    Builder.prototype.align = function (width) {
        var byteWidth = bit_width_util_1.toByteWidth(width);
        this.offset += bit_width_util_1.paddingSize(this.offset, byteWidth);
        return byteWidth;
    };
    Builder.prototype.computeOffset = function (newValueSize) {
        var targetOffset = this.offset + newValueSize;
        var size = this.buffer.byteLength;
        var prevSize = size;
        while (size < targetOffset) {
            size <<= 1;
        }
        if (prevSize < size) {
            var prevBuffer = this.buffer;
            this.buffer = new ArrayBuffer(size);
            this.view = new DataView(this.buffer);
            new Uint8Array(this.buffer).set(new Uint8Array(prevBuffer), 0);
        }
        return targetOffset;
    };
    Builder.prototype.pushInt = function (value, width) {
        if (width === bit_width_1.BitWidth.WIDTH8) {
            this.view.setInt8(this.offset, value);
        }
        else if (width === bit_width_1.BitWidth.WIDTH16) {
            this.view.setInt16(this.offset, value, true);
        }
        else if (width === bit_width_1.BitWidth.WIDTH32) {
            this.view.setInt32(this.offset, value, true);
        }
        else if (width === bit_width_1.BitWidth.WIDTH64) {
            this.view.setBigInt64(this.offset, BigInt(value), true);
        }
        else {
            throw "Unexpected width: " + width + " for value: " + value;
        }
    };
    Builder.prototype.pushUInt = function (value, width) {
        if (width === bit_width_1.BitWidth.WIDTH8) {
            this.view.setUint8(this.offset, value);
        }
        else if (width === bit_width_1.BitWidth.WIDTH16) {
            this.view.setUint16(this.offset, value, true);
        }
        else if (width === bit_width_1.BitWidth.WIDTH32) {
            this.view.setUint32(this.offset, value, true);
        }
        else if (width === bit_width_1.BitWidth.WIDTH64) {
            this.view.setBigUint64(this.offset, BigInt(value), true);
        }
        else {
            throw "Unexpected width: " + width + " for value: " + value;
        }
    };
    Builder.prototype.writeInt = function (value, byteWidth) {
        var newOffset = this.computeOffset(byteWidth);
        this.pushInt(value, bit_width_util_1.fromByteWidth(byteWidth));
        this.offset = newOffset;
    };
    Builder.prototype.writeUInt = function (value, byteWidth) {
        var newOffset = this.computeOffset(byteWidth);
        this.pushUInt(value, bit_width_util_1.fromByteWidth(byteWidth));
        this.offset = newOffset;
    };
    Builder.prototype.writeBlob = function (arrayBuffer) {
        var length = arrayBuffer.byteLength;
        var bitWidth = bit_width_util_1.uwidth(length);
        var byteWidth = this.align(bitWidth);
        this.writeUInt(length, byteWidth);
        var blobOffset = this.offset;
        var newOffset = this.computeOffset(length);
        new Uint8Array(this.buffer).set(new Uint8Array(arrayBuffer), blobOffset);
        this.stack.push(this.offsetStackValue(blobOffset, value_type_1.ValueType.BLOB, bitWidth));
        this.offset = newOffset;
    };
    Builder.prototype.writeString = function (str) {
        if (this.dedupStrings && Object.prototype.hasOwnProperty.call(this.stringLookup, str)) {
            this.stack.push(this.stringLookup[str]);
            return;
        }
        var utf8 = flexbuffers_util_1.toUTF8Array(str);
        var length = utf8.length;
        var bitWidth = bit_width_util_1.uwidth(length);
        var byteWidth = this.align(bitWidth);
        this.writeUInt(length, byteWidth);
        var stringOffset = this.offset;
        var newOffset = this.computeOffset(length + 1);
        new Uint8Array(this.buffer).set(utf8, stringOffset);
        var stackValue = this.offsetStackValue(stringOffset, value_type_1.ValueType.STRING, bitWidth);
        this.stack.push(stackValue);
        if (this.dedupStrings) {
            this.stringLookup[str] = stackValue;
        }
        this.offset = newOffset;
    };
    Builder.prototype.writeKey = function (str) {
        if (this.dedupKeys && Object.prototype.hasOwnProperty.call(this.keyLookup, str)) {
            this.stack.push(this.keyLookup[str]);
            return;
        }
        var utf8 = flexbuffers_util_1.toUTF8Array(str);
        var length = utf8.length;
        var newOffset = this.computeOffset(length + 1);
        new Uint8Array(this.buffer).set(utf8, this.offset);
        var stackValue = this.offsetStackValue(this.offset, value_type_1.ValueType.KEY, bit_width_1.BitWidth.WIDTH8);
        this.stack.push(stackValue);
        if (this.dedupKeys) {
            this.keyLookup[str] = stackValue;
        }
        this.offset = newOffset;
    };
    Builder.prototype.writeStackValue = function (value, byteWidth) {
        var newOffset = this.computeOffset(byteWidth);
        if (value.isOffset()) {
            var relativeOffset = this.offset - value.offset;
            if (byteWidth === 8 || BigInt(relativeOffset) < (BigInt(1) << BigInt(byteWidth * 8))) {
                this.writeUInt(relativeOffset, byteWidth);
            }
            else {
                throw "Unexpected size " + byteWidth + ". This might be a bug. Please create an issue https://github.com/google/flatbuffers/issues/new";
            }
        }
        else {
            value.writeToBuffer(byteWidth);
        }
        this.offset = newOffset;
    };
    Builder.prototype.integrityCheckOnValueAddition = function () {
        if (this.finished) {
            throw "Adding values after finish is prohibited";
        }
        if (this.stackPointers.length !== 0 && this.stackPointers[this.stackPointers.length - 1].isVector === false) {
            if (this.stack[this.stack.length - 1].type !== value_type_1.ValueType.KEY) {
                throw "Adding value to a map before adding a key is prohibited";
            }
        }
    };
    Builder.prototype.integrityCheckOnKeyAddition = function () {
        if (this.finished) {
            throw "Adding values after finish is prohibited";
        }
        if (this.stackPointers.length === 0 || this.stackPointers[this.stackPointers.length - 1].isVector) {
            throw "Adding key before starting a map is prohibited";
        }
    };
    Builder.prototype.startVector = function () {
        this.stackPointers.push({ stackPosition: this.stack.length, isVector: true });
    };
    Builder.prototype.startMap = function (presorted) {
        if (presorted === void 0) { presorted = false; }
        this.stackPointers.push({ stackPosition: this.stack.length, isVector: false, presorted: presorted });
    };
    Builder.prototype.endVector = function (stackPointer) {
        var vecLength = this.stack.length - stackPointer.stackPosition;
        var vec = this.createVector(stackPointer.stackPosition, vecLength, 1);
        this.stack.splice(stackPointer.stackPosition, vecLength);
        this.stack.push(vec);
    };
    Builder.prototype.endMap = function (stackPointer) {
        if (!stackPointer.presorted) {
            this.sort(stackPointer);
        }
        var keyVectorHash = "";
        for (var i = stackPointer.stackPosition; i < this.stack.length; i += 2) {
            keyVectorHash += "," + this.stack[i].offset;
        }
        var vecLength = (this.stack.length - stackPointer.stackPosition) >> 1;
        if (this.dedupKeyVectors && !Object.prototype.hasOwnProperty.call(this.keyVectorLookup, keyVectorHash)) {
            this.keyVectorLookup[keyVectorHash] = this.createVector(stackPointer.stackPosition, vecLength, 2);
        }
        var keysStackValue = this.dedupKeyVectors ? this.keyVectorLookup[keyVectorHash] : this.createVector(stackPointer.stackPosition, vecLength, 2);
        var valuesStackValue = this.createVector(stackPointer.stackPosition + 1, vecLength, 2, keysStackValue);
        this.stack.splice(stackPointer.stackPosition, vecLength << 1);
        this.stack.push(valuesStackValue);
    };
    Builder.prototype.sort = function (stackPointer) {
        var view = this.view;
        var stack = this.stack;
        function shouldFlip(v1, v2) {
            if (v1.type !== value_type_1.ValueType.KEY || v2.type !== value_type_1.ValueType.KEY) {
                throw "Stack values are not keys " + v1 + " | " + v2 + ". Check if you combined [addKey] with add... method calls properly.";
            }
            var c1, c2;
            var index = 0;
            do {
                c1 = view.getUint8(v1.offset + index);
                c2 = view.getUint8(v2.offset + index);
                if (c2 < c1)
                    return true;
                if (c1 < c2)
                    return false;
                index += 1;
            } while (c1 !== 0 && c2 !== 0);
            return false;
        }
        function swap(stack, flipIndex, i) {
            if (flipIndex === i)
                return;
            var k = stack[flipIndex];
            var v = stack[flipIndex + 1];
            stack[flipIndex] = stack[i];
            stack[flipIndex + 1] = stack[i + 1];
            stack[i] = k;
            stack[i + 1] = v;
        }
        function selectionSort() {
            for (var i = stackPointer.stackPosition; i < stack.length; i += 2) {
                var flipIndex = i;
                for (var j = i + 2; j < stack.length; j += 2) {
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
            if (v1.type !== value_type_1.ValueType.KEY || v2.type !== value_type_1.ValueType.KEY) {
                throw "Stack values are not keys " + v1 + " | " + v2 + ". Check if you combined [addKey] with add... method calls properly.";
            }
            if (v1.offset === v2.offset) {
                return false;
            }
            var c1, c2;
            var index = 0;
            do {
                c1 = view.getUint8(v1.offset + index);
                c2 = view.getUint8(v2.offset + index);
                if (c1 < c2)
                    return true;
                if (c2 < c1)
                    return false;
                index += 1;
            } while (c1 !== 0 && c2 !== 0);
            return false;
        }
        function quickSort(left, right) {
            if (left < right) {
                var mid = left + (((right - left) >> 2)) * 2;
                var pivot = stack[mid];
                var left_new = left;
                var right_new = right;
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
        var sorted = true;
        for (var i = stackPointer.stackPosition; i < this.stack.length - 2; i += 2) {
            if (shouldFlip(this.stack[i], this.stack[i + 2])) {
                sorted = false;
                break;
            }
        }
        if (!sorted) {
            if (this.stack.length - stackPointer.stackPosition > 40) {
                quickSort(stackPointer.stackPosition, this.stack.length - 2);
            }
            else {
                selectionSort();
            }
        }
    };
    Builder.prototype.end = function () {
        if (this.stackPointers.length < 1)
            return;
        var pointer = this.stackPointers.pop();
        if (pointer.isVector) {
            this.endVector(pointer);
        }
        else {
            this.endMap(pointer);
        }
    };
    Builder.prototype.createVector = function (start, vecLength, step, keys) {
        if (keys === void 0) { keys = null; }
        var bitWidth = bit_width_util_1.uwidth(vecLength);
        var prefixElements = 1;
        if (keys !== null) {
            var elementWidth = keys.elementWidth(this.offset, 0);
            if (elementWidth > bitWidth) {
                bitWidth = elementWidth;
            }
            prefixElements += 2;
        }
        var vectorType = value_type_1.ValueType.KEY;
        var typed = keys === null;
        for (var i = start; i < this.stack.length; i += step) {
            var elementWidth = this.stack[i].elementWidth(this.offset, i + prefixElements);
            if (elementWidth > bitWidth) {
                bitWidth = elementWidth;
            }
            if (i === start) {
                vectorType = this.stack[i].type;
                typed = typed && value_type_util_1.isTypedVectorElement(vectorType);
            }
            else {
                if (vectorType !== this.stack[i].type) {
                    typed = false;
                }
            }
        }
        var byteWidth = this.align(bitWidth);
        var fix = typed && value_type_util_1.isNumber(vectorType) && vecLength >= 2 && vecLength <= 4;
        if (keys !== null) {
            this.writeStackValue(keys, byteWidth);
            this.writeUInt(1 << keys.width, byteWidth);
        }
        if (!fix) {
            this.writeUInt(vecLength, byteWidth);
        }
        var vecOffset = this.offset;
        for (var i = start; i < this.stack.length; i += step) {
            this.writeStackValue(this.stack[i], byteWidth);
        }
        if (!typed) {
            for (var i = start; i < this.stack.length; i += step) {
                this.writeUInt(this.stack[i].storedPackedType(), 1);
            }
        }
        if (keys !== null) {
            return this.offsetStackValue(vecOffset, value_type_1.ValueType.MAP, bitWidth);
        }
        if (typed) {
            var vType = value_type_util_1.toTypedVector(vectorType, fix ? vecLength : 0);
            return this.offsetStackValue(vecOffset, vType, bitWidth);
        }
        return this.offsetStackValue(vecOffset, value_type_1.ValueType.VECTOR, bitWidth);
    };
    Builder.prototype.nullStackValue = function () {
        return new stack_value_1.StackValue(this, value_type_1.ValueType.NULL, bit_width_1.BitWidth.WIDTH8);
    };
    Builder.prototype.boolStackValue = function (value) {
        return new stack_value_1.StackValue(this, value_type_1.ValueType.BOOL, bit_width_1.BitWidth.WIDTH8, value);
    };
    Builder.prototype.intStackValue = function (value) {
        return new stack_value_1.StackValue(this, value_type_1.ValueType.INT, bit_width_util_1.iwidth(value), value);
    };
    Builder.prototype.uintStackValue = function (value) {
        return new stack_value_1.StackValue(this, value_type_1.ValueType.UINT, bit_width_util_1.uwidth(value), value);
    };
    Builder.prototype.floatStackValue = function (value) {
        return new stack_value_1.StackValue(this, value_type_1.ValueType.FLOAT, bit_width_util_1.fwidth(value), value);
    };
    Builder.prototype.offsetStackValue = function (offset, valueType, bitWidth) {
        return new stack_value_1.StackValue(this, valueType, bitWidth, null, offset);
    };
    Builder.prototype.finishBuffer = function () {
        if (this.stack.length !== 1) {
            throw "Stack has to be exactly 1, but it is " + this.stack.length + ". You have to end all started vectors and maps before calling [finish]";
        }
        var value = this.stack[0];
        var byteWidth = this.align(value.elementWidth(this.offset, 0));
        this.writeStackValue(value, byteWidth);
        this.writeUInt(value.storedPackedType(), 1);
        this.writeUInt(byteWidth, 1);
        this.finished = true;
    };
    Builder.prototype.add = function (value) {
        this.integrityCheckOnValueAddition();
        if (typeof value === 'undefined') {
            throw "You need to provide a value";
        }
        if (value === null) {
            this.stack.push(this.nullStackValue());
        }
        else if (typeof value === "boolean") {
            this.stack.push(this.boolStackValue(value));
        }
        else if (typeof value === "bigint") {
            this.stack.push(this.intStackValue(value));
        }
        else if (typeof value == 'number') {
            if (Number.isInteger(value)) {
                this.stack.push(this.intStackValue(value));
            }
            else {
                this.stack.push(this.floatStackValue(value));
            }
        }
        else if (ArrayBuffer.isView(value)) {
            this.writeBlob(value.buffer);
        }
        else if (typeof value === 'string' || value instanceof String) {
            this.writeString(value);
        }
        else if (Array.isArray(value)) {
            this.startVector();
            for (var i = 0; i < value.length; i++) {
                this.add(value[i]);
            }
            this.end();
        }
        else if (typeof value === 'object') {
            var properties = Object.getOwnPropertyNames(value).sort();
            this.startMap(true);
            for (var i = 0; i < properties.length; i++) {
                var key = properties[i];
                this.addKey(key);
                this.add(value[key]);
            }
            this.end();
        }
        else {
            throw "Unexpected value input " + value;
        }
    };
    Builder.prototype.finish = function () {
        if (!this.finished) {
            this.finishBuffer();
        }
        var result = this.buffer.slice(0, this.offset);
        return new Uint8Array(result);
    };
    Builder.prototype.isFinished = function () {
        return this.finished;
    };
    Builder.prototype.addKey = function (key) {
        this.integrityCheckOnKeyAddition();
        this.writeKey(key);
    };
    Builder.prototype.addInt = function (value, indirect, deduplicate) {
        if (indirect === void 0) { indirect = false; }
        if (deduplicate === void 0) { deduplicate = false; }
        this.integrityCheckOnValueAddition();
        if (!indirect) {
            this.stack.push(this.intStackValue(value));
            return;
        }
        if (deduplicate && Object.prototype.hasOwnProperty.call(this.indirectIntLookup, value)) {
            this.stack.push(this.indirectIntLookup[value]);
            return;
        }
        var stackValue = this.intStackValue(value);
        var byteWidth = this.align(stackValue.width);
        var newOffset = this.computeOffset(byteWidth);
        var valueOffset = this.offset;
        stackValue.writeToBuffer(byteWidth);
        var stackOffset = this.offsetStackValue(valueOffset, value_type_1.ValueType.INDIRECT_INT, stackValue.width);
        this.stack.push(stackOffset);
        this.offset = newOffset;
        if (deduplicate) {
            this.indirectIntLookup[value] = stackOffset;
        }
    };
    Builder.prototype.addUInt = function (value, indirect, deduplicate) {
        if (indirect === void 0) { indirect = false; }
        if (deduplicate === void 0) { deduplicate = false; }
        this.integrityCheckOnValueAddition();
        if (!indirect) {
            this.stack.push(this.uintStackValue(value));
            return;
        }
        if (deduplicate && Object.prototype.hasOwnProperty.call(this.indirectUIntLookup, value)) {
            this.stack.push(this.indirectUIntLookup[value]);
            return;
        }
        var stackValue = this.uintStackValue(value);
        var byteWidth = this.align(stackValue.width);
        var newOffset = this.computeOffset(byteWidth);
        var valueOffset = this.offset;
        stackValue.writeToBuffer(byteWidth);
        var stackOffset = this.offsetStackValue(valueOffset, value_type_1.ValueType.INDIRECT_UINT, stackValue.width);
        this.stack.push(stackOffset);
        this.offset = newOffset;
        if (deduplicate) {
            this.indirectUIntLookup[value] = stackOffset;
        }
    };
    Builder.prototype.addFloat = function (value, indirect, deduplicate) {
        if (indirect === void 0) { indirect = false; }
        if (deduplicate === void 0) { deduplicate = false; }
        this.integrityCheckOnValueAddition();
        if (!indirect) {
            this.stack.push(this.floatStackValue(value));
            return;
        }
        if (deduplicate && Object.prototype.hasOwnProperty.call(this.indirectFloatLookup, value)) {
            this.stack.push(this.indirectFloatLookup[value]);
            return;
        }
        var stackValue = this.floatStackValue(value);
        var byteWidth = this.align(stackValue.width);
        var newOffset = this.computeOffset(byteWidth);
        var valueOffset = this.offset;
        stackValue.writeToBuffer(byteWidth);
        var stackOffset = this.offsetStackValue(valueOffset, value_type_1.ValueType.INDIRECT_FLOAT, stackValue.width);
        this.stack.push(stackOffset);
        this.offset = newOffset;
        if (deduplicate) {
            this.indirectFloatLookup[value] = stackOffset;
        }
    };
    return Builder;
}());
exports.Builder = Builder;
