<?php declare (strict_types=1);
/*
 * Copyright 2015 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace Google\FlatBuffers;

class ByteBuffer
{
    /**
     * @var string
     */
    public $_buffer;

    /**
     * @var int
     */
    private $_pos;

    /**
     * @var bool
     */
    private static $_is_little_endian = null;

    public static function wrap(string $bytes): ByteBuffer
    {
        $bb = new ByteBuffer(0);
        $bb->_buffer = $bytes;

        return $bb;
    }

    public function __construct(int $size)
    {
        $this->_buffer = str_repeat("\0", $size);
    }

    public function capacity(): int
    {
        return strlen($this->_buffer);
    }

    public function getPosition(): int
    {
        return $this->_pos;
    }

    public function setPosition(int $pos): void
    {
        $this->_pos = $pos;
    }

    public function reset(): void
    {
        $this->_pos = 0;
    }

    public function length(): int
    {
        return strlen($this->_buffer);
    }

    public function data(): string
    {
        return substr($this->_buffer, $this->_pos);
    }

    public static function isLittleEndian(): bool
    {
        if (ByteBuffer::$_is_little_endian === null) {
            ByteBuffer::$_is_little_endian = unpack('S', "\x01\x00")[1] === 1;
        }

        return ByteBuffer::$_is_little_endian;
    }

    /**
     * write little endian value to the buffer.
     */
    public function writeLittleEndian(int $offset, int $count, int $data): void
    {
        if (ByteBuffer::isLittleEndian()) {
            for ($i = 0; $i < $count; $i++) {
                $this->_buffer[$offset + $i] = chr($data >> $i * 8);
            }
        } else {
            for ($i = 0; $i < $count; $i++) {
                $this->_buffer[$offset + $count - 1 - $i] = chr($data >> $i * 8);
            }
        }
    }

    /**
     * read little endian value from the buffer
     */
    public function readLittleEndian(int $offset, int $count, bool $force_bigendian = false): int
    {
        $this->assertOffsetAndLength($offset, $count);
        $r = 0;

        if (ByteBuffer::isLittleEndian() && $force_bigendian == false) {
            for ($i = 0; $i < $count; $i++) {
                $r |= ord($this->_buffer[$offset + $i]) << $i * 8;
            }
        } else {
            for ($i = 0; $i < $count; $i++) {
                $r |= ord($this->_buffer[$offset + $count -1 - $i]) << $i * 8;
            }
        }

        return $r;
    }

    public function assertOffsetAndLength(int $offset, int $length): void
    {
        if ($offset < 0 ||
            $offset >= strlen($this->_buffer) ||
            $offset + $length > strlen($this->_buffer)) {
            throw new \OutOfRangeException(sprintf("offset: %d, length: %d, buffer; %d", $offset, $length, strlen($this->_buffer)));
        }
    }

    public function putSbyte(int $offset, string $value): string
    {
        self::validateValue(-128, 127, $value, "sbyte");

        $length = strlen($value);
        $this->assertOffsetAndLength($offset, $length);
        return $this->_buffer[$offset] = $value;
    }

    public function putByte(int $offset, string $value): string
    {
        self::validateValue(0, 255, $value, "byte");

        $length = strlen($value);
        $this->assertOffsetAndLength($offset, $length);
        return $this->_buffer[$offset] = $value;
    }

    public function put(int $offset, string $value): void
    {
        $length = strlen($value);
        $this->assertOffsetAndLength($offset, $length);
        for ($i = 0; $i < $length; $i++) {
            $this->_buffer[$offset + $i] = $value[$i];
        }
    }

    public function putShort(int $offset, int $value): void
    {
        self::validateValue(-32768, 32767, $value, "short");

        $this->assertOffsetAndLength($offset, 2);
        $this->writeLittleEndian($offset, 2, $value);
    }

    public function putUshort(int $offset, int $value): void
    {
        self::validateValue(0, 65535, $value, "short");

        $this->assertOffsetAndLength($offset, 2);
        $this->writeLittleEndian($offset, 2, $value);
    }

    public function putInt(int $offset, int $value): void
    {
        // 2147483647 = (1 << 31) -1 = Maximum signed 32-bit int
        // -2147483648 = -1 << 31 = Minimum signed 32-bit int
        self::validateValue(-2147483648, 2147483647, $value, "int");

        $this->assertOffsetAndLength($offset, 4);
        $this->writeLittleEndian($offset, 4, $value);
    }

    public function putUint(int $offset, int $value): void
    {
        // NOTE: We can't put big integer value. this is PHP limitation.
        // 4294967295 = (1 << 32) -1 = Maximum unsigned 32-bin int
        self::validateValue(0, 4294967295, $value, "uint",  " php has big numbers limitation. check your PHP_INT_MAX");

        $this->assertOffsetAndLength($offset, 4);
        $this->writeLittleEndian($offset, 4, $value);
    }

    public function putLong(int $offset, int $value): void
    {
        // NOTE: We can't put big integer value. this is PHP limitation.
        self::validateValue(~PHP_INT_MAX, PHP_INT_MAX, $value, "long",  " php has big numbers limitation. check your PHP_INT_MAX");

        $this->assertOffsetAndLength($offset, 8);
        $this->writeLittleEndian($offset, 8, $value);
    }

    public function putUlong(int $offset, int $value): void
    {
        // NOTE: We can't put big integer value. this is PHP limitation.
        self::validateValue(0, PHP_INT_MAX, $value, "long", " php has big numbers limitation. check your PHP_INT_MAX");

        $this->assertOffsetAndLength($offset, 8);
        $this->writeLittleEndian($offset, 8, $value);
    }

    public function putFloat(int $offset, float $value): void
    {
        $this->assertOffsetAndLength($offset, 4);

        $floathelper = pack("f", $value);
        $v = unpack("V", $floathelper);
        $this->writeLittleEndian($offset, 4, $v[1]);
    }

    public function putDouble(int $offset, float $value): void
    {
        $this->assertOffsetAndLength($offset, 8);

        $floathelper = pack("d", $value);
        $v = unpack("V*", $floathelper);

        $this->writeLittleEndian($offset, 4, $v[1]);
        $this->writeLittleEndian($offset + 4, 4, $v[2]);
    }

    public function getByte(int $index): int
    {
        return ord($this->_buffer[$index]);
    }

    public function getSbyte(int $index): int
    {
        $v = unpack("c", $this->_buffer[$index]);
        return $v[1];
    }

    public function getX(string &$buffer): void
    {
        for ($i = $this->_pos, $j = 0; $j < strlen($buffer); $i++, $j++) {
            $buffer[$j] = $this->_buffer[$i];
        }
    }

    public function get(int $index): string
    {
        $this->assertOffsetAndLength($index, 1);
        return $this->_buffer[$index];
    }


    public function getBool(int $index): bool
    {
        return (bool)ord($this->_buffer[$index]);
    }

    public function getShort(int $index): int
    {
        $result = $this->readLittleEndian($index, 2);

        $sign = $index + (ByteBuffer::isLittleEndian() ? 1 : 0);
        $issigned = isset($this->_buffer[$sign]) && ord($this->_buffer[$sign]) & 0x80;

        // 65536 = 1 << 16 = Maximum unsigned 16-bit int
        return $issigned ? $result - 65536 : $result;
    }

    public function getUShort(int $index): int
    {
        return $this->readLittleEndian($index, 2);
    }

    public function getInt(int $index): int
    {
        $result = $this->readLittleEndian($index, 4);

        $sign = $index + (ByteBuffer::isLittleEndian() ? 3 : 0);
        $issigned = isset($this->_buffer[$sign]) && ord($this->_buffer[$sign]) & 0x80;

        if (PHP_INT_SIZE > 4) {
            // 4294967296 = 1 << 32 = Maximum unsigned 32-bit int
            return $issigned ? $result - 4294967296 : $result;
        } else {
            // 32bit / Windows treated number as signed integer.
            return $result;
        }
    }

    public function getUint(int $index): int
    {
        return $this->readLittleEndian($index, 4);
    }

    public function getLong(int $index): int
    {
        return $this->readLittleEndian($index, 8);
    }

    public function getUlong(int $index): int
    {
        return $this->readLittleEndian($index, 8);
    }

    public function getFloat(int $index): float
    {
        $i = $this->readLittleEndian($index, 4);

        return self::convertHelper(self::__FLOAT, $i);
    }

    public function getDouble(int $index): float
    {
        $i = $this->readLittleEndian($index, 4);
        $i2 = $this->readLittleEndian($index + 4, 4);

        return self::convertHelper(self::__DOUBLE, $i, $i2);
    }

    const __SHORT = 1;
    const __INT = 2;
    const __LONG = 3;
    const __FLOAT = 4;
    const __DOUBLE = 5;
    /**
     * @param int $type
     * @param int $value
     * @param int|null $value2
     * @return float
     * @throws \ErrorException
     */
    private static function convertHelper(int $type, int $value, ?int $value2 = null) {
        // readLittleEndian construct unsigned integer value from bytes. we have to encode this value to
        // correct bytes, and decode as expected types with `unpack` function.
        // then it returns correct type value.
        // see also: http://php.net/manual/en/function.pack.php

        switch ($type) {
            case self::__FLOAT:
                $inthelper = pack("V", $value);
                $v = unpack("f", $inthelper);
                return $v[1];
                break;
            case self::__DOUBLE:
                $inthelper = pack("VV", $value, $value2);
                $v = unpack("d", $inthelper);
                return $v[1];
                break;
            default:
                throw new \ErrorException(sprintf("unexpected type %d specified", $type));
        }
    }

    /**
     * @param mixed $value
     * @throws \InvalidArgumentException
     */
    private static function validateValue(int $min, int $max, $value, string $type, string $additional_notes = ""): void {
        if(!($min <= $value && $value <= $max)) {
            throw new \InvalidArgumentException(sprintf("bad number %s for type %s.%s", $value, $type, $additional_notes));
        }
    }
}
