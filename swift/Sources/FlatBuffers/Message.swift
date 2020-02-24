public protocol FlatBufferGRPCMessage {
    
    /// Raw pointer which would be pointing to the beginning of the readable bytes
    var rawPointer: UnsafeMutableRawPointer { get }
    
    /// Size of readable bytes in the buffer
    var size: Int { get }
    
    init(byteBuffer: ByteBuffer)
}

/// Message is a wrapper around Buffers to to able to send Flatbuffers `Buffers` through the
/// GRPC library
public final class Message<T: FlatBufferObject>: FlatBufferGRPCMessage {
    internal var buffer: ByteBuffer
    
    /// Returns the an object of type T that would be  read from the buffer
    public var object: T {
        T.init(buffer, o: Int32(buffer.read(def: UOffset.self, position: buffer.reader)) + Int32(buffer.reader))
    }
    
    public var rawPointer: UnsafeMutableRawPointer { return buffer.memory.advanced(by: buffer.reader) }
    
    public var size: Int { return Int(buffer.size) }
    
    /// Initializes the message with the type Flatbuffer.Bytebuffer that is transmitted over
    /// GRPC
    /// - Parameter byteBuffer: Flatbuffer ByteBuffer object
    public init(byteBuffer: ByteBuffer) {
        buffer = byteBuffer
    }
    
    /// Initializes the message by copying the buffer to the message to be sent.
    /// from the builder
    /// - Parameter builder: FlatbufferBuilder that has the bytes created in
    /// - Note: Use  `builder.finish(offset)` before passing the builder without prefixing anything to it
    public init(builder: inout FlatBufferBuilder) {
        buffer = builder.sizedBuffer
        builder.clear()
    }
}
