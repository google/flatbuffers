import Foundation

public struct Struct {
    public private(set) var bb: ByteBuffer
    public private(set) var postion: Int32
    
    public init(bb: ByteBuffer, position: Int32 = 0) {
        self.bb = bb
        self.postion = position
    }
    
    public func readBuffer<T: Scalar>(of type: T.Type, at o: Int32) -> T {
        let r = bb.read(def: T.self, position: Int(o + postion))
        return r
    }
}
