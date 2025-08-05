import assert from 'assert'
import * as flatbuffers from 'flatbuffers'
import * as Test from './UnionUnderlyingType.js'

function main() {
    let a = new Test.AT();
    a.a = 1;
    let b = new Test.BT();
    b.b = "foo";
    let c = new Test.CT();
    c.c = true;
    let d = new Test.DT();
    d.test_union_type = Test.ABC.A;
    d.test_union = a;
    d.test_vector_of_union_type = [Test.ABC.A, Test.ABC.B, Test.ABC.C];
    d.test_vector_of_union = [a, b, c];

    let fbb = new flatbuffers.Builder();
    let offset = d.pack(fbb);
    fbb.finish(offset);

    let unpacked = Test.D.getRootAsD(fbb.dataBuffer()).unpack();
    assert.equal(JSON.stringify(unpacked), JSON.stringify(d));
}

main()