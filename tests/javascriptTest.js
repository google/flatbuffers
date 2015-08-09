/*
 * Copyright 2015 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 /*
  * A small test suite to compare the javascript reading routines to
  * those of the C#/Java routines.
  *
  * This will oad the canonical buffer (monster_test.mon) and validate it
  * Afterwards, it creates a buffer with the javascript flatbuffer builder
  * and compares the results
  *
  * To run, make sure you have installed the package locally via NPM:
  *		npm install .\..\js\flatbuffers
  */
 
var fs = require('fs');
var flatBuffers = require('flatbuffers');
var example = require('./example');

/*
 * Assert helpers for the tests
 */
var assert = 
{
	equals: function(expected, actual) {
		if (expected === actual) {
			return;
		}	
		throw new Error('actual value differed to expected');
	},
	notNull: function(obj) {
		if (typeof obj === 'undefined' || obj === null) {
			throw new Error('value was null');
		}
	},
	isNull: function(obj) {
		if (typeof obj === 'undefined' || obj === null) {
			return;
		}
		throw new Error('value was not null');
	},
	isTrue: function(expr) {
		if (expr === true) {
			return;
		}
		throw new Error('false');
	}
};

/*
 * Tests to verify that the reader can process a flatbuffer from
 * the given buffer
 */
var checkReadBuffer = function(buffer, o) {
	var fb = new flatBuffers.Reader(buffer);
	var monster = example.monster.getRootAsMonster(fb, o|0);
	
	assert.equals(80, monster.getHp());
	assert.equals(150, monster.getMana());
	assert.equals('MyMonster', monster.getName());
	
	// initialize a vec3 from pos
	var vec = monster.getPos();
	assert.notNull(vec);
	assert.equals(1.0, vec.getX());
	assert.equals(2.0, vec.getY());
	assert.equals(3.0, vec.getZ());
	assert.equals(3.0, vec.getTest1());
	assert.equals(example.color.Color.Green, vec.getTest2());
	
	// initialize a test from test3
	var t = new example.test.Test();
	t = vec.getTest3(t);
	assert.notNull(t);
	assert.equals(5, t.getA());
	assert.equals(6, t.getB());
	
	// verify that the enum code matches the enum declaration
	var unionType = example.any.Any;
	assert.equals(unionType.Monster, monster.getTestType());
	
	// initialize a table from union field test
	var table2 = monster.getTest();
	assert.isTrue(table2 instanceof flatBuffers.Table);
	
	// initialize a Monster from the Table from the union
	var monster2 = new example.monster.Monster();
	monster2.init(table2.getBuffer(), table2.getPos());
	
	assert.equals('Fred', monster2.getName());
	
	// iterate through the first monster's inventory
	assert.equals(5, monster.getInventoryLength());
	
	var invsum = 0;
	for(var i = 0; i < monster.getInventoryLength(); ++i) {
		invsum += monster.getInventory(i);
	}
	assert.equals(10, invsum);
	
	assert.equals(2, monster.getTest4Length());
	
	// create a Test object and populate it
	var test0 = monster.getTest4(0);
	assert.isTrue(test0 instanceof example.test.Test);
	
	var test1 = monster.getTest4(1);
	assert.isTrue(test1 instanceof example.test.Test);
	
	var v0 = test0.getA();
	var v1 = test0.getB();
	var v2 = test1.getA();
	var v3 = test1.getB();
	var sumTest = v0 + v1 + v2 + v3;
	assert.equals(100, sumTest);
	
	assert.equals(2, monster.getTestarrayofstringLength());
	assert.equals('test1', monster.getTestarrayofstring(0));
	assert.equals('test2', monster.getTestarrayofstring(1));
	
	assert.isNull(monster.getEnemy());
	
	assert.equals(0, monster.getTestarrayoftablesLength());
	assert.equals(0, monster.getTestnestedflatbufferLength());
	assert.isNull(monster.getTestempty());
};

/*
 * Create a buffer that roughly corresponds to that in the canonical
 * file, this is then used for validation
 */
var createBuffer = function() {
	// Create an empty buffer - this will prove it grows
	var builder = new flatBuffers.Builder(0);
	
	// Create strings
	var str = builder.createString('MyMonster');
	var test1 = builder.createString('test1');
	var test2 = builder.createString('test2');
		
	var monsterBuilder = example.monster.MonsterBuilder;
	monsterBuilder.startInventoryVector(builder, 5);
	builder.prependInt8(4);
	builder.prependInt8(3);
	builder.prependInt8(2);
	builder.prependInt8(1);
	builder.prependInt8(0);
	var inv = builder.endVector();
	
	var fred = builder.createString('Fred');
	monsterBuilder.start(builder);
	monsterBuilder.addName(builder, fred);
	var mon2 = monsterBuilder.end(builder);
	
	monsterBuilder.startTest4Vector(builder, 2);
	example.test.createTest(builder, 10, 20);
	example.test.createTest(builder, 30, 40);
	var test4 = builder.endVector();
	
	monsterBuilder.startTestarrayofstringVector(builder, 2);
	builder.prependOffset(test2);
	builder.prependOffset(test1);
	var testArrayOfString = builder.endVector();
	
	monsterBuilder.start(builder);
	var pos = example.vec3.createVec3(builder, 1.0, 2.0, 3.0, 3.0, example.color.Color.Green, 5, 6);
	monsterBuilder.addPos(builder, pos);
	
	monsterBuilder.addHp(builder, 80);
	monsterBuilder.addName(builder, str);
	monsterBuilder.addInventory(builder, inv);
	monsterBuilder.addTestType(builder, 1);
	monsterBuilder.addTest(builder, mon2);
	monsterBuilder.addTest4(builder, test4);
	monsterBuilder.addTestarrayofstring(builder, testArrayOfString);
	var mon = monsterBuilder.end(builder);
	
	assert.notNull(mon);
	
	builder.finish(mon);
	
	return {
		buffer: builder.getBuffer(),
		head: builder.getHead()
	};
};

// Load the canonical buffer from disc
var goldenBuffer = fs.readFileSync('monsterdata_test.mon');
// validate it
checkReadBuffer(goldenBuffer);

var generatedData = createBuffer();
// validate our created buffer against the canonical buffer rules
checkReadBuffer(generatedData.buffer, generatedData.head);

// write to file for later inspection
fs.writeFileSync('monsterdata_test_js.mon', generatedData.buffer);

process.exit();