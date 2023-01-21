// automatically generated by the FlatBuffers compiler, do not modify

package NamespaceC

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
@ExperimentalUnsignedTypes
class TableInC : Table() {

    fun __init(_i: Int, _bb: ByteBuffer)  {
        __reset(_i, _bb)
    }
    fun __assign(_i: Int, _bb: ByteBuffer) : TableInC {
        __init(_i, _bb)
        return this
    }
    val referToA1 : NamespaceA.TableInFirstNS? get() = referToA1(NamespaceA.TableInFirstNS())
    fun referToA1(obj: NamespaceA.TableInFirstNS) : NamespaceA.TableInFirstNS? {
        val o = __offset(4)
        return if (o != 0) {
            obj.__assign(__indirect(o + bb_pos), bb)
        } else {
            null
        }
    }
    val referToA2 : NamespaceA.SecondTableInA? get() = referToA2(NamespaceA.SecondTableInA())
    fun referToA2(obj: NamespaceA.SecondTableInA) : NamespaceA.SecondTableInA? {
        val o = __offset(6)
        return if (o != 0) {
            obj.__assign(__indirect(o + bb_pos), bb)
        } else {
            null
        }
    }
    companion object {
        fun validateVersion() = Constants.FLATBUFFERS_23_1_21()
        fun getRootAsTableInC(_bb: ByteBuffer): TableInC = getRootAsTableInC(_bb, TableInC())
        fun getRootAsTableInC(_bb: ByteBuffer, obj: TableInC): TableInC {
            _bb.order(ByteOrder.LITTLE_ENDIAN)
            return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb))
        }
        fun createTableInC(builder: FlatBufferBuilder, referToA1Offset: Int, referToA2Offset: Int) : Int {
            builder.startTable(2)
            addReferToA2(builder, referToA2Offset)
            addReferToA1(builder, referToA1Offset)
            return endTableInC(builder)
        }
        fun startTableInC(builder: FlatBufferBuilder) = builder.startTable(2)
        fun addReferToA1(builder: FlatBufferBuilder, referToA1: Int) = builder.addOffset(0, referToA1, 0)
        fun addReferToA2(builder: FlatBufferBuilder, referToA2: Int) = builder.addOffset(1, referToA2, 0)
        fun endTableInC(builder: FlatBufferBuilder) : Int {
            val o = builder.endTable()
            return o
        }
    }
}
