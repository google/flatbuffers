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

/// @file
/// @addtogroup flatbuffers_php_api
/// @{

namespace Google\FlatBuffers;

final class FlatBufferBuilder
{
    /**
     * Internal ByteBuffer for the FlatBuffer data.
     * @var ByteBuffer $bb
     */
    public $bb;

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @var int $space
     */
    protected $space;

    /**
     * @var int
     */
    protected $minalign = 1;

    /**
     * @var int[]|null
     */
    protected $vtable;

    /**
     * @var int
     */
    protected $vtable_in_use = 0;

    /**
     * @var bool $nested
     */
    protected $nested = false;

    /**
     * @var int $object_start
     */
    protected $object_start;

    /**
     * @var int[] $vtables
     */
    protected $vtables = array();

    /**
     * @var int $num_vtables
     */
    protected $num_vtables = 0;

    /**
     * @var int $vector_num_elems
     */
    protected $vector_num_elems = 0;

    /**
     * @var bool $force_defaults
     */
    protected $force_defaults = false;
    /// @endcond

    /**
     * Create a FlatBufferBuilder with a given initial size.
     *
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
     * create new bytebuffer
     */
    private function newByteBuffer(int $size): ByteBuffer
    {
        return new ByteBuffer($size);
    }

    /**
     * Returns the current ByteBuffer offset.
     */
    public function offset(): int
    {
        return $this->bb->capacity() - $this->space;
    }

    /**
     * padding buffer
     */
    public function pad(int $byte_size): void
    {
        for ($i = 0; $i < $byte_size; $i++) {
            $this->bb->putByte(--$this->space, "\0");
        }
    }

    /**
     * prepare bytebuffer
     * @throws \Exception
     */
    public function prep(int $size, int $additional_bytes): void
    {
        if ($size > $this->minalign) {
            $this->minalign = $size;
        }

        $align_size = ((~($this->bb->capacity() - $this->space + $additional_bytes)) + 1) & ($size - 1);
        while ($this->space < $align_size + $size  + $additional_bytes) {
            $old_buf_size = $this->bb->capacity();
            $this->bb = $this->growByteBuffer($this->bb);
            $this->space += $this->bb->capacity() - $old_buf_size;
        }

        $this->pad($align_size);
    }

    /**
     * @throws \ErrorException
     */
    private static function growByteBuffer(ByteBuffer $bb): ByteBuffer
    {
        $old_buf_size = $bb->capacity();
        if (($old_buf_size & 0xC0000000) != 0) {
            throw new \ErrorException("FlatBuffers: cannot grow buffer beyond 2 gigabytes");
        }
        $new_buf_size = $old_buf_size << 1;

        $bb->setPosition(0);
        $nbb = new ByteBuffer($new_buf_size);

        $nbb->setPosition($new_buf_size - $old_buf_size);

        // TODO(chobie): is this little bit faster?
        //$nbb->_buffer = substr_replace($nbb->_buffer, $bb->_buffer, $new_buf_size - $old_buf_size, strlen($bb->_buffer));
        for ($i = $new_buf_size - $old_buf_size, $j = 0; $j < strlen($bb->_buffer); $i++, $j++) {
            $nbb->_buffer[$i] = $bb->_buffer[$j];
        }

        return $nbb;
    }

    public function putBool(bool $x): void
    {
        $this->bb->put($this->space -= 1, chr((int)(bool)($x)));
    }

    public function putByte(int $x): void
    {
        $this->bb->put($this->space -= 1, chr($x));
    }

    public function putSbyte(int $x): void
    {
        $this->bb->put($this->space -= 1, chr($x));
    }

    public function putShort(int $x): void
    {
        $this->bb->putShort($this->space -= 2, $x);
    }

    public function putUshort(int $x): void
    {
        $this->bb->putUshort($this->space -= 2, $x);
    }

    public function putInt(int $x): void
    {
        $this->bb->putInt($this->space -= 4, $x);
    }

