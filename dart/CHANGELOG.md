## 2.0.0

- switch to null safety (#6696)
- add Object APIs (pack/unpack) (#6682, #6723)
- add custom builder buffer allocator support (#6711)
- add Builder.size() - finished buffer size (#6403)
- make `writeString()` argument non-nullable (#6737)
- make tables fixed size (expect the number of fields when creating) (#6735)
- make table deduplication optional (param `deduplicateTables`) (#6734)
- change Builder.reset() to reuse an existing buffer (#6661)
- change table building to assert() instead of exceptions (#6754)
- optimize `writeString()` for ASCII (param `asciiOptimization`) (#6736)
- change `StringReader` to make ASCII optimization optional (param `asciiOptimization`) (#6758) 
- rename `lowFinish()` to `buffer` getter (#6712)
- fix Builder._writeString() - always write trailing zero byte (#6390)
- fix Builder.reset() - clear vTables (#6386)
- make sure added padding is zeroed, same as in C++ (#6716)
- many performance improvements (#6755)

## 1.9.2

- Ensure `_writeString` adds enough padding to null terminate strings.

## 1.9.1

- Changed constant identifiers to be compatible with Dart 2.x
- No longer supports Dart 1.x

## 1.9.0

- Initial release, supports Dart 1.x and many dev versions of Dart 2.x