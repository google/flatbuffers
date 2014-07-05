using System;
using System.IO;
using System.Linq;

/*
 * Copyright 2014 Oli Wilkinson. All rights reserved.
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
 * 
 * 
 * This work is a derivative of Flatbuffers, Copyright 2014 Google Inc.
 * See original: https://github.com/google/flatbuffers
 * 
 */

namespace FlatBuffersNet
{
    /// <summary>
    /// Class to mimick Java's ByteBuffer which is used heavily in Flatbuffers
    /// </summary>
    public class ByteBuffer : IDisposable
    {
        private bool _disposed = false;
        private readonly byte[] _buffer;
        private readonly MemoryStream _memoryStream;
        private readonly BinaryWriter _binaryWriter;
        private readonly BinaryReader _binaryReader;

        public int Length { get { return _buffer.Length; } }

        public byte[] Data { get { return _buffer; } }

        public ByteBuffer(byte[] buffer)
        {
            _buffer = buffer;
            _memoryStream = new MemoryStream(_buffer);
            _memoryStream.Seek(0, SeekOrigin.End);
            _binaryWriter = new BinaryWriter(_memoryStream);
            _binaryReader = new BinaryReader(_memoryStream);
        }

        protected byte[] ToLittleEndian(byte[] data)
        {
            return (BitConverter.IsLittleEndian) ? data : data.Reverse().ToArray();
        }

        protected byte[] ReadLittleEndian(int count)
        {
            var tmp = _binaryReader.ReadBytes(count);
            return (BitConverter.IsLittleEndian) ? tmp : tmp.Reverse().ToArray();
        }

        private void AssertNotDisposed()
        {
            if (_disposed)
                throw new InvalidOperationException("Object has been disposed");
        }

        public void PutByte(int offset, byte value)
        {
            AssertNotDisposed();
            _memoryStream.Seek(offset, SeekOrigin.Begin);
            _binaryWriter.Write(value);
        }

        public void PutShort(int offset, short value)
        {
            AssertNotDisposed();
            _memoryStream.Seek(offset, SeekOrigin.Begin);
            _binaryWriter.Write(ToLittleEndian(BitConverter.GetBytes(value)));
        }

        public void PutInt(int offset, int value)
        {
            AssertNotDisposed();
            _memoryStream.Seek(offset, SeekOrigin.Begin);
            _binaryWriter.Write(ToLittleEndian(BitConverter.GetBytes(value)));
        }

        public void PutLong(int offset, long value)
        {
            AssertNotDisposed();
            _memoryStream.Seek(offset, SeekOrigin.Begin);
            _binaryWriter.Write(ToLittleEndian(BitConverter.GetBytes(value)));
        }

        public void PutFloat(int offset, float value)
        {
            AssertNotDisposed();
            _memoryStream.Seek(offset, SeekOrigin.Begin);
            _binaryWriter.Write(ToLittleEndian(BitConverter.GetBytes(value)));
        }

        public void PutDouble(int offset, double value)
        {
            AssertNotDisposed();
            _memoryStream.Seek(offset, SeekOrigin.Begin);
            _binaryWriter.Write(ToLittleEndian(BitConverter.GetBytes(value)));
        }

        public void Dispose()
        {
            if (_disposed)
                throw new InvalidOperationException("Already disposed");

            _binaryReader.Close();
            _binaryWriter.Close();
            _memoryStream.Dispose();
            _disposed = true;
        }

        public byte Get(int index)
        {
            AssertNotDisposed();
            _memoryStream.Seek(index, SeekOrigin.Begin);
            var value = _binaryReader.ReadByte();
            return value;
        }

        public short GetShort(int index)
        {
            AssertNotDisposed();
            _memoryStream.Seek(index, SeekOrigin.Begin);
            var tmp = ReadLittleEndian(sizeof(short));
            var value = BitConverter.ToInt16(tmp, 0);
            return value;
        }

        public int GetInt(int index)
        {
            AssertNotDisposed();
            _memoryStream.Seek(index, SeekOrigin.Begin);
            var tmp = ReadLittleEndian(sizeof(int));
            var value = BitConverter.ToInt32(tmp, 0);
            return value;
        }

        public long GetLong(int index)
        {
            AssertNotDisposed();
            _memoryStream.Seek(index, SeekOrigin.Begin);
            var tmp = ReadLittleEndian(sizeof(long));
            var value = BitConverter.ToInt64(tmp, 0);
            return value;
        }

        public float GetFloat(int index)
        {
            AssertNotDisposed();
            _memoryStream.Seek(index, SeekOrigin.Begin);
            var tmp = ReadLittleEndian(4);
            var value = BitConverter.ToSingle(tmp, 0);
            return value;
        }

        public double GetDouble(int index)
        {
            AssertNotDisposed();
            _memoryStream.Seek(index, SeekOrigin.Begin);
            var tmp = ReadLittleEndian(8);
            var value = BitConverter.ToDouble(tmp, 0);
            return value;
        }
    }
}