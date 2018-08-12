#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include <clocale>
#include <memory>
#include <regex>
#include <string>

#include "flatbuffers/idl.h"

#define flags_scalar_type 0x0F  // type of scalar value
#define flags_quotes_kind 0x10  // quote " or '
#define flags_json_bracer 0x20  // {} or []
#define flags_0x40 0x40         // reserved flag
#define flags_clocale 0x80      // change default C-locale

#define TEST_OUTPUT_LINE(...)     \
  {                               \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n");        \
  }

static void TestFail(const char *expval, const char *val, const char *exp,
                     const char *file, int line) {
  TEST_OUTPUT_LINE("VALUE: \"%s\"", expval);
  TEST_OUTPUT_LINE("EXPECTED: \"%s\"", val);
  TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s", file, line, exp);
  assert(0);
}

static void TestEqStr(const char *expval, const char *val, const char *exp,
                      const char *file, int line) {
  if (strcmp(expval, val) != 0) { TestFail(expval, val, exp, file, line); }
}

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
  if (U(expval) != val) {
    TestFail(flatbuffers::NumToString(expval).c_str(),
             flatbuffers::NumToString(val).c_str(), exp, file, line);
  }
}

#define TEST_EQ(exp, val) TestEq(exp, val, #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(exp == NULL, false, #exp, __FILE__, __LINE__)
#define TEST_EQ_STR(exp, val) TestEqStr(exp, val, #exp, __FILE__, __LINE__)

static void BreakSequence(std::string &s, const char *subj, char repl) {
  size_t pos = 0;
  while (pos = s.find(subj, pos), pos != std::string::npos) s.at(pos) = repl;
}

static std::string strip_string(const std::string &s, const char *pattern,
                                size_t *pos = nullptr) {
  if (pos) *pos = 0;
  // leading
  auto first = s.find_first_not_of(pattern);
  if (std::string::npos == first) return "";
  if (pos) *pos = first;
  // trailing
  auto last = s.find_last_not_of(pattern);
  assert(last < s.length());
  assert(first <= last);
  return s.substr(first, last - first + 1);
}

class RegexMatcher {
 protected:
  virtual bool match_number(const std::string &input) = 0;

 public:
  virtual ~RegexMatcher() = default;

  struct MatchResult {
    size_t pos{ 0 };
    size_t len{ 0 };
    bool res{ false };
    bool quoted{ false };
  };

  MatchResult math(const std::string &input) {
    MatchResult r;
    // strip leading and trailing "spaces" accepted by flatbuffer
    auto test = strip_string(input, "\t\r\n ", &r.pos);
    r.len = test.size();
    // check quotes
    if (test.size() >= 2) {
      auto fch = test.front();
      auto lch = test.back();
      r.quoted = (fch == lch) && (fch == '\'' || fch == '\"');
      if (r.quoted) {
        // remove quotes for regex test
        test = test.substr(1, test.size() - 2);
      }
    }
    // Fast check:
    // A string with valid scalar shouldn't have non-ascii symbols.
    // If this string had comments "/**/"" or "//" they should be broken before
    // regex matching.
    const auto bad =
        (test.end() != std::find_if(test.begin(), test.end(), [](char c) {
           // locale-independent check, isascii is deprecated
           auto u = static_cast<unsigned char>(c);
           // from ' ' to `~` (ignore DEL)
           return !(0x20 <= u && u < 0x7F);
         }));
    if (!test.empty() && !bad) { r.res = match_number(test); }
    return r;
  }

  bool match_regex_list(const std::string &input,
                        const std::vector<std::regex> &re_list) {
    auto str = strip_string(input, " ");
    if (str.empty()) return false;
    for (auto &re : re_list) {
      std::smatch match;
      auto done = std::regex_match(str, match, re);
      if (done) return true;
    }
    return false;
  }
};

class IntegerRegex : public RegexMatcher {
 protected:
  bool match_number(const std::string &input) override {
    static const std::vector<std::regex> re_list = {
      std::regex{ R"(^[-+]?[0-9]+$)", std::regex_constants::optimize },

      std::regex{
          R"(^[-+]?0[xX][0-9a-fA-F]+$)", std::regex_constants::optimize }
    };
    return match_regex_list(input, re_list);
  }

 public:
  IntegerRegex() = default;
  virtual ~IntegerRegex() = default;
};

