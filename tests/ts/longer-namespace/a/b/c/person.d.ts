import * as flatbuffers from 'flatbuffers';
export declare class Person {
  bb: flatbuffers.ByteBuffer | null;
  bb_pos: number;
  __init(i: number, bb: flatbuffers.ByteBuffer): Person;
  static getRootAsPerson(bb: flatbuffers.ByteBuffer, obj?: Person): Person;
  static getSizePrefixedRootAsPerson(
    bb: flatbuffers.ByteBuffer,
    obj?: Person,
  ): Person;
  name(): string | null;
  name(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
  age(): number;
  static startPerson(builder: flatbuffers.Builder): void;
  static addName(
    builder: flatbuffers.Builder,
    nameOffset: flatbuffers.Offset,
  ): void;
  static addAge(builder: flatbuffers.Builder, age: number): void;
  static endPerson(builder: flatbuffers.Builder): flatbuffers.Offset;
  static finishPersonBuffer(
    builder: flatbuffers.Builder,
    offset: flatbuffers.Offset,
  ): void;
  static finishSizePrefixedPersonBuffer(
    builder: flatbuffers.Builder,
    offset: flatbuffers.Offset,
  ): void;
  static createPerson(
    builder: flatbuffers.Builder,
    nameOffset: flatbuffers.Offset,
    age: number,
  ): flatbuffers.Offset;
}
