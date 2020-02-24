import Foundation

/// FlatbufferObject structures all the Flatbuffers objects
public protocol FlatBufferObject {
    var __buffer: ByteBuffer! { get }
    init(_ bb: ByteBuffer, o: Int32)
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

/// Mutable is a protocol that allows us to mutate Scalar values within the buffer
public protocol Mutable {
    /// makes Flatbuffer accessed within the Protocol
    var bb: ByteBuffer { get }
    /// makes position of the table/struct  accessed within the Protocol
    var postion: Int32 { get }
}

extension Mutable {
    
    /// Mutates the memory in the buffer, this is only called from the access function of table and structs
    /// - Parameters:
    ///   - value: New value to be inserted to the buffer
    ///   - index: index of the Element
    func mutate<T: Scalar>(value: T, o: Int32) -> Bool {
        guard o != 0 else { return false }
        bb.write(value: value, index: Int(o), direct: true)
        return true
    }
}

extension Mutable where Self == Table {
    
    /// Mutates a value by calling mutate with respect to the position in the table
    /// - Parameters:
    ///   - value: New value to be inserted to the buffer
    ///   - index: index of the Element
    public func mutate<T: Scalar>(_ value: T, index: Int32) -> Bool {
        guard index != 0 else { return false }
        return mutate(value: value, o: index + postion)
    }
    
    /// Directly mutates the element by calling mutate
    ///
    /// Mutates the Element at index ignoring the current position by calling mutate
    /// - Parameters:
    ///   - value: New value to be inserted to the buffer
    ///   - index: index of the Element
    public func directMutate<T: Scalar>(_ value: T, index: Int32) -> Bool {
        return mutate(value: value, o: index)
    }
}

extension Mutable where Self == Struct {
    
    /// Mutates a value by calling mutate with respect to the position in the struct
    /// - Parameters:
    ///   - value: New value to be inserted to the buffer
    ///   - index: index of the Element
    public func mutate<T: Scalar>(_ value: T, index: Int32) -> Bool {
        return mutate(value: value, o: index + postion)
    }
    
    /// Directly mutates the element by calling mutate
    ///
    /// Mutates the Element at index ignoring the current position by calling mutate
    /// - Parameters:
    ///   - value: New value to be inserted to the buffer
    ///   - index: index of the Element
    public func directMutate<T: Scalar>(_ value: T, index: Int32) -> Bool {
        return mutate(value: value, o: index)
    }
}
extension Struct: Mutable {}
extension Table: Mutable {}
