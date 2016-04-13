// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;

enum class Function(val value: Short) {
    A(-5),
    B(-2),
    C(1),
    D(4),
    E(7),
    F(10),
    G(13),
    H(16),
    I(19);
    companion object {
        fun from(value : Short) : Function = __enums[(value.toInt() + 5) / 3]
        private val __enums = values()
    }
}