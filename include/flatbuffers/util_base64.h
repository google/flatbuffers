#ifndef FLATBUFFERS_UTIL_B64_H_
#define FLATBUFFERS_UTIL_B64_H_

namespace flatbuffers {
// [BASE64]{https://tools.ietf.org/html/rfc4648}

// a Field doesn't have base64 attribute
static const int kBase64Undefined = 0;

// attribute (base64): strict RFC4648
// general base64 alphabet, padding mandatory both for encoder and decoder
static const int kBase64Strict = 1;

// attribute (base64url): RFC4648 with URL and Filename Safe Alphabet
// URL safe alphabet, padding optional both for decoder and skipped at encoder
static const int kBase64Url = 2;

// Helper: FDT is <idl.h>::FieldDef type
template<typename FDT> inline int field_base64_mode(const FDT *fd) {
  auto mode = kBase64Undefined;
  if (fd) {
    if (fd->attributes.Lookup("base64")) {
      mode = kBase64Strict;
    } else if (fd->attributes.Lookup("base64url")) {
      mode = kBase64Url;
    }
  }
  return mode;
}

// calculate number of bytes required to save Decoded result
// return zero if error.
// error condition : (src_size != 0 && ret == 0)
size_t Base64DecodedSize(int base64_mode, const char *src, size_t src_size,
                         size_t *error_position = nullptr);

// Decode input base64 sequence.
// expects: dst_size >= Base64DecodedSize(mode, src, src_size)
// return number of written bytes or zero
// error condition : (src_size != 0 && ret == 0)
size_t Base64Decode(int base64_mode, const char *src, size_t src_size,
                    uint8_t *dst, size_t dst_size,
                    size_t *error_position = nullptr);

// calculate number of bytes required to save Encoded result
// return zero if error.
// error condition : (src_size != 0 && ret == 0)
size_t Base64EncodedSize(int base64_mode, const uint8_t *src, size_t src_size);

// encode input byte sequence to base64 sequence.
// expects: dst_size >= Base64EncodedSize(mode, src, src_size)
// return number of written bytes or zero.
// error condition : (src_size != 0 && ret == 0)
size_t Base64Encode(int base64_mode, const uint8_t *src, size_t src_size,
                    char *dst, size_t dst_size);

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_B64_H_
