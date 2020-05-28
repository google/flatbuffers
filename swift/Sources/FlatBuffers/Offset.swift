import Foundation

/// Offset object for all the Objects that are written into the buffer
public struct Offset<T> {
    /// Offset of the object in the buffer
    public var o: UOffset
    /// Returns false if the offset is equal to zero
    public var isEmpty: Bool { return o == 0 }
    
    public init(offset: UOffset) { o = offset }
    public init() { o = 0 }
}
