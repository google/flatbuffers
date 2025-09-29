/*
 * Copyright 2024 Google Inc. All rights reserved.
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

import 'package:flat_buffers/flat_buffers.dart';
import 'package:test/test.dart';

import 'union_table_string_test_my_game.example_generated.dart';

void main() {
  test('Object-API: union of table and string (table)', () {
    final builder = Builder(initialSize: 256);

    final testTable = TestTableObjectBuilder(value: 'test');

    final root = RootTableObjectBuilder(
      testUnionType: TestUnionTypeId.test_table,
      testUnion: testTable,
    );

    builder.finish(root.finish(builder));
    final buffer = builder.buffer;

    final result = RootTable(buffer);
    expect(result.testUnionType, TestUnionTypeId.test_table);

    final union = result.testUnion as TestTable;
    expect(union.value, 'test');
  });

  test('Object-API: union of table and string (string)', () {
    final builder = Builder(initialSize: 256);

    final root = RootTableObjectBuilder(
      testUnionType: TestUnionTypeId.test_string,
      testUnion: 'test_string',
    );

    builder.finish(root.finish(builder));
    final buffer = builder.buffer;

    final result = RootTable(buffer);
    expect(result.testUnionType, TestUnionTypeId.test_string);

    final union = result.testUnion as String;
    expect(union, 'test_string');
  });
}