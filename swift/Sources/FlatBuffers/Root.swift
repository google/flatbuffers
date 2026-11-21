/*
 * Copyright 2024 Google Inc. All rights reserved.
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

import Foundation

@inline(__always)
private func verifySizePrefix(
  byteBuffer: inout ByteBuffer,
  requireExactSize: Bool,
  options: VerifierOptions) throws
{
  let verifier = try Verifier(buffer: &byteBuffer, options: options)
  let prefixPosition = byteBuffer.reader
  let prefix: UOffset = try verifier.getValue(at: prefixPosition)
  let availableSize = byteBuffer.size &- UOffset(MemoryLayout<UOffset>.size)

  if requireExactSize {
    guard prefix == availableSize else {
      throw FlatbuffersErrors.prefixedSizeNotEqualToBufferSize
    }
  } else if prefix > availableSize {
    throw FlatbuffersErrors.outOfBounds(
      position: UInt(prefixPosition)
        &+ UInt(MemoryLayout<UOffset>.size)
        &+ UInt(prefix),
      end: byteBuffer.capacity)
  }
}

/// Takes in a prefixed sized buffer, where the prefixed size would be skipped.
/// And would verify that the buffer passed is a valid `Flatbuffers` Object.
/// - Parameters:
///   - byteBuffer: Buffer that needs to be checked and read
///   - options: Verifier options
/// - Throws: FlatbuffersErrors
/// - Returns: Returns a valid, checked Flatbuffers object
///
/// ``getPrefixedSizeCheckedRoot(byteBuffer:options:)`` would skip the first Bytes in
/// the ``ByteBuffer`` and verifies the buffer by calling ``getCheckedRoot(byteBuffer:options:)``
public func getPrefixedSizeCheckedRoot<T: FlatBufferTable & Verifiable>(
  byteBuffer: inout ByteBuffer,
  fileId: String? = nil,
  options: VerifierOptions = .init()) throws -> T
{
  try verifySizePrefix(
    byteBuffer: &byteBuffer,
    requireExactSize: false,
    options: options)
  byteBuffer.skipPrefix()
  return try getCheckedRoot(
    byteBuffer: &byteBuffer,
    fileId: fileId,
    options: options)
}

/// Takes in a prefixed sized buffer, where we check if the sized buffer is equal to prefix size.
/// And would verify that the buffer passed is a valid `Flatbuffers` Object.
/// - Parameters:
///   - byteBuffer: Buffer that needs to be checked and read
///   - options: Verifier options
/// - Throws: FlatbuffersErrors
/// - Returns: Returns a valid, checked Flatbuffers object
///
/// ``getPrefixedSizeCheckedRoot(byteBuffer:options:)`` would skip the first Bytes in
/// the ``ByteBuffer`` and verifies the buffer by calling ``getCheckedRoot(byteBuffer:options:)``
public func getCheckedPrefixedSizeRoot<T: FlatBufferTable & Verifiable>(
  byteBuffer: inout ByteBuffer,
  fileId: String? = nil,
  options: VerifierOptions = .init()) throws -> T
{
  try verifySizePrefix(
    byteBuffer: &byteBuffer,
    requireExactSize: true,
    options: options)
  byteBuffer.skipPrefix()
  return try getCheckedRoot(
    byteBuffer: &byteBuffer,
    fileId: fileId,
    options: options)
}

/// Takes in a prefixed sized buffer, where the prefixed size would be skipped.
/// Returns a `NON-Checked` flatbuffers object
/// - Parameter byteBuffer: Buffer that contains data
/// - Returns: Returns a Flatbuffers object
///
/// ``getPrefixedSizeCheckedRoot(byteBuffer:options:)`` would skip the first Bytes in
/// the ``ByteBuffer`` and then calls ``getRoot(byteBuffer:)``
public func getPrefixedSizeRoot<T: FlatBufferTable>(
  byteBuffer: inout ByteBuffer)
  -> T
{
  byteBuffer.skipPrefix()
  return getRoot(byteBuffer: &byteBuffer)

}

/// Verifies that the buffer passed is a valid `Flatbuffers` Object.
/// - Parameters:
///   - byteBuffer: Buffer that needs to be checked and read
///   - options: Verifier options
/// - Throws: FlatbuffersErrors
/// - Returns: Returns a valid, checked Flatbuffers object
///
/// ``getCheckedRoot(byteBuffer:options:)`` Takes in a ``ByteBuffer`` and verifies
/// that by creating a ``Verifier`` and checkes if all the `Bytes` and correctly aligned
/// and within the ``ByteBuffer`` range.
public func getCheckedRoot<T: FlatBufferTable & Verifiable>(
  byteBuffer: inout ByteBuffer,
  fileId: String? = nil,
  options: VerifierOptions = .init()) throws -> T
{
  var verifier = try Verifier(buffer: &byteBuffer, options: options)
  let rootPosition = byteBuffer.reader
  if let fileId = fileId {
    try verifier.verify(id: fileId, at: rootPosition)
  }
  try ForwardOffset<T>.verify(&verifier, at: rootPosition, of: T.self)
  return T.init(
    byteBuffer,
    o: Int32(byteBuffer.read(def: UOffset.self, position: rootPosition))
      &+ Int32(rootPosition))
}

/// Returns a `NON-Checked` flatbuffers object
/// - Parameter byteBuffer: Buffer that contains data
/// - Returns: Returns a Flatbuffers object
public func getRoot<T: FlatBufferTable>(byteBuffer: inout ByteBuffer) -> T {
  T.init(
    byteBuffer,
    o: Int32(byteBuffer.read(def: UOffset.self, position: byteBuffer.reader))
      &+ Int32(byteBuffer.reader))
}
