#ifndef TESTS_PROTO_TEST_H
#define TESTS_PROTO_TEST_H

#include <string>

#include "flatbuffers/idl.h"
namespace flatbuffers {
namespace tests {

void RunTest(const flatbuffers::IDLOptions &opts, const std::string &protopath, const std::string &protofile,
        const std::string &goldenfile, const std::string importprotofile = {});
void proto_test(std::string protopath, const std::string &protofile);
void proto_test_union(std::string protopath, const std::string &protofile);
void proto_test_union_suffix(std::string protopath, const std::string &protofile);
void proto_test_include(std::string protopath, const std::string &protofile, const std::string &importprotofile);
void proto_test_include_union(std::string protopath, std::string protofile, std::string importprotofile);

void proto_test_id(std::string protopath, const std::string &protofile);
void proto_test_union_id(std::string protopath, const std::string &protofile);
void proto_test_union_suffix_id(std::string protopath, const std::string &protofile);
void proto_test_include_id(std::string protopath, const std::string &protofile, const std::string &importprotofile);
void proto_test_include_union_id(std::string protopath, std::string protofile, std::string importprotofile);

void ParseProtoTest(const std::string& tests_data_path);
void ParseProtoBufAsciiTest();

}  // namespace tests
}  // namespace flatbuffers

#endif
