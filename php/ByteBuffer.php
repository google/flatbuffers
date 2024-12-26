<?php

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

declare(strict_types=1);

namespace Google\FlatBuffers;

use Exception;

// phpcs:disable PSR2.Classes.PropertyDeclaration.Underscore

// /**
//  * @return bool
//  */
// function is_little_endian(): bool
// {
//     $v = unpack('S', "\x01\x00");
//     assert($v !== false && array_key_exists(1, $v) && is_int($v[1]));
//     return $v[1] === 1;
// }

// if (!is_little_endian()) {
//     throw new Exception('Big-endian platform is not supported.');
// }

/**
 * @phpstan-type SizeOfType Constants::SIZEOF_BOOL|Constants::SIZEOF_BYTE|Constants::SIZEOF_SHORT|Constants::SIZEOF_INT|Constants::SIZEOF_LONG|Constants::SIZEOF_FLOAT|Constants::SIZEOF_DOUBLE|Constants::FILE_IDENTIFIER_LENGTH
 */
class ByteBuffer
{
    /**
     * @var string $_buffer;
     */
    public string $_buffer;

    /**
     * @var NPosT $npos;
     */
    private int $npos = 0;

    /**
     * @param string $bytes
     * @return ByteBuffer
     */
    public static function wrap(string $bytes): ByteBuffer
    {
        $bb = new ByteBuffer(0);
        $bb->_buffer = $bytes;

        return $bb;
    }

    /**
     * @param BufSizeT $size
     */
    public function __construct(int $size)
    {
        $this->_buffer = str_repeat("\0", $size);
    }

    /**
     * @return ByteBuffer
     * @throws \Exception Exception.
     */
    public function createByGrowing(): self
    {
        $old_size = $this->capacity();
        if (($old_size & 0xC0000000) !== 0) {
            throw new \Exception("FlatBuffers: cannot grow buffer beyond 2 gigabytes");
        }

        // Double the size of buffer.
        $bb = new self($old_size);
        $bb->_buffer .= $this->_buffer;

        $bb->setPosition($old_size); // Is this correct?

        return $bb;
    }

    /**
     * @return BufSizeT
     */
    public function capacity(): int
    {
        return Constants::asBufSize(strlen($this->_buffer));
    }

    /**
     * @return NPosT
     */
    public function getPosition(): int
    {
        return $this->npos;
    }

    /**
     * @param NPosT $npos
     */
    public function setPosition(int $npos): void
    {
        $this->npos = $npos;
    }

    /**
     *
     */
    public function reset(): void
    {
        $this->npos = 0;
    }

    /**
     * @return BufSizeT
     */
    public function length(): int
    {
        return Constants::asBufSize(strlen($this->_buffer) - $this->npos);
    }

    /**
     * @return string
     */
    public function data(): string
    {
        return substr($this->_buffer, $this->npos);
    }

    /**
     * @param NPosT    $npos
     * @param BufSizeT $size
     */
    public function assertOffsetAndLength(int $npos, int $size): void
    {
        if (strlen($this->_buffer) < $npos + $size) {
            throw new \OutOfRangeException(
                sprintf("npos: %d, size: %d, buffer; %d", $npos, $size, strlen($this->_buffer))
            );
        }
    }

    /**
     * @param NPosT            $npos
     * @param SizeOfType       $size
     * @param string           $pack_code
     * @param int|float|double $value
     */
    private function writeToBuffer(int $npos, int $size, string $pack_code, int|float $value): void
    {
        $this->assertOffsetAndLength($npos, $size);
        $bytes = pack($pack_code, $value);
        assert(strlen($bytes) === $size);
        $this->_buffer = substr_replace($this->_buffer, $bytes, $npos, $size);
    }

    /**
     * @param NPosT      $npos
     * @param SizeOfType $size
     * @param string     $pack_code
     * @return int|float|double
     */
    private function readFromBuffer(int $npos, int $size, string $pack_code): int|float
    {
        $this->assertOffsetAndLength($npos, $size);
        $v = unpack($pack_code, $this->_buffer, $npos);
        assert($v !== false && array_key_exists(1, $v));
        return $v[1];
    }

