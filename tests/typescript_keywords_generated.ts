import * as flatbuffers from 'flatbuffers';
import {Schema as reflectionSchema, SchemaT as reflectionSchemaT} from  './reflection_generated';
import {class_ as foobarclass_} from  './typescript_include_generated';
import {Abc as foobarAbc} from  './typescript_transitive_include_generated';


export enum class_{
  new_ = 0,
  instanceof_ = 1
}

export class Object_ {
  bb: flatbuffers.ByteBuffer|null = null;
  bb_pos = 0;
__init(i:number, bb:flatbuffers.ByteBuffer):Object_ {
  this.bb_pos = i;
  this.bb = bb;
  return this;
}

static getRootAsObject(bb:flatbuffers.ByteBuffer, obj?:Object_):Object_ {
  return (obj || new Object_()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

static getSizePrefixedRootAsObject(bb:flatbuffers.ByteBuffer, obj?:Object_):Object_ {
  bb.setPosition(bb.position() + flatbuffers.SIZE_PREFIX_LENGTH);
  return (obj || new Object_()).__init(bb.readInt32(bb.position()) + bb.position(), bb);
}

return():number {
  const offset = this.bb!.__offset(this.bb_pos, 4);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : 0;
}

mutate_return(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 4);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

if():number {
  const offset = this.bb!.__offset(this.bb_pos, 6);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : 0;
}

mutate_if(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 6);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

switch():number {
  const offset = this.bb!.__offset(this.bb_pos, 8);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : 0;
}

mutate_switch(value:number):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 8);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

enum():class_ {
  const offset = this.bb!.__offset(this.bb_pos, 10);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : class_.new_;
}

mutate_enum(value:class_):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 10);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

enum2():foobarclass_ {
  const offset = this.bb!.__offset(this.bb_pos, 12);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : foobarclass_.arguments_;
}

mutate_enum2(value:foobarclass_):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 12);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

enum3():foobarAbc {
  const offset = this.bb!.__offset(this.bb_pos, 14);
  return offset ? this.bb!.readInt32(this.bb_pos + offset) : foobarAbc.a;
}

mutate_enum3(value:foobarAbc):boolean {
  const offset = this.bb!.__offset(this.bb_pos, 14);

  if (offset === 0) {
    return false;
  }

  this.bb!.writeInt32(this.bb_pos + offset, value);
  return true;
}

reflect(obj?:reflectionSchema):reflectionSchema|null {
  const offset = this.bb!.__offset(this.bb_pos, 16);
  return offset ? (obj || new reflectionSchema()).__init(this.bb!.__indirect(this.bb_pos + offset), this.bb!) : null;
}

static getFullyQualifiedName():string {
  return 'typescript.Object';
}

static startObject(builder:flatbuffers.Builder) {
  builder.startObject(7);
}

static addReturn(builder:flatbuffers.Builder, return_:number) {
  builder.addFieldInt32(0, return_, 0);
}

static addIf(builder:flatbuffers.Builder, if_:number) {
  builder.addFieldInt32(1, if_, 0);
}

static addSwitch(builder:flatbuffers.Builder, switch_:number) {
  builder.addFieldInt32(2, switch_, 0);
}

static addEnum(builder:flatbuffers.Builder, enum_:class_) {
  builder.addFieldInt32(3, enum_, class_.new_);
}

static addEnum2(builder:flatbuffers.Builder, enum2:foobarclass_) {
  builder.addFieldInt32(4, enum2, foobarclass_.arguments_);
}

static addEnum3(builder:flatbuffers.Builder, enum3:foobarAbc) {
  builder.addFieldInt32(5, enum3, foobarAbc.a);
}

static addReflect(builder:flatbuffers.Builder, reflectOffset:flatbuffers.Offset) {
  builder.addFieldOffset(6, reflectOffset, 0);
}

static endObject(builder:flatbuffers.Builder):flatbuffers.Offset {
  const offset = builder.endObject();
  return offset;
}


unpack(): ObjectT {
  return new ObjectT(
    this.return(),
    this.if(),
    this.switch(),
    this.enum(),
    this.enum2(),
    this.enum3(),
    (this.reflect() !== null ? this.reflect()!.unpack() : null)
  );
}


unpackTo(_o: ObjectT): void {
  _o.return_ = this.return();
  _o.if_ = this.if();
  _o.switch_ = this.switch();
  _o.enum_ = this.enum();
  _o.enum2 = this.enum2();
  _o.enum3 = this.enum3();
  _o.reflect = (this.reflect() !== null ? this.reflect()!.unpack() : null);
}
}

export class ObjectT {
constructor(
  public return_: number = 0,
  public if_: number = 0,
  public switch_: number = 0,
  public enum_: class_ = class_.new_,
  public enum2: foobarclass_ = foobarclass_.arguments_,
  public enum3: foobarAbc = foobarAbc.a,
  public reflect: reflectionSchemaT|null = null
){}


pack(builder:flatbuffers.Builder): flatbuffers.Offset {
  const reflect = (this.reflect !== null ? this.reflect!.pack(builder) : 0);

  Object_.startObject(builder);
  Object_.addReturn(builder, this.return_);
  Object_.addIf(builder, this.if_);
  Object_.addSwitch(builder, this.switch_);
  Object_.addEnum(builder, this.enum_);
  Object_.addEnum2(builder, this.enum2);
  Object_.addEnum3(builder, this.enum3);
  Object_.addReflect(builder, reflect);

  return Object_.endObject(builder);
}
}
