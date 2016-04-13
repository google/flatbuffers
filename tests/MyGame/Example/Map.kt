// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;

enum class Map(val value: Int) {
    A(0),
    B(3),
    C(4),
    D(5),
    E(6),
    F(7),
    G(8),
    H(9),
    I(10);
    companion object {
        fun from(value : Byte) : Map= map[value.toInt()] ?: throw Exception("Bad enum value : $value")
        private val map = mapOf(
            0 to A,
            3 to B,
            4 to C,
            5 to D,
            6 to E,
            7 to F,
            8 to G,
            9 to H,
            10 to I)
    }
}