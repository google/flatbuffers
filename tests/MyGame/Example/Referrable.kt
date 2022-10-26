// automatically generated by the FlatBuffers compiler, do not modify

package MyGame.Example

import java.nio.*
import kotlin.math.sign
import com.google.flatbuffers.*

@Suppress("unused")
class Referrable : Table() {

    fun __init(_i: Int, _bb: ByteBuffer)  {
        __reset(_i, _bb)
    }
    fun __assign(_i: Int, _bb: ByteBuffer) : Referrable {
        __init(_i, _bb)
        return this
    }
    val id : ULong
        get() {
            val o = __offset(4)
            return if(o != 0) bb.getLong(o + bb_pos).toULong() else 0UL
        }
    fun mutateId(id: ULong) : Boolean {
        val o = __offset(4)
        return if (o != 0) {
            bb.putLong(o + bb_pos, id.toLong())
            true
        } else {
            false
        }
    }
    override fun keysCompare(o1: Int, o2: Int, _bb: ByteBuffer) : Int {
        val val_1 = _bb.getLong(__offset(4, o1, _bb))
        val val_2 = _bb.getLong(__offset(4, o2, _bb))
        return (val_1 - val_2).sign
    }
    companion object {
        fun validateVersion() = Constants.FLATBUFFERS_22_10_25()
        fun getRootAsReferrable(_bb: ByteBuffer): Referrable = getRootAsReferrable(_bb, Referrable())
        fun getRootAsReferrable(_bb: ByteBuffer, obj: Referrable): Referrable {
            _bb.order(ByteOrder.LITTLE_ENDIAN)
            return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb))
        }
        fun createReferrable(builder: FlatBufferBuilder, id: ULong) : Int {
            builder.startTable(1)
            addId(builder, id)
            return endReferrable(builder)
        }
        fun startReferrable(builder: FlatBufferBuilder) = builder.startTable(1)
        fun addId(builder: FlatBufferBuilder, id: ULong)  {
            builder.addLong(id.toLong())
            builder.slot(0)
        }
        fun endReferrable(builder: FlatBufferBuilder) : Int {
            val o = builder.endTable()
            return o
        }
        fun __lookup_by_key(obj: Referrable?, vectorLocation: Int, key: ULong, bb: ByteBuffer) : Referrable? {
            var span = bb.getInt(vectorLocation - 4)
            var start = 0
            while (span != 0) {
                var middle = span / 2
                val tableOffset = __indirect(vectorLocation + 4 * (start + middle), bb)
                val value = bb.getLong(__offset(4, bb.capacity() - tableOffset, bb)).toULong()
                val comp = value.compareTo(key)
                when {
                    comp > 0 -> span = middle
                    comp < 0 -> {
                        middle++
                        start += middle
                        span -= middle
                    }
                    else -> {
                        return (obj ?: Referrable()).__assign(tableOffset, bb)
                    }
                }
            }
            return null
        }
    }
}
