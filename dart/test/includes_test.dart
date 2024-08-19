import 'package:test/test.dart';

import './includes_c_a_generated.dart';

void main() {
  group("Recursive import test", () {
    test("Test importing a fbs file that imports another fbs file", () {
      CT();
    });
  });
}
