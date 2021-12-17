npm install
../flatc.exe --ts --gen-name-strings --gen-mutable --gen-object-api -I include_test monster_test.fbs
../flatc.exe --gen-object-api -b -I include_test monster_test.fbs unicode_test.json
../flatc.exe --ts --gen-name-strings --gen-mutable --gen-object-api -o union_vector union_vector/union_vector.fbs
../flatc.exe --ts --gen-name-strings optional_scalars.fbs
tsc
node -r esm JavaScriptTest
node -r esm JavaScriptUnionVectorTest
node -r esm JavaScriptFlexBuffersTest
