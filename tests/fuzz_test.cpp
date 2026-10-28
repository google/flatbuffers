#include "fuzz_test.h"

#include <algorithm>

#include "binary_annotator.h"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/reflection.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {
namespace {

// Include simple random number generator to ensure results will be the
// same cross platform.
// http://en.wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator
uint32_t lcg_seed = 48271;
uint32_t lcg_rand() {
  return lcg_seed =
             (static_cast<uint64_t>(lcg_seed) * 279470273UL) % 4294967291UL;
}
void lcg_reset() { lcg_seed = 48271; }

template <typename T>
static void CompareTableFieldValue(flatbuffers::Table* table,
                                   flatbuffers::voffset_t voffset, T val) {
  T read = table->GetField(voffset, static_cast<T>(0));
  TEST_EQ(read, val);
}

}  // namespace

namespace {

// An empty union section must not hide a later string section.
void BinaryAnnotatorEmptySectionTest() {
  flatbuffers::FlatBufferBuilder sb;
  auto empty_fields =
      sb.CreateVectorOfSortedTables<reflection::Field>(nullptr, 0);
  auto empty_struct = reflection::CreateObject(sb, sb.CreateString("EmptyS"),
                                               empty_fields, true, 1, 0);

  auto utype_t = reflection::CreateType(sb, reflection::BaseType::UType,
                                        reflection::BaseType::None, 0);
  auto string_t = reflection::CreateType(sb, reflection::BaseType::String);
  auto union_t = reflection::CreateType(sb, reflection::BaseType::Union,
                                        reflection::BaseType::None, 0);
  std::vector<flatbuffers::Offset<reflection::Field>> holder_f_vec = {
      reflection::CreateField(sb, sb.CreateString("later"), string_t, 2, 8),
      reflection::CreateField(sb, sb.CreateString("u"), union_t, 1, 6),
      reflection::CreateField(sb, sb.CreateString("u_type"), utype_t, 0, 4),
  };
  auto holder_fields = sb.CreateVectorOfSortedTables(&holder_f_vec);
  auto holder_obj = reflection::CreateObject(sb, sb.CreateString("Holder"),
                                             holder_fields, false, 4, 0);

  auto none_t = reflection::CreateType(sb, reflection::BaseType::None);
  auto empty_obj_t = reflection::CreateType(sb, reflection::BaseType::Obj,
                                            reflection::BaseType::None, 0);
  std::vector<flatbuffers::Offset<reflection::EnumVal>> ev_vec = {
      reflection::CreateEnumVal(sb, sb.CreateString("NONE"), 0, none_t),
      reflection::CreateEnumVal(sb, sb.CreateString("EmptyS"), 1, empty_obj_t),
  };
  auto evs = sb.CreateVector(ev_vec);
  auto u_enum =
      reflection::CreateEnum(sb, sb.CreateString("AnyU"), evs, true, utype_t);

  std::vector<flatbuffers::Offset<reflection::Object>> objs_vec = {empty_struct,
                                                                   holder_obj};
  auto objs = sb.CreateVectorOfSortedTables(&objs_vec);
  std::vector<flatbuffers::Offset<reflection::Enum>> enums_vec = {u_enum};
  auto enums = sb.CreateVectorOfSortedTables(&enums_vec);
  auto schema =
      reflection::CreateSchema(sb, objs, enums, sb.CreateString("BFBS"),
                               sb.CreateString("fbs"), holder_obj);
  reflection::FinishSchemaBuffer(sb, schema);

  flatbuffers::Verifier ver(sb.GetBufferPointer(), sb.GetSize());
  TEST_EQ(reflection::VerifySchemaBuffer(ver), true);

  // Visit the empty union first, then a string at a greater binary offset.
  flatbuffers::FlatBufferBuilder data;
  const auto later = data.CreateString("tail");
  data.PushElement<uint8_t>(0);
  const auto empty = flatbuffers::Offset<void>(data.GetSize());
  const auto table_start = data.StartTable();
  data.AddOffset(8, later);
  data.AddOffset(6, empty);
  data.AddElement<uint8_t>(4, 1, 0);
  data.Finish(
      flatbuffers::Offset<flatbuffers::Table>(data.EndTable(table_start)));
  const uint64_t later_offset = data.GetSize() - later.o;

  flatbuffers::BinaryAnnotator annotator(sb.GetBufferPointer(), sb.GetSize(),
                                         data.GetBufferPointer(),
                                         data.GetSize(), false);
  for (int call = 0; call < 2; ++call) {
    const auto sections = annotator.Annotate();
    const auto it = sections.find(later_offset);
    TEST_EQ(it != sections.end(), true);
    if (it == sections.end()) return;
    TEST_EQ_STR(it->second.name.c_str(), "Holder.later");
    TEST_EQ(it->second.regions.size(), static_cast<size_t>(3));
    for (const auto& region : it->second.regions) {
      TEST_EQ(region.comment.status == flatbuffers::BinaryRegionStatus::OK,
              true);
    }
    for (const auto& entry : sections) {
      TEST_EQ(entry.second.regions.empty(), false);
    }
  }
}

void BinaryAnnotatorUnionOffset64Test() {
  flatbuffers::Parser parser;
  TEST_EQ(parser.Parse("table Note { value:uint; } union Item { Note } "
                       "table Root { items:[Item]; } root_type Root;"),
          true);
  // Supply reflection metadata for a 64-bit union companion field.
  auto* type_field = parser.root_struct_def_->fields.Lookup("items_type");
  TEST_NOTNULL(type_field);
  if (type_field == nullptr) return;
  type_field->offset64 = true;
  type_field->value.type.base_type = flatbuffers::BASE_TYPE_VECTOR64;
  parser.Serialize();

  std::vector<uint8_t> bin(64, 0);
  flatbuffers::WriteScalar<uint32_t>(bin.data(), 16);
  flatbuffers::WriteScalar<uint16_t>(bin.data() + 4, 8);
  flatbuffers::WriteScalar<uint16_t>(bin.data() + 6, 16);
  flatbuffers::WriteScalar<uint16_t>(bin.data() + 8, 8);
  flatbuffers::WriteScalar<uint16_t>(bin.data() + 10, 4);
  flatbuffers::WriteScalar<int32_t>(bin.data() + 16, 12);
  flatbuffers::WriteScalar<uint32_t>(bin.data() + 20, 12);
  flatbuffers::WriteScalar<uint64_t>(bin.data() + 24, 24);
  flatbuffers::WriteScalar<uint32_t>(bin.data() + 32, 1);
  flatbuffers::WriteScalar<uint32_t>(bin.data() + 36, 4);
  flatbuffers::WriteScalar<uint64_t>(bin.data() + 48, 1);
  flatbuffers::WriteScalar<int32_t>(bin.data() + 40, -20);
  flatbuffers::WriteScalar<uint16_t>(bin.data() + 60, 4);
  flatbuffers::WriteScalar<uint16_t>(bin.data() + 62, 4);
  bin[56] = 1;  // Item::Note, after the eight-byte length prefix.

  for (int variant = 0; variant < 3; ++variant) {
    if (variant == 1) {
      // The low 32 bits still point into the buffer, but the full offset does
      // not.
      flatbuffers::WriteScalar<uint64_t>(bin.data() + 24, (1ULL << 32) + 24);
    } else if (variant == 2) {
      // Only four bytes remain at the eight-byte companion offset slot.
      flatbuffers::WriteScalar<uint16_t>(bin.data() + 6, 48);
      flatbuffers::WriteScalar<uint16_t>(bin.data() + 8, 44);
    }
    flatbuffers::BinaryAnnotator annotator(parser.builder_.GetBufferPointer(),
                                           parser.builder_.GetSize(),
                                           bin.data(), bin.size(), false);
    const auto sections = annotator.Annotate();
    const auto it = sections.find(32);
    TEST_EQ(it != sections.end(), true);
    if (it == sections.end()) return;
    TEST_EQ(it->second.regions.size(), static_cast<size_t>(2));
    if (it->second.regions.size() != 2) return;
    const auto& value = it->second.regions[1];
    TEST_EQ(value.offset, static_cast<uint64_t>(36));
    TEST_EQ(value.comment.type ==
                flatbuffers::BinaryRegionCommentType::VectorUnionValue,
            true);
    TEST_EQ(
        value.comment.status ==
            (variant == 0
                 ? flatbuffers::BinaryRegionStatus::OK
                 : flatbuffers::BinaryRegionStatus::ERROR_INCOMPLETE_BINARY),
        true);
    if (variant == 0) {
      TEST_EQ_STR(value.comment.default_value.c_str(), "(`Note`)");
      const auto types = sections.find(48);
      TEST_EQ(types != sections.end(), true);
      if (types == sections.end()) return;
      TEST_EQ(types->second.type == flatbuffers::BinarySectionType::Vector64,
              true);
      TEST_EQ(types->second.regions.front().length,
              static_cast<uint64_t>(sizeof(uint64_t)));
    } else {
      TEST_EQ(value.comment.status_message.find(variant == 1 ? "1" : "8") !=
                  std::string::npos,
              true);
    }
  }
}

void BinaryAnnotatorBoundsTest() {
  // Valid inline arrays and repeated calls must preserve traversal state.
  {
    const char* schema_text =
        "struct Leaf { value:int; } "
        "struct Pair { items:[Leaf:2]; tail:Leaf; } "
        "table Root { first:Pair; second:Pair; } root_type Root;";
    flatbuffers::Parser schema_parser;
    TEST_EQ(schema_parser.Parse(schema_text), true);
    schema_parser.Serialize();
    flatbuffers::Parser data_parser;
    TEST_EQ(data_parser.Parse(schema_text), true);
    TEST_EQ(data_parser.Parse(
                "{ first: { items: [{value:1}, {value:2}], tail:{value:3} }, "
                "second: { items: [{value:4}, {value:5}], tail:{value:6} } }"),
            true);
    flatbuffers::BinaryAnnotator annotator(
        schema_parser.builder_.GetBufferPointer(),
        schema_parser.builder_.GetSize(),
        data_parser.builder_.GetBufferPointer(), data_parser.builder_.GetSize(),
        false);
    for (int call = 0; call < 2; ++call) {
      const auto sections = annotator.Annotate();
      size_t scalar_fields = 0;
      for (const auto& section : sections) {
        for (const auto& region : section.second.regions) {
          if (region.comment.type ==
              flatbuffers::BinaryRegionCommentType::StructField) {
            TEST_EQ(
                region.comment.status == flatbuffers::BinaryRegionStatus::OK,
                true);
            ++scalar_fields;
          }
        }
      }
      TEST_EQ(scalar_fields, static_cast<size_t>(6));
    }
  }

  // A truncated nested array stops producing regions once its bytes end.
  {
    const char* schema_text =
        "struct Leaf { value:int; } struct Row { items:[Leaf:4]; } "
        "struct Block { rows:[Row:4]; } "
        "table Root { block:Block; } root_type Root;";
    flatbuffers::Parser schema_parser;
    TEST_EQ(schema_parser.Parse(schema_text), true);
    schema_parser.Serialize();
    const std::string row =
        "{ items:[{value:1},{value:2},{value:3},{value:4}] }";
    const std::string json =
        "{ block:{ rows:[" + row + "," + row + "," + row + "," + row + "] } }";
    flatbuffers::Parser data_parser;
    TEST_EQ(data_parser.Parse(schema_text), true);
    TEST_EQ(data_parser.Parse(json.c_str()), true);
    flatbuffers::BinaryAnnotator complete(
        schema_parser.builder_.GetBufferPointer(),
        schema_parser.builder_.GetSize(),
        data_parser.builder_.GetBufferPointer(), data_parser.builder_.GetSize(),
        false);
    const auto complete_sections = complete.Annotate();
    uint64_t first_scalar = data_parser.builder_.GetSize();
    size_t scalar_count = 0;
    for (const auto& section : complete_sections) {
      for (const auto& region : section.second.regions) {
        if (region.comment.type ==
            flatbuffers::BinaryRegionCommentType::StructField) {
          first_scalar = std::min(first_scalar, region.offset);
          ++scalar_count;
        }
      }
    }
    TEST_EQ(scalar_count, static_cast<size_t>(16));
    if (scalar_count != 16) return;
    flatbuffers::BinaryAnnotator truncated(
        schema_parser.builder_.GetBufferPointer(),
        schema_parser.builder_.GetSize(),
        data_parser.builder_.GetBufferPointer(),
        first_scalar + sizeof(int32_t) + 1, false);
    const auto truncated_sections = truncated.Annotate();
    scalar_count = 0;
    size_t incomplete_count = 0;
    for (const auto& section : truncated_sections) {
      for (const auto& region : section.second.regions) {
        if (region.comment.type ==
            flatbuffers::BinaryRegionCommentType::StructField) {
          ++scalar_count;
          if (region.comment.status ==
              flatbuffers::BinaryRegionStatus::ERROR_INCOMPLETE_BINARY) {
            ++incomplete_count;
          }
        }
      }
    }
    TEST_EQ(scalar_count, static_cast<size_t>(2));
    TEST_EQ(incomplete_count, static_cast<size_t>(1));
  }

  // 1. Positive control + Vector64 length multiplication overflow check.
  {
    const char* vec64_fbs =
        "table Vec64Root { data:[ulong] (vector64); empty_tail:[uint]; }\n"
        "root_type Vec64Root;\n";
    flatbuffers::Parser schema_parser;
    TEST_EQ(schema_parser.Parse(vec64_fbs), true);
    schema_parser.Serialize();
    const uint8_t* bfbs = schema_parser.builder_.GetBufferPointer();
    const uint64_t bfbs_len = schema_parser.builder_.GetSize();

    flatbuffers::Parser data_parser;
    data_parser.builder_.ForceDefaults(true);
    TEST_EQ(data_parser.Parse(vec64_fbs), true);
    TEST_EQ(data_parser.Parse("{ data: [11, 22], empty_tail: [] }"), true);
    const std::vector<uint8_t> valid_bin(
        data_parser.builder_.GetBufferPointer(),
        data_parser.builder_.GetBufferPointer() +
            data_parser.builder_.GetSize());

    // Valid positive control: Vector64 and empty trailing vector annotate OK.
    flatbuffers::BinaryAnnotator valid_annotator(
        bfbs, bfbs_len, valid_bin.data(), valid_bin.size(), false);
    const auto valid_sections = valid_annotator.Annotate();
    uint64_t vec64_offset = 0;
    bool found_vec64 = false;
    bool found_empty_tail = false;
    for (const auto& entry : valid_sections) {
      if (entry.second.name == "Vec64Root.data") {
        found_vec64 = true;
        vec64_offset = entry.first;
        TEST_EQ(entry.second.type == flatbuffers::BinarySectionType::Vector64,
                true);
        TEST_EQ(entry.second.regions.size(), static_cast<size_t>(3));
        TEST_EQ(entry.second.regions[0].comment.status ==
                    flatbuffers::BinaryRegionStatus::OK,
                true);
      } else if (entry.second.name == "Vec64Root.empty_tail") {
        found_empty_tail = true;
        TEST_EQ(entry.second.regions[0].comment.status ==
                    flatbuffers::BinaryRegionStatus::OK,
                true);
      }
    }
    TEST_EQ(found_vec64, true);
    TEST_EQ(found_empty_tail, true);

    // Negative overflow check: 2^61 * sizeof(uint64_t) wraps uint64_t to 0.
    std::vector<uint8_t> overflow_bin = valid_bin;
    flatbuffers::WriteScalar<uint64_t>(overflow_bin.data() + vec64_offset,
                                       0x2000000000000000ULL);
    flatbuffers::BinaryAnnotator overflow_annotator(
        bfbs, bfbs_len, overflow_bin.data(), overflow_bin.size(), false);
    const auto overflow_sections = overflow_annotator.Annotate();
    const auto ov_it = overflow_sections.find(vec64_offset);
    TEST_EQ(ov_it != overflow_sections.end(), true);
    if (ov_it != overflow_sections.end()) {
      TEST_EQ(ov_it->second.type == flatbuffers::BinarySectionType::Vector64,
              true);
      TEST_EQ(ov_it->second.regions.size(), static_cast<size_t>(1));
      TEST_EQ(ov_it->second.regions[0].comment.status ==
                  flatbuffers::BinaryRegionStatus::ERROR_LENGTH_TOO_LONG,
              true);
    }
  }

  // 2. Union vector: 32-bit _type field relative offset >= 64 KiB and
  // per-element offset advancement on invalid elements.
  {
    const char* bag_fbs =
        "table Note { tag:uint = 0; }\n"
        "union AnyItem { Note }\n"
        "table Bag { items:[AnyItem]; }\n"
        "root_type Bag;\n";
    flatbuffers::Parser bag_parser;
    TEST_EQ(bag_parser.Parse(bag_fbs), true);
    bag_parser.Serialize();
    const uint8_t* bfbs = bag_parser.builder_.GetBufferPointer();
    const uint64_t bfbs_len = bag_parser.builder_.GetSize();
    const reflection::Schema* bag_schema = reflection::GetSchema(bfbs);

    // Valid wire buffer (> 64 KiB) where items_type sits at relative offset
    // 0x00010020 from field slot 0x14 (0x10034), while truncated uint16_t
    // 0x0020 would read poison byte 0xFF at 0x38.
    std::vector<uint8_t> large_bag(0x10040, 0);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x00, 0x10);
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x04, 8);
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x06, 12);
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x08, 4);
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x0A, 8);
    flatbuffers::WriteScalar<int32_t>(large_bag.data() + 0x10, 12);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x14, 0x00010020U);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x18, 0x00000008U);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x20, 1);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x24, 0x0000002CU);
    large_bag[0x38] = 0xFF;  // Invalid union tag at 16-bit truncated offset.
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x44, 6);
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x46, 8);
    flatbuffers::WriteScalar<uint16_t>(large_bag.data() + 0x48, 4);
    flatbuffers::WriteScalar<int32_t>(large_bag.data() + 0x50, 12);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x54, 0x12345678U);
    flatbuffers::WriteScalar<uint32_t>(large_bag.data() + 0x10034, 1);
    large_bag[0x10038] = 1;  // AnyItem::Note
    TEST_EQ(flatbuffers::Verify(*bag_schema, *bag_schema->root_table(),
                                large_bag.data(), large_bag.size()),
            true);

    flatbuffers::BinaryAnnotator large_annotator(
        bfbs, bfbs_len, large_bag.data(), large_bag.size(), false);
    const auto large_sections = large_annotator.Annotate();
    const auto items_it = large_sections.find(0x20);
    TEST_EQ(items_it != large_sections.end(), true);
    if (items_it != large_sections.end()) {
      TEST_EQ(items_it->second.regions.size(), static_cast<size_t>(2));
      if (items_it->second.regions.size() == 2) {
        TEST_EQ(items_it->second.regions[1].comment.status ==
                    flatbuffers::BinaryRegionStatus::OK,
                true);
      }
    }
    TEST_EQ(large_sections.find(0x50) != large_sections.end(), true);

    // Negative malformed union vector: element 0 has out-of-bounds offset,
    // element 1 points to valid Note table at 0x50.
    std::vector<uint8_t> mixed_bag(0x80, 0);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x00, 0x10);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x04, 8);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x06, 12);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x08, 4);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x0A, 8);
    flatbuffers::WriteScalar<int32_t>(mixed_bag.data() + 0x10, 12);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x14, 0x0000004CU);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x18, 0x00000008U);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x20, 2);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x24, 0x00F00000U);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x28, 0x00000028U);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x44, 6);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x46, 8);
    flatbuffers::WriteScalar<uint16_t>(mixed_bag.data() + 0x48, 4);
    flatbuffers::WriteScalar<int32_t>(mixed_bag.data() + 0x50, 12);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x54, 0xCAFEBABEU);
    flatbuffers::WriteScalar<uint32_t>(mixed_bag.data() + 0x60, 2);
    mixed_bag[0x64] = 1;
    mixed_bag[0x65] = 1;

    flatbuffers::BinaryAnnotator mixed_annotator(
        bfbs, bfbs_len, mixed_bag.data(), mixed_bag.size(), false);
    const auto mixed_sections = mixed_annotator.Annotate();
    const auto mixed_it = mixed_sections.find(0x20);
    TEST_EQ(mixed_it != mixed_sections.end(), true);
    if (mixed_it != mixed_sections.end()) {
      TEST_EQ(mixed_it->second.regions.size(), static_cast<size_t>(3));
      if (mixed_it->second.regions.size() == 3) {
        TEST_EQ(mixed_it->second.regions[1].offset,
                static_cast<uint64_t>(0x24));
        TEST_EQ(mixed_it->second.regions[1].comment.status ==
                    flatbuffers::BinaryRegionStatus::ERROR_OFFSET_OUT_OF_BINARY,
                true);
        TEST_EQ(mixed_it->second.regions[2].offset,
                static_cast<uint64_t>(0x28));
        TEST_EQ(mixed_it->second.regions[2].comment.status ==
                    flatbuffers::BinaryRegionStatus::OK,
                true);
      }
    }
    TEST_EQ(mixed_sections.find(0x50) != mixed_sections.end(), true);

    // Truncated _type vector data: element offsets still advance monotonically.
    std::vector<uint8_t> trunc_type_bag(mixed_bag.begin(),
                                        mixed_bag.begin() + 0x64);
    flatbuffers::WriteScalar<uint32_t>(trunc_type_bag.data() + 0x24,
                                       0x0000002CU);
    flatbuffers::BinaryAnnotator trunc_annotator(
        bfbs, bfbs_len, trunc_type_bag.data(), trunc_type_bag.size(), false);
    const auto trunc_sections = trunc_annotator.Annotate();
    const auto trunc_it = trunc_sections.find(0x20);
    TEST_EQ(trunc_it != trunc_sections.end(), true);
    if (trunc_it != trunc_sections.end()) {
      TEST_EQ(trunc_it->second.regions.size(), static_cast<size_t>(3));
      if (trunc_it->second.regions.size() == 3) {
        TEST_EQ(trunc_it->second.regions[1].offset,
                static_cast<uint64_t>(0x24));
        TEST_EQ(trunc_it->second.regions[1].length,
                static_cast<uint64_t>(sizeof(uint32_t)));
        TEST_EQ(trunc_it->second.regions[2].offset,
                static_cast<uint64_t>(0x28));
        TEST_EQ(trunc_it->second.regions[2].length,
                static_cast<uint64_t>(sizeof(uint32_t)));
      }
    }
  }

  // 4. Crafted negative schemas: direct and mutual fixed-array inline struct
  // cycles in BuildStruct.
  {
    // 4a. Direct self-referential struct (LoopS.self -> LoopS).
    flatbuffers::FlatBufferBuilder sb;
    auto self_t = reflection::CreateType(sb, reflection::BaseType::Obj,
                                         reflection::BaseType::None, 0);
    std::vector<flatbuffers::Offset<reflection::Field>> loop_f_vec = {
        reflection::CreateField(sb, sb.CreateString("self"), self_t, 0, 0),
    };
    auto loop_fields = sb.CreateVectorOfSortedTables(&loop_f_vec);
    auto loop_struct = reflection::CreateObject(sb, sb.CreateString("LoopS"),
                                                loop_fields, true, 1, 4);

    auto root_s_t = reflection::CreateType(sb, reflection::BaseType::Obj,
                                           reflection::BaseType::None, 0);
    std::vector<flatbuffers::Offset<reflection::Field>> root_f_vec = {
        reflection::CreateField(sb, sb.CreateString("s"), root_s_t, 0, 4),
    };
    auto root_fields = sb.CreateVectorOfSortedTables(&root_f_vec);
    auto root_obj = reflection::CreateObject(sb, sb.CreateString("RootT"),
                                             root_fields, false, 4, 0);

    std::vector<flatbuffers::Offset<reflection::Object>> objs_vec = {
        loop_struct, root_obj};
    auto objs = sb.CreateVectorOfSortedTables(&objs_vec);
    auto enums = sb.CreateVectorOfSortedTables<reflection::Enum>(nullptr, 0);
    auto schema =
        reflection::CreateSchema(sb, objs, enums, sb.CreateString("BFBS"),
                                 sb.CreateString("fbs"), root_obj);
    reflection::FinishSchemaBuffer(sb, schema);

    flatbuffers::Verifier ver(sb.GetBufferPointer(), sb.GetSize());
    TEST_EQ(reflection::VerifySchemaBuffer(ver), true);

    std::vector<uint8_t> bin(24, 0);
    flatbuffers::WriteScalar<uint32_t>(bin.data() + 0, 12);
    flatbuffers::WriteScalar<uint16_t>(bin.data() + 4, 6);
    flatbuffers::WriteScalar<uint16_t>(bin.data() + 6, 8);
    flatbuffers::WriteScalar<uint16_t>(bin.data() + 8, 4);
    flatbuffers::WriteScalar<int32_t>(bin.data() + 12, 8);

    flatbuffers::BinaryAnnotator annotator(sb.GetBufferPointer(), sb.GetSize(),
                                           bin.data(), bin.size(), false);
    const auto sections = annotator.Annotate();
    TEST_EQ(sections.empty(), false);

    // 4b. Mutual 2-struct fixed-array cycle (CycleA[65535] <-> CycleB[65535]).
    flatbuffers::FlatBufferBuilder sb2;
    // Sorted object order by name: CycleA (0), CycleB (1), RootT (2).
    auto arr_to_b = reflection::CreateType(sb2, reflection::BaseType::Array,
                                           reflection::BaseType::Obj, 1, 65535);
    std::vector<flatbuffers::Offset<reflection::Field>> ca_f_vec = {
        reflection::CreateField(sb2, sb2.CreateString("b"), arr_to_b, 0, 0),
    };
    auto ca_obj = reflection::CreateObject(
        sb2, sb2.CreateString("CycleA"),
        sb2.CreateVectorOfSortedTables(&ca_f_vec), true, 1, 4);

    auto arr_to_a = reflection::CreateType(sb2, reflection::BaseType::Array,
                                           reflection::BaseType::Obj, 0, 65535);
    std::vector<flatbuffers::Offset<reflection::Field>> cb_f_vec = {
        reflection::CreateField(sb2, sb2.CreateString("a"), arr_to_a, 0, 0),
    };
    auto cb_obj = reflection::CreateObject(
        sb2, sb2.CreateString("CycleB"),
        sb2.CreateVectorOfSortedTables(&cb_f_vec), true, 1, 4);

    auto root2_s_t = reflection::CreateType(sb2, reflection::BaseType::Obj,
                                            reflection::BaseType::None, 0);
    std::vector<flatbuffers::Offset<reflection::Field>> r2_f_vec = {
        reflection::CreateField(sb2, sb2.CreateString("s"), root2_s_t, 0, 4),
    };
    auto r2_obj = reflection::CreateObject(
        sb2, sb2.CreateString("RootT"),
        sb2.CreateVectorOfSortedTables(&r2_f_vec), false, 4, 0);

    std::vector<flatbuffers::Offset<reflection::Object>> objs2_vec = {
        ca_obj, cb_obj, r2_obj};
    auto objs2 = sb2.CreateVectorOfSortedTables(&objs2_vec);
    auto enums2 = sb2.CreateVectorOfSortedTables<reflection::Enum>(nullptr, 0);
    auto schema2 =
        reflection::CreateSchema(sb2, objs2, enums2, sb2.CreateString("BFBS"),
                                 sb2.CreateString("fbs"), r2_obj);
    reflection::FinishSchemaBuffer(sb2, schema2);

    flatbuffers::Verifier ver2(sb2.GetBufferPointer(), sb2.GetSize());
    TEST_EQ(reflection::VerifySchemaBuffer(ver2), true);

    flatbuffers::BinaryAnnotator annotator2(
        sb2.GetBufferPointer(), sb2.GetSize(), bin.data(), bin.size(), false);
    const auto sections2 = annotator2.Annotate();
    TEST_EQ(sections2.empty(), false);
  }
}

}  // namespace

