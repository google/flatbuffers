import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import optional_scalars from './ts-undefined-for-optionals/optional_scalars_generated.cjs'

const { ScalarStuff, ScalarStuffT } = optional_scalars.optional_scalars;

function testScalarStuffBuf(scalarStuff) {
  assert.strictEqual(scalarStuff.justI8(), -1);
  assert.strictEqual(scalarStuff.maybeI8(), undefined);
  assert.strictEqual(scalarStuff.defaultI8(), 42);
  assert.strictEqual(scalarStuff.justU8(), 1);
  assert.strictEqual(scalarStuff.maybeU8(), undefined);
  assert.strictEqual(scalarStuff.defaultU8(), 42);
}

function testScalarStuffUnpack(scalarStuff) {
  assert.strictEqual(scalarStuff.justI8, -1);
  assert.strictEqual(scalarStuff.maybeI8, undefined);
  assert.strictEqual(scalarStuff.defaultI8, 42);
  assert.strictEqual(scalarStuff.justU8, 1);
  assert.strictEqual(scalarStuff.maybeU8, undefined);
  assert.strictEqual(scalarStuff.defaultU8, 42);
}

function createScalarStuff(fbb) {
  ScalarStuff.startScalarStuff(fbb);
  ScalarStuff.addJustI8(fbb, -1);
  ScalarStuff.addJustU8(fbb, 1);
  var offset = ScalarStuff.endScalarStuff(fbb);
  ScalarStuff.finishScalarStuffBuffer(fbb, offset);
}

function main() {
  var fbb = new flatbuffers.Builder();

  createScalarStuff(fbb);

  var buf = new flatbuffers.ByteBuffer(fbb.asUint8Array());
  var scalarStuff = ScalarStuff.getRootAsScalarStuff(buf);

  testScalarStuffBuf(scalarStuff);

  testScalarStuffUnpack(scalarStuff.unpack());

  var scalarStuff_to = new ScalarStuffT();
  scalarStuff.unpackTo(scalarStuff_to);

  testScalarStuffUnpack(scalarStuff_to);

  fbb.clear();
  ScalarStuff.finishScalarStuffBuffer(fbb, scalarStuff_to.pack(fbb));
  var unpackBuf = new flatbuffers.ByteBuffer(fbb.asUint8Array());

  testScalarStuffBuf(ScalarStuff.getRootAsScalarStuff(unpackBuf));

  console.log('FlatBuffers --ts-undefined-for-optionals test: completed successfully');
}

main();
