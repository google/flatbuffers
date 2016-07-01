<<<<<<< HEAD
../flatc --cpp --java --csharp --go --binary --python --js --php --gen-mutable --no-includes monster_test.fbs monsterdata_test.json
../flatc --binary --schema monster_test.fbs
=======
#!/bin/bash
#
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

../flatc --cpp --java --csharp --go --binary --python --js --php --grpc --gen-mutable --no-includes monster_test.fbs monsterdata_test.json
../flatc --cpp --java --csharp --go --binary --python --js --php --gen-mutable -o namespace_test namespace_test/namespace_test1.fbs namespace_test/namespace_test2.fbs
../flatc --binary --schema monster_test.fbs

>>>>>>> 48f37f9e0a04f2b60046dda7fef20a8b0ebc1a70
