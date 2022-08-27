#include "parser_test.h"

#include <string>

#include "flatbuffers/idl.h"
#include "test_assert.h"

namespace flatbuffers {
namespace tests {
namespace {

// Test that parser errors are actually generated.
static void TestError_(const char *src, const char *error_substr, bool strict_json,
                const char *file, int line, const char *func) {
  flatbuffers::IDLOptions opts;
  opts.strict_json = strict_json;
  flatbuffers::Parser parser(opts);
  if (parser.Parse(src)) {
    TestFail("true", "false",
             ("parser.Parse(\"" + std::string(src) + "\")").c_str(), file, line,
             func);
  } else if (!strstr(parser.error_.c_str(), error_substr)) {
    TestFail(error_substr, parser.error_.c_str(),
             ("parser.Parse(\"" + std::string(src) + "\")").c_str(), file, line,
             func);
  }
}

static void TestError_(const char *src, const char *error_substr, const char *file,
                int line, const char *func) {
  TestError_(src, error_substr, false, file, line, func);
}

#ifdef _WIN32
#  define TestError(src, ...) \
    TestError_(src, __VA_ARGS__, __FILE__, __LINE__, __FUNCTION__)
#else
#  define TestError(src, ...) \
    TestError_(src, __VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

} // namespace

// Test that parsing errors occur as we'd expect.
// Also useful for coverage, making sure these paths are run.
void ErrorTest() {
  // In order they appear in idl_parser.cpp
  TestError("table X { Y:byte; } root_type X; { Y: 999 }", "does not fit");
  TestError("\"\0", "illegal");
  TestError("\"\\q", "escape code");
  TestError("table ///", "documentation");
  TestError("@", "illegal");
  TestError("table 1", "expecting");
  TestError("table X { Y:[[int]]; }", "nested vector");
  TestError("table X { Y:1; }", "illegal type");
  TestError("table X { Y:int; Y:int; }", "field already");
  TestError("table Y {} table X { Y:int; }", "same as table");
  TestError("struct X { Y:string; }", "only scalar");
  TestError("struct X { a:uint = 42; }", "default values");
  TestError("enum Y:byte { Z = 1 } table X { y:Y; }", "not part of enum");
  TestError("struct X { Y:int (deprecated); }", "deprecate");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y: {}, A:1 }",
            "missing type field");
  TestError("union Z { X } table X { Y:Z; } root_type X; { Y_type: 99, Y: {",
            "type id");
  TestError("table X { Y:int; } root_type X; { Z:", "unknown field");
  TestError("table X { Y:int; } root_type X; { Y:", "string constant", true);
  TestError("table X { Y:int; } root_type X; { \"Y\":1, }", "string constant",
            true);
  TestError(
      "struct X { Y:int; Z:int; } table W { V:X; } root_type W; "
      "{ V:{ Y:1 } }",
      "wrong number");
  TestError("enum E:byte { A } table X { Y:E; } root_type X; { Y:U }",
            "unknown enum value");
  TestError("table X { Y:byte; } root_type X; { Y:; }", "starting");
  TestError("enum X:byte { Y } enum X {", "enum already");
  TestError("enum X:float {}", "underlying");
  TestError("enum X:byte { Y, Y }", "value already");
  TestError("enum X:byte { Y=2, Z=2 }", "unique");
  TestError("table X { Y:int; } table X {", "datatype already");
  TestError("table X { } union X { }", "datatype already");
  TestError("union X { } table X { }", "datatype already");
  TestError("namespace A; table X { } namespace A; union X { }",
            "datatype already");
  TestError("namespace A; union X { } namespace A; table X { }",
            "datatype already");
  TestError("struct X (force_align: 7) { Y:int; }", "force_align");
  TestError("struct X {}", "size 0");
  TestError("{}", "no root");
  TestError("table X { Y:byte; } root_type X; { Y:1 } { Y:1 }", "end of file");
  TestError("table X { Y:byte; } root_type X; { Y:1 } table Y{ Z:int }",
            "end of file");
  TestError("root_type X;", "unknown root");
  TestError("struct X { Y:int; } root_type X;", "a table");
  TestError("union X { Y }", "referenced");
  TestError("union Z { X } struct X { Y:int; }", "only tables");
  TestError("table X { Y:[int]; YLength:int; }", "clash");
  TestError("table X { Y:byte; } root_type X; { Y:1, Y:2 }", "more than once");
  // float to integer conversion is forbidden
  TestError("table X { Y:int; } root_type X; { Y:1.0 }", "float");
  TestError("table X { Y:bool; } root_type X; { Y:1.0 }", "float");
  TestError("enum X:bool { Y = true }", "must be integral");
  // Array of non-scalar
  TestError("table X { x:int; } struct Y { y:[X:2]; }",
            "may contain only scalar or struct fields");
  // Non-snake case field names
  TestError("table X { Y: int; } root_type Y: {Y:1.0}", "snake_case");
  // Complex defaults
  TestError("table X { y: string = 1; }", "expecting: string");
  TestError("table X { y: string = []; }", " Cannot assign token");
  TestError("table X { y: [int] = [1]; }", "Expected `]`");
  TestError("table X { y: [int] = [; }", "Expected `]`");
  TestError("table X { y: [int] = \"\"; }", "type mismatch");
  // An identifier can't start from sign (+|-)
  TestError("table X { -Y: int; } root_type Y: {Y:1.0}", "identifier");
  TestError("table X { +Y: int; } root_type Y: {Y:1.0}", "identifier");
}

void EnumOutOfRangeTest() {
  TestError("enum X:byte { Y = 128 }", "enum value does not fit");
  TestError("enum X:byte { Y = -129 }", "enum value does not fit");
  TestError("enum X:byte { Y = 126, Z0, Z1 }", "enum value does not fit");
  TestError("enum X:ubyte { Y = -1 }", "enum value does not fit");
  TestError("enum X:ubyte { Y = 256 }", "enum value does not fit");
  TestError("enum X:ubyte { Y = 255, Z }", "enum value does not fit");
  TestError("table Y{} union X { Y = -1 }", "enum value does not fit");
  TestError("table Y{} union X { Y = 256 }", "enum value does not fit");
  TestError("table Y{} union X { Y = 255, Z:Y }", "enum value does not fit");
  TestError("enum X:int { Y = -2147483649 }", "enum value does not fit");
  TestError("enum X:int { Y = 2147483648 }", "enum value does not fit");
  TestError("enum X:uint { Y = -1 }", "enum value does not fit");
  TestError("enum X:uint { Y = 4294967297 }", "enum value does not fit");
  TestError("enum X:long { Y = 9223372036854775808 }", "does not fit");
  TestError("enum X:long { Y = 9223372036854775807, Z }",
            "enum value does not fit");
  TestError("enum X:ulong { Y = -1 }", "does not fit");
  TestError("enum X:ubyte (bit_flags) { Y=8 }", "bit flag out");
  TestError("enum X:byte (bit_flags) { Y=7 }", "must be unsigned");  // -128
  // bit_flgs out of range
  TestError("enum X:ubyte (bit_flags) { Y0,Y1,Y2,Y3,Y4,Y5,Y6,Y7,Y8 }",
            "out of range");
}

void IntegerOutOfRangeTest() {
  TestError("table T { F:byte; } root_type T; { F:128 }",
            "constant does not fit");
  TestError("table T { F:byte; } root_type T; { F:-129 }",
            "constant does not fit");
  TestError("table T { F:ubyte; } root_type T; { F:256 }",
            "constant does not fit");
  TestError("table T { F:ubyte; } root_type T; { F:-1 }",
            "constant does not fit");
  TestError("table T { F:short; } root_type T; { F:32768 }",
            "constant does not fit");
  TestError("table T { F:short; } root_type T; { F:-32769 }",
            "constant does not fit");
  TestError("table T { F:ushort; } root_type T; { F:65536 }",
            "constant does not fit");
  TestError("table T { F:ushort; } root_type T; { F:-1 }",
            "constant does not fit");
  TestError("table T { F:int; } root_type T; { F:2147483648 }",
            "constant does not fit");
  TestError("table T { F:int; } root_type T; { F:-2147483649 }",
            "constant does not fit");
  TestError("table T { F:uint; } root_type T; { F:4294967296 }",
            "constant does not fit");
  TestError("table T { F:uint; } root_type T; { F:-1 }",
            "constant does not fit");
  // Check fixed width aliases
  TestError("table X { Y:uint8; } root_type X; { Y: -1 }", "does not fit");
  TestError("table X { Y:uint8; } root_type X; { Y: 256 }", "does not fit");
  TestError("table X { Y:uint16; } root_type X; { Y: -1 }", "does not fit");
  TestError("table X { Y:uint16; } root_type X; { Y: 65536 }", "does not fit");
  TestError("table X { Y:uint32; } root_type X; { Y: -1 }", "");
  TestError("table X { Y:uint32; } root_type X; { Y: 4294967296 }",
            "does not fit");
  TestError("table X { Y:uint64; } root_type X; { Y: -1 }", "");
  TestError("table X { Y:uint64; } root_type X; { Y: -9223372036854775809 }",
            "does not fit");
  TestError("table X { Y:uint64; } root_type X; { Y: 18446744073709551616 }",
            "does not fit");

  TestError("table X { Y:int8; } root_type X; { Y: -129 }", "does not fit");
  TestError("table X { Y:int8; } root_type X; { Y: 128 }", "does not fit");
  TestError("table X { Y:int16; } root_type X; { Y: -32769 }", "does not fit");
  TestError("table X { Y:int16; } root_type X; { Y: 32768 }", "does not fit");
  TestError("table X { Y:int32; } root_type X; { Y: -2147483649 }", "");
  TestError("table X { Y:int32; } root_type X; { Y: 2147483648 }",
            "does not fit");
  TestError("table X { Y:int64; } root_type X; { Y: -9223372036854775809 }",
            "does not fit");
  TestError("table X { Y:int64; } root_type X; { Y: 9223372036854775808 }",
            "does not fit");
  // check out-of-int64 as int8
  TestError("table X { Y:int8; } root_type X; { Y: -9223372036854775809 }",
            "does not fit");
  TestError("table X { Y:int8; } root_type X; { Y: 9223372036854775808 }",
            "does not fit");

  // Check default values
  TestError("table X { Y:int64=-9223372036854775809; } root_type X; {}",
            "does not fit");
  TestError("table X { Y:int64= 9223372036854775808; } root_type X; {}",
            "does not fit");
  TestError("table X { Y:uint64; } root_type X; { Y: -1 }", "");
  TestError("table X { Y:uint64=-9223372036854775809; } root_type X; {}",
            "does not fit");
  TestError("table X { Y:uint64= 18446744073709551616; } root_type X; {}",
            "does not fit");
}

void InvalidFloatTest() {
  auto invalid_msg = "invalid number";
  auto comma_msg = "expecting: ,";
  TestError("table T { F:float; } root_type T; { F:1,0 }", "");
  TestError("table T { F:float; } root_type T; { F:. }", "");
  TestError("table T { F:float; } root_type T; { F:- }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:+ }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:-. }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:+. }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:.e }", "");
  TestError("table T { F:float; } root_type T; { F:-e }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:+e }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:-.e }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:+.e }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:-e1 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:+e1 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.0e+ }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.0e- }", invalid_msg);
  // exponent pP is mandatory for hex-float
  TestError("table T { F:float; } root_type T; { F:0x0 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:-0x. }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x. }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0Xe }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"0Xe\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"nan(1)\" }", invalid_msg);
  // eE not exponent in hex-float!
  TestError("table T { F:float; } root_type T; { F:0x0.0e+ }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0e- }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0p }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0p+ }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0p- }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0pa1 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0e+ }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0e- }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0e+0 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0e-0 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0ep+ }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:0x0.0ep- }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.2.3 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.2.e3 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.2e.3 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.2e0.3 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.2e3. }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.2e3.0 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:+-1.0 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.0e+-1 }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"1.0e+-1\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:1.e0e }", comma_msg);
  TestError("table T { F:float; } root_type T; { F:0x1.p0e }", comma_msg);
  TestError("table T { F:float; } root_type T; { F:\" 0x10 \" }", invalid_msg);
  // floats in string
  TestError("table T { F:float; } root_type T; { F:\"1,2.\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"1.2e3.\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"0x1.p0e\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"0x1.0\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\" 0x1.0\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:\"+ 0\" }", invalid_msg);
  // disable escapes for "number-in-string"
  TestError("table T { F:float; } root_type T; { F:\"\\f1.2e3.\" }", "invalid");
  TestError("table T { F:float; } root_type T; { F:\"\\t1.2e3.\" }", "invalid");
  TestError("table T { F:float; } root_type T; { F:\"\\n1.2e3.\" }", "invalid");
  TestError("table T { F:float; } root_type T; { F:\"\\r1.2e3.\" }", "invalid");
  TestError("table T { F:float; } root_type T; { F:\"4\\x005\" }", "invalid");
  TestError("table T { F:float; } root_type T; { F:\"\'12\'\" }", invalid_msg);
  // null is not a number constant!
  TestError("table T { F:float; } root_type T; { F:\"null\" }", invalid_msg);
  TestError("table T { F:float; } root_type T; { F:null }", invalid_msg);
}

void UnicodeInvalidSurrogatesTest() {
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800\"}",
      "unpaired high surrogate");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800abcd\"}",
      "unpaired high surrogate");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800\\n\"}",
      "unpaired high surrogate");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uD800\\uD800\"}",
      "multiple high surrogates");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\\uDC00\"}",
      "unpaired low surrogate");
}