    /**
     * @param NPosT          $npos
     * @param int<-128, 127> $value
     */
    public function putSbyte(int $npos, int $value): void
    {
        self::validateInt(-128, 127, $value, "sbyte");
        $this->writeToBuffer($npos, Constants::SIZEOF_BYTE, 'c', $value);
    }

    /**
     * @param NPosT       $npos
     * @param int<0, 255> $value
     */
    public function putByte(int $npos, int $value): void
    {
        self::validateInt(0, 255, $value, "byte");
        $this->writeToBuffer($npos, Constants::SIZEOF_BYTE, 'C', $value);
    }

    /**
     * @param NPosT  $npos
     * @param string $value
     */
    public function put(int $npos, string $value): void
    {
        $size = Constants::asBufSize(strlen($value));
        $this->assertOffsetAndLength($npos, $size);
        $this->_buffer = substr_replace($this->_buffer, $value, $npos, $size);
    }

    /**
     * @param NPosT $npos
     * @param bool  $value
     */
    public function putBool(int $npos, bool $value): void
    {
        $this->writeToBuffer($npos, Constants::SIZEOF_BYTE, 'C', (int)$value);
    }

    /**
     * @param NPosT              $npos
     * @param int<-32768, 32767> $value
     */
    public function putShort(int $npos, int $value): void
    {
        self::validateInt(-32768, 32767, $value, "short");
        $this->writeToBuffer($npos, Constants::SIZEOF_SHORT, 's', $value);
    }

    /**
     * @param NPosT         $npos
     * @param int<0, 65535> $value
     */
    public function putUshort(int $npos, int $value): void
    {
        self::validateInt(0, 65535, $value, "ushort");
        $this->writeToBuffer($npos, Constants::SIZEOF_SHORT, 'v', $value);
    }

    /**
     * @param NPosT                        $npos
     * @param int<-2147483648, 2147483647> $value
     */
    public function putInt(int $npos, int $value): void
    {
        // 2147483647 = (1 << 31) -1 = Maximum signed 32-bit int
        // -2147483648 = -1 << 31 = Minimum signed 32-bit int
        self::validateInt(-2147483648, 2147483647, $value, "int");
        $this->writeToBuffer($npos, Constants::SIZEOF_INT, 'i', $value);
    }

    /**
     * @param NPosT    $npos
     * @param SOffsetT $value
     */
    public function putSOffset(int $npos, int $value): void
    {
        $this->putInt($npos, $value);
    }

    /**
     * @param NPosT              $npos
     * @param int<0, 4294967295> $value
     */
    public function putUint(int $npos, int $value): void
    {
        // NOTE: We can't put big integer value. this is PHP limitation.
        // 4294967295 = (1 << 32) -1 = Maximum unsigned 32-bin int
        self::validateInt(0, 4294967295, $value, "uint", " php has big numbers limitation. check your PHP_INT_MAX");
        $this->writeToBuffer($npos, Constants::SIZEOF_INT, 'I', $value);
    }

    /**
     * @param NPosT $npos
     * @param int   $value
     */
    public function putLong(int $npos, int $value): void
    {
        // NOTE: We can't put big integer value. this is PHP limitation.
        self::validateInt(
            ~PHP_INT_MAX,
            PHP_INT_MAX,
            $value,
            "long",
            " php has big numbers limitation. check your PHP_INT_MAX"
        );
        $this->writeToBuffer($npos, Constants::SIZEOF_LONG, 'q', $value);
    }

    /**
     * @param NPosT       $npos
     * @param int<0, max> $value
     */
    public function putUlong(int $npos, int $value): void
    {
        // NOTE: We can't put big integer value. this is PHP limitation.
        self::validateInt(0, PHP_INT_MAX, $value, "long", " php has big numbers limitation. check your PHP_INT_MAX");
        $this->writeToBuffer($npos, Constants::SIZEOF_LONG, 'P', $value);
    }

    /**
     * @param NPosT $npos
     * @param float $value
     */
    public function putFloat(int $npos, float $value): void
    {
        $this->assertOffsetAndLength($npos, Constants::SIZEOF_FLOAT);
        $this->writeToBuffer($npos, Constants::SIZEOF_FLOAT, 'f', $value);
    }

