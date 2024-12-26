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

/// @file
/// @addtogroup flatbuffers_php_api
/// @{

// phpcs:disable PSR1.Methods.CamelCapsMethodName.NotCamelCaps

declare(strict_types=1);

namespace Google\FlatBuffers;

final class FlatbufferBuilder
{
    /**
     * Internal ByteBuffer for the FlatBuffer data.
     *
     * @var ByteBuffer $bb
     */
    public ByteBuffer $bb;

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @var NPosT $space Space: [0, $space).
     */
    protected int $space;

    /**
     * @var int<1, 8> $minalign
     */
    protected int $minalign = 1;

    /**
     * @var ?array<VOffsetT, WPosT> $vtable
     */
    protected ?array $vtable = null;

    /**
     * @var VOffsetT $vtable_in_use
     */
    protected int $vtable_in_use = 0;

    /**
     * @var bool $nested
     */
    protected bool $nested = false;

    /**
     * @var WPosT $object_start
     */
    protected int $object_start;

    /**
     * @var array<int<0, max>, WPosT> $vtables
     */
    protected array $vtables = [];

    /**
     * @var int<0, max> $num_vtables
     */
    protected int $num_vtables = 0;

    /**
     * @var UOffsetT $vector_num_elems
     */
    protected int $vector_num_elems = 0;

    /**
     * @var bool $force_defaults
     */
    protected bool $force_defaults = false;
    /// @endcond

