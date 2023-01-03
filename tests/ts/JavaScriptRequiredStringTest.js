import assert from 'assert'
import * as flatbuffers from 'flatbuffers';
// import * as flatbuffers from '~/src/mediasoup/node_modules/flatbuffers/js'
import { Foo } from './required-strings/foo.js';


var builder = new flatbuffers.Builder();

function main() {
  testMissingRequiredStringA();
  builder.clear();
  testMissingRequiredStringB();
}

function testMissingRequiredStringA() {
	const strA = builder.createString(undefined);
	const strB = builder.createString('B');

	assert.throws(() => Foo.createFoo(
		builder, strA, strB
	));
}

function testMissingRequiredStringB() {
	const strA = builder.createString('A');
	const strB = builder.createString(undefined);

	assert.throws(() => Foo.createFoo(
		builder, strA, strB
	));
}

main();
