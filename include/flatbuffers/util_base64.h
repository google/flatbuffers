#ifndef FLATBUFFERS_UTIL_B64_H_
#define FLATBUFFERS_UTIL_B64_H_
#include "flatbuffers/idl.h"

namespace flatbuffers {
// [BASE64]{https://tools.ietf.org/html/rfc4648}

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
  Base64ModeUrlSafeWithoutPadding = (Base64ModeUrlSafe | 4)
};

// Helper for Base64Mode enum.
inline Base64Mode Base64CancelPadding(Base64Mode mode) {
  return (Base64ModeUrlSafe == mode) ? Base64ModeUrlSafeWithoutPadding : mode;
}

// Helper for FieldDef attributes check.
inline Base64Mode FieldGetBase64Mode(const FieldDef *fd) {
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

// Returns the number of bytes, need to save a decoded result.
// Return zero if an error or if src_size==0.
// Error condition : (src_size != 0 && ret == 0).
size_t Base64DecodedSize(Base64Mode base64_mode, const char *src,
                         size_t src_size, size_t *error_position = nullptr);

// Decodes the input base64 string into the binary sequence of bytes.
// Expects: dst_size >= Base64DecodedSize(mode, src, src_size).
// On success, returns the number of bytes written to the destination memory.
// Returned Zero indicates nothing was written.
// Error condition : (src_size != 0 && ret == 0).
size_t Base64Decode(Base64Mode base64_mode, const char *src, size_t src_size,
                    uint8_t *dst, size_t dst_size,
                    size_t *error_position = nullptr);

// Encodes the input byte sequence into the base64 encoded string.
void Base64Encode(Base64Mode base64_mode, const uint8_t *src, size_t src_size,
                  std::string *_text);

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_B64_H_
