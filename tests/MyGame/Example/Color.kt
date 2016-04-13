// automatically generated, do not modify
package MyGame.Example

import java.nio.*;
import com.google.flatbuffers.kotlin.*;

enum class Color(val value: Byte) {
    Red(1),
    Green(2),
    Blue(8);
    companion object {
        fun from(value : Byte) : Color = when (value.toInt()) {
            1 -> Red
            2 -> Green
            8 -> Blue
            else -> throw Exception("Bad enum value : $value")
        }
    }
}