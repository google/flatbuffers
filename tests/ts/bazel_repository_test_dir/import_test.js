import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import two_cjs from './two.cjs'

const bazel_repository_test = two_cjs.bazel_repository_test;

function main() {
  // Validate building a table.
  var fbb = new flatbuffers.Builder(1);

  bazel_repository_test.Two.startTwo(fbb);
  var two = bazel_repository_test.Two.endTwo(fbb);

  fbb.finish(two);

  console.log('FlatBuffers bazel repository test: completed successfully');
}

main();
