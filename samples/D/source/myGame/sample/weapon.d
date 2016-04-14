// automatically generated, do not modify

module myGame.sample.weapon;

import google.flatbuffers;

struct Weapon {
  mixin Table!Weapon;

  static Weapon getRootAsWeapon(ByteBuffer _bb) {  return Weapon.init_(_bb.get!int(_bb.position()) + _bb.position(), _bb); }
  @property   Nullable!string name() { int o = __offset(4); return o != 0 ? Nullable!string(__string(o + _pos)) : Nullable!string.init; }
  @property   short damage() { int o = __offset(6); return o != 0 ? _buffer.get!short(o + _pos) : 0; }

  static int createWeapon(FlatBufferBuilder builder,
      int name,
      short damage) {
    builder.startObject(2);
    Weapon.addName(builder, name);
    Weapon.addDamage(builder, damage);
    return Weapon.endWeapon(builder);
  }

  static void startWeapon(FlatBufferBuilder builder) { builder.startObject(2); }
  static void addName(FlatBufferBuilder builder, int nameOffset) { builder.addOffset(0, nameOffset, 0); }
  static void addDamage(FlatBufferBuilder builder, short damage) { builder.addShort(1, damage, 0); }
  static int endWeapon(FlatBufferBuilder builder) {
    int o = builder.endObject();
    return o;
  }
}

