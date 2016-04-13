// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;
open class Stat(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Table(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {
    fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : Stat = apply {bb = byteBuffer; bb_pos = position}
    override fun toString() : String = "Stat(id=$id,val_=$val_,count=$count)"
    val id : String? get() {val o = __offset(4); return if (o == 0) null else __string(o + bb_pos)}
    val idBuffer : ByteBuffer get() = __vector_as_bytebuffer(4, 1)
    val val_ : Long get() {val o = __offset(6); return if (o == 0) 0L else bb.getLong(o + bb_pos)}
    fun mutateVal(value : Long) :Boolean {val o = __offset(6); return if (o == 0) false else {bb.putLong(o + bb_pos, value); true}}
    val count : Int get() {val o = __offset(8); return if (o == 0) 0.toInt().and(0xFFFF) else bb.getShort(o + bb_pos).toInt().and(0xFFFF)}
    fun mutateCount(value : Int) :Boolean {val o = __offset(8); return if (o == 0) false else {bb.putShort(o + bb_pos, value.and(0xFFFF).toShort()); true}}
    companion object {
        fun FlatBufferBuilder.finishBuffer(offset : Int) { finish(offset) }
    }
}
fun FlatBufferBuilder.stat(id : Int  = 0, val_ : Long = 0L, count : Int = 0.toInt().and(0xFFFF)) = with(this) {
    with(Stat ) {
        startObject(3)
        addLong(1, val_, 0)
        addOffset(0, id, 0)
        addShort(2, count.and(0xFFFF).toShort(), 0)
        endObject()
    }
}