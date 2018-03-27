#include "flatbuffers/base.h"

#include "flatbuffers/util_base64.h"

namespace flatbuffers {

enum Base64Mode {
  // a Field doesn't have base64/base64url attribute.
  Base64ModeNotSet = 0,

  // Attribute (base64): standard RFC4648 alphabet.
  // Padding is optional for the decoder.
  // Valid input strings:
  //  {"/43+AergFA==", "/43+AergFA"}.
  // The encoder output always with padding if required:
  // [255, 141, 254, 1, 234, 224, 20] => "/43+AergFA==".
  Base64ModeStandard = 1,

  // Attribute (base64url): RFC4648 with URL and Filename Safe Alphabet.
  // Padding is optional for the decoder.
  // The decoder accepts both alphabets: UrlSafe and Standard.
  // Valid input strings:
  //  {"/43+AergFA==", "/43+AergFA", "_43-AergFA==", "_43-AergFA"}.
  // The encoder output always with padding if required:
  // [255, 141, 254, 1, 234, 224, 20] encoded to "_43-AergFA==".
  Base64ModeUrlSafe = 2,

  // Extension of (base64url) attribute.
  // The decoder is the same as for Base64ModeUrlSafe.
  // The encoder omits a padding for output sequence.
  // [255, 141, 254, 1, 234, 224, 20] encoded to "_43-AergFA".
  Base64ModeUrlSafeWithoutPadding = 3
};

// Base64 Decoder implementaton.
// 1) If condition ((nullptr == dst) && (0 == dst_size)) is satisfied,
// returns the number of bytes, need to save a decoded result.
// 2) Returns the number of bytes written to the destination memory.
// Returned Zero indicates nothing was written.
// Error condition: (src_size!=0 && ret==0).
static size_t B64DecodeImpl(const int mode, const char *const src,
                            const size_t src_size, uint8_t *const dst,
                            const size_t dst_size,
                            size_t *const error_position) {
  // Standard base64 table (RFC 4648).
  static const uint8_t strict_table[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64,
    64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64
  };

  // Mixed table with URL and Filename Safe Alphabet and Standard symbols.
  static const uint8_t url_table[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 62, 64, 62, 64, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64,
    63, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64
  };
  *error_position = 0;
  if (0 == src_size) return 0;
  const auto b64_tbl = (mode == Base64ModeStandard) ? strict_table : url_table;
  // The base64 decoder transforms 4 encoded characters to 3 data bytes.
  // Prepare stage:
  // Number of perfect char[4] blocks.
  auto C4full = src_size / 4;
  // Number of remained characters in the last char[4] block.
  auto C4rem = src_size % 4;
  // If C4rem==0, then move the last char[4] block to remainder.
  if (C4rem == 0) {
    C4full = C4full - 1;
    C4rem = 4;
  }
  // The C4rem is strictly positive value from: {1,2,3,4}.
  // Build char[4] perfect block for the remainder.
  // If padding is mandatory, preset incomplete characters as invalid symbols
  // ('#'). If it is optional, preset as zero symbols ('A').
  // Now, padding is optional for the decoder.
  char last_enc4[4] = { 'A', 'A', 'A', 'A' };
  uint8_t last_dec3[3];
  // Make the local copy of the remainder.
  memcpy(&last_enc4[0], &src[C4full * 4], C4rem);
  // Size of the last decoded block can be 1,2 or 3.
  auto last_dec_len = C4rem - 1;
  // The input sequence can have padding symbols '=' in the last char[4] block.
  // In this case are allowed only two variants: {x,y,=,=} or {x,y,z,=}.
  // Replace '=' by 'zeros' and recalculate decoded size of the last block.
  if (last_enc4[3] == '=') {
    last_dec_len = 2;
    last_enc4[3] = 'A';
    if (last_enc4[2] == '=') {
      last_dec_len = 1;
      last_enc4[2] = 'A';
    }
  }
  // Size of the decoded sequence if it has no errors.
  const auto decoded_len = (C4full * 3 + last_dec_len);
  // If requested the only the number of required bytes for decoding, return it.
  if ((nullptr == dst) && (0 == dst_size)) return decoded_len;
  // Insufficient memory test.
  if (dst_size < decoded_len) return 0;

  // Main decode loop:
  // At the first iteration decode the last block (remainder).
  auto dst_ = &last_dec3[0];
  const char *src_ = &last_enc4[0];
  // Loop counter: the last block plus all perfect blocks.
  auto loop_cnt = (1 + C4full);
  // Error mask can take only two states: (0)-ok and (1)-fail.
  // Type of <err_mask> must match with unsigned <loop_cnt>.
  auto err_mask = loop_cnt * 0;
  // Use (err_mask - 1) = {0,~0} as conditional gate for the loop_cnt.
  for (size_t k = 0; loop_cnt & (err_mask - 1); loop_cnt--, k++) {
    const auto a0 = b64_tbl[static_cast<uint8_t>(src_[0])];
    const auto a1 = b64_tbl[static_cast<uint8_t>(src_[1])];
    const auto a2 = b64_tbl[static_cast<uint8_t>(src_[2])];
    const auto a3 = b64_tbl[static_cast<uint8_t>(src_[3])];
    // Decode by RFC4648 algorithm.
    dst_[0] = ((a0 << 2) | (a1 >> 4)) & 0xff;
    dst_[1] = ((a1 << 4) | (a2 >> 2)) & 0xff;
    dst_[2] = ((a2 << 6) | (a3 >> 0)) & 0xff;
    // Extract 0x40 bit from all decoded bytes and transform to {0,1}.
    err_mask = ((a0 | a1 | a2 | a3) >> 6);
    src_ = src + (k * 4);
    dst_ = dst + (k * 3);
  }

  const auto done = (0 == err_mask);
  if (done) {
    // Copy the decoded remainder to the destination.
    memcpy(&dst[C4full * 3], &last_dec3[0], last_dec_len);
    *error_position = src_size;
  } else {
    // If an error detected:
    // Number of blocks processed and written to [src] memory:
    const auto indx = C4full - loop_cnt;
    // Reject written data.
    memset(dst, 0, indx * 3);
    // Save the position of corrupted character as left closed boundary
    *error_position = 4 * (indx ? (indx - 1) : C4full) + 3;
  }
  return done ? decoded_len : 0;
}

// Base64 Encoder implementaton.
// 1) If condition ((nullptr == dst) && (0 == dst_size)) is satisfied,
// returns the number of bytes, need to save a encoded result.
// 2) Returns the number of bytes written to the destination memory.
// Returned Zero indicates nothing was written.
// Error condition: (src_size!=0 && ret==0).
static size_t B64EncodeImpl(const int mode, const uint8_t *const src,
                            const size_t src_size, char *const dst,
                            const size_t dst_size) {
  static const char strict_table[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
  };

  static const char url_table[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
  };
  if (0 == src_size) return 0;
  const auto b64_tbl = (mode == Base64ModeStandard) ? strict_table : url_table;
  // Padding is mandatory for the standard "base64" mode.
  // The padding can be canceled by the user for the "base64url" mode.
  auto cancel_pad = (Base64ModeUrlSafeWithoutPadding == mode);
  // The base64 encoder transforms 3 data bytes to 4 encoded characters.
  // Number of complete blocks:
  auto B3full = src_size / 3;
  // Size of remainder:
  auto B3rem = src_size % 3;
  // If B3rem, then move the last complete block to remainder.
  if (B3rem == 0) {
    B3full = B3full - 1;
    B3rem = 3;
  }
  // If the padding is mandatory, the remainder shall be encoded to a complete
  // char[4] sequence.
  const auto last_enc_len = cancel_pad ? (B3rem + 1) : 4;
  const auto encoded_len = (B3full * 4) + last_enc_len;
  // If requested the only number of required bytes for encoding, return it.
  if ((nullptr == dst) && (0 == dst_size)) return encoded_len;
  // Insufficient memory test.
  if (dst_size < encoded_len) return 0;

  // Main encode loop:
  char last_enc4[4];
  uint8_t last_bin3[3] = { 0, 0, 0 };
  memcpy(&last_bin3[0], &src[B3full * 3], B3rem);
  // First, process the complemented remainder.
  const uint8_t *src_ = &last_bin3[0];
  auto dst_ = &last_enc4[0];
  for (size_t k = 0; k < B3full + 1; k++) {
    dst_[0] = b64_tbl[src_[0] >> 2];
    dst_[1] = b64_tbl[(0x3f & (src_[0] << 4)) | (src_[1] >> 4)];
    dst_[2] = b64_tbl[(0x3f & (src_[1] << 2)) | (src_[2] >> 6)];
    dst_[3] = b64_tbl[0x3f & src_[2]];
    src_ = src + (k * 3);
    dst_ = dst + (k * 4);
  }
  // Set padding ({a,_,_}=>{'x','y','=','='}, {a,b,_}=>{'x','y','z','='}).
  if (B3rem < 3) last_enc4[3] = '=';
  if (B3rem < 2) last_enc4[2] = '=';
  // Copy remainder.
  memcpy(&dst[B3full * 4], &last_enc4[0], last_enc_len);
  return encoded_len;
}

// Returns the number of bytes, need to save a decoded result.
// Return zero if an error or if src_size==0.
// Error condition : (src_size != 0 && ret == 0).
static size_t Base64DecodedSize(Base64Mode base64_mode, const char *src,
                                size_t src_size, size_t *error_position) {
  size_t err_pos = 0;
  auto rc = B64DecodeImpl(base64_mode, src, src_size, nullptr, 0, &err_pos);
  if (error_position) *error_position = err_pos;
  return rc;
}

// Decodes the input base64 string into the binary sequence of bytes.
// Expects: dst_size >= Base64DecodedSize(mode, src, src_size).
// On success, returns the number of bytes written to the destination memory.
// Returned Zero indicates nothing was written.
// Error condition : (src_size != 0 && ret == 0).
static size_t Base64Decode(Base64Mode base64_mode, const char *src,
                           size_t src_size, uint8_t *dst, size_t dst_size,
                           size_t *error_position) {
  size_t err_pos = 0;
  size_t rc = 0;
  // If (0==dst_size) && (nullptr==src) then decode_b64 return the required
  // size. Avoid this by check (dst_size>0).
  if (dst_size > 0) {
    rc = B64DecodeImpl(base64_mode, src, src_size, dst, dst_size, &err_pos);
  }
  if (error_position) *error_position = err_pos;
  return rc;
}

// Encodes the input byte sequence into the base64 encoded string.
static void Base64Encode(Base64Mode base64_mode, const uint8_t *src,
                         size_t src_size, std::string *_text) {
  const auto req_size = B64EncodeImpl(base64_mode, src, src_size, nullptr, 0);
  if (req_size > 0) {
    // std::string is a contiguous memory container since C++11.
    // data() + i == &operator[](i) for every i in [0, size()].	(since C++11)
    // http://en.cppreference.com/w/cpp/string/basic_string/data
    // http://en.cppreference.com/w/cpp/string/basic_string/operator_at
    // https://stackoverflow.com/questions/39200665/directly-write-into-char-buffer-of-stdstring
    // Allocate memory for a result.
    std::string &text = *_text;
    const auto text_pos = text.size();
    // Character '#' is out of the base64 alphabet.
    text.append(req_size, '#');
    B64EncodeImpl(base64_mode, src, src_size, &text[text_pos], req_size);
  }
}

// Helper for FieldDef attributes check.
static Base64Mode FieldGetBase64Mode(const FieldDef *fd) {
  auto mode = Base64ModeNotSet;
  if (fd) {
    if (fd->attributes.Lookup("base64")) {
      mode = Base64ModeStandard;
    } else if (fd->attributes.Lookup("base64url")) {
      mode = Base64ModeUrlSafe;
    }
  }
  return mode;
}

// On success, returns true.
bool PrintBase64Vector(const flatbuffers::Vector<uint8_t> &v, Type type,
                       int indent, const IDLOptions &opts, std::string *_text,
                       const FieldDef *fd) {
  (void)type;
  (void)indent;

  // An empty vector can't be present as true base64 string.
  if (0 == v.size()) return false;

  auto b64mode = FieldGetBase64Mode(fd);
  if (!b64mode) return false;

  auto b64m = ((Base64ModeUrlSafe == b64mode) && opts.base64_cancel_padding)
                  ? Base64ModeUrlSafeWithoutPadding
                  : b64mode;

  std::string &text = *_text;
  text += '\"';
  Base64Encode(b64m, v.data(), v.size(), &text);
  text += '\"';
  return true;
}

// Returns number of Decoded charaters from the text string until error.
// Returns zero if string is empty or has not base64 attribute.
// On success, returns text.size().
size_t ParseBase64Vector(Type type, const IDLOptions &opts,
                         const std::string &text, const FieldDef *fd,
                         uoffset_t *ovalue, FlatBufferBuilder *_builder) {
  (void)opts;
  // Only UCHAR supported.
  if ((type.base_type != BASE_TYPE_UCHAR) || text.empty()) return 0;
  // Leave if string has not base64 attribute.
  auto b64mode = FieldGetBase64Mode(fd);
  if (!b64mode) return 0;

  const auto src_data = text.c_str();
  const auto src_size = text.size();
  size_t decode_pos = 0;
  // Calculate the number of required bytes.
  auto dec_size = Base64DecodedSize(b64mode, src_data, src_size, &decode_pos);
  if (dec_size > 0) {
    // Allocate memory.
    uint8_t *dst = nullptr;
    *ovalue = _builder->CreateUninitializedVector(dec_size, 1, &dst);
    // Decode the string to the memory.
    Base64Decode(b64mode, src_data, src_size, dst, dec_size, &decode_pos);
  }
  return decode_pos;
}

// Public interface of util_base64 very hard for testing.
int UtilBase64InerTest() {
  // Use internal test for coverage.
  struct _B64T {
    static std::vector<uint8_t> Decode(const std::string &base64,
                                       const Base64Mode b64m,
                                       size_t *const err_pos = nullptr) {
      if (err_pos) *err_pos = 0;
      std::vector<uint8_t> out;
      const auto indat = base64.c_str();
      const auto inlen = base64.size();
      // DecodeSize can fail if the input sequence is bad.
      const auto req_size = Base64DecodedSize(b64m, indat, inlen, err_pos);
      if (req_size > 0) {
        // Allocate memory for a result.
        out.resize(req_size);
        const auto out_data = vector_data(out);
        const auto out_size = out.size();
        // Insufficient memory test must return zero.
        if (Base64Decode(b64m, indat, inlen, out_data, req_size - 1, err_pos)) {
          return std::vector<uint8_t>();
        }
        // Decode can fail if the input sequence is bad.
        const auto rc =
            Base64Decode(b64m, indat, inlen, out_data, out_size, err_pos);
        // Reset the result if decode failed.
        if (rc != req_size) { return std::vector<uint8_t>(); }
      }
      return out;
    }

    static bool DecEncTest(const Base64Mode dec_mode, const Base64Mode enc_mode,
                           size_t *const err_pos, const std::string &input,
                           const std::string &expected) {
      auto decoded = Decode(input, dec_mode, err_pos);
      std::string encoded("");
      if (false == decoded.empty())
        Base64Encode(enc_mode, vector_data(decoded), decoded.size(), &encoded);
      return expected == encoded;
    }

    static bool EncDecTest(const Base64Mode enc_mode, const Base64Mode dec_mode,
                           const std::vector<uint8_t> &input) {
      std::string encoded;
      Base64Encode(enc_mode, vector_data(input), input.size(), &encoded);
      return input == Decode(encoded, dec_mode);
    }
  };

  static const size_t B64ModeNum = 2;
  const Base64Mode BaseSet[B64ModeNum] = { Base64ModeStandard,
                                           Base64ModeUrlSafe };
  // clang-format off
  // The padding is optional both for Standard and UrlSafe decoders.
  // Standard decoder can't decode a UrlSafe string.
  const bool expects[B64ModeNum * B64ModeNum] = {
    true,  true, // encoder: Standard
    false, true, // encoder: UrlSafe
                 //    |     |_______decoder: UrlSafe
                 //    |_____________decoder: Standard
  };
  // clang-format on

  // Auto-generated encode-decode loop:
  for (size_t enc_index = 0; enc_index < B64ModeNum; enc_index++) {
    const auto enc_mode = BaseSet[enc_index];
    for (size_t dec_index = 0; dec_index < B64ModeNum; dec_index++) {
      const auto match_expects = expects[enc_index * B64ModeNum + dec_index];
      const auto dec_mode = BaseSet[dec_index];
      const size_t B = 128;
      const auto N = 2 * B + 1;
      size_t k = 0;
      for (k = 0; k < N; k++) {
        const auto M = (k % (B - 1));
        // Generate a binary data.
        std::vector<uint8_t> bin(M);
        for (size_t j = 0; j < M; j++) {
          bin[j] = static_cast<uint8_t>(((k + j) % B) % 0x100);
        }
        // Check: Binary->Encode->Decode->Compare.
        if (false == _B64T::EncDecTest(enc_mode, dec_mode, bin)) break;
      }
      if ((N == k) != match_expects) return __LINE__;
    }
  }

  // Test of error position detector.
  size_t epos = 0;
  for (size_t mode_ind = 0; mode_ind < B64ModeNum; mode_ind++) {
    const auto mode = BaseSet[mode_ind];
    const std::string ref = "IARXA18BIg==";
    for (size_t k = 0; k < ref.size(); k++) {
      auto t = ref;
      t[k] = '#';
      if (!_B64T::DecEncTest(mode, mode, &epos, t, "")) return __LINE__;
      // the position of error is in half-open range [k, ref.size())
      if (!(epos >= k && epos < ref.size())) return __LINE__;
    }
  }

  // Input is a standard RFC4648 sequence.
  if (!_B64T::DecEncTest(Base64ModeStandard, Base64ModeStandard, &epos,
                         "/43+AergFA==", "/43+AergFA==") ||
      (epos != 12))
    return __LINE__;

  // Input is RFC4648 sequence without padding.
  if (!_B64T::DecEncTest(Base64ModeStandard, Base64ModeStandard, &epos,
                         "/43+AergFA", "/43+AergFA==") ||
      (epos != 10))
    return __LINE__;

  // Test with the standard decoder and url-safe encoder.
  if (!_B64T::DecEncTest(Base64ModeStandard, Base64ModeUrlSafe, &epos,
                         "/43+AergFA==", "_43-AergFA==") ||
      (epos != 12))
    return __LINE__;

  // Test with url-safe decoder and encoder.
  if (!_B64T::DecEncTest(Base64ModeUrlSafe, Base64ModeUrlSafe, &epos,
                         "_43-AergFA==", "_43-AergFA==") ||
      (epos != 12))
    return __LINE__;

  // Test using base64 string without padding symbols.
  if (!_B64T::DecEncTest(Base64ModeUrlSafe, Base64ModeUrlSafe, &epos,
                         "_43-AergFA", "_43-AergFA==") ||
      (epos != 10))
    return __LINE__;

  // Cancel output padding for Base64Url encoder.
  if (!_B64T::DecEncTest(Base64ModeUrlSafe, Base64ModeUrlSafeWithoutPadding,
                         &epos, "_43-AergFA==", "_43-AergFA") ||
      (epos != 12))
    return __LINE__;
  // done
  return 0;
}

}  // namespace flatbuffers
