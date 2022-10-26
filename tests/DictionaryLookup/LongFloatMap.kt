// automatically generated by the FlatBuffers compiler, do not modify

package DictionaryLookup

import java.nio.*
import kotlin.math.sign
import com.google.flatbuffers.*

@Suppress("unused")
class LongFloatMap : Table() {

    fun __init(_i: Int, _bb: ByteBuffer)  {
        __reset(_i, _bb)
    }
    fun __assign(_i: Int, _bb: ByteBuffer) : LongFloatMap {
        __init(_i, _bb)
        return this
    }
    fun entries(j: Int) : DictionaryLookup.LongFloatEntry? = entries(DictionaryLookup.LongFloatEntry(), j)
    fun entries(obj: DictionaryLookup.LongFloatEntry, j: Int) : DictionaryLookup.LongFloatEntry? {
        val o = __offset(4)
        return if (o != 0) {
            obj.__assign(__indirect(__vector(o) + j * 4), bb)
        } else {
            null
        }
    }
    val entriesLength : Int
        get() {
            val o = __offset(4); return if (o != 0) __vector_len(o) else 0
        }
    fun entriesByKey(key: Long) : DictionaryLookup.LongFloatEntry? {
        val o = __offset(4)
        return if (o != 0) {
            DictionaryLookup.LongFloatEntry.__lookup_by_key(null, __vector(o), key, bb)
        } else {
            null
        }
    }
    fun entriesByKey(obj: DictionaryLookup.LongFloatEntry, key: Long) : DictionaryLookup.LongFloatEntry? {
        val o = __offset(4)
        return if (o != 0) {
            DictionaryLookup.LongFloatEntry.__lookup_by_key(obj, __vector(o), key, bb)
        } else {
            null
        }
    }
    companion object {
        fun validateVersion() = Constants.FLATBUFFERS_22_10_25()
        fun getRootAsLongFloatMap(_bb: ByteBuffer): LongFloatMap = getRootAsLongFloatMap(_bb, LongFloatMap())
        fun getRootAsLongFloatMap(_bb: ByteBuffer, obj: LongFloatMap): LongFloatMap {
            _bb.order(ByteOrder.LITTLE_ENDIAN)
            return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb))
        }
        fun createLongFloatMap(builder: FlatBufferBuilder, entriesOffset: Int) : Int {
            builder.startTable(1)
            addEntries(builder, entriesOffset)
            return endLongFloatMap(builder)
        }
        fun startLongFloatMap(builder: FlatBufferBuilder) = builder.startTable(1)
        fun addEntries(builder: FlatBufferBuilder, entries: Int) = builder.addOffset(0, entries, 0)
        fun createEntriesVector(builder: FlatBufferBuilder, data: IntArray) : Int {
            builder.startVector(4, data.size, 4)
            for (i in data.size - 1 downTo 0) {
                builder.addOffset(data[i])
            }
            return builder.endVector()
        }
        fun startEntriesVector(builder: FlatBufferBuilder, numElems: Int) = builder.startVector(4, numElems, 4)
        fun endLongFloatMap(builder: FlatBufferBuilder) : Int {
            val o = builder.endTable()
            return o
        }
        fun finishLongFloatMapBuffer(builder: FlatBufferBuilder, offset: Int) = builder.finish(offset)
        fun finishSizePrefixedLongFloatMapBuffer(builder: FlatBufferBuilder, offset: Int) = builder.finishSizePrefixed(offset)
    }
}
