import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import {UnionUnderlyingType as Test} from './union_underlying_type_test.js'
import {
  unionListToAbc,
  unionToAbc,
} from './union-underlying-type/abc.js'

function testUnionToAbc() {
  const a = new Test.AT();
  a.a = 1;

  const b = new Test.BT();
  b.b = 'foo';

  const c = new Test.CT();
  c.c = true;

  assert.equal(
    unionToAbc(Test.ABC.NONE, () => null),
    null,
  );

  const convertedA = unionToAbc(
    Test.ABC.A,
    (obj) => {
      assert.ok(obj instanceof Test.A);
      return obj;
    },
  );

  assert.ok(convertedA instanceof Test.A);
  assert.equal(convertedA.a, 1);

  const convertedB = unionToAbc(
    Test.ABC.B,
    (obj) => {
      assert.ok(obj instanceof Test.B);
      return obj;
    },
  );

  assert.ok(convertedB instanceof Test.B);
  assert.equal(convertedB.b, 'foo');

  const convertedC = unionToAbc(
    Test.ABC.C,
    (obj) => {
      assert.ok(obj instanceof Test.C);
      return obj;
    },
  );

  assert.ok(convertedC instanceof Test.C);
  assert.equal(convertedC.c, true);

  assert.equal(
    unionToAbc(999, () => null),
    null,
  );
}

function testUnionListToAbc() {
  const values = [
    {
      type: Test.ABC.A,
      object: new Test.AT(),
    },
    {
      type: Test.ABC.B,
      object: new Test.BT(),
    },
    {
      type: Test.ABC.C,
      object: new Test.CT(),
    },
  ];

  values[0].object.a = 10;
  values[1].object.b = 'bar';
  values[2].object.c = false;

  for (let index = 0; index < values.length; index++) {
    const value = values[index];

    const converted = unionListToAbc(
      value.type,
      (accessorIndex, obj) => {
        assert.equal(accessorIndex, index);
        return obj;
      },
      index,
    );

    assert.ok(converted !== null);
  }

  assert.equal(
    unionListToAbc(
      Test.ABC.NONE,
      () => null,
      0,
    ),
    null,
  );

  assert.equal(
    unionListToAbc(
      999,
      () => null,
      0,
    ),
    null,
  );
}

function testObjectApiRoundTrip() {
  const a = new Test.AT();
  a.a = 1;

  const b = new Test.BT();
  b.b = 'foo';

  const c = new Test.CT();
  c.c = true;

  const d = new Test.DT();

  d.testUnionType = Test.ABC.A;
  d.testUnion = a;

  d.testVectorOfUnionType = [
    Test.ABC.A,
    Test.ABC.B,
    Test.ABC.C,
  ];

  d.testVectorOfUnion = [
    a,
    b,
    c,
  ];

  const fbb = new flatbuffers.Builder();
  const offset = d.pack(fbb);

  fbb.finish(offset);

  const unpacked = Test.D
    .getRootAsD(fbb.dataBuffer())
    .unpack();

  assert.equal(
    JSON.stringify(unpacked),
    JSON.stringify(d),
  );
}

function testNoneUnionRoundTrip() {
  const none = new Test.DT();

  none.testUnionType = Test.ABC.NONE;
  none.testUnion = null;
  none.testVectorOfUnionType = [];
  none.testVectorOfUnion = [];

  const builder = new flatbuffers.Builder();
  const offset = none.pack(builder);

  builder.finish(offset);

  const unpacked = Test.D
    .getRootAsD(builder.dataBuffer())
    .unpack();

  assert.equal(
    unpacked.testUnionType,
    Test.ABC.NONE,
  );

  assert.equal(
    unpacked.testUnion,
    null,
  );

  assert.deepEqual(
    unpacked.testVectorOfUnionType,
    [],
  );

  assert.deepEqual(
    unpacked.testVectorOfUnion,
    [],
  );
}

function main() {
  testUnionToAbc();
  testUnionListToAbc();
  testObjectApiRoundTrip();
  testNoneUnionRoundTrip();
}

main()
