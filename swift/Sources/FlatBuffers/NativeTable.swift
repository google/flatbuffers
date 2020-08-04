import Foundation

public protocol NativeTable {}

extension NativeTable {
    
    /// Serialize is a helper function that serailizes the data from the Object API to a bytebuffer directly th
    /// - Parameter type: Type of the Flatbuffer object
    /// - Returns: returns the encoded sized ByteBuffer
    public func serialize<T: ObjectAPI>(type: T.Type) -> ByteBuffer where T.T == Self {
        var builder = FlatBufferBuilder(initialSize: 1024)
        return serialize(builder: &builder, type: type.self)
    }
    
    /// Serialize is a helper function that serailizes the data from the Object API to a bytebuffer directly.
    ///
    /// - Parameters:
    ///   - builder: A FlatBufferBuilder
    ///   - type: Type of the Flatbuffer object
    /// - Returns: returns the encoded sized ByteBuffer
    /// - Note: The `serialize(builder:type)` can be considered as a function that allows you to create smaller builder instead of the default `1024`.
    ///  It can be considered less expensive in terms of memory allocation
    public func serialize<T: ObjectAPI>(builder: inout FlatBufferBuilder, type: T.Type) -> ByteBuffer where T.T == Self {
        var s = self
        let root = type.pack(&builder, obj: &s)
        builder.finish(offset: root)
        return builder.sizedBuffer
    }
}