    public function putUint(int $x): void
    {
        if ($x > PHP_INT_MAX) {
            throw new \InvalidArgumentException("your platform can't handle uint correctly. use 64bit machine.");
        }

        $this->bb->putUint($this->space -= 4, $x);
    }

    public function putLong(int $x): void
    {
        if ($x > PHP_INT_MAX) {
            throw new \InvalidArgumentException("Your platform can't handle long correctly. Use a 64bit machine.");
        }

        $this->bb->putLong($this->space -= 8, $x);
    }

    public function putUlong(int $x): void
    {
        if ($x > PHP_INT_MAX) {
            throw new \InvalidArgumentException("Your platform can't handle ulong correctly. This is a php limitation. Please wait for the extension release.");
        }

        $this->bb->putUlong($this->space -= 8, $x);
    }

    public function putFloat(float $x): void
    {
        $this->bb->putFloat($this->space -= 4, $x);
    }

    public function putDouble(float $x): void
    {
        $this->bb->putDouble($this->space -= 8, $x);
    }

    public function putOffset(int $off): void
    {
        $new_off = $this->offset() - $off + Constants::SIZEOF_INT;
        $this->putInt($new_off);
    }
    /// @endcond

    /**
     * Add a `bool` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param bool $x The `bool` to add to the buffer.
     */
    public function addBool(bool $x): void
    {
        $this->prep(1, 0);
        $this->putBool($x);
    }

    /**
     * Add a `byte` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `byte` to add to the buffer.
     */
    public function addByte(int $x): void
    {
        $this->prep(1, 0);
        $this->putByte($x);
    }

    /**
     * Add a `signed byte` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `signed byte` to add to the buffer.
     */
    public function addSbyte(int $x): void
    {
        $this->prep(1, 0);
        $this->putSbyte($x);
    }

    /**
     * Add a `short` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `short` to add to the buffer.
     */
    public function addShort(int $x): void
    {
        $this->prep(2, 0);
        $this->putShort($x);
    }

    /**
     * Add an `unsigned short` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `unsigned short` to add to the buffer.
     */
    public function addUshort(int $x): void
    {
        $this->prep(2, 0);
        $this->putUshort($x);
    }

    /**
     * Add an `int` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `int` to add to the buffer.
     */
    public function addInt(int $x): void
    {
        $this->prep(4, 0);
        $this->putInt($x);
    }

    /**
     * Add an `unsigned int` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `unsigned int` to add to the buffer.
     */
    public function addUint(int $x): void
    {
        $this->prep(4, 0);
        $this->putUint($x);
    }

    /**
     * Add a `long` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `long` to add to the buffer.
     */
    public function addLong(int $x): void
    {
        $this->prep(8, 0);
        $this->putLong($x);
    }

    /**
     * Add an `unsigned long` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param int $x The `unsigned long` to add to the buffer.
     */
    public function addUlong(int $x): void
    {
        $this->prep(8, 0);
        $this->putUlong($x);
    }

    /**
     * Add a `float` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param float $x The `float` to add to the buffer.
     */
    public function addFloat(float $x): void
    {
        $this->prep(4, 0);
        $this->putFloat($x);
    }

    /**
     * Add a `double` to the buffer, properly aligned, and grows the buffer (if necessary).
     * @param float $x The `double` to add to the buffer.
     */
    public function addDouble(float $x): void
    {
        $this->prep(8, 0);
        $this->putDouble($x);
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @param int $o
     * @param bool $x
     * @param bool $d
     */
    public function addBoolX(int $o, bool $x, bool $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addBool($x);
            $this->slot($o);
        }
    }

