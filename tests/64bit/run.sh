./flatc -b -o tests/64bit tests/64bit/test_64bit.fbs tests/64bit/test_64bit.json

./flatc -o tests/64bit --annotate tests/64bit/test_64bit.fbs -- tests/64bit/test_64bit.bin
