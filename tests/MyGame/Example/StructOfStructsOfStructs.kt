// automatically generated by the FlatBuffers compiler, do not modify

package MyGame.Example

import com.google.flatbuffers.BaseVector
import com.google.flatbuffers.BooleanVector
import com.google.flatbuffers.ByteVector
import com.google.flatbuffers.Constants
import com.google.flatbuffers.DoubleVector
import com.google.flatbuffers.FlatBufferBuilder
import com.google.flatbuffers.FloatVector
import com.google.flatbuffers.LongVector
import com.google.flatbuffers.StringVector
import com.google.flatbuffers.Struct
import com.google.flatbuffers.Table
import com.google.flatbuffers.UnionVector
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlin.math.sign

@Suppress("unused")
@kotlin.ExperimentalUnsignedTypes
class StructOfStructsOfStructs : Struct() {

    fun __init(_i: Int, _bb: ByteBuffer)  {
        __reset(_i, _bb)
    }
    fun __assign(_i: Int, _bb: ByteBuffer) : StructOfStructsOfStructs {
        __init(_i, _bb)
        return this
    }
    val a : MyGame.Example.StructOfStructs? get() = a(MyGame.Example.StructOfStructs())
    fun a(obj: MyGame.Example.StructOfStructs) : MyGame.Example.StructOfStructs? = obj.__assign(bb_pos + 0, bb)
    companion object {
        fun createStructOfStructsOfStructs(builder: FlatBufferBuilder, a_a_id: UInt, a_a_distance: UInt, a_b_a: Short, a_b_b: Byte, a_c_id: UInt, a_c_distance: UInt) : StructOfStructsOfStructsOffset {
            builder.prep(4, 20)
            builder.prep(4, 20)
            builder.prep(4, 8)
            builder.putInt(a_c_distance.toInt())
            builder.putInt(a_c_id.toInt())
            builder.prep(2, 4)
            builder.pad(1)
            builder.putByte(a_b_b)
            builder.putShort(a_b_a)
            builder.prep(4, 8)
            builder.putInt(a_a_distance.toInt())
            builder.putInt(a_a_id.toInt())
            return StructOfStructsOfStructsOffset(builder.offset())
        }
    }
}

@JvmInline
value class StructOfStructsOfStructsOffset(val offset: Int)