    public function addByteX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addByte($x);
            $this->slot($o);
        }
    }

    public function addSbyteX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addSbyte($x);
            $this->slot($o);
        }
    }

    public function addShortX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addShort($x);
            $this->slot($o);
        }
    }

    public function addUshortX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addUshort($x);
            $this->slot($o);
        }
    }

    public function addIntX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addInt($x);
            $this->slot($o);
        }
    }

    public function addUintX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addUint($x);
            $this->slot($o);
        }
    }

    public function addLongX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addLong($x);
            $this->slot($o);
        }
    }

    public function addUlongX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addUlong($x);
            $this->slot($o);
        }
    }


    public function addFloatX(int $o, float $x, float $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addFloat($x);
            $this->slot($o);
        }
    }

    public function addDoubleX(int $o, float $x, float $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addDouble($x);
            $this->slot($o);
        }
    }

    /**
     * @throws \Exception
     */
    public function addOffsetX(int $o, int $x, int $d): void
    {
        if ($this->force_defaults || $x != $d) {
            $this->addOffset($x);
            $this->slot($o);
        }
    }
    /// @endcond

    /**
     * Adds on offset, relative to where it will be written.
     * @param int $off The offset to add to the buffer.
     * @throws \ErrorException Throws an exception if `$off` is greater than the underlying ByteBuffer's
     * offest.
     */
    public function addOffset(int $off): void
    {
        $this->prep(Constants::SIZEOF_INT, 0); // Ensure alignment is already done
        if ($off > $this->offset()) {
            throw new \ErrorException("");
        }
        $this->putOffset($off);
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @throws \Exception
     */
    public function startVector(int $elem_size, int $num_elems, int $alignment): void
    {
        $this->notNested();
        $this->vector_num_elems = $num_elems;
        $this->prep(Constants::SIZEOF_INT, $elem_size * $num_elems);
        $this->prep($alignment, $elem_size * $num_elems); // Just in case alignemnt > int;
    }

    public function endVector(): int
    {
        $this->putUint($this->vector_num_elems);
        return $this->offset();
    }

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
            if ($bytes[$j] == "\x09" ||
                $bytes[$j] == "\x0A" ||
                $bytes[$j] == "\x0D" ||
                ($bytes[$j] >= "\x20" && $bytes[$j] <= "\x7E")) {
                $j++;
                continue;
            }

            /* non-overlong 2-byte */
            if ((($i+1) <= $len) &&
                ($bytes[$j] >= "\xC2" && $bytes[$j] <= "\xDF" &&
                    ($bytes[$j+1] >= "\x80" && $bytes[$j+1] <= "\xBF"))) {
                $j += 2;
                $i++;
                continue;
            }

            /* excluding overlongs */
            if ((($i + 2) <= $len) &&
                $bytes[$j] == "\xE0" &&
                ($bytes[$j+1] >= "\xA0" && $bytes[$j+1] <= "\xBF" &&
                    ($bytes[$j+2] >= "\x80" && $bytes[$j+2] <= "\xBF"))) {
                $bytes += 3;
                $i +=2;
                continue;
            }

            /* straight 3-byte */
            if ((($i+2) <= $len) &&
                (($bytes[$j] >= "\xE1" && $bytes[$j] <= "\xEC") ||
                    $bytes[$j] == "\xEE" ||
                    $bytes[$j] == "\xEF") &&
                ($bytes[$j+1] >= "\x80" && $bytes[$j+1] <= "\xBF") &&
                ($bytes[$j+2] >= "\x80" && $bytes[$j+2] <= "\xBF")) {
                $j += 3;
                $i += 2;
                continue;
            }

            /* excluding surrogates */
            if ((($i+2) <= $len) &&
                $bytes[$j] == "\xED" &&
                ($bytes[$j+1] >= "\x80" && $bytes[$j+1] <= "\x9f" &&
                    ($bytes[$j+2] >= "\x80" && $bytes[$j+2] <= "\xBF"))) {
                $j += 3;
                $i += 2;
                continue;
            }

            /* planes 1-3 */
            if ((($i + 3) <= $len) &&
                $bytes[$j] == "\xF0" &&
                ($bytes[$j+1] >= "\x90" && $bytes[$j+1] <= "\xBF") &&
                ($bytes[$j+2] >= "\x80" && $bytes[$j+2] <= "\xBF") &&
                ($bytes[$j+3] >= "\x80" && $bytes[$j+3] <= "\xBF")) {
                $j += 4;
                $i += 3;
                continue;
            }


            /* planes 4-15 */
            if ((($i+3) <= $len) &&
                $bytes[$j] >= "\xF1" && $bytes[$j] <= "\xF3" &&
                $bytes[$j+1] >= "\x80" && $bytes[$j+1] <= "\xBF" &&
                $bytes[$j+2] >= "\x80" && $bytes[$j+2] <= "\xBF" &&
                $bytes[$j+3] >= "\x80" && $bytes[$j+3] <= "\xBF"
            ) {
                $j += 4;
                $i += 3;
                continue;
            }

            /* plane 16 */
            if ((($i+3) <= $len) &&
                $bytes[$j] == "\xF4" &&
                ($bytes[$j+1] >= "\x80" && $bytes[$j+1] <= "\x8F") &&
                ($bytes[$j+2] >= "\x80" && $bytes[$j+2] <= "\xBF") &&
                ($bytes[$j+3] >= "\x80" && $bytes[$j+3] <= "\xBF")
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
     * @param string $s The string to encode.
     * @return int The offset in the buffer where the encoded string starts.
     * @throws \InvalidArgumentException Thrown if the input string `$s` is not
     *     UTF-8.
     */
    public function createString(string $s): int
    {
        if (!$this->is_utf8($s)) {
            throw new \InvalidArgumentException("string must be utf-8 encoded value.");
        }

        $this->notNested();
        $this->addByte(0); // null terminated
        $this->startVector(1, strlen($s), 1);
        $this->space -= strlen($s);
        for ($i =  $this->space, $j = 0 ; $j < strlen($s) ; $i++, $j++) {
            $this->bb->_buffer[$i] = $s[$j];
        }
        return $this->endVector();
    }

    /// @cond FLATBUFFERS_INTERNAL
    /**
     * @throws \ErrorException
     */
    public function notNested(): void
    {
        if ($this->nested) {
            throw new \ErrorException("FlatBuffers; object serialization must not be nested");
        }
    }

    /**
     * @throws \ErrorException
     */
    public function nested(int $obj): void
    {
        if ($obj != $this->offset()) {
            throw new \ErrorException("FlatBuffers: struct must be serialized inline");
        }
    }

    /**
     * @throws \ErrorException
     */
    public function startObject(int $numfields): void
    {
        $this->notNested();
        if ($this->vtable == null || count($this->vtable) < $numfields) {
            $this->vtable = array();
        }

        $this->vtable_in_use = $numfields;
        for ($i = 0; $i < $numfields; $i++) {
            $this->vtable[$i] = 0;
        }

        $this->nested = true;
        $this->object_start = $this->offset();
    }

    /**
     * @throws \ErrorException
     */
    public function addStructX(int $voffset, int $x, int $d): void
    {
        if ($x != $d) {
            $this->nested($x);
            $this->slot($voffset);
        }
    }

    /**
     * @throws \ErrorException
     */
    public function addStruct(int $voffset, int $x, int $d): void
    {
        if ($x != $d) {
            $this->nested($x);
            $this->slot($voffset);
        }
    }

    public function slot(int $voffset): void
    {
        $this->vtable[$voffset] = $this->offset();
    }

    /**
     * @throws \ErrorException
     */
    public function endObject(): int
    {
        if ($this->vtable == null || !$this->nested) {
            throw new \ErrorException("FlatBuffers: endObject called without startObject");
        }

        $this->addInt(0);
        $vtableloc = $this->offset();

        $i = $this->vtable_in_use -1;
        // Trim trailing zeroes.
        for (; $i >= 0 && $this->vtable[$i] == 0; $i--) {}
        $trimmed_size = $i + 1;
        for (; $i >= 0; $i--) {
            $off = ($this->vtable[$i] != 0) ? $vtableloc - $this->vtable[$i] : 0;
            $this->addShort($off);
        }

        $standard_fields = 2; // the fields below
        $this->addShort($vtableloc - $this->object_start);
        $this->addShort(($trimmed_size + $standard_fields) * Constants::SIZEOF_SHORT);

        // search for an existing vtable that matches the current one.
        $existing_vtable = 0;

        for ($i = 0; $i < $this->num_vtables; $i++) {
            $vt1 = $this->bb->capacity() - $this->vtables[$i];
            $vt2 = $this->space;

            $len = $this->bb->getShort($vt1);

            if ($len == $this->bb->getShort($vt2)) {
                for ($j = Constants::SIZEOF_SHORT; $j < $len; $j += Constants::SIZEOF_SHORT) {
                    if ($this->bb->getShort($vt1 + $j) != $this->bb->getShort($vt2 + $j)) {
                        continue 2;
                    }
                }
                $existing_vtable = $this->vtables[$i];
                break;
            }
        }

        if ($existing_vtable != 0) {
            // Found a match:
            // Remove the current vtable
            $this->space = $this->bb->capacity() - $vtableloc;
            $this->bb->putInt($this->space, $existing_vtable - $vtableloc);
        } else {
            // No Match:
            // Add the location of the current vtable to the list of vtables
            if ($this->num_vtables == count($this->vtables)) {
                $vtables = $this->vtables;
                $this->vtables = array();
                // copy of
                for ($i = 0; $i < count($vtables) * 2; $i++) {
                    $this->vtables[$i] = ($i < count($vtables)) ? $vtables[$i] : 0;
                }
            }
            $this->vtables[$this->num_vtables++] = $this->offset();
            $this->bb->putInt($this->bb->capacity() - $vtableloc, $this->offset() - $vtableloc);
        }

        $this->nested = false;
        $this->vtable = null;
        return $vtableloc;
    }

    /**
     * @throws \ErrorException
     */
    public function required(int $table, int $field): void
    {
        $table_start = $this->bb->capacity() - $table;
        $vtable_start = $table_start - $this->bb->getInt($table_start);
        $ok = $this->bb->getShort($vtable_start + $field) != 0;

        if (!$ok) {
            throw new \ErrorException("FlatBuffers: field "  . $field  .  " must be set");
        }
    }
    /// @endcond

    /**
     * Finalize a buffer, pointing to the given `$root_table`.
     * @param int $root_table An offest to be added to the buffer.
     * @param string|null $identifier A FlatBuffer file identifier to be added to the
     *     buffer before `$root_table`. This defaults to `null`.
     * @throws \InvalidArgumentException Thrown if an invalid `$identifier` is
     *     given, where its length is not equal to
     *    `Constants::FILE_IDENTIFIER_LENGTH`.
     */
    public function finish(int $root_table, ?string $identifier = null): void
    {
        if ($identifier == null) {
            $this->prep($this->minalign, Constants::SIZEOF_INT);
            $this->addOffset($root_table);
            $this->bb->setPosition($this->space);
        } else {
            $this->prep($this->minalign, Constants::SIZEOF_INT + Constants::FILE_IDENTIFIER_LENGTH);
            if (strlen($identifier) != Constants::FILE_IDENTIFIER_LENGTH) {
                throw new \InvalidArgumentException(
                    sprintf("FlatBuffers: file identifier must be length %d",
                        Constants::FILE_IDENTIFIER_LENGTH));
            }

            for ($i = Constants::FILE_IDENTIFIER_LENGTH - 1; $i >= 0;
                  $i--) {
                $this->addByte(ord($identifier[$i]));
            }
            $this->finish($root_table);
        }
    }

    /**
     * In order to save space, fields that are set to their default value don't
     * get serialized into the buffer.
     * @param bool $forceDefaults When set to `true`, always serializes default
     *     values.
     */
    public function forceDefaults(bool $forceDefaults): void
    {
        $this->force_defaults = $forceDefaults;
    }

    /**
     * Get the ByteBuffer representing the FlatBuffer.
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
     * @return string A string (representing a byte[]) that contains a copy
     * of the FlatBuffer data.
     */
    public function sizedByteArray(): string
    {
        $start = $this->space;
        $length = $this->bb->capacity() - $this->space;

        $result = str_repeat("\0", $length);
        $this->bb->setPosition($start);
        $this->bb->getX($result);

        return $result;
    }
}

/// @}
