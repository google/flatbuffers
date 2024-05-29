// This test has nothing to do with flatbuffers. It only exists to validate
// that other projects can use their own set of dependencies without having to
// explicitly pull in flatbuffers's dependencies.
//
// We pick lodash here not for any particular reason. It could be any package,
// really. I chose it because it's a relatively simple package.

import assert from 'node:assert/strict'

import _ from 'lodash'

function main() {
  console.log(_);
  assert.deepStrictEqual(_.defaults({ 'a': 1 }, { 'a': 3, 'b': 2 }), { 'a': 1, 'b': 2 });
  assert.deepStrictEqual(_.partition([1, 2, 3, 4], n => n % 2), [[1, 3], [2, 4]]);
}

main();
