#include "TestFilegroup_generated.h"
#include "TestFilegroup2_generated.h"
#include "flatbuffers/flatbuffers.h"
#include <cassert>

int main() {
  flatbuffers::FlatBufferBuilder builder;
  
  // Test creating a message from first schema
  auto name = builder.CreateString("test_name");
  TestFilegroup::TestMessageBuilder msg_builder(builder);
  msg_builder.add_id(42);
  msg_builder.add_name(name);
  auto msg = msg_builder.Finish();
  
  builder.Finish(msg);
  
  // Verify we can read it back
  auto verify_msg = flatbuffers::GetRoot<TestFilegroup::TestMessage>(builder.GetBufferPointer());
  assert(verify_msg->id() == 42);
  assert(verify_msg->name()->str() == "test_name");
  
  // Test creating a message from second schema
  flatbuffers::FlatBufferBuilder builder2;
  TestFilegroup2::TestMessage2Builder msg_builder2(builder2);
  msg_builder2.add_id(100);
  msg_builder2.add_value(3.14);
  auto msg2 = msg_builder2.Finish();
  
  builder2.Finish(msg2);
  
  // Verify we can read it back
  auto verify_msg2 = flatbuffers::GetRoot<TestFilegroup2::TestMessage2>(builder2.GetBufferPointer());
  assert(verify_msg2->id() == 100);
  assert(verify_msg2->value() == 3.14);
  
  return 0;
}