    /**
     * Create a FlatBufferBuilder with a given initial size.
     *
     * @param BufSizeT $initial_size Initial byte buffer size.
     */
    public function __construct(int $initial_size)
    {
        if ($initial_size <= 0) {
            $initial_size = 1;
        }
        $this->space = $initial_size;
        $this->bb = $this->newByteBuffer($initial_size);
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * Create new bytebuffer.
     *
     * @param BufSizeT $size
     * @return ByteBuffer
     */
    private function newByteBuffer(int $size): ByteBuffer
    {
        return new ByteBuffer($size);
    }

    /**
     * Returns the current ByteBuffer offset.
     *
     * @return WPosT
     */
    public function offset(): int
    {
        return $this->wpos();
    }

    /**
     * Returns the current ByteBuffer offset.
     *
     * @return WPosT
     */
    public function wpos(): int
    {
        return $this->bb->nposToWpos($this->space);
    }

    /**
     * Padding buffer.
     *
     * @param int<0, 8> $byte_size
     */
    public function pad(int $byte_size): void
    {
        for ($i = 0; $i < $byte_size; $i++) {
            assert(0 < $this->space);
            $this->bb->putByte(--$this->space, 0);
        }
    }

    /**
     * Prepare bytebuffer.
     *
     * @param int<1, 8> $size
     * @param int       $additional_bytes
     */
    public function prep(int $size, int $additional_bytes): void
    {
        if ($size > $this->minalign) {
            // TODO: Is this correct?
            $this->minalign = $size;
        }

        $align_size = (~($this->wpos() + $additional_bytes) + 1) & ($size - 1);
        assert(0 <= $align_size && $align_size < 8);
        while ($this->space < $align_size + $size + $additional_bytes) {
            $old_buf_size = $this->bb->capacity();
            $this->bb = $this->bb->createByGrowing();
            $new_buf_size = $this->bb->capacity();
            $this->space = Constants::asNPos($new_buf_size - $old_buf_size + $this->space);
        }

        $this->pad($align_size);
    }

    /**
     * @param BoolT $x
     */
    public function putBool(bool $x): void
    {
        assert(Constants::SIZEOF_BOOL <= $this->space);
        $this->bb->putBool($this->space -= Constants::SIZEOF_BOOL, $x);
    }

    /**
     * @param ByteT $x
     */
    public function putByte(int $x): void
    {
        assert(Constants::SIZEOF_BYTE <= $this->space);
        $this->bb->putByte($this->space -= Constants::SIZEOF_BYTE, $x);
    }

    /**
     * @param SbyteT $x
     */
    public function putSbyte(int $x): void
    {
        assert(Constants::SIZEOF_BYTE <= $this->space);
        $this->bb->putSbyte($this->space -= Constants::SIZEOF_BYTE, $x);
    }

    /**
     * @param ShortT $x
     */
    public function putShort(int $x): void
    {
        assert(Constants::SIZEOF_SHORT <= $this->space);
        $this->bb->putShort($this->space -= Constants::SIZEOF_SHORT, $x);
    }

    /**
     * @param UshortT $x
     */
    public function putUshort(int $x): void
    {
        assert(Constants::SIZEOF_SHORT <= $this->space);
        $this->bb->putUshort($this->space -= Constants::SIZEOF_SHORT, $x);
    }

    /**
     * @param IntT $x
     */
    public function putInt(int $x): void
    {
        assert(Constants::SIZEOF_INT <= $this->space);
        $this->bb->putInt($this->space -= Constants::SIZEOF_INT, $x);
    }

    /**
     * @param UintT $x
     */
    public function putUint(int $x): void
    {
        if ($x > PHP_INT_MAX) {
            throw new \InvalidArgumentException("your platform can't handle uint correctly. use 64bit machine.");
        }

        assert(Constants::SIZEOF_INT <= $this->space);
        $this->bb->putUint($this->space -= Constants::SIZEOF_INT, $x);
    }

    /**
     * @param UOffsetT $x
     */
    public function putUOffset(int $x): void
    {
        $this->putUint($x);
    }

    /**
     * @param LongT $x
     */
    public function putLong(int $x): void
    {
        if ($x > PHP_INT_MAX) {
            throw new \InvalidArgumentException("Your platform can't handle long correctly. Use a 64bit machine.");
        }

        assert(Constants::SIZEOF_LONG <= $this->space);
        $this->bb->putLong($this->space -= Constants::SIZEOF_LONG, $x);
    }

    /**
     * @param UlongT $x
     */
    public function putUlong(int $x): void
    {
        if ($x > PHP_INT_MAX) {
            throw new \InvalidArgumentException(
                "Your platform can't handle ulong correctly. This is a php limitation. " .
                "Please wait for the extension release."
            );
        }

        assert(Constants::SIZEOF_LONG <= $this->space);
        $this->bb->putUlong($this->space -= Constants::SIZEOF_LONG, $x);
    }

    /**
     * @param FloatT $x
     */
    public function putFloat(float $x): void
    {
        assert(Constants::SIZEOF_FLOAT <= $this->space);
        $this->bb->putFloat($this->space -= Constants::SIZEOF_FLOAT, $x);
    }

    /**
     * @param DoubleT $x
     */
    public function putDouble(float $x): void
    {
        assert(Constants::SIZEOF_DOUBLE <= $this->space);
        $this->bb->putDouble($this->space -= Constants::SIZEOF_DOUBLE, $x);
    }

    /**
     * TODO: Rename to "putUOffsetOf".
     *
     * @param WPosT $wpos
     */
    public function putOffset(int $wpos): void
    {
        $this->putUOffset(Constants::asUOffset($this->wpos() - $wpos + Constants::SIZEOF_UOFFSET));
    }
    /// @endcond

    /**
     * Add a `bool` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param BoolT $x The `bool` to add to the buffer.
     */
    public function addBool(bool $x): void
    {
        $this->prep(Constants::SIZEOF_BOOL, 0);
        $this->putBool($x);
    }

    /**
     * Add a `byte` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param ByteT $x The `byte` to add to the buffer.
     */
    public function addByte(int $x): void
    {
        $this->prep(Constants::SIZEOF_BYTE, 0);
        $this->putByte($x);
    }

    /**
     * Add a `signed byte` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param SbyteT $x The `signed byte` to add to the buffer.
     */
    public function addSbyte(int $x): void
    {
        $this->prep(Constants::SIZEOF_BYTE, 0);
        $this->putSbyte($x);
    }

    /**
     * Add a `short` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param ShortT $x The `short` to add to the buffer.
     */
    public function addShort(int $x): void
    {
        $this->prep(Constants::SIZEOF_SHORT, 0);
        $this->putShort($x);
    }

    /**
     * Add an `unsigned short` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param UshortT $x The `unsigned short` to add to the buffer.
     */
    public function addUshort(int $x): void
    {
        $this->prep(Constants::SIZEOF_SHORT, 0);
        $this->putUshort($x);
    }

    /**
     * @param VOffsetT $x The VOffset value to add to the buffer.
     */
    public function addVOffset(int $x): void
    {
        $this->addUshort($x);
    }

    /**
     * Add an `int` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param IntT $x The `int` to add to the buffer.
     */
    public function addInt(int $x): void
    {
        $this->prep(Constants::SIZEOF_INT, 0);
        $this->putInt($x);
    }

    /**
     * @param SOffsetT $x The SOffset value to add to the buffer.
     */
    public function addSOffset(int $x): void
    {
        $this->prep(Constants::SIZEOF_INT, 0);
        $this->putInt($x);
    }

    /**
     * Add an `unsigned int` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param UintT $x The `unsigned int` to add to the buffer.
     */
    public function addUint(int $x): void
    {
        $this->prep(Constants::SIZEOF_INT, 0);
        $this->putUint($x);
    }

    /**
     * Add a `long` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param LongT $x The `long` to add to the buffer.
     */
    public function addLong(int $x): void
    {
        $this->prep(Constants::SIZEOF_LONG, 0);
        $this->putLong($x);
    }

    /**
     * Add an `unsigned long` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param UlongT $x The `unsigned long` to add to the buffer.
     */
    public function addUlong(int $x): void
    {
        $this->prep(Constants::SIZEOF_LONG, 0);
        $this->putUlong($x);
    }

    /**
     * Add a `float` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param FloatT $x The `float` to add to the buffer.
     */
    public function addFloat(float $x): void
    {
        $this->prep(Constants::SIZEOF_FLOAT, 0);
        $this->putFloat($x);
    }

    /**
     * Add a `double` to the buffer, properly aligned, and grows the buffer (if necessary).
     *
     * @param DoubleT $x The `double` to add to the buffer.
     */
    public function addDouble(float $x): void
    {
        $this->prep(Constants::SIZEOF_DOUBLE, 0);
        $this->putDouble($x);
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @param VOffsetT $o
     * @param bool     $x
     * @param bool     $d
     */
    public function addBoolX(int $o, bool $x, bool $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addBool($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param ByteT    $x
     * @param ByteT    $d
     */
    public function addByteX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addByte($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param SbyteT   $x
     * @param SbyteT   $d
     */
    public function addSbyteX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addSbyte($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param ShortT   $x
     * @param ShortT   $d
     */
    public function addShortX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addShort($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param UshortT  $x
     * @param UshortT  $d
     */
    public function addUshortX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addUshort($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param IntT     $x
     * @param IntT     $d
     */
    public function addIntX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addInt($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param UintT    $x
     * @param UintT    $d
     */
    public function addUintX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addUint($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param LongT    $x
     * @param LongT    $d
     */
    public function addLongX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addLong($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param UlongT   $x
     * @param UlongT   $d
     */
    public function addUlongX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addUlong($x);
            $this->slot($o);
        }
    }


    /**
     * @param VOffsetT $o
     * @param FloatT   $x
     * @param FloatT   $d
     */
    public function addFloatX(int $o, float $x, float $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addFloat($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param DoubleT  $x
     * @param DoubleT  $d
     */
    public function addDoubleX(int $o, float $x, float $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addDouble($x);
            $this->slot($o);
        }
    }

    /**
     * @param VOffsetT $o
     * @param WPosT    $x
     * @param WPosT    $d
     * @throws \Exception Exception.
     */
    public function addOffsetX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x !== $d) {
            $this->addOffset($x);
            $this->slot($o);
        }
    }
    /// @endcond

    /**
     * Adds on offset, relative to where it will be written.
     *
     * @param WPosT $wpos The offset to add to the buffer.
     * @throws \Exception Throws an exception if `$wpos` is greater than the underlying ByteBuffer's offest.
     */
    public function addOffset(int $wpos): void
    {
        $this->prep(Constants::SIZEOF_UOFFSET, 0); // Ensure alignment is already done
        if ($wpos > $this->wpos()) {
            throw new \Exception("");
        }
        $this->putOffset($wpos);
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @param int<1, 8> $elem_size
     * @param UOffsetT  $num_elems
     * @param int<1, 8> $alignment
     * @throws \Exception Exception.
     */
    public function startVector(int $elem_size, int $num_elems, int $alignment): void
    {
        $this->notNested();
        $this->vector_num_elems = $num_elems;
        $this->prep(Constants::SIZEOF_UOFFSET, $elem_size * $num_elems);
        // Just in case alignemnt > UOffsetT;
        $this->prep($alignment, $elem_size * $num_elems);
    }

    /**
     * @return WPosT
     */
    public function endVector(): int
    {
        $this->putUOffset($this->vector_num_elems);
        return $this->wpos();
    }

    /**
     * @param string $bytes
     * @return bool
     */
    protected function is_utf8(string $bytes): bool
    {
        if (function_exists('mb_detect_encoding')) {
            return (bool) mb_detect_encoding($bytes, 'UTF-8', true);
        }

        $len = strlen($bytes);
        if ($len < 1) {
            /* NOTE: always return 1 when passed string is null */
            return true;
        }

        for ($j = 0, $i = 0; $i < $len; $i++) {
            // check ACII
            if (
                $bytes[$j] === "\x09" ||
                $bytes[$j] === "\x0A" ||
                $bytes[$j] === "\x0D" ||
                ($bytes[$j] >= "\x20" && $bytes[$j] <= "\x7E")
            ) {
                $j++;
                continue;
            }

            /* non-overlong 2-byte */
            if (
                (($i + 1) <= $len) &&
                ($bytes[$j] >= "\xC2" && $bytes[$j] <= "\xDF" &&
                    ($bytes[$j + 1] >= "\x80" && $bytes[$j + 1] <= "\xBF"))
            ) {
                $j += 2;
                $i++;
                continue;
            }

            /* excluding overlongs */
            if (
                (($i + 2) <= $len) &&
                $bytes[$j] === "\xE0" &&
                ($bytes[$j + 1] >= "\xA0" && $bytes[$j + 1] <= "\xBF" &&
                    ($bytes[$j + 2] >= "\x80" && $bytes[$j + 2] <= "\xBF"))
            ) {
                $bytes += 3;
                $i += 2;
                continue;
            }

            /* straight 3-byte */
            if (
                (($i + 2) <= $len) &&
                (($bytes[$j] >= "\xE1" && $bytes[$j] <= "\xEC") ||
                    $bytes[$j] === "\xEE" ||
                    $bytes[$j] === "\xEF") &&
                ($bytes[$j + 1] >= "\x80" && $bytes[$j + 1] <= "\xBF") &&
                ($bytes[$j + 2] >= "\x80" && $bytes[$j + 2] <= "\xBF")
            ) {
                $j += 3;
                $i += 2;
                continue;
            }

            /* excluding surrogates */
            if (
                (($i + 2) <= $len) &&
                $bytes[$j] === "\xED" &&
                ($bytes[$j + 1] >= "\x80" && $bytes[$j + 1] <= "\x9f" &&
                    ($bytes[$j + 2] >= "\x80" && $bytes[$j + 2] <= "\xBF"))
            ) {
                $j += 3;
                $i += 2;
                continue;
            }

            /* planes 1-3 */
            if (
                (($i + 3) <= $len) &&
                $bytes[$j] === "\xF0" &&
                ($bytes[$j + 1] >= "\x90" && $bytes[$j + 1] <= "\xBF") &&
                ($bytes[$j + 2] >= "\x80" && $bytes[$j + 2] <= "\xBF") &&
                ($bytes[$j + 3] >= "\x80" && $bytes[$j + 3] <= "\xBF")
            ) {
                $j += 4;
                $i += 3;
                continue;
            }


            /* planes 4-15 */
            if (
                (($i + 3) <= $len) &&
                $bytes[$j] >= "\xF1" && $bytes[$j] <= "\xF3" &&
                $bytes[$j + 1] >= "\x80" && $bytes[$j + 1] <= "\xBF" &&
                $bytes[$j + 2] >= "\x80" && $bytes[$j + 2] <= "\xBF" &&
                $bytes[$j + 3] >= "\x80" && $bytes[$j + 3] <= "\xBF"
            ) {
                $j += 4;
                $i += 3;
                continue;
            }

            /* plane 16 */
            if (
                (($i + 3) <= $len) &&
                $bytes[$j] === "\xF4" &&
                ($bytes[$j + 1] >= "\x80" && $bytes[$j + 1] <= "\x8F") &&
                ($bytes[$j + 2] >= "\x80" && $bytes[$j + 2] <= "\xBF") &&
                ($bytes[$j + 3] >= "\x80" && $bytes[$j + 3] <= "\xBF")
            ) {
                $bytes += 4;
                $i += 3;
                continue;
            }


            return false;
        }

        return true;
    }
    /// @endcond

    /**
     * Encode the string `$s` in the buffer using UTF-8.
     *
     * @param string $s The string to encode.
     * @return WPosT The offset in the buffer where the encoded string starts.
     * @throws \InvalidArgumentException Thrown if the input string `$s` is not UTF-8.
     */
    public function createString(string $s): int
    {
        if (!$this->is_utf8($s)) {
            throw new \InvalidArgumentException("string must be utf-8 encoded value.");
        }

        $s_size = strlen($s);

        $this->notNested();
        $this->addByte(0); // null terminated
        $this->startVector(1, Constants::asUOffset($s_size), 1);
        $this->space = Constants::asNPos($this->space - $s_size);
        $this->bb->put($this->space, $s);
        return $this->endVector();
    }

    /**
     * Encode the byte string `$bs` in the buffer.
     *
     * @param string $bs The byte string to encode.
     * @return WPosT The offset in the buffer where the encoded string starts.
     */
    public function createByteString(string $bs): int
    {
        $s_size = strlen($bs);

        $this->notNested();
        $this->startVector(1, Constants::asUOffset($s_size), 1);
        $this->space = Constants::asNPos($this->space - $s_size);
        $this->bb->put($this->space, $bs);
        return $this->endVector();
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @throws \Exception Exception.
     */
    public function notNested(): void
    {
        if ($this->nested) {
            throw new \Exception("FlatBuffers; object serialization must not be nested");
        }
    }

    /**
     * @param WPosT $obj
     * @throws \Exception Exception.
     */
    public function nested(int $obj): void
    {
        if ($obj !== $this->wpos()) {
            throw new \Exception("FlatBuffers: struct must be serialized inline");
        }
    }

    /**
     * @param VOffsetT $numfields
     * @throws \Exception Exception.
     */
    public function startObject(int $numfields): void
    {
        $this->notNested();
        if ($this->vtable === null || count($this->vtable) < $numfields) {
            $this->vtable = [];
        }

        $this->vtable_in_use = $numfields;
        for ($i = 0; $i < $numfields; $i++) {
            $this->vtable[$i] = 0;
        }

        $this->nested = true;
        $this->object_start = $this->wpos();
    }

    // TODO: Is this correct? Both `addStructX` and `addStruct` have the same definition.
    /**
     * @param VOffsetT $voffset
     * @param WPosT    $x
     * @param WPosT    $d
     * @throws \Exception Exception.
     */
    public function addStructX(int $voffset, int $x, int $d): void
    {
        if ($x !== $d) {
            $this->nested($x);
            $this->slot($voffset);
        }
    }

    /**
     * @param VOffsetT $voffset
     * @param WPosT    $x
     * @param WPosT    $d
     * @throws \Exception Exception.
     */
    public function addStruct(int $voffset, int $x, int $d): void
    {
        if ($x !== $d) {
            $this->nested($x);
            $this->slot($voffset);
        }
    }

    /**
     * @param VOffsetT $voffset
     */
    public function slot(int $voffset): void
    {
        $this->vtable[$voffset] = $this->wpos();
    }

    /**
     * @return WPosT
     * @throws \Exception Exception.
     */
    public function endObject(): int
    {
        if ($this->vtable === null || !$this->nested) {
            throw new \Exception("FlatBuffers: endObject called without startObject");
        }

        $this->addSOffset(0);
        $t_wpos = $this->wpos();

        assert($this->vtable !== null);
        $i = $this->vtable_in_use - 1;
        // Trim trailing zeroes.
        for (; $i >= 0 && $this->vtable[$i] === 0; $i--) {
        }
        $effective_size = $i + 1;
        for (; $i >= 0; $i--) {
            assert($this->vtable !== null);
            $this->addVOffset(
                Constants::asVOffset($this->vtable[$i] !== 0 ? $t_wpos - $this->vtable[$i] : 0)
            );
        }

        $this->addVOffset(Constants::asVOffset($t_wpos - $this->object_start));
        $this->addVOffset(Constants::asVOffset(($effective_size + 2) * Constants::SIZEOF_VOFFSET));

        // search for an existing vtable that matches the current one.
        $shared_vt_wpos = null;

        $current_vt_npos = $this->space;
        $current_vt_voffset = $this->bb->getVOffset($current_vt_npos);
        for ($i = 0; $i < $this->num_vtables; $i++) {
            $vt_npos = $this->bb->wposToNpos($this->vtables[$i]);
            $vt_voffset = $this->bb->getVOffset($vt_npos);

            if ($vt_voffset === $current_vt_voffset) {
                for ($j = Constants::SIZEOF_VOFFSET; $j < $vt_voffset; $j += Constants::SIZEOF_VOFFSET) {
                    if (
                        $this->bb->getVOffset(Constants::asVOffset($vt_npos + $j)) !==
                        $this->bb->getVOffset(Constants::asVOffset($current_vt_npos + $j))
                    ) {
                        continue 2;
                    }
                }
                $shared_vt_wpos = $this->vtables[$i];
                break;
            }
        }

        if ($shared_vt_wpos !== null) {
            // Found a match:
            // Remove the current vtable
            $this->space = $this->bb->wposToNpos($t_wpos);
            $this->bb->putSOffset(
                $this->space,
                Constants::asSOffset($shared_vt_wpos - $t_wpos)
            );
        } else {
            // No Match:
            // Add the location of the current vtable to the list of vtables
            if ($this->num_vtables === count($this->vtables)) {
                $vtables = $this->vtables;
                $this->vtables = [];
                // copy of
                for ($i = 0; $i < count($vtables) * 2; $i++) {
                    $this->vtables[$i] = $i < count($vtables) ? $vtables[$i] : 0;
                }
            }
            $this->vtables[$this->num_vtables++] = $this->wpos();
            $this->bb->putSOffset(
                $this->bb->wposToNpos($t_wpos),
                Constants::asSOffset($this->wpos() - $t_wpos)
            );
        }

        $this->nested = false;
        $this->vtable = null;
        return $t_wpos;
    }

    /**
     * @param WPosT    $t_wpos
     * @param VOffsetT $vt_f_voffset
     * @throws \Exception Exception.
     */
    public function required(int $t_wpos, int $vt_f_voffset): void
    {
        $vt_npos = $this->bb->followSOffset($this->bb->wposToNpos($t_wpos));
        $ok = $this->bb->getVOffset(Constants::asNPos($vt_npos + $vt_f_voffset)) !== 0;

        if (!$ok) {
            throw new \Exception("FlatBuffers: field "  . $vt_f_voffset  .  " must be set");
        }
    }
    /// @endcond

    /**
     * Finalize a buffer, pointing to the given `$root_table_wpos`.
     *
     * @param WPosT   $root_table_wpos An offest to be added to the buffer.
     * @param ?string $identifier      A FlatBuffer file identifier to be added to the
     *                                 buffer before `$root_table_wpos`. This defaults to `null`.
     * @throws \InvalidArgumentException Thrown if an invalid `$identifier` is given, where its
     *                                   length is not equal to `Constants::FILE_IDENTIFIER_LENGTH`.
     */
    public function finish(int $root_table_wpos, ?string $identifier = null): void
    {
        if ($identifier === null) {
            $this->prep($this->minalign, Constants::SIZEOF_INT);
            $this->addOffset($root_table_wpos);
            $this->bb->setPosition($this->space);
        } else {
            $this->prep($this->minalign, Constants::SIZEOF_INT + Constants::FILE_IDENTIFIER_LENGTH);
            if (strlen($identifier) !== Constants::FILE_IDENTIFIER_LENGTH) {
                throw new \InvalidArgumentException(
                    sprintf(
                        "FlatBuffers: file identifier must be length %d",
                        Constants::FILE_IDENTIFIER_LENGTH
                    )
                );
            }

            for ($i = Constants::FILE_IDENTIFIER_LENGTH - 1; $i >= 0; $i--) {
                $this->addByte(ord($identifier[$i]));
            }
            $this->finish($root_table_wpos);
        }
    }

    /**
     * In order to save space, fields that are set to their default value don't
     * get serialized into the buffer.
     *
     * @param bool $forceDefaults When set to `true`, always serializes default values.
     */
    public function forceDefaults(bool $forceDefaults): void
    {
        $this->force_defaults = $forceDefaults;
    }

    /**
     * Get the ByteBuffer representing the FlatBuffer.
     *
     * @return ByteBuffer The ByteBuffer containing the FlatBuffer data.
     */
    public function dataBuffer(): ByteBuffer
    {
        return $this->bb;
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @return int
     */
    public function dataStart(): int
    {
        return $this->space;
    }
    /// @endcond

    /**
     * Utility function to copy and return the FlatBuffer data from the
     * underlying ByteBuffer.
     *
     * @return string A string (representing a byte[]) that contains a copy of the FlatBuffer data.
     */
    public function sizedByteArray(): string
    {
        $this->bb->setPosition($this->space);
        return $this->bb->get(
            $this->bb->getPosition(),
            Constants::asBufSize($this->bb->nposToWpos($this->space))
        );
    }
}

/// @}
