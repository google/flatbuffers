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
using System.Collections.Generic;
using System.Text;
using Google.FlatSpanBuffers.Operations;

namespace Google.FlatSpanBuffers
{
    /// <summary>
    /// Common BufferBuilder operations for FlatBuffer construction.
    /// Contains shared state for reuse with BufferBuider vairants (class and ref struct builders)
    /// </summary>
    public struct BufferBuilder<TBuffer>
        where TBuffer : IByteBuffer, allows ref struct
    {
        private int _space;
        private int _minAlign;
        // The size of the vtable. -1 indicates no vtable
        private int _vtableSize;
        // Starting offset of the current struct/table.
        private int _objectStart;
        // Number of entries in `vtables` in use.
        private int _numVtables;
        // For the current vector being built.
        private int _vectorNumElems;

        // Optional allocator.
        IByteBufferAllocator<TBuffer> _bufferAllocator;

        // For CreateSharedString
        private Dictionary<string, StringOffset> _sharedStringMap;

        /// <summary>
        /// Initialize a new BufferBuilder with an initial size and optional 
        /// allocator to grow the buffer as needed
        /// </summary>
        public BufferBuilder(int bufferSize, IByteBufferAllocator<TBuffer> bufferAllocator)
        {
            _space = bufferSize;
            _minAlign = 1;
            _vtableSize = -1;
            _objectStart = 0;
            _numVtables = 0;
            _vectorNumElems = 0;
            _bufferAllocator = bufferAllocator;
            _sharedStringMap = null;
        }

        /// <summary>
        /// Current offset in the buffer
        /// </summary>
        public readonly int GetOffset(ref TBuffer buffer)
            => buffer.Length - _space;

        /// <summary>
        /// Set the shared string map for CreateSharedString functionality
        /// </summary>
        public void SetSharedStringCache(Dictionary<string, StringOffset> sharedStringMap)
        {
            _sharedStringMap = sharedStringMap;
        }

        /// <summary>
        /// Reset the builder state for a new buffer
        /// </summary>
        public void Reset(ref TBuffer buffer, Span<int> vtable)
        {
            if (_vtableSize > 0)
                vtable.Slice(0, _vtableSize).Clear();
            _vtableSize = -1;
            _space = buffer.Length;
            _minAlign = 1;
            _objectStart = 0;
            _numVtables = 0;
            _vectorNumElems = 0;
            if (_sharedStringMap != null)
            {
                _sharedStringMap.Clear();
            }

            buffer.Reset();
        }

        public void Pad(ref TBuffer buffer, int size)
        {
            buffer.PadBytes(_space -= size, size);
        }

        // Prepare to write an element of size after additional_bytes have been written
        public void Prep(ref TBuffer buffer, int size, int additionalBytes)
        {
            // Track the biggest thing we've ever aligned to.
            if (size > _minAlign)
                _minAlign = size;

            // Find the amount of alignment needed such that `size` is properly
            // aligned after `additionalBytes`
            int alignSize =
                ((~(buffer.Length - _space + additionalBytes)) + 1) &
                (size - 1);

            // Check if we have enough space, realign if necessary
            int requiredSize = alignSize + size + additionalBytes;
            while (_space < requiredSize)
            {
                if (_bufferAllocator == null)
                    throw new OutOfMemoryException("Builder was not provided enough space to build the flatbuffer.");

                var oldBufSize = buffer.Length;
                var minRequiredBufferSize = buffer.Length - _space + requiredSize;
                _bufferAllocator.GrowFront(ref buffer, minRequiredBufferSize);
                _space += buffer.Length - oldBufSize;
            }

            if (alignSize > 0)
                Pad(ref buffer, alignSize);
        }

        public void Put<T>(ref TBuffer buffer, T value)
            where T : unmanaged
        {
            buffer.Put(_space -= BufferOperations.SizeOf<T>(), value);
        }

        public void Put<T>(ref TBuffer buffer, scoped ReadOnlySpan<T> value)
            where T : unmanaged
        {
            buffer.PutSpan<T>(_space -= BufferOperations.SizeOf<T>() * value.Length, value);
        }

        public void Add<T>(ref TBuffer buffer, T value)
            where T : unmanaged
        {
            Prep(ref buffer, BufferOperations.SizeOf<T>(), 0);
            buffer.Put<T>(_space -= BufferOperations.SizeOf<T>(), value);
        }

        public void AddSpan<T>(ref TBuffer buffer, scoped ReadOnlySpan<T> values)
            where T : unmanaged
        {
            if (values.IsEmpty)
                return;

            int elementSize = BufferOperations.SizeOf<T>();
            int spanSize = elementSize * values.Length;
            Prep(ref buffer, elementSize, spanSize - elementSize);
            buffer.PutSpan(_space -= spanSize, values);
        }

        public readonly void ValidateNotNested()
        {
            // You should not be creating any other objects or strings/vectors
            // while an object is being constructed
            if (_vtableSize >= 0)
                throw new InvalidOperationException("FlatBuffers: object serialization must not be nested.");
        }

        public void StartTable(ref TBuffer buffer, int numfields, Span<int> vtable)
        {
            ValidateNotNested();
            if (numfields < 0)
                throw new ArgumentOutOfRangeException("Flatbuffers: invalid numfields");
            if (vtable.Length < numfields)
                throw new OutOfMemoryException("Number of table fields exceeds provided vtable buffer space.");

            _vtableSize = numfields;
            _objectStart = GetOffset(ref buffer);
        }

        /// <summary>
        /// End table construction and return the table offset
        /// 'numVtables' provides the current number of saved vtables to track memory use.
        /// </summary>
        public int EndTable(ref TBuffer buffer, Span<int> vtable, Span<int> vtables, out int numVtables)
        {
            if (_vtableSize < 0)
                throw new InvalidOperationException("FlatBuffers: calling EndTable without a StartTable");

            numVtables = _numVtables;

            // Add table header
            Add<int>(ref buffer, 0);
            int vtableloc = GetOffset(ref buffer);

            // Write out the current vtable.
            int i = _vtableSize - 1;
            // Trim trailing zeroes.
            for (; i >= 0 && vtable[i] == 0; i--) { }
            int trimmedSize = i + 1;

            // Reserve space for the header and fields.
            const int standardFields = 2;
            short vtableSizeBytes = (short)((trimmedSize + standardFields) * sizeof(short));
            Prep(ref buffer, sizeof(short), vtableSizeBytes - sizeof(short));

            for (; i >= 0; i--)
            {
                // Offset relative to the start of the table.
                short off = (short)(vtable[i] != 0 ? vtableloc - vtable[i] : 0);
                buffer.Put<short>(_space -= sizeof(short), off);
                vtable[i] = 0;
            }

            // Then write vtable header: table size, then vtable size
            buffer.Put<short>(_space -= sizeof(short), (short)(vtableloc - _objectStart));
            buffer.Put<short>(_space -= sizeof(short), vtableSizeBytes);

            // Search for an existing vtable that matches the current one.
            ReadOnlySpan<byte> currentVtable = buffer.ToReadOnlySpan(_space, vtableSizeBytes);
            for (i = 0; i < _numVtables; i++)
            {
                int vt2 = buffer.Length - vtables[i];
                if (currentVtable.Length == buffer.Get<short>(vt2))
                {
                    bool match = currentVtable.SequenceEqual(
                        buffer.ToReadOnlySpan(vt2, currentVtable.Length));
                    if (match)
                    {
                        // Remove the current vtable.
                        _space = buffer.Length - vtableloc;

                        // Point table to existing vtable.
                        buffer.Put<int>(_space, vtables[i] - vtableloc);

                        _vtableSize = -1;
                        return vtableloc;
                    }
                }
            }

            if (_numVtables >= vtables.Length)
                throw new InvalidOperationException("FlatBuffers: not enough vtables offset space reserved to store this table.");

            // No existing vtable found, add this vtable to list 
            var newVtableOffset = GetOffset(ref buffer);
            vtables[_numVtables++] = newVtableOffset;
            numVtables = _numVtables;

            // Point table to current vtable.
            int objLoc = buffer.Length - vtableloc;
            buffer.Put<int>(objLoc, newVtableOffset - vtableloc);

            _vtableSize = -1;
            return vtableloc;
        }

        // Set the current vtable at `voffset` to the current location in the
        // buffer.
        public void SetVtableSlot(ref TBuffer buffer, Span<int> vtable, int voffset)
        {
            vtable[voffset] = GetOffset(ref buffer);
        }

        public void AddToTable<T>(ref TBuffer buffer, Span<int> vtable, int o, T x, T d, bool forceDefaults)
            where T : unmanaged, IEquatable<T>
        {
            if (forceDefaults || !x.Equals(d))
            {
                Add(ref buffer, x);
                SetVtableSlot(ref buffer, vtable, o);
            }
        }

        public void AddToTable<T>(ref TBuffer buffer, Span<int> vtable, int o, T? x)
            where T : unmanaged
        {
            if (x.HasValue)
            {
                Add(ref buffer, x.Value);
                SetVtableSlot(ref buffer, vtable, o);
            }
        }

        /// <summary>
        /// Add an offset value to the buffer, relative to where it will be written.
        /// </summary>
        public void AddOffset(ref TBuffer buffer, int off)
        {
            Prep(ref buffer, sizeof(int), 0); // Ensure alignment is already done.
            int offset = off != 0 ? GetOffset(ref buffer) - off + sizeof(int) : 0;
            buffer.Put<int>(_space -= sizeof(int), offset);
        }

        public void AddStruct(ref TBuffer buffer, Span<int> vtable, int voffset, int x, int d)
        {
            // Structs are stored inline, so nothing additional is being added.
            // `d` is always 0.
            if (x != d)
            {
                if (x != GetOffset(ref buffer))
                {
                    throw new InvalidOperationException(
    "                   FlatBuffers: struct must be serialized inline.");
                }

                SetVtableSlot(ref buffer, vtable, voffset);
            }
        }

        public void StartVector(ref TBuffer buffer, int elemSize, int count, int alignment)
        {
            ValidateNotNested();
            _vectorNumElems = count;
            Prep(ref buffer, sizeof(int), elemSize * count);
            Prep(ref buffer, alignment, elemSize * count); // Just in case alignment > int.
        }

        public VectorOffset EndVector(ref TBuffer buffer)
        {
            buffer.Put<int>(_space -= sizeof(int), _vectorNumElems);
            return new VectorOffset(GetOffset(ref buffer));
        }

        /// <summary>
        /// Adds a buffer offset to the Table at index `o` in its vtable using an offset value and default value
        /// </summary>
        public void AddOffsetToTable(ref TBuffer buffer, Span<int> vtable, int o, int x, int d)
        {
            if (x != d)
            {
                AddOffset(ref buffer, x);
                SetVtableSlot(ref buffer, vtable, o);
            }
        }

        public void AddOffsetSpan(ref TBuffer buffer, scoped ReadOnlySpan<int> offsets)
        {
            if (offsets.IsEmpty)
                return;

            // Prepare space for all offsets
            Prep(ref buffer, sizeof(int), (offsets.Length - 1) * sizeof(int));
            for (var i = offsets.Length - 1; i >= 0; i--)
            {
                int adjustedOffset = GetOffset(ref buffer) - offsets[i] + sizeof(int);
                buffer.Put<int>(_space -= sizeof(int), adjustedOffset);
            }
        }

        public void AddOffsetSpan<TOffset>(ref TBuffer buffer, scoped ReadOnlySpan<TOffset> offsets)
            where TOffset : IFlatBufferOffset
        {
            if (offsets.IsEmpty)
                return;

            // Prepare space for all offsets
            Prep(ref buffer, sizeof(int), (offsets.Length - 1) * sizeof(int));
            for (var i = offsets.Length - 1; i >= 0; i--)
            {
                int adjustedOffset = GetOffset(ref buffer) - offsets[i].Value + sizeof(int);
                buffer.Put<int>(_space -= sizeof(int), adjustedOffset);
            }
        }

        public VectorOffset CreateVectorOfTables<T>(ref TBuffer buffer, scoped ReadOnlySpan<Offset<T>> offsets)
            where T : struct, allows ref struct
        {
            ValidateNotNested();
            StartVector(ref buffer, sizeof(int), offsets.Length, sizeof(int));
            AddOffsetSpan(ref buffer, offsets);
            return EndVector(ref buffer);
        }

        /// <summary>
        /// Encode the string `s` in the buffer using UTF-8.
        /// </summary>
        public StringOffset CreateString(ref TBuffer buffer, scoped ReadOnlySpan<char> s)
        {
            ValidateNotNested();
            if (s.IsEmpty)
                return new StringOffset(0);

            Add<byte>(ref buffer, 0);
            var utf8StringLength = Encoding.UTF8.GetByteCount(s);
            StartVector(ref buffer, 1, utf8StringLength, 1);
            buffer.PutStringUTF8(_space -= utf8StringLength, s);
            return new StringOffset(EndVector(ref buffer).Value);
        }

        /// <summary>
        /// Encode the utf8 bytes `s` in the buffer.
        /// </summary>
        public StringOffset CreateUTF8String(ref TBuffer buffer, scoped ReadOnlySpan<byte> s)
        {
            ValidateNotNested();
            if (s.IsEmpty)
                return new StringOffset(0);

            Add<byte>(ref buffer, 0);
            StartVector(ref buffer, 1, s.Length, 1);
            buffer.PutSpan(_space -= s.Length, s);
            return new StringOffset(EndVector(ref buffer).Value);
        }

        /// <summary>
        /// Create a shared string using the internal shared string map.
        /// If the string already exists, this returns the offset of the existing string.
        /// </summary>
        public StringOffset CreateSharedString(ref TBuffer buffer, string s)
        {
            if (s == null)
                return new StringOffset(0);

            var sharedStringMap = _sharedStringMap ??= new Dictionary<string, StringOffset>();

            if (sharedStringMap.TryGetValue(s, out StringOffset stringOffset))
                return stringOffset;

            stringOffset = CreateString(ref buffer, s.AsSpan());
            sharedStringMap.Add(s, stringOffset);
            return stringOffset;
        }

        public void Finish(ref TBuffer buffer, int rootTable)
        {
            InternalFinishBuffer(ref buffer, rootTable);
        }

        public void FinishWithSizePrefix(ref TBuffer buffer, int rootTable)
        {
            int additionalBytes = sizeof(int) + FlatBufferConstants.SizePrefixLength;
            Prep(ref buffer, _minAlign, additionalBytes);
            InternalFinishBufferWithSizePrefix(ref buffer, rootTable);
        }

        public void FinishWithSizePrefix(ref TBuffer buffer, int rootTable, string fileIdentifier)
        {
            ValidateFileIdentifier(fileIdentifier);

            int additionalBytes = sizeof(int) + FlatBufferConstants.FileIdentifierLength + FlatBufferConstants.SizePrefixLength;
            Prep(ref buffer, _minAlign, additionalBytes);
            buffer.PutStringUTF8(_space -= FlatBufferConstants.FileIdentifierLength, fileIdentifier);
            InternalFinishBufferWithSizePrefix(ref buffer, rootTable);
        }

        public void FinishWithFileId(ref TBuffer buffer, int rootTable, string fileIdentifier)
        {
            ValidateFileIdentifier(fileIdentifier);

            int additionalBytes = sizeof(int) + FlatBufferConstants.FileIdentifierLength;
            Prep(ref buffer, _minAlign, additionalBytes);
            buffer.PutStringUTF8(_space -= FlatBufferConstants.FileIdentifierLength, fileIdentifier);
            InternalFinishBuffer(ref buffer, rootTable);
        }

        private static void ValidateFileIdentifier(string fileIdentifier)
        {
            if (string.IsNullOrEmpty(fileIdentifier))
                throw new ArgumentException("FlatBuffers: file identifier is null or empty");
            if (fileIdentifier.Length != FlatBufferConstants.FileIdentifierLength)
                throw new ArgumentException($"FlatBuffers: file identifier must be length {FlatBufferConstants.FileIdentifierLength}", nameof(fileIdentifier));
        }

        private void InternalFinishBuffer(ref TBuffer buffer, int rootTable)
        {
            AddOffset(ref buffer, rootTable);
            buffer.Position = _space;
        }

        private void InternalFinishBufferWithSizePrefix(ref TBuffer buffer, int rootTable)
        {
            AddOffset(ref buffer, rootTable);
            Add<int>(ref buffer, buffer.Length - _space);
            buffer.Position = _space;
        }

        // This checks a required field has been set in a given table that has
        // just been constructed.
        public void ValidateRequiredField(ref TBuffer buffer, int table, int field)
        {
            int tableStart = buffer.Length - table;
            int vtableStart = tableStart - (int)buffer.Get<int>(tableStart);
            bool ok = (vtableStart + field) != 0;
            if (!ok)
                throw new InvalidOperationException("FlatBuffers: required field " + field + " must be set");
        }
    }
}
