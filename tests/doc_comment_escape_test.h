#ifndef THIRDPARTY_FLATBUFFERS_TESTS_DOC_COMMENT_ESCAPE_TEST_H_
#define THIRDPARTY_FLATBUFFERS_TESTS_DOC_COMMENT_ESCAPE_TEST_H_

namespace flatbuffers {
namespace tests {

// Verifies that the block-comment doc emitters (Java, Kotlin, Kotlin
// multiplatform) neutralize the block-comment terminator inside user doc
// text, so doc text cannot end a generated doc-comment block early and leave
// the rest of the doc as bare tokens. Also locks the TypeScript generator's
// existing neutralization (the model this follows) and checks that doc text
// without the terminator passes through unchanged.
void DocCommentEscapeTest();

}  // namespace tests
}  // namespace flatbuffers

#endif  // THIRDPARTY_FLATBUFFERS_TESTS_DOC_COMMENT_ESCAPE_TEST_H_
