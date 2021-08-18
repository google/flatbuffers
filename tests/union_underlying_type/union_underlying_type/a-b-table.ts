// automatically generated by the FlatBuffers compiler, do not modify

import * as flatbuffers from 'flatbuffers';

import { A, AT } from '../union_underlying_type/a';
import { AB, unionToAB, unionListToAB } from '../union_underlying_type/a-b';
import { B, BT } from '../union_underlying_type/b';


export class ABTable {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
__init(i:number, bb:flatbuffers.ByteBuffer):ABTable {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsABTable(bb:flatbuffers.ByteBuffer, obj?:ABTable):ABTable {
  return (obj || new ABTable()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsABTable(bb:flatbuffers.ByteBuffer, obj?:ABTable):ABTable {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new ABTable()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

abType():AB {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : AB.NONE;
}

ab<T extends flatbuffers.Table>(obj:any):any|null {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.__union(obj, this.bb_pos + offset) : null;
}

static getFullyQualifiedName():string {
  return 'union_underlying_type.ABTable';
}

static startABTable(builder:flatbuffers.Builder) {
  builder.startObject(2);
}

static addAbType(builder:flatbuffers.Builder, abType:AB) {
  builder.addFieldInt32(0, abType, AB.NONE);
}

static addAb(builder:flatbuffers.Builder, abOffset:flatbuffers.Offset) {
  builder.addFieldOffset(1, abOffset, 0);
}

static endABTable(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}

static finishABTableBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset);
}

static finishSizePrefixedABTableBuffer(builder:flatbuffers.Builder, offset:flatbuffers.Offset) {
  builder.finish(offset, undefined, true);
}

static createABTable(builder:flatbuffers.Builder, abType:AB, abOffset:flatbuffers.Offset):flatbuffers.Offset {
  ABTable.startABTable(builder);
  ABTable.addAbType(builder, abType);
  ABTable.addAb(builder, abOffset);
  return ABTable.endABTable(builder);
}

unpack(): ABTableT {
  return new ABTableT(
    this.abType(),
    (() => {
      let temp = unionToAB(this.abType(), this.ab.bind(this));
      if(temp === null) { return null; }
      return temp.unpack()
  })()
  );
}


unpackTo(_o: ABTableT): void {
  _o.abType = this.abType();
  _o.ab = (() => {
      let temp = unionToAB(this.abType(), this.ab.bind(this));
      if(temp === null) { return null; }
      return temp.unpack()
  })();
}
}

export class ABTableT {
constructor(
  public abType: AB = AB.NONE,
  public ab: AT|BT|null = null
){}


pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const ab = builder.createObjectOffset(this.ab);

  return ABTable.createABTable(builder,
    this.abType,
    ab
  );
}
}
