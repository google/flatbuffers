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

abstract class Struct
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
}