// Low level stress/fuzz test: serialize/deserialize a variety of
// different kinds of data in different combinations
void FuzzTest1() {
  // Values we're testing against: chosen to ensure no bits get chopped
  // off anywhere, and also be different from eachother.
  const uint8_t bool_val = true;
  const int8_t char_val = -127;  // 0x81
  const uint8_t uchar_val = 0xFF;
  const int16_t short_val = -32222;  // 0x8222;
  const uint16_t ushort_val = 0xFEEE;
  const int32_t int_val = 0x83333333;
  const uint32_t uint_val = 0xFDDDDDDD;
  const int64_t long_val = 0x8444444444444444LL;
  const uint64_t ulong_val = 0xFCCCCCCCCCCCCCCCULL;
  const float float_val = 3.14159f;
  const double double_val = 3.14159265359;

  const int test_values_max = 11;
  const flatbuffers::voffset_t fields_per_object = 4;
  const int num_fuzz_objects = 10000;  // The higher, the more thorough :)

  flatbuffers::FlatBufferBuilder builder;

  lcg_reset();  // Keep it deterministic.

  flatbuffers::uoffset_t objects[num_fuzz_objects];

  // Generate num_fuzz_objects random objects each consisting of
  // fields_per_object fields, each of a random type.
  for (int i = 0; i < num_fuzz_objects; i++) {
    auto start = builder.StartTable();
    for (flatbuffers::voffset_t f = 0; f < fields_per_object; f++) {
      int choice = lcg_rand() % test_values_max;
      auto off = flatbuffers::FieldIndexToOffset(f);
      switch (choice) {
        case 0:
          builder.AddElement<uint8_t>(off, bool_val, 0);
          break;
        case 1:
          builder.AddElement<int8_t>(off, char_val, 0);
          break;
        case 2:
          builder.AddElement<uint8_t>(off, uchar_val, 0);
          break;
        case 3:
          builder.AddElement<int16_t>(off, short_val, 0);
          break;
        case 4:
          builder.AddElement<uint16_t>(off, ushort_val, 0);
          break;
        case 5:
          builder.AddElement<int32_t>(off, int_val, 0);
          break;
        case 6:
          builder.AddElement<uint32_t>(off, uint_val, 0);
          break;
        case 7:
          builder.AddElement<int64_t>(off, long_val, 0);
          break;
        case 8:
          builder.AddElement<uint64_t>(off, ulong_val, 0);
          break;
        case 9:
          builder.AddElement<float>(off, float_val, 0);
          break;
        case 10:
          builder.AddElement<double>(off, double_val, 0);
          break;
      }
    }
    objects[i] = builder.EndTable(start);
  }
  builder.PreAlign<flatbuffers::largest_scalar_t>(0);  // Align whole buffer.

  lcg_reset();  // Reset.

  uint8_t* eob = builder.GetCurrentBufferPointer() + builder.GetSize();

  // Test that all objects we generated are readable and return the
  // expected values. We generate random objects in the same order
  // so this is deterministic.
  for (int i = 0; i < num_fuzz_objects; i++) {
    auto table = reinterpret_cast<flatbuffers::Table*>(eob - objects[i]);
    for (flatbuffers::voffset_t f = 0; f < fields_per_object; f++) {
      int choice = lcg_rand() % test_values_max;
      flatbuffers::voffset_t off = flatbuffers::FieldIndexToOffset(f);
      switch (choice) {
        case 0:
          CompareTableFieldValue(table, off, bool_val);
          break;
        case 1:
          CompareTableFieldValue(table, off, char_val);
          break;
        case 2:
          CompareTableFieldValue(table, off, uchar_val);
          break;
        case 3:
          CompareTableFieldValue(table, off, short_val);
          break;
        case 4:
          CompareTableFieldValue(table, off, ushort_val);
          break;
        case 5:
          CompareTableFieldValue(table, off, int_val);
          break;
        case 6:
          CompareTableFieldValue(table, off, uint_val);
          break;
        case 7:
          CompareTableFieldValue(table, off, long_val);
          break;
        case 8:
          CompareTableFieldValue(table, off, ulong_val);
          break;
        case 9:
          CompareTableFieldValue(table, off, float_val);
          break;
        case 10:
          CompareTableFieldValue(table, off, double_val);
          break;
      }
    }
  }
}

