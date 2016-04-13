// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;


open class Vec3(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Struct(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {
    fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : Vec3 = apply {bb = byteBuffer; bb_pos = position}
    override fun toString() : String = "Vec3(x=$x,y=$y,z=$z,test1=$test1,test2=$test2,test3=$test3)"
    var x : Float get() = bb.getFloat(bb_pos + 0); set(value) { bb.putFloat(bb_pos + 0, value) }
    var y : Float get() = bb.getFloat(bb_pos + 4); set(value) { bb.putFloat(bb_pos + 4, value) }
    var z : Float get() = bb.getFloat(bb_pos + 8); set(value) { bb.putFloat(bb_pos + 8, value) }
    var test1 : Double get() = bb.getDouble(bb_pos + 16); set(value) { bb.putDouble(bb_pos + 16, value) }
    var test2 : Color get() = Color.from(bb.get(bb_pos + 24)); set(value) { bb.put(bb_pos + 24, value.value) }
    val test3 : Test get() = test3(Test())
    fun test3(reuse : Test) : Test = reuse.wrap(bb, bb_pos + 26)
}

inline 
fun FlatBufferBuilder.vec3Raw(x : Float, y : Float, z : Float, test1 : Double, test2 : Color, crossinline test3 : FlatBufferBuilder.()->Int):Int = with(this) {
    prep(16, 32)
    pad(2)
    test3()
    pad(1)
    addByte(test2.value)
    addDouble(test1)
    pad(4)
    addFloat(z)
    addFloat(y)
    addFloat(x)
    offset()
}
inline fun FlatBufferBuilder.vec3(x : Float, y : Float, z : Float, test1 : Double, test2 : Color, crossinline test3 : FlatBufferBuilder.()->Int):FlatBufferBuilder.()->Int = {vec3Raw(x, y, z, test1, test2, test3)}