    /**
     * @param NPosT  $npos
     * @param double $value
     */
    public function putDouble(int $npos, float $value): void
    {
        $this->assertOffsetAndLength($npos, Constants::SIZEOF_DOUBLE);
        $this->writeToBuffer($npos, Constants::SIZEOF_DOUBLE, 'd', $value);
    }

    /**
     * @param NPosT $npos
     * @return int<0, 255>
     */
    public function getByte(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_BYTE, 'C');
        assert(is_int($value) && 0 <= $value && $value <= 255);
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return int<-128, 127>
     */
    public function getSbyte(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_BYTE, 'c');
        assert(is_int($value) && -128 <= $value && $value <= 127);
        return $value;
    }

    /**
     * @param NPosT    $npos
     * @param BufSizeT $size
     * @return string
     */
    public function get(int $npos, int $size): string
    {
        $this->assertOffsetAndLength($npos, $size);
        return substr($this->_buffer, $npos, $size);
    }

    /**
     * @param NPosT $npos
     * @return bool
     */
    public function getBool(int $npos): bool
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_BYTE, 'C');
        assert(is_int($value) && 0 <= $value && $value <= 255);
        return (bool)$value;
    }

    /**
     * @param NPosT $npos
     * @return int<-32768, 32767>
     */
    public function getShort(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_SHORT, 's');
        assert(is_int($value) && -32768 <= $value && $value <= 32767);
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return int<0, 65535>
     */
    public function getUshort(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_SHORT, 'v');
        assert(is_int($value) && 0 <= $value && $value <= 65535);
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return VOffsetT
     */
    public function getVOffset(int $npos): int
    {
        return $this->getUshort($npos);
    }

    /**
     * @param NPosT $npos
     * @return int<-2147483648, 2147483647>
     */
    public function getInt(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_INT, 'i');
        assert(is_int($value) && -2147483648 <= $value && $value <= 2147483647);
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return SOffsetT
     */
    public function getSOffset(int $npos): int
    {
        return $this->getInt($npos);
    }

    /**
     * @param NPosT $npos
     * @return int<0, 4294967295>
     */
    public function getUint(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_INT, 'I');
        assert(is_int($value) && 0 <= $value && $value <= 4294967295);
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return UOffsetT
     */
    public function getUOffset(int $npos): int
    {
        return $this->getUint($npos);
    }

    /**
     * @param NPosT $npos
     * @return int
     */
    public function getLong(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_LONG, 'q');
        assert(is_int($value));
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return int<0, max>
     */
    public function getUlong(int $npos): int
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_LONG, 'P');
        assert(is_int($value) && 0 <= $value);
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return float
     */
    public function getFloat(int $npos): float
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_FLOAT, 'f');
        assert(is_float($value));
        return $value;
    }

    /**
     * @param NPosT $npos
     * @return double
     */
    public function getDouble(int $npos): float
    {
        $value = $this->readFromBuffer($npos, Constants::SIZEOF_DOUBLE, 'd');
        assert(is_double($value));
        return $value;
    }

    /**
     * @param int    $min
     * @param int    $max
     * @param int    $value
     * @param string $type
     * @param string $additional_notes
     */
    private static function validateInt(
        int $min,
        int $max,
        int $value,
        string $type,
        string $additional_notes = ""
    ): void {
        if ($min <= $value && $value <= $max) {
            return;
        }

        throw new \InvalidArgumentException(sprintf("bad number %s for type %s.%s", $value, $type, $additional_notes));
    }

    /**
     * @param NPosT $t_npos
     * @return NPosT
     */
    public function followSOffset(int $t_npos): int
    {
        return Constants::asNPos($t_npos - $this->getSOffset($t_npos));
    }

    /**
     * @param NPosT $npos
     * @return NPosT
     */
    public function followUOffset(int $npos): int
    {
        return Constants::asNPos($this->getUOffset($npos) + $npos);
    }

    /**
     * Write position to normal position conversion.
     *
     * @param WPosT $wpos
     * @return NPosT
     */
    public function wposToNpos(int $wpos): int
    {
        return Constants::asNPos($this->capacity() - $wpos);
    }

    /**
     * Normal position to write position conversion.
     *
     * @param NPosT $npos
     * @return WPosT
     */
    public function nposToWpos(int $npos): int
    {
        return Constants::asWPos($this->capacity() - $npos);
    }
}
