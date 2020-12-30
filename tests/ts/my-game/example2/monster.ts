// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';



/**
 * @constructor
 */
export class Monster {
  /**
   * @type flatbuffers.ByteBuffer
   */
  bb: flatbuffers.ByteBuffer|null = null;

  /**
   * @type number
   */
  bb_pos = 0;
/**
 * @param number i
 * @param flatbuffers.ByteBuffer bb
 * @returns Monster
 */
__init(i:number, bb:flatbuffers.ByteBuffer):Monster {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

/**
 * @param flatbuffers.ByteBuffer bb
 * @param Monster= obj
 * @returns Monster
 */
static getRootAsMonster(bb:flatbuffers.ByteBuffer, obj?:Monster):Monster {
  return (obj || new Monster()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

/**
 * @param flatbuffers.ByteBuffer bb
 * @param Monster= obj
 * @returns Monster
 */
static getSizePrefixedRootAsMonster(bb:flatbuffers.ByteBuffer, obj?:Monster):Monster {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new Monster()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

/**
 * @returns string
 */
static getFullyQualifiedName():string {
  return 'MyGame.Example2.Monster';
}

/**
 * @param flatbuffers.Builder builder
 */
static startMonster(builder:flatbuffers.Builder) {
  builder.startObject(0);
}

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
static endMonster(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static createMonster(builder:flatbuffers.Builder):flatbuffers.Offset {
  Monster.startMonster(builder);
  return Monster.endMonster(builder);
}

serialize():Uint8Array {
  return this.bb!.bytes();
}

static deserialize(buffer: Uint8Array):Monster {
  return Monster.getRootAsMonster(new flatbuffers.ByteBuffer(buffer))
}

/**
 * @returns MonsterT
 */
unpack(): MonsterT {
  return new MonsterT();
}

/**
 * @param MonsterT _o
 */
unpackTo(_o: MonsterT): void {}
}

export class MonsterT {
/**
 * @constructor
 */
constructor(){}

/**
 * @param flatbuffers.Builder builder
 * @returns flatbuffers.Offset
 */
pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  return Monster.createMonster(builder);
}
}
