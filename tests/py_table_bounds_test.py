# Copyright 2026 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Bounds-enforcement tests for reading untrusted buffers with Table.

Malformed FlatBuffers previously produced silent wrong values instead of
errors: `struct.unpack_from` accepts negative offsets (reading relative to
the end of the buffer) and slicing clamps out-of-range ranges.
"""

import unittest

from flatbuffers.table import Table

import struct


def _i32(value):
  return struct.pack('<i', value)


class TestTableBounds(unittest.TestCase):

  def test_indirect_rejects_out_of_range_target(self):
    # forward offset 100 at position 0 -> target 100, buffer is 8 bytes
    buf = _i32(100) + b'\x00' * 4
    table = Table(bytearray(buf), 4)
    with self.assertRaises(IndexError):
      table.Indirect(0)

  def test_indirect_rejects_negative_target(self):
    # forward offset -8 at position 4 -> target -4; unpack_from would have
    # silently read relative to the buffer end
    buf = b'\x00' * 4 + _i32(-8)
    table = Table(bytearray(buf), 4)
    with self.assertRaises(IndexError):
      table.Indirect(4)

  def test_indirect_accepts_valid_target(self):
    buf = _i32(4) + b'\x00' * 4
    table = Table(bytearray(buf), 0)
    self.assertEqual(table.Indirect(0), 4)

  def test_string_rejects_length_past_end(self):
    # forward offset at 0 = 4 -> header at 4 with length 100, only 2 bytes
    # remain after the header
    buf = _i32(4) + _i32(100) + b'ab'
    table = Table(bytearray(buf), 0)
    with self.assertRaises(IndexError):
      table.String(0)

  def test_string_rejects_huge_length(self):
    # lengths are unsigned: -1 reads back as 0xffffffff
    buf = _i32(4) + _i32(-1) + b'ab'
    table = Table(bytearray(buf), 0)
    with self.assertRaises(IndexError):
      table.String(0)

  def test_string_accepts_valid_string(self):
    buf = _i32(4) + _i32(2) + b'ab'
    table = Table(bytearray(buf), 0)
    self.assertEqual(table.String(0), b'ab')

  def test_string_huge_length_value_is_unsigned(self):
    # the malformed length reads back as 0xffffffff, not a negative number
    buf = _i32(4) + _i32(-1) + b'ab'
    table = Table(bytearray(buf), 0)
    with self.assertRaises(IndexError):
      table.String(0)

  def test_vector_rejects_start_past_end(self):
    # offset field 100 at Pos 0 -> start lands past the buffer
    buf = _i32(100) + b'\x00' * 4
    table = Table(bytearray(buf), 0)
    with self.assertRaises(IndexError):
      table.Vector(0)

  def test_vector_accepts_empty_vector_at_end(self):
    # forward offset 4 -> length prefix at 4 (0), data starts at 8 == len
    buf = _i32(4) + _i32(0)
    table = Table(bytearray(buf), 0)
    self.assertEqual(table.Vector(0), len(buf))

  def test_offset_rejects_vtable_out_of_range(self):
    # soffset at Pos 4 = -100 -> vtable far outside the buffer
    buf = b'\x00' * 4 + _i32(-100)
    table = Table(bytearray(buf), 4)
    with self.assertRaises(IndexError):
      table.Offset(4)


if __name__ == '__main__':
  unittest.main()