void InvalidUTF8Test() {
  // "1 byte" pattern, under min length of 2 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\x80\"}",
      "illegal UTF-8 sequence");
  // 2 byte pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xDF\"}",
      "illegal UTF-8 sequence");
  // 3 byte pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xEF\xBF\"}",
      "illegal UTF-8 sequence");
  // 4 byte pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF7\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "5 byte" pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFB\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "6 byte" pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFD\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "7 byte" pattern, string too short
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFE\xBF\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "5 byte" pattern, over max length of 4 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFB\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "6 byte" pattern, over max length of 4 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFD\xBF\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");
  // "7 byte" pattern, over max length of 4 bytes
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xFE\xBF\xBF\xBF\xBF\xBF\xBF\"}",
      "illegal UTF-8 sequence");

  // Three invalid encodings for U+000A (\n, aka NEWLINE)
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xC0\x8A\"}",
      "illegal UTF-8 sequence");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xE0\x80\x8A\"}",
      "illegal UTF-8 sequence");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF0\x80\x80\x8A\"}",
      "illegal UTF-8 sequence");

  // Two invalid encodings for U+00A9 (COPYRIGHT SYMBOL)
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xE0\x81\xA9\"}",
      "illegal UTF-8 sequence");
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF0\x80\x81\xA9\"}",
      "illegal UTF-8 sequence");

  // Invalid encoding for U+20AC (EURO SYMBOL)
  TestError(
      "table T { F:string; }"
      "root_type T;"
      "{ F:\"\xF0\x82\x82\xAC\"}",
      "illegal UTF-8 sequence");

  // UTF-16 surrogate values between U+D800 and U+DFFF cannot be encoded in
  // UTF-8
  TestError(
      "table T { F:string; }"
      "root_type T;"
      // U+10400 "encoded" as U+D801 U+DC00
      "{ F:\"\xED\xA0\x81\xED\xB0\x80\"}",
      "illegal UTF-8 sequence");

  // Check independence of identifier from locale.
  std::string locale_ident;
  locale_ident += "table T { F";
  locale_ident += static_cast<char>(-32);  // unsigned 0xE0
  locale_ident += " :string; }";
  locale_ident += "root_type T;";
  locale_ident += "{}";
  TestError(locale_ident.c_str(), "");
}

}  // namespace tests
}  // namespace flatbuffers
