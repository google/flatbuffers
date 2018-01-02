#include "flatbuffers/base.h"

#include "flatbuffers/util_base64.h"

// BASE64 - https://tools.ietf.org/html/rfc4648

namespace flatbuffers {

static inline bool Base64ModeIsStrict(int base64_mode) {
  return 0 == (base64_mode & kBase64Url);
}

// return number of written bytes if success or zero if an error.
// error condition: (src_size!=0 && ret==0)
// return number of bytes requred for decoding if condition:
// ((nullptr == dst) && (0 == dst_size))
// <error_position> MUST be not-null
static size_t decode_b64(const int mode, const char *const src,
                         const size_t src_size, uint8_t *const dst,
                         const size_t dst_size, size_t *const error_position) {
  // strict base64 table (RFC 4648)
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

  // Base 64 Encoding with URL and Filename Safe Alphabet, allow "+/-_"
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
  if (error_position) *error_position = 0;
  const auto b64_tbl = Base64ModeIsStrict(mode) ? strict_table : url_table;
  // padding mandatory for strict base64, and optional for base64url
  const auto padding_mandatory = Base64ModeIsStrict(mode);
  // base64 decode transform 4 encoded charaters to 3 data bytes
  if (0 == src_size) return 0;
  // decode prepare stage
  // number of perfect char[4] blocks
  auto C4full = src_size / 4;
  // number of remained characters in the last char[4] block
  auto C4rem = src_size % 4;
  // if C4rem==0, then move the last char[4] block to remainder
  if (C4rem == 0) {
    C4full = C4full - 1;
    C4rem = 4;
  }
  // the C4rem is strictly positive value from: {1,2,3,4}
  // build char[4] perfect block for the remainder
  // if padding is mandatory, sets incomplete characters as invalid symbols
  // if it is optional, sets to zero symbol ('A')
  char last_enc4[4];
  uint8_t last_dec3[3];
  memset(&last_enc4[0], padding_mandatory ? '#' : 'A', 4);
  // copy available bytes
  memcpy(&last_enc4[0], &src[C4full * 4], C4rem);
  // guest for size of last decoded block, can be 1,2 or 3
  auto last_dec_len = C4rem - 1;
  // encoded sequence can has padding '=' symbols at the last char[4] block
  // in this case are allowed only two variants: {x,y,=,=} or {x,y,z,=}
  // replace '=' by 'zeros' and recalculate decoded size of the last block
  if (last_enc4[3] == '=') {
    last_dec_len = 2;
    last_enc4[3] = 'A';
    if (last_enc4[2] == '=') {
      last_dec_len = 1;
      last_enc4[2] = 'A';
    }
  }
  // calculate size of decoded sequence if it has no errors
  const auto decoded_len = (C4full * 3 + last_dec_len);
  // if requested the only number of required bytes for decoding, return it
  if ((nullptr == dst) && (0 == dst_size)) return decoded_len;
  // insufficient memory test
  if (dst_size < decoded_len) return 0;
  // decode loop:
  // at first iteration decode the last block
  auto dst_ = &last_dec3[0];
  const char *src_ = &last_enc4[0];
  // loop counter: the last block plus all perfect blocks
  auto loop_cnt = (1 + C4full);
  // error mask can take only two states: (0)-ok and (1)-fail
  // type of <err_mask> must match with unsigned <loop_cnt>
  auto err_mask = loop_cnt * 0;
  // use (err_mask - 1) = {0,~0} as conditional gate for the loop_cnt
  for (size_t k = 0; loop_cnt & (err_mask - 1); loop_cnt--, k++) {
    const auto a0 = b64_tbl[static_cast<uint8_t>(src_[0])];
    const auto a1 = b64_tbl[static_cast<uint8_t>(src_[1])];
    const auto a2 = b64_tbl[static_cast<uint8_t>(src_[2])];
    const auto a3 = b64_tbl[static_cast<uint8_t>(src_[3])];
    // decode by RFC 4648 alghorithm
    dst_[0] = ((a0 << 2) | (a1 >> 4)) & 0xff;
    dst_[1] = ((a1 << 4) | (a2 >> 2)) & 0xff;
    dst_[2] = ((a2 << 6) | (a3 >> 0)) & 0xff;
    // extract 0x40 bit from all decoded bytes and transform to {0,1}
    err_mask = ((a0 | a1 | a2 | a3) >> 6);
    src_ = src + (k * 4);
    dst_ = dst + (k * 3);
  }
  const auto done = (0 == err_mask);
  if (done) {
    // copy the last decoded block to destanation
    memcpy(&dst[C4full * 3], &last_dec3[0], last_dec_len);
    *error_position = (src_size + 1);
  } else {
    // number of blocks processed and written to [src] memory
    auto indx = C4full - loop_cnt;
    // reject writen data
    memset(dst, 0, indx * 3);
    // seve position of corrupted characters
    *error_position = 4 * (indx ? (indx - 1) : C4full);
  }
  return done ? decoded_len : 0;
}

// return number of written bytes or zero if error.
// return number of bytes requred for decoding if:
// ((nullptr == dst) && (0 == dst_size))
static size_t encode_b64(const int mode, const uint8_t *const src,
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

  const auto tbl64 = Base64ModeIsStrict(mode) ? strict_table : url_table;
  // padding mandatory for strict base64, and skipped for base64url
  const auto force_padding = Base64ModeIsStrict(mode);
  // base64 encode transform 3 data bytes to 4 encoded charaters
  if (0 == src_size) return 0;
  // number of complete blocks
  auto B3full = src_size / 3;
  // incomplete remainder
  auto B3rem = src_size % 3;
  // if B3rem, then move the last comlete block to remainder
  if (B3rem == 0) {
    B3full = B3full - 1;
    B3rem = 3;
  }
  // if pad is mandatory, the remainder shall be encoded to the complete char[4]
  const auto last_enc_len = force_padding ? 4 : (B3rem + 1);
  const auto encoded_len = (B3full * 4) + last_enc_len;
  // if requested the only number of required bytes for encoding, return it
  if ((nullptr == dst) && (0 == dst_size)) return encoded_len;
  // insufficient memory test
  if (dst_size < encoded_len) return 0;
  // encode loop
  char last_enc4[4];
  uint8_t last_bin3[3] = { 0, 0, 0 };
  memcpy(&last_bin3[0], &src[B3full * 3], B3rem);
  // complemented remainder encoded first
  const uint8_t *src_ = &last_bin3[0];
  auto dst_ = &last_enc4[0];
  for (size_t k = 0; k < B3full + 1; k++) {
    dst_[0] = tbl64[src_[0] >> 2];
    dst_[1] = tbl64[(0x3f & (src_[0] << 4)) | (src_[1] >> 4)];
    dst_[2] = tbl64[(0x3f & (src_[1] << 2)) | (src_[2] >> 6)];
    dst_[3] = tbl64[0x3f & src_[2]];
    src_ = src + (k * 3);
    dst_ = dst + (k * 4);
  }
  // set padding ({a,_,_}=>{'x','y','=','='}, {a,b,_}=>{'x','y','z','='})
  if (B3rem < 3) last_enc4[3] = '=';
  if (B3rem < 2) last_enc4[2] = '=';
  // copy remainder
  memcpy(&dst[B3full * 4], &last_enc4[0], last_enc_len);
  return encoded_len;
}

size_t Base64DecodedSize(int base64_mode, const char *src, size_t src_size,
                         size_t *error_position) {
  size_t err_pos = 0;
  auto rc = decode_b64(base64_mode, src, src_size, nullptr, 0, &err_pos);
  if (error_position) *error_position = err_pos;
  return rc;
}

size_t Base64Decode(int base64_mode, const char *src, size_t src_size,
                    uint8_t *dst, size_t dst_size, size_t *error_position) {
  size_t err_pos = 0;
  size_t rc = 0;
  // if 0==dst_size && nullptr==src function decode_b64 return required size
  // avoid this by check dst_size>0
  if (dst_size > 0) {
    rc = decode_b64(base64_mode, src, src_size, dst, dst_size, &err_pos);
  }
  if (error_position) *error_position = err_pos;
  return rc;
}

size_t Base64EncodedSize(int base64_mode, const uint8_t *src, size_t src_size) {
  return encode_b64(base64_mode, src, src_size, nullptr, 0);
}

size_t Base64Encode(int base64_mode, const uint8_t *src, size_t src_size,
                    char *dst, size_t dst_size) {
  if (0 == dst_size) return 0;
  return encode_b64(base64_mode, src, src_size, dst, dst_size);
}
}  // namespace flatbuffers
