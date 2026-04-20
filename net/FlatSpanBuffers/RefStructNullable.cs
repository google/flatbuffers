/*
 * Copyright 2014 Google Inc. All rights reserved.
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

using System;

namespace Google.FlatSpanBuffers
{
    public readonly ref struct RefStructNullable<T>
        where T : struct, allows ref struct
    {
        public readonly bool HasValue { get; }
        public readonly T Value { get; }

        public RefStructNullable()
        {
            HasValue = false;
            Value = default;
        }

        public RefStructNullable(T value)
        {
            HasValue = true;
            Value = value;
        }

        public T GetValueOrDefault() => Value;
        public T GetValueOrDefault(T defaultValue) => HasValue ? Value : defaultValue;
    }
}