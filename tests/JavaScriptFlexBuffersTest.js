// Run this using JavaScriptTest.sh
var assert = require('assert');
var fs = require('fs');

var flexbuffers = require('../js/flexbuffers').flexbuffers;
global.flexbuffers = flexbuffers;

function main() {
  testSingleValueBuffers();
  testGoldBuffer();
}

function testSingleValueBuffers() {
  { // null
    const ref = flexbuffers.read(new Uint8Array([0, 0, 1]).buffer);
    assert.strictEqual(true, ref.isNull());
  }
  { // bool
    const ref = flexbuffers.read(new Uint8Array([1, 104, 1]).buffer);
    assert.strictEqual(ref.toObject(), true);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([0, 104, 1]).buffer);
    assert.strictEqual(ref.toObject(), false);
  }
  { // int and uint
    const ref = flexbuffers.read(new Uint8Array([25, 4, 1]).buffer);
    assert.strictEqual(ref.toObject(), 25);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([231, 4, 1]).buffer);
    assert.strictEqual(ref.toObject(), -25);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([230, 8, 1]).buffer);
    assert.strictEqual(ref.toObject(), 230);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([230, 0, 5, 2]).buffer);
    assert.strictEqual(ref.toObject(), 230);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([1, 4, 5, 2]).buffer);
    assert.strictEqual(ref.toObject(), 1025);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([255, 251, 5, 2]).buffer);
    assert.strictEqual(ref.toObject(), -1025);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([1, 4, 9, 2]).buffer);
    assert.strictEqual(ref.toObject(), 1025);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([255, 255, 255, 127, 6, 4]).buffer);
    assert.strictEqual(ref.toObject(), 2147483647);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([0, 0, 0, 128, 6, 4]).buffer);
    assert.strictEqual(ref.toObject(), -2147483648);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([255, 255, 255, 255, 0, 0, 0, 0, 7, 8]).buffer);
    assert.strictEqual(ref.toObject(), 4294967295n);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([255, 255, 255, 255, 255, 255, 255, 127, 7, 8]).buffer);
    assert.strictEqual(ref.toObject(), 9223372036854775807n);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([0, 0, 0, 0, 0, 0, 0, 128, 7, 8]).buffer);
    assert.strictEqual(ref.toObject(), -9223372036854775808n);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([255, 255, 255, 255, 255, 255, 255, 255, 11, 8]).buffer);
    assert.strictEqual(ref.toObject(), 18446744073709551615n);
  }
  { // float
    const ref = flexbuffers.read(new Uint8Array([0, 0, 144, 64, 14, 4]).buffer);
    assert.strictEqual(ref.toObject(), 4.5);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([205, 204, 204, 61, 14, 4]).buffer);
    assert.deepStrictEqual(ref.toObject(), 0.10000000149011612);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([154, 153, 153, 153, 153, 153, 185, 63, 15, 8]).buffer);
    assert.deepStrictEqual(ref.toObject(), 0.1);
  }
  {
    const ref = flexbuffers.read(new Uint8Array([255, 251, 5, 2]).buffer);
    assert.deepStrictEqual(ref.toObject(), -1025);
  }
  { // string
    const ref = flexbuffers.read(new Uint8Array([5, 77, 97, 120, 105, 109, 0, 6, 20, 1]).buffer);
    assert.deepStrictEqual(ref.toObject(), "Maxim");
  }
  {
    const ref = flexbuffers.read(new Uint8Array([10, 104, 101, 108, 108, 111, 32, 240, 159, 152, 177, 0, 11, 20, 1]).buffer);
    assert.deepStrictEqual(ref.toObject(), "hello 😱");
  }
}

function testGoldBuffer() {
  const data = new Uint8Array(fs.readFileSync('gold_flexbuffer_example.bin')).buffer;
  const b1 = flexbuffers.read(data).get("bools").get(1);
  assert.strictEqual(b1.isBool(), true);
  assert.strictEqual(b1.boolValue(), false);

  const blob = flexbuffers.read(data).get("vec").get(3);
  assert.strictEqual(blob.isBlob(), true);
  assert.deepStrictEqual(blob.blobValue(), new Uint8Array([77]));

  const o = flexbuffers.toObject(data);
  assert.deepStrictEqual(o, {
    bool: true,
    bools: [true, false, true, false],
    bar: [1, 2, 3],
    bar3: [1, 2, 3],
    foo: 100,
    mymap: {foo:'Fred'},
    vec: [-100, 'Fred', 4, new Uint8Array([77]), false, 4]
  });
}

main();

