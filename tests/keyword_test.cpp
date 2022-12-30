#include "keyword_test.h"

#include "test_assert.h"

namespace flatbuffers {
namespace tests {

using namespace namespace_;

void check_keyword() {
  flatbuffers::FlatBufferBuilder fbb;
  auto noex =
      Createnoexcept_(fbb, namespace_::explicit__extern_,
                      namespace_::public__NONE, true, fbb.CreateString("TEST"));

  std::vector<double> vec = { 1.5, 2.5, 0 };
  auto f_vec = fbb.CreateVector(vec);

  std::vector<flatbuffers::Offset<namespace_::noexcept_>> noexcept_s;
  noexcept_s.push_back(Createnoexcept_(fbb, namespace_::explicit__extern_,
                                       namespace_::public__NONE, true,
                                       fbb.CreateString("TEST_1")));
  noexcept_s.push_back(Createnoexcept_(fbb, namespace_::explicit__extern_,
                                       namespace_::public__NONE, true,
                                       fbb.CreateString("TEST_2")));
  auto f_noex_vec = fbb.CreateVector(noexcept_s);

  auto del = Createdelete_(fbb, namespace_::throw__static_, noex.Union(),
                           f_noex_vec, 69, f_vec);

  fbb.Finish(del);
  uint8_t *buf = fbb.GetBufferPointer();

  auto del_ = Getdelete_(buf);
  TEST_EQ(del_->dynamic_cast_(), 69);
  TEST_EQ(del_->constexpr_type(), namespace_::throw__static_);
}

}  // namespace tests
}  // namespace flatbuffers
