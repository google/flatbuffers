import Foundation

public final class FlatBuffersUtils {
    
    /// Gets the size of the prefix
    /// - Parameter bb: Flatbuffer object
    public static func getSizePrefix(bb: ByteBuffer) -> Int32 {
        return bb.read(def: Int32.self, position: bb.reader)
    }
    
    /// Removes the prefix by duplicating the Flatbuffer
    /// - Parameter bb: Flatbuffer object
    public static func removeSizePrefix(bb: ByteBuffer) -> ByteBuffer {
        return bb.duplicate(removing: MemoryLayout<Int32>.size)
    }
}
