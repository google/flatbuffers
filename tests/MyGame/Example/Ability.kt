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
class Ability : Struct() {

    fun __init(_i: Int, _bb: ByteBuffer)  {
        __reset(_i, _bb)
    }
    fun __assign(_i: Int, _bb: ByteBuffer) : Ability {
        __init(_i, _bb)
        return this
    }
    val id : UInt get() = bb.getInt(bb_pos + 0).toUInt()
    fun mutateId(id: UInt) : ByteBuffer = bb.putInt(bb_pos + 0, id.toInt())
    val distance : UInt get() = bb.getInt(bb_pos + 4).toUInt()
    fun mutateDistance(distance: UInt) : ByteBuffer = bb.putInt(bb_pos + 4, distance.toInt())
    companion object {
        fun createAbility(builder: FlatBufferBuilder, id: UInt, distance: UInt) : AbilityOffset {
            builder.prep(4, 8)
            builder.putInt(distance.toInt())
            builder.putInt(id.toInt())
            return AbilityOffset(builder.offset())
        }
    }
}

@JvmInline
value class AbilityOffset(val offset: Int)