class UIntegerRegex : public RegexMatcher {
 protected:
  bool match_number(const std::string &input) override {
    static const std::vector<std::regex> re_list = {
      std::regex{ R"(^[+]?[0-9]+$)", std::regex_constants::optimize },
      std::regex{
          R"(^[+]?0[xX][0-9a-fA-F]+$)", std::regex_constants::optimize },
      // accept -0 number
      std::regex{ R"(^[-](?:0[xX])?0+$)", std::regex_constants::optimize }
    };
    return match_regex_list(input, re_list);
  }

 public:
  UIntegerRegex() = default;
  virtual ~UIntegerRegex() = default;
};

class BooleanRegex : public IntegerRegex {
 protected:
  bool match_number(const std::string &input) override {
    if (input == "true" || input == "false") return true;
    return IntegerRegex::match_number(input);
  }

 public:
  BooleanRegex() = default;
  virtual ~BooleanRegex() = default;
};

class FloatRegex : public RegexMatcher {
 protected:
  bool match_number(const std::string &input) override {
    static const std::vector<std::regex> re_list = {
      // hex-float
      std::regex{
          R"(^[-+]?0[xX](?:(?:[.][0-9a-fA-F]+)|(?:[0-9a-fA-F]+[.][0-9a-fA-F]*)|(?:[0-9a-fA-F]+))[pP][-+]?[0-9]+$)",
          std::regex_constants::optimize },
      // dec-float
      std::regex{
          R"(^[-+]?(?:(?:[.][0-9]+)|(?:[0-9]+[.][0-9]*)|(?:[0-9]+))(?:[eE][-+]?[0-9]+)?$)",
          std::regex_constants::optimize },

      std::regex{ R"(^[-+]?(?:nan|inf|infinity)$)",
                  std::regex_constants::optimize | std::regex_constants::icase }
    };
    return match_regex_list(input, re_list);
  }

 public:
  FloatRegex() = default;
  virtual ~FloatRegex() = default;
};

std::pair<std::string, RegexMatcher::MatchResult> GetScalarType(
    uint8_t flags, const std::string &input) {
  std::string type = "";
  std::unique_ptr<RegexMatcher> matcher;

  switch (flags & flags_scalar_type) {
    case 0x0: {
      type = "float";
      matcher.reset(new FloatRegex());
      break;
    }
    case 0x1: {
      type = "double";
      matcher.reset(new FloatRegex());
      break;
    }
    case 0x2: {
      type = "int8";
      matcher.reset(new IntegerRegex());
      break;
    }
    case 0x3: {
      type = "uint8";
      matcher.reset(new UIntegerRegex());
      break;
    }
    case 0x4: {
      type = "int16";
      matcher.reset(new IntegerRegex());
      break;
    }
    case 0x5: {
      type = "uint16";
      matcher.reset(new UIntegerRegex());
      break;
    }
    case 0x6: {
      type = "int32";
      matcher.reset(new IntegerRegex());
      break;
    }
    case 0x7: {
      type = "uint32";
      matcher.reset(new UIntegerRegex());
      break;
    }
    case 0x8: {
      type = "int64";
      matcher.reset(new IntegerRegex());
      break;
    }
    case 0x9: {
      type = "uint64";
      matcher.reset(new UIntegerRegex());
      break;
    }
    case 0xA: {
      type = "bool";
      matcher.reset(new BooleanRegex());
      break;
    }
    default: {
      type = "float";
      matcher.reset(new FloatRegex());
      break;
    }
  }
  auto check = matcher->math(input);
  return { type, check };
}

static bool ParseTest(flatbuffers::Parser &parser, const std::string &json,
                      std::string *_text) {
  auto done = parser.Parse(json.c_str());
  if (done) {
    TEST_EQ(GenerateText(parser, parser.builder_.GetBufferPointer(), _text),
            true);
  } else {
    *_text = parser.error_;
  }
  return done;
}

// llvm std::regex have problem with stack overflow, limit maximum length.
// ./scalar_fuzzer -max_len=3000 -timeout=10 ../.corpus/ ../.seed/
// Flag -only_ascii=1 is usefull for fast number-compatibilty checking.
// Additional flags: -reduce_depth=1 -use_value_profile=1 -shrink=1
// Help: -help=1
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Reserve one byte for Parser flags.
  if (size < 1) return 0;
  // REMEMBER: the first character in crash dump is not part of input!
  // Extract single byte for fuzzing flags value.
  const uint8_t flags = data[0];
  data += 1;  // move to next
  size -= 1;

