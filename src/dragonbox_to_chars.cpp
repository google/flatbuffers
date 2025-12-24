// Copyright 2020-2025 Junekey Jeon
//
// The contents of this file may be used under the terms of
// the Apache License v2.0 with LLVM Exceptions.
//
//    (See accompanying file LICENSE-Apache or copy at
//     https://llvm.org/foundation/relicensing/LICENSE.txt)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

#define JKJ_DRAGONBOX_TO_CHARS_LEAK_MACROS
#include "flatbuffers/dragonbox_to_chars.h"

namespace JKJ_NAMESPACE {
    namespace dragonbox {
        namespace detail {
            // These "//"'s are to prevent clang-format to ruin this nice alignment.
            // Thanks to reddit user u/mcmcc:
            // https://www.reddit.com/r/cpp/comments/so3wx9/dragonbox_110_is_released_a_fast_floattostring/hw8z26r/?context=3
            struct byte_pair {
                char bytes[2];
            };
            static constexpr byte_pair radix_100_table[100] JKJ_STATIC_DATA_SECTION = {
                {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, //
                {'0', '5'}, {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, //
                {'1', '0'}, {'1', '1'}, {'1', '2'}, {'1', '3'}, {'1', '4'}, //
                {'1', '5'}, {'1', '6'}, {'1', '7'}, {'1', '8'}, {'1', '9'}, //
                {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'}, {'2', '4'}, //
                {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'}, //
                {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, //
                {'3', '5'}, {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, //
                {'4', '0'}, {'4', '1'}, {'4', '2'}, {'4', '3'}, {'4', '4'}, //
                {'4', '5'}, {'4', '6'}, {'4', '7'}, {'4', '8'}, {'4', '9'}, //
                {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'}, {'5', '4'}, //
                {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'}, //
                {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'}, //
                {'6', '5'}, {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'}, //
                {'7', '0'}, {'7', '1'}, {'7', '2'}, {'7', '3'}, {'7', '4'}, //
                {'7', '5'}, {'7', '6'}, {'7', '7'}, {'7', '8'}, {'7', '9'}, //
                {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'}, {'8', '4'}, //
                {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'}, //
                {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'}, //
                {'9', '5'}, {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'}  //
            };

            static constexpr byte_pair radix_100_head_table[100] JKJ_STATIC_DATA_SECTION = {
                {'0', '.'}, {'1', '.'}, {'2', '.'}, {'3', '.'}, {'4', '.'}, //
                {'5', '.'}, {'6', '.'}, {'7', '.'}, {'8', '.'}, {'9', '.'}, //
                {'1', '.'}, {'1', '.'}, {'1', '.'}, {'1', '.'}, {'1', '.'}, //
                {'1', '.'}, {'1', '.'}, {'1', '.'}, {'1', '.'}, {'1', '.'}, //
                {'2', '.'}, {'2', '.'}, {'2', '.'}, {'2', '.'}, {'2', '.'}, //
                {'2', '.'}, {'2', '.'}, {'2', '.'}, {'2', '.'}, {'2', '.'}, //
                {'3', '.'}, {'3', '.'}, {'3', '.'}, {'3', '.'}, {'3', '.'}, //
                {'3', '.'}, {'3', '.'}, {'3', '.'}, {'3', '.'}, {'3', '.'}, //
                {'4', '.'}, {'4', '.'}, {'4', '.'}, {'4', '.'}, {'4', '.'}, //
                {'4', '.'}, {'4', '.'}, {'4', '.'}, {'4', '.'}, {'4', '.'}, //
                {'5', '.'}, {'5', '.'}, {'5', '.'}, {'5', '.'}, {'5', '.'}, //
                {'5', '.'}, {'5', '.'}, {'5', '.'}, {'5', '.'}, {'5', '.'}, //
                {'6', '.'}, {'6', '.'}, {'6', '.'}, {'6', '.'}, {'6', '.'}, //
                {'6', '.'}, {'6', '.'}, {'6', '.'}, {'6', '.'}, {'6', '.'}, //
                {'7', '.'}, {'7', '.'}, {'7', '.'}, {'7', '.'}, {'7', '.'}, //
                {'7', '.'}, {'7', '.'}, {'7', '.'}, {'7', '.'}, {'7', '.'}, //
                {'8', '.'}, {'8', '.'}, {'8', '.'}, {'8', '.'}, {'8', '.'}, //
                {'8', '.'}, {'8', '.'}, {'8', '.'}, {'8', '.'}, {'8', '.'}, //
                {'9', '.'}, {'9', '.'}, {'9', '.'}, {'9', '.'}, {'9', '.'}, //
                {'9', '.'}, {'9', '.'}, {'9', '.'}, {'9', '.'}, {'9', '.'}  //
            };

            static void print_1_digit(int n, char* buffer) noexcept {
                JKJ_IF_CONSTEXPR(('0' & 0xf) == 0) { *buffer = char('0' | n); }
                else {
                    *buffer = char('0' + n);
                }
            }

            static void print_2_digits(int n, char* buffer) noexcept {
                auto bp = read_static_data(radix_100_table + n);
                stdr::memcpy(buffer, &bp, 2);
            }

            static void print_head_chars(int n, char* buffer) noexcept {
                auto bp = read_static_data(radix_100_head_table + n);
                stdr::memcpy(buffer, &bp, 2);
            }

            // These digit generation routines are inspired by James Anhalt's itoa algorithm:
            // https://github.com/jeaiii/itoa
            // The main idea is for given n, find y such that floor(10^k * y / 2^32) = n holds,
            // where k is an appropriate integer depending on the length of n.
            // For example, if n = 1234567, we set k = 6. In this case, we have
            // floor(y / 2^32) = 1,
            // floor(10^2 * ((10^0 * y) mod 2^32) / 2^32) = 23,
            // floor(10^2 * ((10^2 * y) mod 2^32) / 2^32) = 45, and
            // floor(10^2 * ((10^4 * y) mod 2^32) / 2^32) = 67.
            // See https://jk-jeon.github.io/posts/2022/02/jeaiii-algorithm/ for more explanation.

            JKJ_FORCEINLINE static void print_9_digits(stdr::uint_least32_t s32, int& exponent,
                                                       char*& buffer) noexcept {
                // -- IEEE-754 binary32
                // Since we do not cut trailing zeros in advance, s32 must be of 6~9 digits
                // unless the original input was subnormal.
                // In particular, when it is of 9 digits it shouldn't have any trailing zeros.
                // -- IEEE-754 binary64
                // In this case, s32 must be of 7~9 digits unless the input is subnormal,
                // and it shouldn't have any trailing zeros if it is of 9 digits.
                if (s32 >= UINT32_C(100000000)) {
                    // 9 digits.
                    // 1441151882 = ceil(2^57 / 1'0000'0000) + 1
                    auto prod = s32 * UINT64_C(1441151882);
                    prod >>= 25;
                    print_head_chars(int(prod >> 32), buffer);

                    prod = (prod & UINT32_C(0xffffffff)) * 100;
                    print_2_digits(int(prod >> 32), buffer + 2);
                    prod = (prod & UINT32_C(0xffffffff)) * 100;
                    print_2_digits(int(prod >> 32), buffer + 4);
                    prod = (prod & UINT32_C(0xffffffff)) * 100;
                    print_2_digits(int(prod >> 32), buffer + 6);
                    prod = (prod & UINT32_C(0xffffffff)) * 100;
                    print_2_digits(int(prod >> 32), buffer + 8);

                    exponent += 8;
                    buffer += 10;
                }
                else if (s32 >= UINT32_C(1000000)) {
                    // 7 or 8 digits.
                    // 281474978 = ceil(2^48 / 100'0000) + 1
                    auto prod = s32 * UINT64_C(281474978);
                    prod >>= 16;
                    auto const head_digits = int(prod >> 32);
                    // If s32 is of 8 digits, increase the exponent by 7.
                    // Otherwise, increase it by 6.
                    exponent += (6 + int(head_digits >= 10));

                    // Write the first digit and the decimal point.
                    print_head_chars(head_digits, buffer);
                    // This third character may be overwritten later but we don't care.
                    buffer[2] = radix_100_table[head_digits].bytes[1];

                    // Remaining 6 digits are all zero?
                    if ((prod & UINT32_C(0xffffffff)) <=
                        stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / UINT32_C(1000000))) {
                        // The number of characters actually need to be written is:
                        //   1, if only the first digit is nonzero, which means that either s32 is of 7
                        //   digits or it is of 8 digits but the second digit is zero, or
                        //   3, otherwise.
                        // Note that buffer[2] is never '0' if s32 is of 7 digits, because the input is
                        // never zero.
                        buffer += (1 + (int(head_digits >= 10) & int(buffer[2] > '0')) * 2);
                    }
                    else {
                        // At least one of the remaining 6 digits are nonzero.
                        // After this adjustment, now the first destination becomes buffer + 2.
                        buffer += int(head_digits >= 10);

                        // Obtain the next two digits.
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 2);

                        // Remaining 4 digits are all zero?
                        if ((prod & UINT32_C(0xffffffff)) <=
                            stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 10000)) {
                            buffer += (3 + int(buffer[3] > '0'));
                        }
                        else {
                            // At least one of the remaining 4 digits are nonzero.

                            // Obtain the next two digits.
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 4);

                            // Remaining 2 digits are all zero?
                            if ((prod & UINT32_C(0xffffffff)) <=
                                stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 100)) {
                                buffer += (5 + int(buffer[5] > '0'));
                            }
                            else {
                                // Obtain the last two digits.
                                prod = (prod & UINT32_C(0xffffffff)) * 100;
                                print_2_digits(int(prod >> 32), buffer + 6);

                                buffer += (7 + int(buffer[7] > '0'));
                            }
                        }
                    }
                }
                else if (s32 >= 10000) {
                    // 5 or 6 digits.
                    // 429497 = ceil(2^32 / 1'0000)
                    auto prod = s32 * UINT64_C(429497);
                    auto const head_digits = int(prod >> 32);

                    // If s32 is of 6 digits, increase the exponent by 5.
                    // Otherwise, increase it by 4.
                    exponent += (4 + int(head_digits >= 10));

                    // Write the first digit and the decimal point.
                    print_head_chars(head_digits, buffer);
                    // This third character may be overwritten later but we don't care.
                    buffer[2] = radix_100_table[head_digits].bytes[1];

                    // Remaining 4 digits are all zero?
                    if ((prod & UINT32_C(0xffffffff)) <=
                        stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 10000)) {
                        // The number of characters actually written is 1 or 3, similarly to the case of
                        // 7 or 8 digits.
                        buffer += (1 + (int(head_digits >= 10) & int(buffer[2] > '0')) * 2);
                    }
                    else {
                        // At least one of the remaining 4 digits are nonzero.
                        // After this adjustment, now the first destination becomes buffer + 2.
                        buffer += int(head_digits >= 10);

                        // Obtain the next two digits.
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 2);

                        // Remaining 2 digits are all zero?
                        if ((prod & UINT32_C(0xffffffff)) <=
                            stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 100)) {
                            buffer += (3 + int(buffer[3] > '0'));
                        }
                        else {
                            // Obtain the last two digits.
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 4);

                            buffer += (5 + int(buffer[5] > '0'));
                        }
                    }
                }
                else if (s32 >= 100) {
                    // 3 or 4 digits.
                    // 42949673 = ceil(2^32 / 100)
                    auto prod = s32 * UINT64_C(42949673);
                    auto const head_digits = int(prod >> 32);

                    // If s32 is of 4 digits, increase the exponent by 3.
                    // Otherwise, increase it by 2.
                    exponent += (2 + int(head_digits >= 10));

                    // Write the first digit and the decimal point.
                    print_head_chars(head_digits, buffer);
                    // This third character may be overwritten later but we don't care.
                    buffer[2] = radix_100_table[head_digits].bytes[1];

                    // Remaining 2 digits are all zero?
                    if ((prod & UINT32_C(0xffffffff)) <=
                        stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 100)) {
                        // The number of characters actually written is 1 or 3, similarly to the case of
                        // 7 or 8 digits.
                        buffer += (1 + (int(head_digits >= 10) & int(buffer[2] > '0')) * 2);
                    }
                    else {
                        // At least one of the remaining 2 digits are nonzero.
                        // After this adjustment, now the first destination becomes buffer + 2.
                        buffer += int(head_digits >= 10);

                        // Obtain the last two digits.
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 2);

                        buffer += (3 + int(buffer[3] > '0'));
                    }
                }
                else {
                    // 1 or 2 digits.
                    // If s32 is of 2 digits, increase the exponent by 1.
                    exponent += int(s32 >= 10);

                    // Write the first digit and the decimal point.
                    print_head_chars(int(s32), buffer);
                    // This third character may be overwritten later but we don't care.
                    buffer[2] = radix_100_table[s32].bytes[1];

                    // The number of characters actually written is 1 or 3, similarly to the case of
                    // 7 or 8 digits.
                    buffer += (1 + (int(s32 >= 10) & int(buffer[2] > '0')) * 2);
                }
            }

            template <>
            char* to_chars<ieee754_binary32, stdr::uint_least32_t>(stdr::uint_least32_t s32,
                                                                   int exponent,
                                                                   char* buffer) noexcept {
                // Print significand.
                print_9_digits(s32, exponent, buffer);

                // Print exponent and return
                if (exponent < 0) {
                    stdr::memcpy(buffer, "E-", 2);
                    buffer += 2;
                    exponent = -exponent;
                }
                else {
                    buffer[0] = 'E';
                    buffer += 1;
                }

                if (exponent >= 10) {
                    print_2_digits(exponent, buffer);
                    buffer += 2;
                }
                else {
                    print_1_digit(exponent, buffer);
                    buffer += 1;
                }

                return buffer;
            }

            template <>
            char*
            to_chars<ieee754_binary64, stdr::uint_least64_t>(stdr::uint_least64_t const significand,
                                                             int exponent, char* buffer) noexcept {
                // Print significand by decomposing it into a 9-digit block and a 8-digit block.
                stdr::uint_least32_t first_block, second_block;
                bool no_second_block;

                if (significand >= UINT64_C(100000000)) {
                    first_block = stdr::uint_least32_t(significand / UINT64_C(100000000));
                    second_block =
                        stdr::uint_least32_t(significand) - first_block * UINT32_C(100000000);
                    exponent += 8;
                    no_second_block = (second_block == 0);
                }
                else {
                    first_block = stdr::uint_least32_t(significand);
                    no_second_block = true;
                }

                if (no_second_block) {
                    print_9_digits(first_block, exponent, buffer);
                }
                else {
                    // We proceed similarly to print_9_digits(), but since we do not need to remove
                    // trailing zeros, the procedure is a bit simpler.
                    if (first_block >= UINT32_C(100000000)) {
                        // The input is of 17 digits, thus there should be no trailing zero at all.
                        // The first block is of 9 digits.
                        // 1441151882 = ceil(2^57 / 1'0000'0000) + 1
                        auto prod = first_block * UINT64_C(1441151882);
                        prod >>= 25;
                        print_head_chars(int(prod >> 32), buffer);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 2);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 4);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 6);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 8);

                        // The second block is of 8 digits.
                        // 281474978 = ceil(2^48 / 100'0000) + 1
                        prod = second_block * UINT64_C(281474978);
                        prod >>= 16;
                        prod += 1;
                        print_2_digits(int(prod >> 32), buffer + 10);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 12);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 14);
                        prod = (prod & UINT32_C(0xffffffff)) * 100;
                        print_2_digits(int(prod >> 32), buffer + 16);

