#include "binary_annotator.h"

#include <iostream>
#include <limits>
#include <vector>

#include "flatbuffers/reflection.h"
#include "flatbuffers/verifier.h"

namespace flatbuffers {
namespace {

static bool BinaryRegionSort(const BinaryRegion &a, const BinaryRegion &b) {
  return a.offset < b.offset;
}

static BinaryRegion MakeBinaryRegion(
    const uint64_t offset = 0, const uint64_t length = 0,
    const BinaryRegionType type = BinaryRegionType::Unknown,
    const uint64_t array_length = 0, const uint64_t points_to_offset = 0,
    const std::string &comment = "") {
  BinaryRegion region;
  region.offset = offset;
  region.length = length;
  region.type = type;
  region.array_length = array_length;
  region.points_to_offset = points_to_offset;
  region.comment = comment;
  return region;
}

static BinarySection MakeBinarySection(
    const std::string &name, const BinarySectionType type,
    const std::vector<BinaryRegion> &regions) {
  BinarySection section;
  section.name = name;
  section.type = type;
  section.regions = regions;
  return section;
}

static BinarySection MakeSingleRegionBinarySection(const std::string &name,
                                                   const BinarySectionType type,
                                                   const BinaryRegion &region) {
  std::vector<BinaryRegion> regions;
  regions.push_back(region);
  return MakeBinarySection(name, type, regions);
}

static uint64_t BuildStructureField(const uint64_t offset,
                                    const reflection::Object *object,
                                    const reflection::Field *field,
                                    std::vector<BinaryRegion> &regions) {
  const uint64_t type_size = GetTypeSize(field->type()->base_type());
  regions.push_back(MakeBinaryRegion(
      offset, type_size, GetRegionType(field->type()->base_type()), 0, 0,
      std::string("struct field `") + object->name()->c_str() + "." +
          field->name()->c_str() + "` (" +
          reflection::EnumNameBaseType(field->type()->base_type()) + ")"));
  return offset + type_size;
}

static uint64_t BuildArrayField(uint64_t offset,
                                const reflection::Object *object,
                                const reflection::Field *field,
                                const uint16_t array_length,
                                std::vector<BinaryRegion> &regions) {
  const uint64_t type_size = GetTypeSize(field->type()->element());
  for (uint16_t i = 0; i < array_length; ++i) {
    regions.push_back(MakeBinaryRegion(
        offset, type_size, GetRegionType(field->type()->element()), 0, 0,
        std::string("array field `") + object->name()->c_str() + "." +
            field->name()->c_str() + "[" + std::to_string(i) + "]` (" +
            reflection::EnumNameBaseType(field->type()->element()) + ")"));
    offset += type_size;
  }

  // The following groups the complete array together which shows up nicely as
  // an array, but then we don't show the individual values. So the above method
  // treats each field of the array as a separate region.
  // regions.push_back(
  //     BinaryRegion{ offset, array_length * type_size,
  //                   GetRegionType(field->type()->element()), array_length, 0,
  //                   std::string("array field '") + object->name()->c_str() +
  //                       "." + field->name()->c_str() + "' value" });

  return offset;
}

static bool IsNonZeroRegion(uint64_t offset, uint64_t length,
                            const uint8_t *binary) {
  for (uint64_t i = offset; i < offset + length; ++i) {
    if (binary[i] != 0) { return true; }
  }
  return false;
}

static BinarySection GenerateMissingSection(const uint64_t offset,
                                            const uint64_t length,
                                            const uint8_t *const binary) {
  std::vector<BinaryRegion> regions;

  // Check if the region is all zeros or not, as that can tell us if it is
  // padding or not.
  if (IsNonZeroRegion(offset, length, binary)) {
    // Some of the padding bytes are non-zero, so this might be an unknown
    // section of the binary.
    regions.push_back(MakeBinaryRegion(
        offset, length * sizeof(uint8_t), BinaryRegionType::Unknown, length, 0,
        length < 8 ? "could be a corrupted padding region (non zero) "
                     "due to the length < 8 bytes."
                   : "WARN: nothing refers to this section."));

    return MakeBinarySection("no known references", BinarySectionType::Unknown,
                             std::move(regions));
  }

  // This region is most likely padding.
  regions.push_back(MakeBinaryRegion(
      offset, length * sizeof(uint8_t), BinaryRegionType::Uint8, length, 0,
      // Output a different annotation if the pad bytes exceed what we
      // expect to be the maximum padding.
      length > 7 ? "likely padding but might be an unknown section "
                   "due to being larger than 7 bytes"
                 : "padding"));

  return MakeBinarySection("", BinarySectionType::Padding, std::move(regions));
}

}  // namespace

std::map<uint64_t, BinarySection> BinaryAnnotator::Annotate() {
  flatbuffers::Verifier verifier(bfbs_, static_cast<size_t>(bfbs_length_));
  if (!reflection::VerifySchemaBuffer(verifier)) { return {}; }

  // The binary is too short to read as a flatbuffers.
  // TODO(dbaileychess): We could spit out the annotated buffer sections, but
  // I'm not sure if it is worth it.
  if (binary_length_ < 4) { return {}; }

  // Make sure we start with a clean slate.
  vtables_.clear();
  strings_.clear();
  sections_.clear();

  // First parse the header region which always start at offset 0.
  // The returned offset will point to the root_table location.
  const uint64_t root_table_offset = BuildHeader(0);

  if (IsValidOffset(root_table_offset)) {
    // Build the root table, and all else will be referenced from it.
    BuildTable(root_table_offset, BinarySectionType::RootTable,
               schema_->root_table());
  }

  // Now that all the sections are built, make sure the binary sections are
  // contiguous.
  FixMissingRegions();

  // Then scan the area between BinarySections insert padding sections that are
  // implied.
  FixMissingSections();

  return sections_;
}

uint64_t BinaryAnnotator::BuildHeader(uint64_t offset) {
  if (!IsValidRead<uint32_t>(offset)) {
    // This shouldn't occur, since we validate the min size of the buffer
    // before. But for completion sake, we shouldn't read passed the binary end.
    return std::numeric_limits<uint64_t>::max();
  }

  std::vector<BinaryRegion> regions;
  const uint64_t header_offset = offset;
  // TODO(dbaileychess): sized prefixed value

  const uint32_t root_table_offset = GetScalar<uint32_t>(offset);
  if (IsValidOffset(root_table_offset)) {
    regions.push_back(
        MakeBinaryRegion(offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
                         root_table_offset,
                         std::string("offset to root table `") +
                             schema_->root_table()->name()->str() + "`"));
  } else {
    regions.push_back(
        MakeBinaryRegion(offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
                         root_table_offset,
                         std::string("ERROR: invalid offset to root table `") +
                             schema_->root_table()->name()->str() + "`"));
  }
  offset += sizeof(uint32_t);

  if (IsValidRead<uint32_t>(offset) &&
      IsNonZeroRegion(offset, sizeof(uint32_t), binary_)) {
    // Check if the file identifier region has non-zero data, and assume its the
    // file identifier. Otherwise, it will get filled in with padding later.
    regions.push_back(MakeBinaryRegion(offset, 4 * sizeof(uint8_t),
                                       BinaryRegionType::Char, 4, 0,
                                       std::string("File Identifier")));
  }

  sections_.insert(std::make_pair(
      header_offset,
      MakeBinarySection("", BinarySectionType::Header, regions)));

  return root_table_offset;
}

void BinaryAnnotator::BuildVTable(uint64_t offset,
                                  const reflection::Object *table,
                                  const uint64_t offset_of_referring_table) {
  const uint64_t vtable_offset = offset;

  // First see if we have used this vtable before, if so skip building it again.
  auto it = vtables_.find(vtable_offset);
  if (it != vtables_.end()) { return; }

  if (!IsValidRead<uint16_t>(offset)) {
    const uint64_t remaining = RemainingBytes(offset);

    AddSection(
        offset,
        MakeSingleRegionBinarySection(
            table->name()->str(), BinarySectionType::VTable,
            MakeBinaryRegion(
                offset, remaining, BinaryRegionType::Unknown, remaining, 0,
                "ERROR: incomplete binary, expected to read 2 bytes here")));
    return;
  }

  // Vtables start with the size of the vtable
  const uint16_t vtable_size = GetScalar<uint16_t>(offset);

  if (!IsValidOffset(offset + vtable_size - 1)) {
    // The vtable_size points to off the end of the binary.
    AddSection(offset,
               MakeSingleRegionBinarySection(
                   table->name()->str(), BinarySectionType::VTable,
                   MakeBinaryRegion(
                       offset, sizeof(uint16_t), BinaryRegionType::Uint16, 0, 0,
                       "ERROR: size of this vtable. Longer than the binary")));

    return;
  } else if (vtable_size < 2 * sizeof(uint16_t)) {
    // The size includes itself and the table size which are both uint16_t.
    AddSection(offset,
               MakeSingleRegionBinarySection(
                   table->name()->str(), BinarySectionType::VTable,
                   MakeBinaryRegion(offset, sizeof(uint16_t),
                                    BinaryRegionType::Uint16, 0, 0,
                                    "ERROR: size of this vtable. Shorter than "
                                    "the minimum 4 bytes")));
    return;
  }

  std::vector<BinaryRegion> regions;

  regions.push_back(MakeBinaryRegion(offset, sizeof(uint16_t),
                                     BinaryRegionType::Uint16, 0, 0,
                                     std::string("size of this vtable")));
  offset += sizeof(uint16_t);

  // Ensure we can read the next uint16_t field, which is the size of the
  // referring table.
  if (!IsValidRead<uint16_t>(offset)) {
    const uint64_t remaining = RemainingBytes(offset);

    AddSection(
        offset,
        MakeSingleRegionBinarySection(
            table->name()->str(), BinarySectionType::VTable,
            MakeBinaryRegion(
                offset, remaining, BinaryRegionType::Unknown, remaining, 0,
                "ERROR: incomplete binary, expected to read 2 bytes here")));
    return;
  }

  // Then they have the size of the table they reference.
  const uint16_t table_size = GetScalar<uint16_t>(offset);

  if (!IsValidOffset(offset_of_referring_table + table_size - 1)) {
    regions.push_back(MakeBinaryRegion(
        offset, sizeof(uint16_t), BinaryRegionType::Uint16, 0, 0,
        std::string("ERROR: size of referring table. Size of table is larger "
                    "than the binary.")));
  } else if (table_size < 4) {
    regions.push_back(MakeBinaryRegion(
        offset, sizeof(uint16_t), BinaryRegionType::Uint16, 0, 0,
        std::string("ERROR: size of referring table. Size of table is "
                    "smaller than the minimum 4 bytes")));
  } else {
    regions.push_back(MakeBinaryRegion(offset, sizeof(uint16_t),
                                       BinaryRegionType::Uint16, 0, 0,
                                       std::string("size of referring table")));
  }
  offset += sizeof(uint16_t);

  const uint64_t offset_start = offset;

  // A mapping between field (and its id) to the relative offset (uin16_t) from
  // the start of the table.
  std::map<uint16_t, VTable::Entry> fields;

  // Counter for determining if the binary has more vtable entries than the
  // schema provided. This can occur if the binary was created at a newer schema
  // version and is being processed with an older one.
  uint16_t fields_processed = 0;

  // Loop over all the fields.
  ForAllFields(table, /*reverse=*/false, [&](const reflection::Field *field) {
    const uint64_t field_offset = offset_start + field->id() * sizeof(uint16_t);

    if (field_offset >= vtable_offset + vtable_size) {
      // This field_offset is too large for this vtable, so it must come from a
      // newer schema than the binary was create with or the binary writer did
      // not write it. For either case, it is safe to ignore.

      // TODO(dbaileychess): We could show which fields are not set an their
      // default values if we want. We just need a way to make it obvious that
      // it isn't part of the buffer.
      return;
    }

    const std::string field_comment =
        std::string("offset to field `") + field->name()->c_str() +
        "` (id: " + std::to_string(field->id()) + ")";

    if (!IsValidRead<uint16_t>(field_offset)) {
      const uint64_t remaining = RemainingBytes(field_offset);

      regions.push_back(MakeBinaryRegion(
          field_offset, remaining, BinaryRegionType::Unknown, remaining, 0,
          "ERROR: incomplete binary, expected to read 2 bytes here"));

      return;
    }

    const uint16_t offset_from_table = GetScalar<uint16_t>(field_offset);

    if (!IsValidOffset(offset_of_referring_table + offset_from_table - 1)) {
      regions.push_back(MakeBinaryRegion(
          field_offset, sizeof(uint16_t), BinaryRegionType::VOffset, 0, 0,
          std::string("ERROR: ") + field_comment +
              ". Offset points to outside the binary."));
      return;
    }

    VTable::Entry entry;
    entry.field = field;
    entry.offset_from_table = offset_from_table;
    fields.insert(std::make_pair(field->id(), entry));

    std::string default_label;
    if (offset_from_table == 0) {
      // Not present, so could be default or be optional.
      if (field->required()) {
        // If this is a required field, make it known this is an error.
        regions.push_back(MakeBinaryRegion(
            field_offset, sizeof(uint16_t), BinaryRegionType::VOffset, 0, 0,
            std::string("ERROR: required field `") + field->name()->c_str() +
                "` (id: " + std::to_string(field->id()) + ") is not present!"));
        return;
      } else {
        // Its an optional field, so get the default value and interpret and
        // provided an annotation for it.
        if (IsScalar(field->type()->base_type())) {
          default_label += " <defaults to ";
          default_label += IsFloat(field->type()->base_type())
                               ? std::to_string(field->default_real())
                               : std::to_string(field->default_integer());
          default_label += "> (";
        } else {
          default_label += " <null> (";
        }
        default_label +=
            reflection::EnumNameBaseType(field->type()->base_type());
        default_label += ")";
      }
    }

    regions.push_back(MakeBinaryRegion(field_offset, sizeof(uint16_t),
                                       BinaryRegionType::VOffset, 0, 0,
                                       field_comment + default_label));

    fields_processed++;
  });

  // Check if we covered all the expectant fields. If not, we need to add them
  // as unknown fields.
  const uint16_t expectant_vtable_fields =
      (vtable_size - sizeof(uint16_t) - sizeof(uint16_t)) / sizeof(uint16_t);

  for (uint16_t id = fields_processed; id < expectant_vtable_fields; ++id) {
    const uint64_t field_offset = offset_start + id * sizeof(uint16_t);

    if (!IsValidRead<uint16_t>(field_offset)) {
      const uint64_t remaining = RemainingBytes(field_offset);

      regions.push_back(MakeBinaryRegion(
          field_offset, remaining, BinaryRegionType::Unknown, remaining, 0,
          "ERROR: incomplete binary, expected to read 2 bytes here"));
      continue;
    }

    const uint16_t offset_from_table = GetScalar<uint16_t>(field_offset);

    VTable::Entry entry;
    entry.field = nullptr;  // No field to reference.
    entry.offset_from_table = offset_from_table;
    fields.insert(std::make_pair(id, entry));

    regions.push_back(MakeBinaryRegion(
        field_offset, sizeof(uint16_t), BinaryRegionType::VOffset, 0, 0,
        std::string("offset to unknown field (id: " + std::to_string(id) +
                    ")")));
  }

  sections_[vtable_offset] = MakeBinarySection(
      table->name()->str(), BinarySectionType::VTable, std::move(regions));

  VTable vtable;
  vtable.fields = std::move(fields);
  vtable.table_size = table_size;
  vtable.vtable_size = vtable_size;

  vtables_[vtable_offset] = vtable;
}

void BinaryAnnotator::BuildTable(const uint64_t table_offset,
                                 const BinarySectionType type,
                                 const reflection::Object *table) {
  const auto vtable_soffset = ReadScalar<int32_t>(table_offset);

  if (!vtable_soffset.has_value()) {
    const uint64_t remaining = RemainingBytes(table_offset);

    AddSection(
        table_offset,
        MakeSingleRegionBinarySection(
            table->name()->str(), type,
            MakeBinaryRegion(
                table_offset, remaining, BinaryRegionType::Unknown, remaining,
                0, "ERROR: incomplete binary, expected to read 4 bytes here")));

    // If there aren't enough bytes left to read the vtable offset, there is
    // nothing we can do.
    return;
  }

  // Tables start with the vtable
  const uint64_t vtable_offset = table_offset - vtable_soffset.value();

  if (!IsValidOffset(vtable_offset)) {
    AddSection(table_offset,
               MakeSingleRegionBinarySection(
                   table->name()->str(), type,
                   MakeBinaryRegion(table_offset, sizeof(int32_t),
                                    BinaryRegionType::SOffset, 0, vtable_offset,
                                    "ERROR: invalid offset to vtable")));

    // There isn't much to do with an invalid vtable offset, as we won't be able
    // to intepret the rest of the table fields.
    return;
  }

  std::vector<BinaryRegion> regions;
  regions.push_back(
      MakeBinaryRegion(table_offset, sizeof(int32_t), BinaryRegionType::SOffset,
                       0, vtable_offset, std::string("offset to vtable")));

  // Parse the vtable first so we know what the rest of the fields in the table
  // are.
  BuildVTable(vtable_offset, table, table_offset);

  auto vtable_entry = vtables_.find(vtable_offset);
  if (vtable_entry == vtables_.end()) {
    // There is no valid vtable for this table, so we cannot process the rest of
    // the table entries.
    return;
  }

  const VTable &vtable = vtable_entry->second;

  // This is the size and length of this table.
  const uint16_t table_size = vtable.table_size;
  uint64_t table_end_offset = table_offset + table_size;

  if (!IsValidOffset(table_end_offset - 1)) {
    // We already validated the table size in BuildVTable, but we have to make
    // sure we don't use a bad value here.
    table_end_offset = binary_length_;
  }

  // We need to iterate over the vtable fields by their offset in the binary,
  // not by their IDs. So copy them over to another vector that we can sort on
  // the offset_from_table property.
  std::vector<VTable::Entry> fields;
  for (const auto &vtable_field : vtable.fields) {
    fields.push_back(vtable_field.second);
  }

  std::stable_sort(fields.begin(), fields.end(),
                   [](const VTable::Entry &a, const VTable::Entry &b) {
                     return a.offset_from_table < b.offset_from_table;
                   });

  // Iterate over all the fields by order of their offset.
  for (size_t i = 0; i < fields.size(); ++i) {
    const reflection::Field *field = fields[i].field;
    const uint16_t offset_from_table = fields[i].offset_from_table;

    if (offset_from_table == 0) {
      // Skip non-present fields.
      continue;
    }

    // The field offsets are relative to the start of the table.
    const uint64_t field_offset = table_offset + offset_from_table;

    if (!IsValidOffset(field_offset)) {
      // The field offset is larger than the binary, nothing we can do.
      continue;
    }

    // We have a vtable entry for a non-existant field, that means its a binary
    // generated by a newer schema than we are currently processing.
    if (field == nullptr) {
      // Calculate the length of this unknown field.
      const uint64_t unknown_field_length =
          // Check if there is another unknown field after this one.
          ((i + 1 < fields.size())
               ? table_offset + fields[i + 1].offset_from_table
               // Otherwise use the known end of the table.
               : table_end_offset) -
          field_offset;

      std::string hint;
      if (unknown_field_length == 4) {
        // The field is 4 in length, so it could be an offset? Provide a hint.
        hint += " <possibly an offset? Check Loc: +0x";
        hint += ToHex(field_offset + GetScalar<uint32_t>(field_offset));
        hint += ">";
      }

      regions.push_back(
          MakeBinaryRegion(field_offset, unknown_field_length * sizeof(uint8_t),
                           BinaryRegionType::Unknown, unknown_field_length, 0,
                           std::string("Unknown field") + hint));
      continue;
    }

    if (IsScalar(field->type()->base_type())) {
      // These are the raw values store in the table.
      const uint64_t type_size = GetTypeSize(field->type()->base_type());
      const BinaryRegionType region_type =
          GetRegionType(field->type()->base_type());

      if (!IsValidRead(field_offset, type_size)) {
        const uint64_t remaining = RemainingBytes(field_offset);

        regions.push_back(MakeBinaryRegion(
            field_offset, remaining, BinaryRegionType::Unknown, remaining, 0,
            std::string("ERROR: table field `") + field->name()->c_str() +
                "` (" +
                reflection::EnumNameBaseType(field->type()->base_type()) +
                "). Expected " + std::to_string(type_size) + " bytes."));
        continue;
      }

      regions.push_back(MakeBinaryRegion(
          field_offset, type_size, region_type, 0, 0,
          std::string("table field `") + field->name()->c_str() + "` (" +
              reflection::EnumNameBaseType(field->type()->base_type()) + ")"));
      continue;
    }

    // Read the offset
    const auto offset_from_field = ReadScalar<uint32_t>(field_offset);
    uint64_t offset_of_next_item = 0;
    const std::string offset_prefix =
        "offset to field `" + std::string(field->name()->c_str()) + "`";

    // Validate any field that isn't inline (i.e., non-structs).
    if (!IsInlineField(field)) {
      if (!offset_from_field.has_value()) {
        const uint64_t remaining = RemainingBytes(field_offset);
        regions.push_back(MakeBinaryRegion(
            field_offset, remaining, BinaryRegionType::Unknown, remaining, 0,
            std::string("ERROR: ") + offset_prefix +
                ". Incomplete binary, expected to read 4 bytes here"));
        continue;
      }

      offset_of_next_item = field_offset + offset_from_field.value();

      if (!IsValidOffset(offset_of_next_item)) {
        regions.push_back(
            MakeBinaryRegion(field_offset, sizeof(uint32_t),
                             BinaryRegionType::UOffset, 0, offset_of_next_item,
                             std::string("ERROR: ") + offset_prefix +
                                 ". Location is outside the binary."));
        continue;
      }
    }

    switch (field->type()->base_type()) {
      case reflection::BaseType::Obj: {
        const reflection::Object *next_object =
            schema_->objects()->Get(field->type()->index());

        if (next_object->is_struct()) {
          // Structs are stored inline.
          BuildStruct(field_offset, regions, next_object);
        } else {
          regions.push_back(MakeBinaryRegion(
              field_offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
              offset_of_next_item, offset_prefix + " (table)"));

          BuildTable(offset_of_next_item, BinarySectionType::Table,
                     next_object);
        }
      } break;

      case reflection::BaseType::String: {
        regions.push_back(MakeBinaryRegion(
            field_offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
            offset_of_next_item, offset_prefix + " (string)"));
        BuildString(offset_of_next_item, table, field);
      } break;

      case reflection::BaseType::Vector: {
        regions.push_back(MakeBinaryRegion(
            field_offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
            offset_of_next_item, offset_prefix + " (vector)"));
        BuildVector(offset_of_next_item, table, field, table_offset, vtable);
      } break;

      case reflection::BaseType::Union: {
        const uint64_t union_offset = offset_of_next_item;

        // The union type field is always one less than the union itself.
        const uint16_t union_type_id = field->id() - 1;

        auto vtable_field = vtable.fields.find(union_type_id);
        if (vtable_field == vtable.fields.end()) {
          // TODO(dbaileychess): need to capture this error condition.
          break;
        }

        const uint64_t type_offset =
            table_offset + vtable_field->second.offset_from_table;

        const auto realized_type = ReadScalar<uint8_t>(type_offset);
        if (!realized_type.has_value()) {
          const uint64_t remaining = RemainingBytes(type_offset);
          regions.push_back(MakeBinaryRegion(
              type_offset, remaining, BinaryRegionType::Unknown, remaining, 0,
              std::string("ERROR: ") + offset_prefix +
                  " (union). Incomplete binary, expected to read 1 byts here"));
          continue;
        }

        const std::string enum_type =
            BuildUnion(union_offset, realized_type.value(), field);

        regions.push_back(MakeBinaryRegion(
            field_offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
            union_offset,
            offset_prefix + " (union of type `" + enum_type + "`)"));

      } break;

      default: break;
    }
  }

  // Handle the case where there is padding after the last known binary
  // region. Calculate where we left off towards the expected end of the
  // table.
  const uint64_t i = regions.back().offset + regions.back().length + 1;

  if (i < table_end_offset) {
    const uint64_t pad_bytes = table_end_offset - i + 1;
    regions.push_back(MakeBinaryRegion(i - 1, pad_bytes * sizeof(uint8_t),
                                       BinaryRegionType::Uint8, pad_bytes, 0,
                                       "padding"));
  }

  AddSection(table_offset,
             MakeBinarySection(table->name()->str(), type, std::move(regions)));
}

uint64_t BinaryAnnotator::BuildStruct(uint64_t offset,
                                      std::vector<BinaryRegion> &regions,
                                      const reflection::Object *object) {
  if (!object->is_struct()) { return offset; }

  // Loop over all the fields in increasing order
  ForAllFields(object, /*reverse=*/false, [&](const reflection::Field *field) {
    if (IsScalar(field->type()->base_type())) {
      offset = BuildStructureField(offset, object, field, regions);

    } else if (field->type()->base_type() == reflection::BaseType::Obj) {
      // Structs are stored inline, even when nested.
      offset = BuildStruct(offset, regions,
                           schema_->objects()->Get(field->type()->index()));
    } else if (field->type()->base_type() == reflection::BaseType::Array) {
      // Arrays are just repeated structures.
      if (IsScalar(field->type()->element())) {
        offset = BuildArrayField(offset, object, field,
                                 field->type()->fixed_length(), regions);
      } else {
        for (uint16_t i = 0; i < field->type()->fixed_length(); ++i) {
          // TODO(dbaileychess): This works, but the comments on the fields lose
          // some context. Need to figure a way how to plumb the nested arrays
          // comments together that isn't too confusing.
          offset = BuildStruct(offset, regions,
                               schema_->objects()->Get(field->type()->index()));
        }
      }
    }
  });

  return offset;
}

void BinaryAnnotator::BuildString(uint64_t offset,
                                  const reflection::Object *table,
                                  const reflection::Field *field) {
  // Check if we have already generated this string section, and this is a
  // shared string instance.
  if (strings_.find(offset) != strings_.end()) { return; }

  std::vector<BinaryRegion> regions;
  const uint32_t string_length = GetScalar<uint32_t>(offset);

  const uint64_t string_soffset = offset;

  regions.push_back(MakeBinaryRegion(offset, sizeof(uint32_t),
                                     BinaryRegionType::Uint32, 0, 0,
                                     std::string("length of string")));
  offset += sizeof(uint32_t);

  regions.push_back(MakeBinaryRegion(offset, string_length * sizeof(char),
                                     BinaryRegionType::Char, string_length, 0,
                                     ""));
  offset += string_length * sizeof(char);

  regions.push_back(MakeBinaryRegion(offset, sizeof(char),
                                     BinaryRegionType::Char, 0, 0,
                                     std::string("string terminator")));
  offset += sizeof(char);

  sections_.insert(std::make_pair(
      string_soffset,
      MakeBinarySection(
          std::string(table->name()->c_str()) + "." + field->name()->c_str(),
          BinarySectionType::String, std::move(regions))));

  // Insert into the strings set to find possible instances of shared strings.
  strings_.insert(string_soffset);
}

void BinaryAnnotator::BuildVector(uint64_t offset,
                                  const reflection::Object *table,
                                  const reflection::Field *field,
                                  const uint64_t parent_table_offset,
                                  const VTable &vtable) {
  std::vector<BinaryRegion> regions;
  const uint32_t vector_length = GetScalar<uint32_t>(offset);

  const uint64_t vector_offset = offset;

  regions.push_back(
      MakeBinaryRegion(offset, sizeof(uint32_t), BinaryRegionType::Uint32, 0, 0,
                       std::string("length of vector (# items)")));
  offset += sizeof(uint32_t);

  switch (field->type()->element()) {
    case reflection::BaseType::Obj: {
      const reflection::Object *object =
          schema_->objects()->Get(field->type()->index());

      if (object->is_struct()) {
        // Vector of structs
        for (size_t i = 0; i < vector_length; ++i) {
          // Structs are inline to the vector.
          offset = BuildStruct(offset, regions, object);
        }
      } else {
        // Vector of objects
        for (size_t i = 0; i < vector_length; ++i) {
          // The table offset is relative from the offset location itself.
          const uint64_t table_offset = offset + GetScalar<uint32_t>(offset);

          regions.push_back(MakeBinaryRegion(
              offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
              table_offset,
              std::string("offset to table[") + std::to_string(i) + "]"));

          BuildTable(table_offset, BinarySectionType::Table, object);

          offset += sizeof(uint32_t);
        }
      }
    } break;
    case reflection::BaseType::String: {
      // Vector of strings
      for (size_t i = 0; i < vector_length; ++i) {
        // The string offset is relative from the offset location itself.
        const uint64_t string_offset = offset + GetScalar<uint32_t>(offset);

        regions.push_back(MakeBinaryRegion(
            offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
            string_offset,
            std::string("offset to string[") + std::to_string(i) + "]"));

        BuildString(string_offset, table, field);

        offset += sizeof(uint32_t);
      }
    } break;
    case reflection::BaseType::Union: {
      // Vector of unions
      // Unions have both their realized type (uint8_t for now) that are
      // stored sperately. These are stored in the field->index() - 1
      // location.
      const uint16_t union_type_vector_id = field->id() - 1;

      auto vtable_entry = vtable.fields.find(union_type_vector_id);
      if (vtable_entry == vtable.fields.end()) {
        // TODO(dbaileychess): need to capture this error condition.
        break;
      }

      const uint64_t union_type_vector_field_offset =
          parent_table_offset + vtable_entry->second.offset_from_table;

      // Get the offset to the first type (the + sizeof(uint32_t) is to skip
      // over the vector length which we already know)
      const uint64_t union_type_vector_data_offset =
          union_type_vector_field_offset +
          GetScalar<uint16_t>(union_type_vector_field_offset) +
          sizeof(uint32_t);

      for (size_t i = 0; i < vector_length; ++i) {
        // The union offset is relative from the offset location itself.
        const uint64_t union_offset = offset + GetScalar<uint32_t>(offset);

        const uint8_t realized_type = GetScalar<uint8_t>(
            union_type_vector_data_offset + i * sizeof(uint8_t));

        const std::string enum_type =
            BuildUnion(union_offset, realized_type, field);

        regions.push_back(MakeBinaryRegion(
            offset, sizeof(uint32_t), BinaryRegionType::UOffset, 0,
            union_offset,
            std::string("offset to union[") + std::to_string(i) + "] (`" +
                enum_type + "`)"));

        offset += sizeof(uint32_t);
      }
    } break;
    default: {
      if (IsScalar(field->type()->element())) {
        const BinaryRegionType binary_region_type =
            GetRegionType(field->type()->element());

        const uint64_t type_size = GetTypeSize(field->type()->element());

        // TODO(dbaileychess): It might be nicer to user the
        // BinaryRegion.array_length field to indicate this.
        for (size_t i = 0; i < vector_length; ++i) {
          regions.push_back(MakeBinaryRegion(
              offset, type_size, binary_region_type, 0, 0,
              std::string("value[") + std::to_string(i) + "]"));
          offset += type_size;
        }
      }
    } break;
  }

  sections_.insert(std::make_pair(
      vector_offset,
      MakeBinarySection(
          std::string(table->name()->c_str()) + "." + field->name()->c_str(),
          BinarySectionType::Vector, std::move(regions))));
}

std::string BinaryAnnotator::BuildUnion(uint64_t offset,
                                        const uint8_t realized_type,
                                        const reflection::Field *field) {
  const reflection::Enum *next_enum =
      schema_->enums()->Get(field->type()->index());

  const reflection::EnumVal *enum_val = next_enum->values()->Get(realized_type);

  const reflection::Type *union_type = enum_val->union_type();

  if (union_type->base_type() == reflection::BaseType::Obj) {
    const reflection::Object *object =
        schema_->objects()->Get(union_type->index());

    if (object->is_struct()) {
      // Union of vectors point to a new Binary section
      std::vector<BinaryRegion> regions;

      offset = BuildStruct(offset, regions, object);

      sections_.insert(std::make_pair(
          regions[0].offset,
          MakeBinarySection(std::string(object->name()->c_str()) + "." +
                                field->name()->c_str(),
                            BinarySectionType::Union, std::move(regions))));
    } else {
      BuildTable(offset, BinarySectionType::Table, object);
    }
  }
  // TODO(dbaileychess): handle the other union types.

  return enum_val->name()->c_str();
}

void BinaryAnnotator::FixMissingRegions() {
  std::vector<BinaryRegion> regions_to_insert;
  for (auto &current_section : sections_) {
    BinarySection &section = current_section.second;
    if (section.regions.empty()) {
      // TODO(dbaileychess): is this possible?
      continue;
    }

    uint64_t offset = section.regions[0].offset + section.regions[0].length;
    for (size_t i = 1; i < section.regions.size(); ++i) {
      BinaryRegion &region = section.regions[i];

      const uint64_t next_offset = region.offset;

      if (offset < next_offset) {
        const uint64_t padding_bytes = next_offset - offset;

        if (IsNonZeroRegion(offset, padding_bytes, binary_)) {
          regions_to_insert.push_back(MakeBinaryRegion(
              offset, padding_bytes, BinaryRegionType::Unknown, padding_bytes,
              0, "WARN: nothing refers to this region."));
        } else {
          regions_to_insert.push_back(
              MakeBinaryRegion(offset, padding_bytes, BinaryRegionType::Uint8,
                               padding_bytes, 0, "padding"));
        }
      }
      offset = next_offset + region.length;
    }

    if (!regions_to_insert.empty()) {
      section.regions.insert(section.regions.end(), regions_to_insert.begin(),
                             regions_to_insert.end());
      std::stable_sort(section.regions.begin(), section.regions.end(),
                       BinaryRegionSort);
      regions_to_insert.clear();
    }
  }
}

void BinaryAnnotator::FixMissingSections() {
  uint64_t offset = 0;

  std::vector<BinarySection> sections_to_insert;

  for (auto &current_section : sections_) {
    BinarySection &section = current_section.second;
    const uint64_t section_start_offset = current_section.first;
    const uint64_t section_end_offset =
        section.regions.back().offset + section.regions.back().length;

    if (offset < section_start_offset) {
      // We are at an offset that is less then the current section.
      const uint64_t pad_bytes = section_start_offset - offset + 1;

      sections_to_insert.push_back(
          GenerateMissingSection(offset - 1, pad_bytes, binary_));
    }
    offset = section_end_offset + 1;
  }

  // Handle the case where there are still bytes left in the binary that are
  // unaccounted for.
  if (offset < binary_length_) {
    const uint64_t pad_bytes = binary_length_ - offset + 1;
    sections_to_insert.push_back(
        GenerateMissingSection(offset - 1, pad_bytes, binary_));
  }

  for (const BinarySection &section_to_insert : sections_to_insert) {
    sections_.insert(
        std::make_pair(section_to_insert.regions[0].offset, section_to_insert));
  }
}

}  // namespace flatbuffers