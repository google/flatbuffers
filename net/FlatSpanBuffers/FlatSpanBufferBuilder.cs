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

namespace Google.FlatSpanBuffers
{
    /// <summary>
    /// Responsible for building up and accessing a FlatBuffer formatted byte
    /// array (via ByteSpanBuffer).
    /// </summary>
    public ref struct FlatSpanBufferBuilder
    {
        private ByteSpanBuffer _bb;
        private BufferBuilder<ByteSpanBuffer> _builder;

        // The vtable for the current table
        private Span<int> _vtable;
        // List of offsets of all vtables.
        private Span<int> _vtables;

        /// <summary>
        /// Gets and sets a Boolean to disable the optimization when serializing
        /// default values to a Table.
        ///
        /// In order to save space, fields that are set to their default value
        /// don't get serialized into the buffer.S
        /// </summary>
        public bool ForceDefaults { get; set; }

        public int Offset => _builder.GetOffset(ref _bb);

        /// <summary>
        /// Get the ByteBuffer representing the FlatBuffer.
        /// </summary>
        public ByteSpanBuffer DataBuffer => _bb;

        /// <summary>
        /// Creates a FlatBufferBuilder backed by the provided ByteSpanBuffer.
        /// Provide enough vtable workspace for expected table sizes, 16 is the traditional default.
        /// </summary>
        /// <param name="buffer">The ByteSpanBuffer to write to</param>
        public FlatSpanBufferBuilder(ByteSpanBuffer buffer,
            Span<int> vtableSpace,
            Span<int> vtableOffsetSpace)
        {
            _bb = buffer;
            _builder = new BufferBuilder<ByteSpanBuffer>(buffer.Length, null);
            buffer.Reset();

            _vtable = vtableSpace;
            _vtables = vtableOffsetSpace;
            ForceDefaults = false;
        }

        public FlatSpanBufferBuilder(ByteSpanBuffer buffer,
            Span<int> vtableSpace,
            Span<int> vtableOffsetSpace,
            Dictionary<string, StringOffset> sharedStringMap)
            : this(buffer, vtableSpace, vtableOffsetSpace)
        {
            _builder.SetSharedStringCache(sharedStringMap);
        }

        public ReadOnlySpan<byte> SizedReadOnlySpan()
        {
            return _bb.ToSizedReadOnlySpan();
        }

        /// <summary>
        /// Reset the FlatBufferBuilder by purging all data that it holds.
        /// </summary>
        public void Clear()
        {
            _builder.Reset(ref _bb, _vtable);
        }

        /// <summary>
        /// Assign new ByteSpanBuffer and reset the FlatBufferBuilder by purging all data that it holds.
        /// </summary>
        public void Clear(ByteSpanBuffer buffer)
        {
            _bb = buffer;
            Clear();
        }

        public void Pad(int size)
        {
            _builder.Pad(ref _bb, size);
        }

        // Prepare to write an element of `size` after `additional_bytes`
        // have been written, e.g. if you write a string, you need to align
        // such the int length field is aligned to SIZEOF_INT, and the string
        // data follows it directly.
        // If all you need to do is align, `additional_bytes` will be 0.
        public void Prep(int size, int additionalBytes)
        {
            _builder.Prep(ref _bb, size, additionalBytes);
        }

        public void Put<T>(T value)
            where T : unmanaged
        {
            _builder.Put(ref _bb, value);
        }

        public void Put<T>(scoped ReadOnlySpan<T> x)
            where T : unmanaged
        {
            _builder.Put(ref _bb, x);
        }

        public void Add<T>(T value)
            where T : unmanaged
        {
            _builder.Add(ref _bb, value);
        }

        public void AddSpan<T>(scoped ReadOnlySpan<T> x)
            where T : unmanaged
        {
            _builder.AddSpan(ref _bb, x);
        }

        /// <summary>
        /// Adds a value to the Table at index `o` in its vtable using the value `x` and default `d`
        /// </summary>
        /// <typeparam name="T">The unmanaged type to add</typeparam>
        /// <param name="o">The index into the vtable</param>
        /// <param name="x">The value to put into the buffer. If the value is equal to the default
        /// and <see cref="ForceDefaults"/> is false, the value will be skipped.</param>
        /// <param name="d">The default value to compare the value against</param>
        public void Add<T>(int o, T x, T d)
            where T : unmanaged, IEquatable<T>
        {
            _builder.AddToTable(ref _bb, _vtable, o, x, d, ForceDefaults);
        }

        /// <summary>
        /// Adds a nullable value to the Table at index `o` in its vtable
        /// </summary>
        /// <typeparam name="T">The unmanaged type to add</typeparam>
        /// <param name="o">The index into the vtable</param>
        /// <param name="x">The nullable value to put into the buffer. If it doesn't have a value
        /// it will skip writing to the buffer.</param>
        public void Add<T>(int o, T? x)
            where T : unmanaged
        {
            _builder.AddToTable(ref _bb, _vtable, o, x);
        }

        public void StartVector(int elemSize, int count, int alignment)
        {
            _builder.StartVector(ref _bb, elemSize, count, alignment);
        }

        /// <summary>
        /// Writes data necessary to finish a vector construction.
        /// </summary>
        public VectorOffset EndVector()
        {
            return _builder.EndVector(ref _bb);
        }

        /// <summary>
        /// Creates a vector of tables from a span of offsets
        /// </summary>
        /// <param name="offsets">Span of offsets of the tables.</param>
        public VectorOffset CreateVectorOfTables<T>(scoped ReadOnlySpan<Offset<T>> offsets)
            where T : struct, allows ref struct
        {
            return _builder.CreateVectorOfTables(ref _bb, offsets);
        }

        public void StartTable(int numfields)
        {
            _builder.StartTable(ref _bb, numfields, _vtable);
        }

        public int EndTable()
        {
            return _builder.EndTable(ref _bb, _vtable, _vtables, out _);
        }

        /// <summary>
        /// Adds an offset, relative to where it will be written.
        /// </summary>
        /// <param name="offset">The offset to add to the buffer.</param>
        public void AddOffset(int offset)
        {
            _builder.AddOffset(ref _bb, offset);
        }

        /// <summary>
        /// Adds a buffer offset to the Table at index `o` in its vtable using the value `x` and default `d`
        /// </summary>
        /// <param name="o">The index into the vtable</param>
        /// <param name="x">The value to put into the buffer. If the value is equal to the default
        /// the value will be skipped.</param>
        /// <param name="d">The default value to compare the value against</param>
        public void AddOffset(int o, int x, int d)
        {
            _builder.AddOffsetToTable(ref _bb, _vtable, o, x, d);
        }

        public void AddOffset<TOffset>(int o, TOffset x, int d)
            where TOffset : IFlatBufferOffset
        {
            _builder.AddOffsetToTable(ref _bb, _vtable, o, x.Value, d);
        }

        /// <summary>
        /// Adds offsets, relative to where they will be written.
        /// </summary>
        /// <param name="offsets">The offsets to add to the buffer.</param>
        public void AddOffsetSpan(scoped ReadOnlySpan<int> offsets)
        {
            _builder.AddOffsetSpan(ref _bb, offsets);
        }

        /// <summary>
        /// Adds typed offsets, relative to where they will be written.
        /// </summary>
        public void AddOffsetSpan<TOffset>(scoped ReadOnlySpan<TOffset> offsets)
            where TOffset : IFlatBufferOffset
        {
            _builder.AddOffsetSpan(ref _bb, offsets);
        }

        /// <summary>
        /// Encode the string `s` in the buffer using UTF-8.
        /// </summary>
        /// <param name="s">The string to encode.</param>
        /// <returns>
        /// The offset in the buffer where the encoded string starts.
        /// </returns>
        public StringOffset CreateString(scoped ReadOnlySpan<char> s)
        {
            return _builder.CreateString(ref _bb, s);
        }

        /// <summary>
        /// Creates a string in the buffer from a Span containing
        /// a UTF8 string.
        /// </summary>
        /// <param name="chars">the UTF8 string to add to the buffer</param>
        /// <returns>
        /// The offset in the buffer where the encoded string starts.
        /// </returns>
        public StringOffset CreateUTF8String(scoped ReadOnlySpan<byte> chars)
        {
            return _builder.CreateUTF8String(ref _bb, chars);
        }

        /// <summary>
        /// Store a string in the buffer, which can contain any binary data.
        /// If a string with this exact contents has already been serialized before,
        /// instead simply returns the offset of the existing string.
        /// </summary>
        /// <param name="s">The string to encode.</param>
        /// <returns>
        /// The offset in the buffer where the encoded string starts.
        /// </returns>
        public StringOffset CreateSharedString(string s)
        {
            return _builder.CreateSharedString(ref _bb, s);
        }

        // Structs are stored inline, so nothing additional is being added.
        // `d` is always 0.
        public void AddStruct(int voffset, int x, int d)
        {
            _builder.AddStruct(ref _bb, _vtable, voffset, x, d);
        }

        // Structs are stored inline, so nothing additional is being added.
        // `d` is always 0.
        public void AddStruct<TOffset>(int o, TOffset x, int d)
            where TOffset : IFlatBufferOffset
        {
            _builder.AddStruct(ref _bb, _vtable, o, x.Value, d);
        }

        // This checks a required field has been set in a given table that has
        // just been constructed.
        public void Required(int table, int field)
        {
            _builder.ValidateRequiredField(ref _bb, table, field);
        }

        /// <summary>
        /// Finalize a buffer, pointing to the given `root_table`.
        /// </summary>
        /// <param name="rootTable">
        /// An offset to be added to the buffer.
        /// </param>
        public void Finish(int rootTable)
        {
            _builder.Finish(ref _bb, rootTable);
        }

        /// <summary>
        /// Finalize a buffer, pointing to the given `rootTable`.
        /// </summary>
        /// <param name="rootTable">
        /// An offset to be added to the buffer.
        /// </param>
        /// <param name="fileIdentifier">
        /// A FlatBuffer file identifier to be added to the buffer before
        /// `root_table`.
        /// </param>
        public void Finish(int rootTable, string fileIdentifier)
        {
            _builder.FinishWithFileId(ref _bb, rootTable, fileIdentifier);
        }
        
        /// <summary>
        /// Finalize a buffer, pointing to the given `root_table`, with the size prefixed.
        /// </summary>
        /// <param name="rootTable">
        /// An offset to be added to the buffer.
        /// </param>
        public void FinishSizePrefixed(int rootTable)
        {
            _builder.FinishWithSizePrefix(ref _bb, rootTable);
        }

        /// <summary>
        /// Finalize a buffer, pointing to the given `rootTable`, with the size prefixed.
        /// </summary>
        /// <param name="rootTable">
        /// An offset to be added to the buffer.
        /// </param>
        /// <param name="fileIdentifier">
        /// A FlatBuffer file identifier to be added to the buffer before
        /// `root_table`.
        /// </param>
        public void FinishSizePrefixed(int rootTable, string fileIdentifier)
        {
            _builder.FinishWithSizePrefix(ref _bb, rootTable, fileIdentifier);
        }
    }
}
