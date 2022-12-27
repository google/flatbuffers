#include "keyword_test.h"

#include "test_assert.h"

namespace flatbuffers {
namespace tests {

using namespace KeywordTest;

void check_keyword() {
  flatbuffers::FlatBufferBuilder builder;

  flatbuffers::Offset<KeywordsInTable> arr[2];

  KeywordsInTableBuilder keyword_builder_1(builder);
  keyword_builder_1.add_is(ABC_where);
  arr[0] = keyword_builder_1.Finish();

  static constexpr int32_t MAGIC = 43;
  KeywordsInTableBuilder keyword_builder_2(builder);
  keyword_builder_2.add_type(MAGIC);
  arr[1] = keyword_builder_2.Finish();

  builder.Finish(CreateTable2(builder, KeywordsInUnion_static_, arr[0].Union(),
                              KeywordsInUnion_internal, arr[1].Union()));
}

}  // namespace tests
}  // namespace flatbuffers
