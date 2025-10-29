import static com.google.common.truth.Truth.assertThat;

import com.google.common.io.ByteStreams;
import com.google.flatbuffers.FlatBufferBuilder;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import org.junit.Test;
import preservecase.MyGame.Example.Any;
import preservecase.MyGame.Example.Monster;
import preservecase.MyGame.Example.Vec3;
import preservecase.optional_scalars.ScalarStuff;

public class JavaPreserveCaseTest {

  private static ByteBuffer loadResource(String name) throws IOException {
    try (InputStream is = JavaPreserveCaseTest.class.getClassLoader().getResourceAsStream(name)) {
      if (is == null) {
        throw new IOException("Resource not found: " + name);
      }
      return ByteBuffer.wrap(ByteStreams.toByteArray(is));
    }
  }

  @Test
  public void readExistingMonsterBuffer() throws IOException {
    ByteBuffer bb = loadResource("monsterdata_test.mon");
    Monster monster = Monster.getRootAsMonster(bb);

    assertThat(monster.test_type()).isEqualTo((byte) Any.Monster);
    assertThat(monster.testhashu32_fnv1()).isEqualTo(3715746113L);
    assertThat(monster.testhashs32_fnv1()).isEqualTo(-579221183);

    Vec3 pos = monster.pos();
    assertThat(pos.test1()).isWithin(0.0001).of(3.0);
    preservecase.MyGame.Example.Test firstTest = monster.test4(0);
    assertThat(firstTest.a()).isEqualTo((short) 10);
  }

  @Test
  public void buildAndReadSnakeCaseFields() {
    FlatBufferBuilder builder = new FlatBufferBuilder();
    int nameOffset = builder.createString("PreserveCase");
    Monster.startMonster(builder);
    Monster.addName(builder, nameOffset);
    Monster.addHp(builder, (short) 42);
    Monster.addTestType(builder, Any.Monster);
    int monsterOffset = Monster.endMonster(builder);
    Monster.finishMonsterBuffer(builder, monsterOffset);

    ByteBuffer bb = builder.dataBuffer();
    Monster monster = Monster.getRootAsMonster(bb);

    assertThat(monster.name()).isEqualTo("PreserveCase");
    assertThat(monster.hp()).isEqualTo((short) 42);
    assertThat(monster.test_type()).isEqualTo((byte) Any.Monster);
  }

  @Test
  public void optionalScalarsSnakeCaseAccessors() {
    FlatBufferBuilder builder = new FlatBufferBuilder();
    ScalarStuff.startScalarStuff(builder);
    ScalarStuff.addJustI8(builder, (byte) 7);
    ScalarStuff.addMaybeI8(builder, (byte) 9);
    ScalarStuff.addJustBool(builder, true);
    ScalarStuff.addMaybeBool(builder, false);
    ScalarStuff.addJustEnum(builder, (byte) 2);
    ScalarStuff.addMaybeEnum(builder, (byte) 3);
    ScalarStuff.addJustU8(builder, 200);
    ScalarStuff.addMaybeU8(builder, 201);
    ScalarStuff.addJustI16(builder, (short) 3000);
    ScalarStuff.addMaybeI16(builder, (short) -123);
    ScalarStuff.addJustU16(builder, 65000);
    ScalarStuff.addMaybeU16(builder, 1234);
    ScalarStuff.addJustU32(builder, 0xFEDCBAABL);
    ScalarStuff.addJustF32(builder, 0.5f);
    ScalarStuff.addJustF64(builder, 123.25);
    ScalarStuff.addJustU64(builder, 123456789L);
    ScalarStuff.addMaybeU64(builder, 10L);
    ScalarStuff.addMaybeI8(builder, (byte) 9);
    int scalarStuffOffset = ScalarStuff.endScalarStuff(builder);
    builder.finish(scalarStuffOffset);

    ScalarStuff scalarStuff = ScalarStuff.getRootAsScalarStuff(builder.dataBuffer());
    assertThat(scalarStuff.just_i8()).isEqualTo((byte) 7);
    assertThat(scalarStuff.maybe_i8()).isEqualTo((byte) 9);
    assertThat(scalarStuff.just_bool()).isTrue();
    assertThat(scalarStuff.maybe_bool()).isFalse();
    assertThat(scalarStuff.just_enum()).isEqualTo((byte) 2);
    assertThat(scalarStuff.maybe_enum()).isEqualTo((byte) 3);
    assertThat(scalarStuff.just_u32()).isEqualTo(0xFEDCBAABL);
    assertThat(scalarStuff.just_f32()).isWithin(0.0001f).of(0.5f);
    assertThat(scalarStuff.just_f64()).isWithin(0.0001).of(123.25);
    assertThat(scalarStuff.just_u64()).isEqualTo(123456789L);
  }
}
