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

// phpcs:disable PHPCompatibility.FunctionNameRestrictions.ReservedFunctionNames.MethodDoubleUnderscore
// phpcs:disable PSR1.Methods.CamelCapsMethodName.NotCamelCaps

abstract class Table
{
    /**
     * @var NPosT $bb_pos
     */
    protected int $bb_pos;

    /**
     * @var ByteBuffer $bb
     */
    protected ByteBuffer $bb;

    /**
     *
     */
    public function __construct()
    {
    }

    /**
     * @param NPosT $npos
     */
    public function setByteBufferPos(int $npos): void
    {
        $this->bb_pos = $npos;
    }

    /**
     * @param ByteBuffer $bb
     */
    public function setByteBuffer(ByteBuffer $bb): void
    {
        $this->bb = $bb;
    }

    /**
     * @param NPosT $npos
     * @return NPosT
     */
    private function followUOffset(int $npos): int
    {
        return Constants::asNPos($npos + $this->bb->getUOffset($npos));
    }

    // TODO: Use null.
    /**
     * returns actual vtable offset
     *
     * @param VOffsetT $vt_f_voffset
     * @return VOffsetT Offset > 0 means exist value. 0 means not exist.
     */
    protected function __offset(int $vt_f_voffset): int
    {
        $vt_npos = $this->bb->followSOffset($this->bb_pos);
        $vt_len_voffset = $this->bb->getVOffset($vt_npos);
        return $vt_f_voffset < $vt_len_voffset ?
            $this->bb->getVOffset(Constants::asVOffset($vt_npos + $vt_f_voffset)) :
            0;
    }

    /**
     * @param NPosT $t_f_npos
     * @return NPosT
     */
    protected function __indirect(int $t_f_npos): int
    {
        return $this->followUOffset($t_f_npos);
    }

    /**
     * Fetch utf8 encoded string.
     *
     * @param NPosT $t_f_npos
     * @return string
     */
    protected function __string(int $t_f_npos): string
    {
        $s_npos = $this->followUOffset($t_f_npos);
        $s_len_uoffset = $this->bb->getUOffset($s_npos);
        $s_body_uoffset = $s_npos + Constants::SIZEOF_UOFFSET;
        return substr($this->bb->_buffer, $s_body_uoffset, $s_len_uoffset);
    }

    /**
     * @param VOffsetT $t_f_voffset
     * @return UOffsetT
     */
    protected function __vector_len(int $t_f_voffset): int
    {
        return $this->bb->getUOffset(
            $this->followUOffset(Constants::asUOffset($t_f_voffset + $this->bb_pos))
        );
    }

    /**
     * @param VOffsetT $t_f_voffset
     * @return NPosT
     */
    protected function __vector(int $t_f_voffset): int
    {
        return Constants::asNPos(
            $this->followUOffset(Constants::asNPos($t_f_voffset + $this->bb_pos)) +
            Constants::SIZEOF_UOFFSET
        );
    }

    /**
     * @param VOffsetT $vt_f_voffset
     * @return ?string
     */
    protected function __vector_as_bytes(int $vt_f_voffset): ?string
    {
        $t_f_voffset = $this->__offset($vt_f_voffset);
        if ($t_f_voffset === 0) {
            return null;
        }

        return substr($this->bb->_buffer, $this->__vector($t_f_voffset), $this->__vector_len($t_f_voffset));
    }

    /**
     * @template T of Table|Struct
     *
     * @param T        $table
     * @param VOffsetT $t_f_voffset
     * @return T
     */
    protected function __union(Table|Struct $table, int $t_f_voffset): Table|Struct
    {
        $table->setByteBufferPos(
            $this->followUOffset(Constants::asNPos($t_f_voffset + $this->bb_pos))
        );
        $table->setByteBuffer($this->bb);
        return $table;
    }

    /**
     * @param ByteBuffer $bb
     * @param string     $ident
     * @return bool
     * @throws \InvalidArgumentException Invalid argument exception.
     */
    protected static function __has_identifier(ByteBuffer $bb, string $ident): bool
    {
        if (strlen($ident) !== Constants::FILE_IDENTIFIER_LENGTH) {
            throw new \InvalidArgumentException(
                "FlatBuffers: file identifier must be length "  . Constants::FILE_IDENTIFIER_LENGTH
            );
        }

        return $ident === $bb->get(Constants::asNPos($bb->getPosition() + Constants::SIZEOF_UOFFSET), 4);
    }
}
