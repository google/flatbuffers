#include <iostream>

#include "flatbuffers/util.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: reflection_debug <path to fuzzer input file>\n";
    return 0;
  }
  std::string input_file_name(argv[1]);
  std::string input_file_data;
  auto done =
      flatbuffers::LoadFile(input_file_name.c_str(), true, &input_file_data);
  if (!done) {
    std::cerr << "Can not load file: '" << input_file_name << "'";
    return -1;
  }
  if (input_file_data.size() < 8) {
    std::cerr << "Invalid file data: '" << input_file_data << "'";
    return -2;
  }
  auto rc = LLVMFuzzerTestOneInput(
      reinterpret_cast<const uint8_t*>(input_file_data.data()),
      input_file_data.size());
  std::cout << "LLVMFuzzerTestOneInput finished with code " << rc << "\n\n";
  return rc;
}
