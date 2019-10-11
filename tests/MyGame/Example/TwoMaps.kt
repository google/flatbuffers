// automatically generated by the FlatBuffers compiler, do not modify

package MyGame.Example

import java.nio.*
import kotlin.math.sign
import com.google.flatbuffers.*

@Suppress("unused")
@ExperimentalUnsignedTypes
class TwoMaps : Table() {

    fun __init(_i: Int, _bb: ByteBuffer)  {
        __reset(_i, _bb)
    }
    fun __assign(_i: Int, _bb: ByteBuffer) : TwoMaps {
        __init(_i, _bb)
        return this
    }
    fun mapFromStringToInt(j: Int) : MyGame.Example.KeyValStringInt? = mapFromStringToInt(MyGame.Example.KeyValStringInt(), j)
    fun mapFromStringToInt(obj: MyGame.Example.KeyValStringInt, j: Int) : MyGame.Example.KeyValStringInt? {
        val o = __offset(4)
        return if (o != 0) {
            obj.__assign(__indirect(__vector(o) + j * 4), bb)
        } else {
            null
        }
    }
    val mapFromStringToIntLength : Int
        get() {
            val o = __offset(4); return if (o != 0) __vector_len(o) else 0
        }
    fun mapFromStringToBool(j: Int) : MyGame.Example.KeyValStringBool? = mapFromStringToBool(MyGame.Example.KeyValStringBool(), j)
    fun mapFromStringToBool(obj: MyGame.Example.KeyValStringBool, j: Int) : MyGame.Example.KeyValStringBool? {
        val o = __offset(6)
        return if (o != 0) {
            obj.__assign(__indirect(__vector(o) + j * 4), bb)
        } else {
            null
        }
    }
    val mapFromStringToBoolLength : Int
        get() {
            val o = __offset(6); return if (o != 0) __vector_len(o) else 0
        }
    companion object {
        fun validateVersion() = Constants.FLATBUFFERS_1_11_1()
        fun getRootAsTwoMaps(_bb: ByteBuffer): TwoMaps = getRootAsTwoMaps(_bb, TwoMaps())
        fun getRootAsTwoMaps(_bb: ByteBuffer, obj: TwoMaps): TwoMaps {
            _bb.order(ByteOrder.LITTLE_ENDIAN)
            return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb))
        }
        fun createTwoMaps(builder: FlatBufferBuilder, mapFromStringToIntOffset: Int, mapFromStringToBoolOffset: Int) : Int {
            builder.startTable(2)
            addMapFromStringToBool(builder, mapFromStringToBoolOffset)
            addMapFromStringToInt(builder, mapFromStringToIntOffset)
            return endTwoMaps(builder)
        }
        fun startTwoMaps(builder: FlatBufferBuilder) = builder.startTable(2)
        fun addMapFromStringToInt(builder: FlatBufferBuilder, mapFromStringToInt: Int) = builder.addOffset(0, mapFromStringToInt, 0)
        fun createMapFromStringToIntVector(builder: FlatBufferBuilder, data: IntArray) : Int {
            builder.startVector(4, data.size, 4)
            for (i in data.size - 1 downTo 0) {
                builder.addOffset(data[i])
            }
            return builder.endVector()
        }
        fun startMapFromStringToIntVector(builder: FlatBufferBuilder, numElems: Int) = builder.startVector(4, numElems, 4)
        fun addMapFromStringToBool(builder: FlatBufferBuilder, mapFromStringToBool: Int) = builder.addOffset(1, mapFromStringToBool, 0)
        fun createMapFromStringToBoolVector(builder: FlatBufferBuilder, data: IntArray) : Int {
            builder.startVector(4, data.size, 4)
            for (i in data.size - 1 downTo 0) {
                builder.addOffset(data[i])
            }
            return builder.endVector()
        }
        fun startMapFromStringToBoolVector(builder: FlatBufferBuilder, numElems: Int) = builder.startVector(4, numElems, 4)
        fun endTwoMaps(builder: FlatBufferBuilder) : Int {
            val o = builder.endTable()
            return o
        }
    }
}
