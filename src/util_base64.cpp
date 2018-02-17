#include "flatbuffers/base.h"

#include "flatbuffers/util_base64.h"

namespace flatbuffers {

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
    *error_position = (src_size + 1);
  } else {
    // If an error detected:
    // Number of blocks processed and written to [src] memory:
    const auto indx = C4full - loop_cnt;
    // Reject written data.
    memset(dst, 0, indx * 3);
    // Save a position of corrupted character.
    *error_position = 4 * (indx ? (indx - 1) : C4full);
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

size_t Base64DecodedSize(Base64Mode base64_mode, const char *src,
                         size_t src_size, size_t *error_position) {
  size_t err_pos = 0;
  auto rc = B64DecodeImpl(base64_mode, src, src_size, nullptr, 0, &err_pos);
  if (error_position) *error_position = err_pos;
  return rc;
}

size_t Base64Decode(Base64Mode base64_mode, const char *src, size_t src_size,
                    uint8_t *dst, size_t dst_size, size_t *error_position) {
  size_t err_pos = 0;
  size_t rc = 0;
  // If (0==dst_size) && (nullptr==src) then decode_b64 return the required size.
  // Avoid this by check (dst_size>0).
  if (dst_size > 0) {
    rc = B64DecodeImpl(base64_mode, src, src_size, dst, dst_size, &err_pos);
  }
  if (error_position) *error_position = err_pos;
  return rc;
}

void Base64Encode(Base64Mode base64_mode, const uint8_t *src, size_t src_size,
                  std::string *_text) {
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

}  // namespace flatbuffers
