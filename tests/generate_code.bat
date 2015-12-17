..\flatc.exe -c -j -n -g -b -p --php -s --gen-mutable --no-includes monster_test.fbs monsterdata_test.json
..\flatc.exe -b --schema monster_test.fbs
..\flatc.exe -b .\monster_test.fbs .\monsterdata_indirect.json