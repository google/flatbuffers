import './enum_names_generated.dart';

import 'package:test/test.dart';

void main() {
  test("Generated code is valid", () {
    EnumWithReservedNames.$value;
  });
}