                        exponent += 8;
                        buffer += 18;
                    }
                    else {
                        if (first_block >= UINT32_C(1000000)) {
                            // 7 or 8 digits.
                            // 281474978 = ceil(2^48 / 100'0000) + 1
                            auto prod = first_block * UINT64_C(281474978);
                            prod >>= 16;
                            auto const head_digits = int(prod >> 32);

                            print_head_chars(head_digits, buffer);
                            buffer[2] = radix_100_table[head_digits].bytes[1];

                            exponent += (6 + int(head_digits >= 10));
                            buffer += int(head_digits >= 10);

                            // Print remaining 6 digits.
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 2);
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 4);
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 6);

                            buffer += 8;
                        }
                        else if (first_block >= 10000) {
                            // 5 or 6 digits.
                            // 429497 = ceil(2^32 / 1'0000)
                            auto prod = first_block * UINT64_C(429497);
                            auto const head_digits = int(prod >> 32);

                            print_head_chars(head_digits, buffer);
                            buffer[2] = radix_100_table[head_digits].bytes[1];

                            exponent += (4 + int(head_digits >= 10));
                            buffer += int(head_digits >= 10);

                            // Print remaining 4 digits.
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 2);
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 4);

                            buffer += 6;
                        }
                        else if (first_block >= 100) {
                            // 3 or 4 digits.
                            // 42949673 = ceil(2^32 / 100)
                            auto prod = first_block * UINT64_C(42949673);
                            auto const head_digits = int(prod >> 32);

                            print_head_chars(head_digits, buffer);
                            buffer[2] = radix_100_table[head_digits].bytes[1];

                            exponent += (2 + int(head_digits >= 10));
                            buffer += int(head_digits >= 10);

                            // Print remaining 2 digits.
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 2);

                            buffer += 4;
                        }
                        else {
                            // 1 or 2 digits.
                            print_head_chars(int(first_block), buffer);
                            buffer[2] = radix_100_table[first_block].bytes[1];

                            exponent += int(first_block >= 10);
                            buffer += (2 + int(first_block >= 10));
                        }

                        // Next, print the second block.
                        // The second block is of 8 digits, but we may have trailing zeros.
                        // 281474978 = ceil(2^48 / 100'0000) + 1
                        auto prod = second_block * UINT64_C(281474978);
                        prod >>= 16;
                        prod += 1;
                        print_2_digits(int(prod >> 32), buffer);

                        // Remaining 6 digits are all zero?
                        if ((prod & UINT32_C(0xffffffff)) <=
                            stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / UINT64_C(1000000))) {
                            buffer += (1 + int(buffer[1] > '0'));
                        }
                        else {
                            // Obtain the next two digits.
                            prod = (prod & UINT32_C(0xffffffff)) * 100;
                            print_2_digits(int(prod >> 32), buffer + 2);

                            // Remaining 4 digits are all zero?
                            if ((prod & UINT32_C(0xffffffff)) <=
                                stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 10000)) {
                                buffer += (3 + int(buffer[3] > '0'));
                            }
                            else {
                                // Obtain the next two digits.
                                prod = (prod & UINT32_C(0xffffffff)) * 100;
                                print_2_digits(int(prod >> 32), buffer + 4);

                                // Remaining 2 digits are all zero?
                                if ((prod & UINT32_C(0xffffffff)) <=
                                    stdr::uint_least32_t((stdr::uint_least64_t(1) << 32) / 100)) {
                                    buffer += (5 + int(buffer[5] > '0'));
                                }
                                else {
                                    // Obtain the last two digits.
                                    prod = (prod & UINT32_C(0xffffffff)) * 100;
                                    print_2_digits(int(prod >> 32), buffer + 6);
                                    buffer += (7 + int(buffer[7] > '0'));
                                }
                            }
                        }
                    }
                }

                // Print exponent and return
                if (exponent < 0) {
                    stdr::memcpy(buffer, "E-", 2);
                    buffer += 2;
                    exponent = -exponent;
                }
                else {
                    buffer[0] = 'E';
                    buffer += 1;
                }

                if (exponent >= 100) {
                    // d1 = exponent / 10; d2 = exponent % 10;
                    // 6554 = ceil(2^16 / 10)
                    auto d1 = (std::uint_least32_t(exponent) * UINT32_C(6554)) >> 16;
                    auto d2 = std::uint_least32_t(exponent) - UINT32_C(10) * d1;
                    print_2_digits(int(d1), buffer);
                    print_1_digit(int(d2), buffer + 2);
                    buffer += 3;
                }
                else if (exponent >= 10) {
                    print_2_digits(exponent, buffer);
                    buffer += 2;
                }
                else {
                    print_1_digit(exponent, buffer);
                    buffer += 1;
                }

                return buffer;
            }
        }
    }
}