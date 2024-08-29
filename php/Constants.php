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

class Constants
{
    public const SIZEOF_BOOL = 1;
    public const SIZEOF_BYTE = 1;
    public const SIZEOF_SHORT = 2;
    public const SIZEOF_INT = 4;
    public const SIZEOF_LONG = 8;
    public const SIZEOF_FLOAT = 4;
    public const SIZEOF_DOUBLE = 8;
    public const FILE_IDENTIFIER_LENGTH = 4;

    public const SIZEOF_VOFFSET = 2;
    public const SIZEOF_UOFFSET = 4;
    public const SIZEOF_SOFFSET = 4;

    public const POS_UPPER = 4294967295;
    public const SIZE_UPPER = 4294967295;
    public const UOFFSET_UPPER = 4294967295;
    public const VOFFSET_UPPER = 65535;
    public const SOFFSET_UPPER = 2147483647;
    public const SOFFSET_LOWER = -2147483648;

    /**
     * @param int $value
     * @return VOffsetT
     */
    public static function asVOffset(int $value): int
    {
        assert(0 <= $value && $value <= self::VOFFSET_UPPER);
        return $value;
    }

    /**
     * @param int $value
     * @return SOffsetT
     */
    public static function asSOffset(int $value): int
    {
        assert(self::SOFFSET_LOWER <= $value && $value <= self::SOFFSET_UPPER);
        return $value;
    }

    /**
     * @param int $value
     * @return UOffsetT
     */
    public static function asUOffset(int $value): int
    {
        assert(0 <= $value && $value <= self::UOFFSET_UPPER);
        return $value;
    }

    /**
     * @param int $value
     * @return NPosT
     */
    public static function asNPos(int $value): int
    {
        assert(0 <= $value && $value <= self::POS_UPPER);
        return $value;
    }

    /**
     * @param int $value
     * @return WPosT
     */
    public static function asWPos(int $value): int
    {
        assert(0 <= $value && $value <= self::POS_UPPER);
        return $value;
    }

    /**
     * @param int $value
     * @return BufSizeT
     */
    public static function asBufSize(int $value): int
    {
        assert(0 <= $value && $value <= self::SIZE_UPPER);
        return $value;
    }
}
