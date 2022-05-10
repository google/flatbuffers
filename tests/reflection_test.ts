import {Builder, ByteBuffer} from 'flatbuffers';
import {Parser, Table} from 'flatbuffers_reflection/reflection'
import {BaseType, EnumVal, Schema, Type} from 'flatbuffers_reflection/reflection_generated';
import {readFileSync} from 'fs';

import {Object_ as TestObject, typeof_} from './typescript_keywords_generated';

// Set up a test where we use the schema of the Schema type itself to test
// the reflection library, because in practice the Schema type has ~all of
// the interesting features that we support reflection for.
const reflectionSchemaBuffer: Buffer =
    readFileSync('reflection/ts/reflection.bfbs');
const reflectionSchemaByteBuffer: ByteBuffer =
    new ByteBuffer(reflectionSchemaBuffer);
const schema = Schema.getRootAsSchema(reflectionSchemaByteBuffer);
const parser = new Parser(schema);
const table = Table.getRootTable(reflectionSchemaByteBuffer);
const schemaObject = parser.toObject(table);
function assert(condition: any, msg?: string): asserts condition {
  if (!condition) {
    throw new Error(msg);
  }
}
function assertEqual(a: any, b: any, msg?: string) {
  if (a !== b) {
    throw new Error(a + ' !== ' + b + ': ' + msg);
  }
}
function stringify(obj: any): string {
  return JSON.stringify(
      obj,
      (key, value) => typeof value === 'bigint' ? value.toString() : value);
}
// Spot-check some individual features of the reflection schema. This covers
// testing that we can read vectors of tables.
assertEqual(schemaObject['objects'].length, schema.objectsLength());
assertEqual(schemaObject['objects'].length, 10);
assertEqual(schemaObject['objects'][0]['name'], 'reflection.Enum');
assertEqual(schemaObject['file_ident'], 'BFBS');
assertEqual(schemaObject['file_ext'], 'bfbs');
assertEqual(
    schemaObject['fbs_files'][0]['filename'].substr(-14), 'reflection.fbs');
assertEqual(schemaObject['fbs_files'][0]['included_filenames'].length, 0);


// Test constructing a small object that specifically lets us exercise some edge
// cases that the actual schema doesn't.
// Covered reflection features:
// * Reading numbers & BigInt's
// * Vectors of strings
// * Table fields
// * Default scalars
{
  const builder = new Builder();
  const docOffsets = [builder.createString('abc'), builder.createString('def')];
  const docVector = EnumVal.createDocumentationVector(builder, docOffsets);
  const nameOffset = builder.createString('name');
  Type.startType(builder);
  Type.addBaseType(builder, BaseType.Int);
  Type.addIndex(builder, 123);
  const typeOffset = Type.endType(builder);
  EnumVal.startEnumVal(builder);
  EnumVal.addName(builder, nameOffset);
  // Make sure that we are testing an integer that will exceed the normal
  // precision bounds.
  EnumVal.addValue(builder, BigInt(Number.MAX_SAFE_INTEGER) + BigInt(1));
  EnumVal.addDocumentation(builder, docVector);
  EnumVal.addUnionType(builder, typeOffset);
  builder.finish(EnumVal.endEnumVal(builder));
  const array = builder.asUint8Array();
  const fbBuffer = new ByteBuffer(array);

  const reflectionFb =
      Table.getNamedTable(fbBuffer, schema, 'reflection.EnumVal');
  assertEqual(
      '{"documentation":["abc","def"],"name":"name","union_type":{"base_type":' +
          BaseType.Int + ',"index":123},"value":"9007199254740992"}',
      stringify(parser.toObject(reflectionFb)));
  const typeFb = parser.readTable(reflectionFb, 'union_type');
  if (typeFb === null) {
    throw new Error();
  }
  // Confirm that readDefaults works.
  assertEqual(BigInt(4), parser.readScalar(typeFb, 'base_size', true));
  assertEqual(null, parser.readScalar(typeFb, 'base_size', false));
}

// Finally, to cover some things not covered by the reflection schema we use
// the typescript_keywords things. We can't actually use monster_test.fbs
// because it attempts to do a circular include, which the typescript flat-file
// codegen doesn't support.
// This covers:
// * Structs
// * Vectors of structs
// * Vectors of numbers
{
  const builder = new Builder();

  TestObject.startStructuresVector(builder, 2);
  // Note: because the builder builds top-down, we write these in reverse order.
  typeof_.createtypeof(
      builder, 3.125, 2, BigInt(Number.MAX_SAFE_INTEGER) + BigInt(2));
  typeof_.createtypeof(
      builder, 3.125, 2, BigInt(Number.MAX_SAFE_INTEGER) + BigInt(1));
  const structVector = builder.endVector();

  const constVector =
      TestObject.createConstVector(builder, [BigInt(1), BigInt(2), BigInt(3)]);

  TestObject.startObject(builder);
  TestObject.addStructure(
      builder, typeof_.createtypeof(builder, 1.0625, 3, BigInt(3)));
  TestObject.addStructures(builder, structVector);
  TestObject.addConst(builder, constVector);

  builder.finish(TestObject.endObject(builder));

  const array = builder.asUint8Array();
  const fbBuffer = new ByteBuffer(array);

  const schemaBuffer: ByteBuffer =
      new ByteBuffer(readFileSync('tests/typescript_keywords.bfbs'));
  const schema = Schema.getRootAsSchema(schemaBuffer);
  const reflectionFb =
      Table.getNamedTable(fbBuffer, schema, 'typescript.Object');
  const parser = new Parser(schema);
  assertEqual(
      '{"const":["1","2","3"],"structure":{"x":1.0625,"y":3,"z":"3"},"structures":[{"x":3.125,"y":2,"z":"9007199254740992"},{"x":3.125,"y":2,"z":"9007199254740993"}]}',
      stringify(parser.toObject(reflectionFb)));
}
