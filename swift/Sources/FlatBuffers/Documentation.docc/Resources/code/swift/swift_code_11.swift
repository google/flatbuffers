import FlatBuffers
import Foundation

func run() {
  // create a ByteBuffer(:) from an [UInt8] or Data()
  let buf = [] // Get your data

  // Get an accessor to the root object inside the buffer.
  let monster: Monster = try! getCheckedRoot(byteBuffer: ByteBuffer(bytes: buf))
  // let monster: Monster = getRoot(byteBuffer: ByteBuffer(bytes: buf))
}
