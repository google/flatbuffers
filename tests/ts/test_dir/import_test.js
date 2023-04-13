import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import fsoobar from './typescript_include_generated.cjs'

function main() {
  console.log(fsoobar);
  console.log(foobar.foobar);
  console.log(Object.keys(foobar));
  console.log(foobar.Abc);
  console.log(Object.keys(foobar.Abc));
  assert.strictEqual(foobar.Abc.a, 234);

  console.log('FlatBuffers Import test: completed successfully');
}

main();
