// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;

enum class Any(val value: Byte) {
    NONE(0),
    Monster(1),
    TestSimpleTableWithEnum(2);
    companion object {
        fun from(value : Byte) : Any = __enums[value.toInt()]
        private val __enums = values()
        fun toTable( value : Any) : Table = when (value) {
            NONE -> throw Exception("void union")
            Monster -> MyGame.Example.Monster()
            TestSimpleTableWithEnum -> MyGame.Example.TestSimpleTableWithEnum()
        }
    }
}