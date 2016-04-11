// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;
open class TestSimpleTableWithEnum(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Table(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {
    fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : TestSimpleTableWithEnum = apply {bb = byteBuffer; bb_pos = position}
    override fun toString() : String = "TestSimpleTableWithEnum(color=$color)"
    val color : Color get() {val o = __offset(4); return if (o == 0) Color.from((2).toByte()) else Color.from(bb.get(o + bb_pos))}
    fun mutateColor(value : Color) :Boolean {val o = __offset(4); return if (o == 0) false else {bb.put(o + bb_pos, value.value); true}}
    companion object {
        fun FlatBufferBuilder.finishBuffer(offset : Int) { finish(offset) }
    }
}
fun FlatBufferBuilder.testSimpleTableWithEnum(color : Color = Color.from((2).toByte())) = with(this) {
    with(TestSimpleTableWithEnum ) {
        startObject(1)
        addByte(0, color.value, 2)
        endObject()
    }
}