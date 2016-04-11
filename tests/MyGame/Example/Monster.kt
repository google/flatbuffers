// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;
/// an example documentation comment: monster object
open class Monster(byteBuffer : ByteBuffer = EMPTY_BYTEBUFFER) : Table(byteBuffer.order(ByteOrder.LITTLE_ENDIAN), if (byteBuffer === EMPTY_BYTEBUFFER) 0 else byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) {
    fun wrap(byteBuffer : ByteBuffer, position : Int = byteBuffer.getInt(byteBuffer.position()) + byteBuffer.position()) : Monster = apply {bb = byteBuffer; bb_pos = position}
    override fun toString() : String = "Monster(pos=$pos,mana=$mana,hp=$hp,name=$name,inventory=${(0 until inventorySize).map({inventory(it).toString()}).joinToString(", ", "[","]")},color=$color,testType=$testType,test=$test,test4=${(0 until test4Size).map({test4(it).toString()}).joinToString(", ", "[","]")},testarrayofstring=${(0 until testarrayofstringSize).map({testarrayofstring(it).toString()}).joinToString(", ", "[","]")},testarrayoftables=${(0 until testarrayoftablesSize).map({testarrayoftables(it).toString()}).joinToString(", ", "[","]")},enemy=$enemy,testnestedflatbuffer=${(0 until testnestedflatbufferSize).map({testnestedflatbuffer(it).toString()}).joinToString(", ", "[","]")},testempty=$testempty,testbool=$testbool,testhashs32Fnv1=$testhashs32Fnv1,testhashu32Fnv1=$testhashu32Fnv1,testhashs64Fnv1=$testhashs64Fnv1,testhashu64Fnv1=$testhashu64Fnv1,testhashs32Fnv1a=$testhashs32Fnv1a,testhashu32Fnv1a=$testhashu32Fnv1a,testhashs64Fnv1a=$testhashs64Fnv1a,testhashu64Fnv1a=$testhashu64Fnv1a,testarrayofbools=${(0 until testarrayofboolsSize).map({testarrayofbools(it).toString()}).joinToString(", ", "[","]")},function=$function,maps=${(0 until mapsSize).map({maps(it).toString()}).joinToString(", ", "[","]")})"
    val pos : Vec3? get() {val o = __offset(4); return if (o == 0) null else Vec3().wrap(bb, o + bb_pos)}
    fun pos(reuse : Vec3) : Vec3? {val o = __offset(4); return if (o == 0) null else reuse.wrap(bb, o + bb_pos)}
    val mana : Short get() {val o = __offset(6); return if (o == 0) (150).toShort() else bb.getShort(o + bb_pos)}
    fun mutateMana(value : Short) :Boolean {val o = __offset(6); return if (o == 0) false else {bb.putShort(o + bb_pos, value); true}}
    val hp : Short get() {val o = __offset(8); return if (o == 0) (100).toShort() else bb.getShort(o + bb_pos)}
    fun mutateHp(value : Short) :Boolean {val o = __offset(8); return if (o == 0) false else {bb.putShort(o + bb_pos, value); true}}
    val name : String? get() {val o = __offset(10); return if (o == 0) null else __string(o + bb_pos)}
    val nameBuffer : ByteBuffer get() = __vector_as_bytebuffer(10, 1)
    val inventorySize : Int get() {val o = __offset(14); return if (o == 0) 0 else __vector_len(o)}
    fun inventory(j : Int) : Int {val o = __offset(14); return if (o == 0) throw Exception("calling member $j of array inventory which is either empty or unset") else bb.get(__vector(o) + j).toInt().and(0xFF)}
    fun mutateInventory(j : Int, value : Int) :Boolean {val o = __offset(14); return if (o == 0) false else {bb.put(__vector(o) + j, value.and(0xFF).toByte()); true}}
    val inventory : ByteBuffer get() = __vector_as_bytebuffer(14, 1)
    val color : Color get() {val o = __offset(16); return if (o == 0) Color.from((8).toByte()) else Color.from(bb.get(o + bb_pos))}
    fun mutateColor(value : Color) :Boolean {val o = __offset(16); return if (o == 0) false else {bb.put(o + bb_pos, value.value); true}}
    val testType : Any get() {val o = __offset(18); return if (o == 0) Any.from(0) else Any.from(bb.get(o + bb_pos))}
    fun mutateTestType(value : Any) :Boolean {val o = __offset(18); return if (o == 0) false else {bb.put(o + bb_pos, value.value); true}}
    fun test(reuse : Table) : Table? {val o = __offset(20); return if (o == 0) null else __union(reuse, o)}
    val test : Table? get() {val o = __offset(20); return if (o == 0) null else __union(Any.toTable(testType), o)}
    val test4Size : Int get() {val o = __offset(22); return if (o == 0) 0 else __vector_len(o)}
    fun test4(j :Int, reuse : Test? = null) : Test {val o = __offset(22); return if (o == 0) throw Exception("calling member $j of array test4 which is either empty or unset") else {val x = __vector(o) + j * 4; (reuse ?: Test() ).wrap(bb, x)}}
    val testarrayofstringSize : Int get() {val o = __offset(24); return if (o == 0) 0 else __vector_len(o)}
    fun testarrayofstring(j : Int) : String? {val o = __offset(24); return if (o == 0) throw Exception("calling member $j of array testarrayofstring which is either empty or unset") else __string(__vector(o) + j * 4)}
    fun testarrayofstringBuffer(j : Int) : ByteBuffer?  {val o = __offset(24); return if (o == 0) throw Exception("calling member $j of array testarrayofstring which is either empty or unset") else __string_element_as_bytebuffer(o, j)}
    val testarrayoftablesSize : Int get() {val o = __offset(26); return if (o == 0) 0 else __vector_len(o)}
    fun testarrayoftables(j :Int, reuse : Monster? = null) : Monster {val o = __offset(26); return if (o == 0) throw Exception("calling member $j of array testarrayoftables which is either empty or unset") else {val x = __vector(o) + j * 4; (reuse ?: Monster() ).wrap(bb, x + __indirect(x))}}
    val enemy : Monster? get() {val o = __offset(28); return if (o == 0) null else Monster().wrap(bb, __indirect(o + bb_pos))}
    fun enemy(reuse : Monster) : Monster? {val o = __offset(28); return if (o == 0) null else reuse.wrap(bb, __indirect(o + bb_pos))}
    val testnestedflatbufferSize : Int get() {val o = __offset(30); return if (o == 0) 0 else __vector_len(o)}
    fun testnestedflatbuffer(j : Int) : Int {val o = __offset(30); return if (o == 0) throw Exception("calling member $j of array testnestedflatbuffer which is either empty or unset") else bb.get(__vector(o) + j).toInt().and(0xFF)}
    fun mutateTestnestedflatbuffer(j : Int, value : Int) :Boolean {val o = __offset(30); return if (o == 0) false else {bb.put(__vector(o) + j, value.and(0xFF).toByte()); true}}
    val testnestedflatbuffer : ByteBuffer get() = __vector_as_bytebuffer(30, 1)
    val testempty : Stat? get() {val o = __offset(32); return if (o == 0) null else Stat().wrap(bb, __indirect(o + bb_pos))}
    fun testempty(reuse : Stat) : Stat? {val o = __offset(32); return if (o == 0) null else reuse.wrap(bb, __indirect(o + bb_pos))}
    val testbool : Boolean get() {val o = __offset(34); return if (o == 0) false else 0.toByte()!=bb.get(o + bb_pos)}
    fun mutateTestbool(value : Boolean) :Boolean {val o = __offset(34); return if (o == 0) false else {bb.put(o + bb_pos, if (value) 1.toByte() else 0.toByte()); true}}
    val testhashs32Fnv1 : Int get() {val o = __offset(36); return if (o == 0) 0 else bb.getInt(o + bb_pos)}
    fun mutateTesthashs32Fnv1(value : Int) :Boolean {val o = __offset(36); return if (o == 0) false else {bb.putInt(o + bb_pos, value); true}}
    val testhashu32Fnv1 : Long get() {val o = __offset(38); return if (o == 0) 0.toLong().and(0xFFFFFFFFL) else bb.getInt(o + bb_pos).toLong().and(0xFFFFFFFFL)}
    fun mutateTesthashu32Fnv1(value : Long) :Boolean {val o = __offset(38); return if (o == 0) false else {bb.putInt(o + bb_pos, value.and(0xFFFFFFFFL).toInt()); true}}
    val testhashs64Fnv1 : Long get() {val o = __offset(40); return if (o == 0) 0L else bb.getLong(o + bb_pos)}
    fun mutateTesthashs64Fnv1(value : Long) :Boolean {val o = __offset(40); return if (o == 0) false else {bb.putLong(o + bb_pos, value); true}}
    val testhashu64Fnv1 : Long get() {val o = __offset(42); return if (o == 0) 0L else bb.getLong(o + bb_pos)}
    fun mutateTesthashu64Fnv1(value : Long) :Boolean {val o = __offset(42); return if (o == 0) false else {bb.putLong(o + bb_pos, value); true}}
    val testhashs32Fnv1a : Int get() {val o = __offset(44); return if (o == 0) 0 else bb.getInt(o + bb_pos)}
    fun mutateTesthashs32Fnv1a(value : Int) :Boolean {val o = __offset(44); return if (o == 0) false else {bb.putInt(o + bb_pos, value); true}}
    val testhashu32Fnv1a : Long get() {val o = __offset(46); return if (o == 0) 0.toLong().and(0xFFFFFFFFL) else bb.getInt(o + bb_pos).toLong().and(0xFFFFFFFFL)}
    fun mutateTesthashu32Fnv1a(value : Long) :Boolean {val o = __offset(46); return if (o == 0) false else {bb.putInt(o + bb_pos, value.and(0xFFFFFFFFL).toInt()); true}}
    val testhashs64Fnv1a : Long get() {val o = __offset(48); return if (o == 0) 0L else bb.getLong(o + bb_pos)}
    fun mutateTesthashs64Fnv1a(value : Long) :Boolean {val o = __offset(48); return if (o == 0) false else {bb.putLong(o + bb_pos, value); true}}
    val testhashu64Fnv1a : Long get() {val o = __offset(50); return if (o == 0) 0L else bb.getLong(o + bb_pos)}
    fun mutateTesthashu64Fnv1a(value : Long) :Boolean {val o = __offset(50); return if (o == 0) false else {bb.putLong(o + bb_pos, value); true}}
    val testarrayofboolsSize : Int get() {val o = __offset(52); return if (o == 0) 0 else __vector_len(o)}
    fun testarrayofbools(j : Int) : Boolean {val o = __offset(52); return if (o == 0) throw Exception("calling member $j of array testarrayofbools which is either empty or unset") else 0.toByte()!=bb.get(__vector(o) + j)}
    fun mutateTestarrayofbools(j : Int, value : Boolean) :Boolean {val o = __offset(52); return if (o == 0) false else {bb.put(__vector(o) + j, if (value) 1.toByte() else 0.toByte()); true}}
    val testarrayofbools : ByteBuffer get() = __vector_as_bytebuffer(52, 1)
    val function : Function get() {val o = __offset(54); return if (o == 0) Function.from((-2).toShort()) else Function.from(bb.getShort(o + bb_pos))}
    fun mutateFunction(value : Function) :Boolean {val o = __offset(54); return if (o == 0) false else {bb.putShort(o + bb_pos, value.value); true}}
    val mapsSize : Int get() {val o = __offset(56); return if (o == 0) 0 else __vector_len(o)}
    fun maps(j : Int) : Map {val o = __offset(56); return if (o == 0) throw Exception("calling member $j of array maps which is either empty or unset") else Map.from(bb.get(__vector(o) + j))}
    fun mutateMaps(j : Int, value : Map) :Boolean {val o = __offset(56); return if (o == 0) false else {bb.put(__vector(o) + j, value.value.and(0xFF).toByte()); true}}
    val maps : ByteBuffer get() = __vector_as_bytebuffer(56, 1)
    companion object {
        fun hasIdentifier(byteBuffer : ByteBuffer) :Boolean = Table.hasIdentifier(byteBuffer, "MONS")
        inline fun FlatBufferBuilder.inventory(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(1, numElems, 1); action(); return endArray()}
        fun FlatBufferBuilder.inventory(data : IntArray): Int {startArray(1, data.size, 1); for (i in data.size - 1 downTo 0) addByte(data[i].and(0xFF).toByte()); return endArray(); }
        inline fun FlatBufferBuilder.test4(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(4, numElems, 2); action(); return endArray()}
        inline fun FlatBufferBuilder.testarrayofstring(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(4, numElems, 4); action(); return endArray()}
        fun FlatBufferBuilder.testarrayofstring(offsets : IntArray)  : Int {startArray(4, offsets.size, 4); for (i in offsets.size - 1 downTo 0) addOffset(offsets[i]); return endArray(); }
        inline fun FlatBufferBuilder.testarrayoftables(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(4, numElems, 4); action(); return endArray()}
        fun FlatBufferBuilder.testarrayoftables(offsets : IntArray)  : Int {startArray(4, offsets.size, 1); for (i in offsets.size - 1 downTo 0) addOffset(offsets[i]); return endArray(); }
        inline fun FlatBufferBuilder.testnestedflatbuffer(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(1, numElems, 1); action(); return endArray()}
        fun FlatBufferBuilder.testnestedflatbuffer(data : IntArray): Int {startArray(1, data.size, 1); for (i in data.size - 1 downTo 0) addByte(data[i].and(0xFF).toByte()); return endArray(); }
        inline fun FlatBufferBuilder.testarrayofbools(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(1, numElems, 1); action(); return endArray()}
        fun FlatBufferBuilder.testarrayofbools(data : BooleanArray): Int {startArray(1, data.size, 1); for (i in data.size - 1 downTo 0) addBoolean(data[i]); return endArray(); }
        inline fun FlatBufferBuilder.maps(numElems : Int, action : FlatBufferBuilder.()->Unit) : Int {startArray(1, numElems, 1); action(); return endArray()}
        fun FlatBufferBuilder.maps(data : Array<Map>): Int {startArray(1, data.size, 1); for (i in data.size - 1 downTo 0) addByte(data[i].value.and(0xFF).toByte()); return endArray(); }
        fun FlatBufferBuilder.finishBuffer(offset : Int) { finish(offset, "MONS") }
    }
}
fun FlatBufferBuilder.monster(name : Int , pos : FlatBufferBuilder.()->Int  = {0}, mana : Short = (150).toShort(), hp : Short = (100).toShort(), inventory : Int  = 0, color : Color = Color.from((8).toByte()), testType : Any = Any.from(0), test : Int  = 0, test4 : Int  = 0, testarrayofstring : Int  = 0, testarrayoftables : Int  = 0, enemy : Int  = 0, testnestedflatbuffer : Int  = 0, testempty : Int  = 0, testbool : Boolean = false, testhashs32Fnv1 : Int = 0, testhashu32Fnv1 : Long = 0.toLong().and(0xFFFFFFFFL), testhashs64Fnv1 : Long = 0L, testhashu64Fnv1 : Long = 0L, testhashs32Fnv1a : Int = 0, testhashu32Fnv1a : Long = 0.toLong().and(0xFFFFFFFFL), testhashs64Fnv1a : Long = 0L, testhashu64Fnv1a : Long = 0L, testarrayofbools : Int  = 0, function : Function = Function.from((-2).toShort()), maps : Int  = 0) = with(this) {
    with(Monster ) {
        startObject(27)
        addLong(23, testhashu64Fnv1a, 0)
        addLong(22, testhashs64Fnv1a, 0)
        addLong(19, testhashu64Fnv1, 0)
        addLong(18, testhashs64Fnv1, 0)
        addOffset(26, maps, 0)
        addOffset(24, testarrayofbools, 0)
        addInt(21, testhashu32Fnv1a.and(0xFFFFFFFFL).toInt(), 0)
        addInt(20, testhashs32Fnv1a, 0)
        addInt(17, testhashu32Fnv1.and(0xFFFFFFFFL).toInt(), 0)
        addInt(16, testhashs32Fnv1, 0)
        addOffset(14, testempty, 0)
        addOffset(13, testnestedflatbuffer, 0)
        addOffset(12, enemy, 0)
        addOffset(11, testarrayoftables, 0)
        addOffset(10, testarrayofstring, 0)
        addOffset(9, test4, 0)
        addOffset(8, test, 0)
        addOffset(5, inventory, 0)
        addOffset(3, name, 0)
        addStruct(0, pos(), 0)
        addShort(25, function.value, -2)
        addShort(2, hp, 100)
        addShort(1, mana, 150)
        addBoolean(15, testbool, 0!=0)
        addByte(7, testType.value, 0)
        addByte(6, color.value, 8)
        endObject()
    }
}