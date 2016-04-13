// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;


open class Test(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Struct(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {
    fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : Test = apply {bb = byteBuffer; bb_pos = position}
    override fun toString() : String = "Test(a=$a,b=$b)"
    var a : Short get() = bb.getShort(bb_pos + 0); set(value) { bb.putShort(bb_pos + 0, value) }
    var b : Byte get() = bb.get(bb_pos + 2); set(value) { bb.put(bb_pos + 2, value) }
}


fun FlatBufferBuilder.testRaw(a : Short, b : Byte):Int = with(this) {
    prep(2, 4)
    pad(1)
    addByte(b)
    addShort(a)
    offset()
}
fun FlatBufferBuilder.test(a : Short, b : Byte):FlatBufferBuilder.()->Int = {testRaw(a, b)}