// High level stress/fuzz test: generate a big schema and
// matching json data in random combinations, then parse both,
// generate json back from the binary, and compare with the original.
void FuzzTest2() {
  lcg_reset();  // Keep it deterministic.

  const int num_definitions = 30;
  const int num_struct_definitions = 5;  // Subset of num_definitions.
  const int fields_per_definition = 15;
  const int instances_per_definition = 5;
  const int deprecation_rate = 10;  // 1 in deprecation_rate fields will
                                    // be deprecated.

  std::string schema = "namespace test;\n\n";

  struct RndDef {
    std::string instances[instances_per_definition];

    // Since we're generating schema and corresponding data in tandem,
    // this convenience function adds strings to both at once.
    static void Add(RndDef (&definitions_l)[num_definitions],
                    std::string& schema_l, const int instances_per_definition_l,
                    const char* schema_add, const char* instance_add,
                    int definition) {
      schema_l += schema_add;
      for (int i = 0; i < instances_per_definition_l; i++)
        definitions_l[definition].instances[i] += instance_add;
    }
  };

  // clang-format off
  #define AddToSchemaAndInstances(schema_add, instance_add) \
    RndDef::Add(definitions, schema, instances_per_definition, \
                schema_add, instance_add, definition)

  #define Dummy() \
    RndDef::Add(definitions, schema, instances_per_definition, \
                "byte", "1", definition)
  // clang-format on

  RndDef definitions[num_definitions];

  // We are going to generate num_definitions, the first
  // num_struct_definitions will be structs, the rest tables. For each
  // generate random fields, some of which may be struct/table types
  // referring to previously generated structs/tables.
  // Simultanenously, we generate instances_per_definition JSON data
  // definitions, which will have identical structure to the schema
  // being generated. We generate multiple instances such that when creating
  // hierarchy, we get some variety by picking one randomly.
  for (int definition = 0; definition < num_definitions; definition++) {
    std::string definition_name = "D" + flatbuffers::NumToString(definition);

    bool is_struct = definition < num_struct_definitions;

    AddToSchemaAndInstances(
        ((is_struct ? "struct " : "table ") + definition_name + " {\n").c_str(),
        "{\n");

    for (int field = 0; field < fields_per_definition; field++) {
      const bool is_last_field = field == fields_per_definition - 1;

      // Deprecate 1 in deprecation_rate fields. Only table fields can be
      // deprecated.
      // Don't deprecate the last field to avoid dangling commas in JSON.
      const bool deprecated =
          !is_struct && !is_last_field && (lcg_rand() % deprecation_rate == 0);

      std::string field_name = "f" + flatbuffers::NumToString(field);
      AddToSchemaAndInstances(("  " + field_name + ":").c_str(),
                              deprecated ? "" : (field_name + ": ").c_str());
      // Pick random type:
      auto base_type = static_cast<flatbuffers::BaseType>(
          lcg_rand() % (flatbuffers::BASE_TYPE_UNION + 1));
      switch (base_type) {
        case flatbuffers::BASE_TYPE_STRING:
          if (is_struct) {
            Dummy();  // No strings in structs.
          } else {
            AddToSchemaAndInstances("string", deprecated ? "" : "\"hi\"");
          }
          break;
        case flatbuffers::BASE_TYPE_VECTOR:
          if (is_struct) {
            Dummy();  // No vectors in structs.
          } else {
            AddToSchemaAndInstances("[ubyte]",
                                    deprecated ? "" : "[\n0,\n1,\n255\n]");
          }
          break;
        case flatbuffers::BASE_TYPE_NONE:
        case flatbuffers::BASE_TYPE_UTYPE:
        case flatbuffers::BASE_TYPE_STRUCT:
        case flatbuffers::BASE_TYPE_UNION:
          if (definition) {
            // Pick a random previous definition and random data instance of
            // that definition.
            int defref = lcg_rand() % definition;
            int instance = lcg_rand() % instances_per_definition;
            AddToSchemaAndInstances(
                ("D" + flatbuffers::NumToString(defref)).c_str(),
                deprecated ? ""
                           : definitions[defref].instances[instance].c_str());
          } else {
            // If this is the first definition, we have no definition we can
            // refer to.
            Dummy();
          }
          break;
        case flatbuffers::BASE_TYPE_BOOL:
          AddToSchemaAndInstances(
              "bool", deprecated ? "" : (lcg_rand() % 2 ? "true" : "false"));
          break;
        case flatbuffers::BASE_TYPE_ARRAY:
          if (!is_struct) {
            AddToSchemaAndInstances(
                "ubyte",
                deprecated ? "" : "255");  // No fixed-length arrays in tables.
          } else {
            AddToSchemaAndInstances("[int:3]", deprecated ? "" : "[\n,\n,\n]");
          }
          break;
        default:
          // All the scalar types.
          schema += flatbuffers::TypeName(base_type);

          if (!deprecated) {
            // We want each instance to use its own random value.
            for (int inst = 0; inst < instances_per_definition; inst++)
              definitions[definition].instances[inst] +=
                  flatbuffers::IsFloat(base_type)
                      ? flatbuffers::NumToString<double>(lcg_rand() % 128)
                            .c_str()
                      : flatbuffers::NumToString<int>(lcg_rand() % 128).c_str();
          }
      }
      AddToSchemaAndInstances(deprecated ? "(deprecated);\n" : ";\n",
                              deprecated      ? ""
                              : is_last_field ? "\n"
                                              : ",\n");
    }
    AddToSchemaAndInstances("}\n\n", "}");
  }

  schema += "root_type D" + flatbuffers::NumToString(num_definitions - 1);
  schema += ";\n";

  flatbuffers::Parser parser;

  // Will not compare against the original if we don't write defaults
  parser.builder_.ForceDefaults(true);

  // Parse the schema, parse the generated data, then generate text back
  // from the binary and compare against the original.
  TEST_EQ(parser.Parse(schema.c_str()), true);

  const std::string& json =
      definitions[num_definitions - 1].instances[0] + "\n";

  TEST_EQ(parser.Parse(json.c_str()), true);

  std::string jsongen;
  parser.opts.indent_step = 0;
  auto result = GenText(parser, parser.builder_.GetBufferPointer(), &jsongen);
  TEST_NULL(result);

  if (jsongen != json) {
    // These strings are larger than a megabyte, so we show the bytes around
    // the first bytes that are different rather than the whole string.
    size_t len = std::min(json.length(), jsongen.length());
    for (size_t i = 0; i < len; i++) {
      if (json[i] != jsongen[i]) {
        i -= std::min(static_cast<size_t>(10), i);  // show some context;
        size_t end = std::min(len, i + 20);
        for (; i < end; i++)
          TEST_OUTPUT_LINE("at %d: found \"%c\", expected \"%c\"\n",
                           static_cast<int>(i), jsongen[i], json[i]);
        break;
      }
    }
    TEST_NOTNULL(nullptr);  //-V501 (this comment suppresses CWE-570 warning)
  }

  // clang-format off
  #ifdef FLATBUFFERS_TEST_VERBOSE
    TEST_OUTPUT_LINE("%dk schema tested with %dk of json\n",
                     static_cast<int>(schema.length() / 1024),
                     static_cast<int>(json.length() / 1024));
  #endif
  // clang-format on

  BinaryAnnotatorEmptySectionTest();
  BinaryAnnotatorUnionOffset64Test();
  BinaryAnnotatorBoundsTest();
}

}  // namespace tests
}  // namespace flatbuffers
