#include <memory>
#include <string>
#include <vector>

#include "flatbuffers/flatbuffers.h"
#include "monster_test_generated.h"

int main() {
  flatbuffers::FlatBufferBuilder builder;

  auto stat_id = builder.CreateString("Sword");
  auto stat = MyGame::Example::CreateStat(builder, stat_id, 10, 0);
  auto abilities = builder.CreateVector(std::vector<uint8_t>{1, 2, 3, 4});
  auto string_vector =
      builder.CreateVectorOfStrings(std::vector<std::string>{"Alpha", "Beta"});
  auto longs_vector = builder.CreateVector(std::vector<int64_t>{11, 22, 33});
  auto simple_table = MyGame::Example::CreateTestSimpleTableWithEnum(
      builder, MyGame::Example::Color_Red);

  auto monster_name = builder.CreateString("PreserveCase");
  MyGame::Example::MonsterBuilder monster_builder(builder);
  monster_builder.add_mana(150);
  monster_builder.add_hp(80);
  monster_builder.add_name(monster_name);
  monster_builder.add_testempty(stat);
  monster_builder.add_testbool(true);
  monster_builder.add_inventory(abilities);
  monster_builder.add_testarrayofstring(string_vector);
  monster_builder.add_vector_of_longs(longs_vector);
  monster_builder.add_test_type(MyGame::Example::Any_TestSimpleTableWithEnum);
  monster_builder.add_test(simple_table.Union());
  auto monster = monster_builder.Finish();
  builder.Finish(monster, MyGame::Example::MonsterIdentifier());

  auto buffer = builder.GetBufferPointer();
  flatbuffers::Verifier verifier(buffer, builder.GetSize());
  if (!MyGame::Example::VerifyMonsterBuffer(verifier)) {
    return 1;
  }

  auto root = MyGame::Example::GetMonster(buffer);
  if (root == nullptr) return 2;
  if (root->mana() != 150) return 3;
  if (root->hp() != 80) return 4;
  if (root->name() == nullptr) return 5;
  if (std::string(root->name()->c_str()) != "PreserveCase") return 6;
  if (!MyGame::Example::MonsterBufferHasIdentifier(buffer)) return 7;
  auto collected_stat = root->testempty();
  if (collected_stat == nullptr) return 8;
  if (std::string(collected_stat->id()->c_str()) != "Sword") return 9;
  if (!root->testbool()) return 10;
  auto inventory = root->inventory();
  if (inventory == nullptr || inventory->size() != 4) return 11;
  if (inventory->Get(0) != 1 || inventory->Get(3) != 4) return 12;
  auto strings = root->testarrayofstring();
  if (strings == nullptr || strings->size() != 2) return 13;
  if (strings->Get(0)->str() != "Alpha") return 14;
  auto longs = root->vector_of_longs();
  if (longs == nullptr || longs->size() != 3) return 15;
  if (longs->Get(2) != 33) return 16;
  if (root->test_type() != MyGame::Example::Any_TestSimpleTableWithEnum)
    return 17;
  auto union_table = root->test_as_TestSimpleTableWithEnum();
  if (union_table == nullptr ||
      union_table->color() != MyGame::Example::Color_Red)
    return 18;

  std::unique_ptr<MyGame::Example::MonsterT> unpacked(root->UnPack());
  if (unpacked->name != "PreserveCase") return 19;
  if (unpacked->mana != 150) return 20;
  if (unpacked->testempty == nullptr) return 21;
  if (unpacked->testempty->id != "Sword") return 22;
  if (!unpacked->testbool) return 23;
  if (unpacked->inventory.size() != 4 || unpacked->inventory[1] != 2) return 24;
  if (unpacked->testarrayofstring.size() != 2 ||
      unpacked->testarrayofstring[1] != "Beta")
    return 25;
  if (unpacked->vector_of_longs.size() != 3 ||
      unpacked->vector_of_longs[0] != 11)
    return 26;
  if (unpacked->test.type != MyGame::Example::Any_TestSimpleTableWithEnum)
    return 27;
  if (unpacked->test.AsTestSimpleTableWithEnum()->color !=
      MyGame::Example::Color_Red)
    return 28;

  flatbuffers::FlatBufferBuilder roundtrip;
  roundtrip.Finish(MyGame::Example::Monster::Pack(roundtrip, unpacked.get()),
                   MyGame::Example::MonsterIdentifier());
  if (!MyGame::Example::MonsterBufferHasIdentifier(
          roundtrip.GetBufferPointer()))
    return 29;

  return 0;
}