// Change default ASCII locale (affects to isalpha, isalnum, decimal
// delimiters, other). https://en.cppreference.com/w/cpp/locale/setlocale
#if defined(FLATBUFFERS_FORCE_LOCALE_INDEPENDENT)
  if (flags & flags_clocale) {
    // The ru_RU.CP1251 use ',' as decimal point delimiter instead of '.'.
    // std::to_string(12.0) will return "12,0000".
    // Ubuntu:>sudo locale-gen ru_RU.CP1251
    assert(std::setlocale(LC_ALL, FLATBUFFERS_FORCE_LOCALE_INDEPENDENT));
  }
#endif

  // Guarantee 0-termination.
  const std::string original(reinterpret_cast<const char *>(data), size);
  auto input = std::string(original.c_str());  // until '\0'

  if (input.empty()) return 0;
  // Break comments in json to avoid complexity with regex matcher.
  BreakSequence(input, "//", '@');  // "//" -> "@/"
  BreakSequence(input, "/*", '@');  // "/*" -> "@*"

  // Detect type of scalar and check input with regex patterns.
  const auto atype = GetScalarType(flags, input);
  const auto &recheck = atype.second;

  // Create parser
  flatbuffers::IDLOptions opts;
  opts.force_defaults = true;
  opts.output_default_scalars_in_json = true;
  opts.indent_step = -1;
  opts.strict_json = true;
  flatbuffers::Parser parser(opts);
  auto schema = "table X { Y: " + atype.first + "; } root_type X;";
  TEST_EQ(parser.Parse(schema.c_str()), true);

  // Parse original input as-is.
  auto orig_scalar = "{ \"Y\" : " + input + " }";
  std::string orig_back;
  auto orig_done = ParseTest(parser, orig_scalar, &orig_back);
  if (recheck.res != orig_done) {
    // look for "does not fit" or "doesn't fit" or "out of range"
    if ((orig_back.find("does not fit") == std::string::npos) &&
        (orig_back.find("out of range") == std::string::npos)) {
      // Print error message for debug purpose.
      TEST_OUTPUT_LINE("Original FAILED: %d != %d", orig_done, recheck.res);
      TEST_EQ_STR(orig_back.c_str(),
                  input.substr(recheck.pos, recheck.len).c_str());
      TEST_EQ(orig_done, recheck.res);
    }
  }

#if defined(FLATBUFFERS_FORCE_LOCALE_INDEPENDENT)
  // restore locale to default
  if (flags & flags_clocale) {
    assert(std::setlocale(LC_ALL, "C"));
    std::string orig_back_c;
    auto orig_done_c = ParseTest(parser, orig_scalar, &orig_back_c);
    TEST_EQ_STR(orig_back_c.c_str(), orig_back.c_str());
    TEST_EQ(orig_done_c, orig_done);
  }
#endif

  bool fixed = false;
  if (true == recheck.quoted) {
    // we can't simply remove quotes, it maybe nested "'12'".
    // original string "\'12\'" converted to "'12'".
    // The string maybe an invalid string by JSON rules but after quotes removed
    // can be valid.
    assert(recheck.len >= 2);
  } else {
    const auto quote = (flags & flags_quotes_kind) ? '\"' : '\'';
    input.insert(recheck.pos + recheck.len, 1, quote);
    input.insert(recheck.pos, 1, quote);
    fixed = true;
  }

  if (fixed) {
    auto fix_scalar = "{ \"Y\" : " + input + " }";
    std::string fix_back;
    auto fix_done = ParseTest(parser, fix_scalar, &fix_back);
    if (orig_done != fix_done) {
      // Print error message for debug purpose.
      TEST_OUTPUT_LINE("Fix FAILED: %d != %d", fix_done, orig_done);
      TEST_EQ_STR(fix_back.c_str(), orig_back.c_str());
    }
    if (orig_done) { TEST_EQ_STR(fix_back.c_str(), orig_back.c_str()); }
    TEST_EQ(fix_done, orig_done);
  }
  return 0;
}
