#ifndef FLATBUFFERS_UTIL_B64_H_
#define FLATBUFFERS_UTIL_B64_H_
#include "flatbuffers/idl.h"

namespace flatbuffers {
// [BASE64]{https://tools.ietf.org/html/rfc4648}
//
// Field attribute (base64): accepts standard RFC4648 alphabet.
// Padding is optional for the decoder.
// Valid input strings: {"/43+AergFA==", "/43+AergFA"}.
// The encoder output always with padding if required:
// Vector [255, 141, 254, 1, 234, 224, 20] => "/43+AergFA==".
//
// Field attribute (base64url): RFC4648 with URL and Filename Safe Alphabet.
// Padding is optional for the decoder.
// The decoder accepts both alphabets: UrlSafe and Standard.
// Valid input strings:
//  {"/43+AergFA==", "/43+AergFA", "_43-AergFA==", "_43-AergFA"}.
// The encoder output always with padding if required:
// Vector [255, 141, 254, 1, 234, 224, 20] encoded to "_43-AergFA==".
//
// Print extension of (base64url) attribute:
// If IDLOptions::base64_cancel_padding is true, then the encoder omits
// a padding for the output sequence.
// Vector [255, 141, 254, 1, 234, 224, 20] encoded to // "_43-AergFA".

// Parse ubyte vector from a base64 string.
// Return number of processed symbols from the string.
// Return zero if any:
//  1) type != BASE_TYPE_UCHAR,
//  2) text.size() < 1,
//  3) fd doesn't have (base64) or (base64url) attribute.
// On succsess, returned value is equal to text.size().
// If the text has non-base64 symbols return the location of the first one.
size_t ParseBase64Vector(Type type, const IDLOptions &opts,
                         const std::string &text, const FieldDef *fd,
                         uoffset_t *ovalue, FlatBufferBuilder *_builder);

// Print a Vector as a base64 encoded string.
// Return False if any:
//  1) type T is not the same as ubyte type,
//  2) v.size() < 1,
//  3) fd doesn't have (base64) or (base64url) attribute.
// On succsess, append to the _text quoted base64 string.
// Takes into account the "opts.base64_cancel_padding" option if a field has
// (base64url) attribute and cancel trailing padding for the output string.
template<typename T>
static inline bool PrintBase64Vector(const Vector<T> &v, Type type, int indent,
                                     const IDLOptions &opts, std::string *_text,
                                     const FieldDef *fd) {
  (void)v;
  (void)type;
  (void)indent;
  (void)opts;
  (void)_text;
  (void)fd;
  return false;
}

// Override (not specialisation) for Vector<uint8_t>
bool PrintBase64Vector(const flatbuffers::Vector<uint8_t> &v, Type type,
                       int indent, const IDLOptions &opts, std::string *_text,
                       const FieldDef *fd);

// Internal tests for decode/encode routines.
int UtilBase64InerTest();

}  // namespace flatbuffers

#endif  // FLATBUFFERS_UTIL_B64_H_
