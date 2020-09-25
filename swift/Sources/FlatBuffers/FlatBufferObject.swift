import Foundation

/// FlatbufferObject structures all the Flatbuffers objects
public protocol FlatBufferObject {
    var __buffer: ByteBuffer! { get }
    init(_ bb: ByteBuffer, o: Int32)
}

public protocol ObjectAPI {
    associatedtype T
    static func pack(_ builder: inout FlatBufferBuilder, obj: inout T) -> Offset<UOffset>
    mutating func unpack() -> T
}

/// Readable is structures all the Flatbuffers structs
///
/// Readable is a procotol that each Flatbuffer struct should confirm to since
/// FlatBufferBuilder would require a Type to both create(struct:) and createVector(structs:) functions
public protocol Readable: FlatBufferObject {
    static var size: Int { get }
    static var alignment: Int { get }
}

public protocol Enum {
    associatedtype T: Scalar
    static var byteSize: Int { get }
    var value: T { get }
}
