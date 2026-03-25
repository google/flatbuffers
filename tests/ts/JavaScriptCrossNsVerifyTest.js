// Test that cross-namespace verify imports use proper aliasing.
// When two namespaces define a table with the same name (Mapping), the
// verifier import must use "verifyMapping as verifyInner_Mapping" so
// the imported symbol matches the actual export name.
import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import { Container, ContainerT, verifyRootAsContainer } from './cross_ns_verify/outer/container.js'
import { Mapping, MappingT } from './cross_ns_verify/outer/mapping.js'
import { Mapping as Inner_Mapping, MappingT as Inner_MappingT } from './cross_ns_verify/inner/mapping.js'

function main() {
  testCrossNsRoundTrip();
  testCrossNsCloneEquals();
  testCrossNsVerify();

  console.log('Cross-namespace verify test: completed successfully');
}

function testCrossNsRoundTrip() {
  var fbb = new flatbuffers.Builder(256);

  // Build inner mapping
  var innerKey = fbb.createString('inner_key');
  Inner_Mapping.startMapping(fbb);
  Inner_Mapping.addKey(fbb, innerKey);
  Inner_Mapping.addValue(fbb, 42);
  var innerOffset = Inner_Mapping.endMapping(fbb);

  // Build outer mapping
  var outerName = fbb.createString('outer_name');
  Mapping.startMapping(fbb);
  Mapping.addName(fbb, outerName);
  var outerOffset = Mapping.endMapping(fbb);

  // Build container
  Container.startContainer(fbb);
  Container.addLocalMapping(fbb, outerOffset);
  Container.addInnerMapping(fbb, innerOffset);
  Container.finishContainerBuffer(fbb, Container.endContainer(fbb));

  // Read back
  var container = Container.getRootAsContainer(fbb.dataBuffer());
  assert.strictEqual(container.localMapping().name(), 'outer_name');
  assert.strictEqual(container.innerMapping().key(), 'inner_key');
  assert.strictEqual(container.innerMapping().value(), 42);
}

function testCrossNsCloneEquals() {
  // Test Object API with cross-namespace types
  var ct = new ContainerT();
  ct.localMapping = new MappingT('local');
  ct.innerMapping = new Inner_MappingT('key1', 100);

  var cloned = ct.clone();
  assert.ok(ct.equals(cloned));

  // Verify the inner mapping was properly cloned
  assert.ok(cloned.innerMapping instanceof Inner_MappingT);
  assert.notStrictEqual(cloned.innerMapping, ct.innerMapping);
}

function testCrossNsVerify() {
  // The actual regression test is the static import of verifyRootAsContainer
  // at the top of this file. If the import aliasing was broken (the generated
  // code emitting verifyCga_Mapping instead of verifyMapping as
  // verifyCga_Mapping), the esbuild bundle would fail or the function
  // would be undefined at runtime.
  assert.strictEqual(typeof verifyRootAsContainer, 'function',
    'verifyRootAsContainer should be importable (cross-ns verify alias works)');
}

main();
