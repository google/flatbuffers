#ifndef THIRDPARTY_FLATBUFFERS_TESTS_FILE_IDENTIFIER_ESCAPE_TEST_H_
#define THIRDPARTY_FLATBUFFERS_TESTS_FILE_IDENTIFIER_ESCAPE_TEST_H_

namespace flatbuffers {
namespace tests {

// Verifies that every language generator embeds parser_.file_identifier_
// into generated string literals as a well-formed, faithfully escaped
// literal -- regardless of which bytes the identifier carries (the .fbs text
// parser restricts the length to kFileIdentifierLength but not the character
// set, and the .bfbs reader restricts neither).
void FileIdentifierEscapeTest();

// Verifies the same canonicalization on the .bfbs (binary schema) input
// route, where file_ident reaches the generators through
// Parser::Deserialize().
void FileIdentifierBfbsEscapeTest();

// Verifies that Parser::Deserialize() rejects a binary schema whose
// file_ident is not exactly kFileIdentifierLength bytes (the rule the .fbs
// parser already enforces; a FlatBuffer file identifier is 4 bytes wide).
void FileIdentifierBfbsLengthTest();

}  // namespace tests
}  // namespace flatbuffers

#endif  // THIRDPARTY_FLATBUFFERS_TESTS_FILE_IDENTIFIER_ESCAPE_TEST_H_
