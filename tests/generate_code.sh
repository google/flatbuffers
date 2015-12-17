../flatc --cpp --java --csharp --go --binary --python --js --php --gen-mutable --no-includes monster_test.fbs monsterdata_test.json
../flatc --binary --schema monster_test.fbs
../flatc --binary monster_test.fbs monsterdata_indirect.json