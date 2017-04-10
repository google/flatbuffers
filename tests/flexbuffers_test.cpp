//
//  flexbuffers_test.cpp
//  FlatBuffers
//
//  Created by Maxim Zaks on 01.04.17.
//
//

#include "flatbuffers/flexbuffers.h"

#define TEST_OUTPUT_LINE(...) \
{ printf(__VA_ARGS__); printf("\n"); }

static int testing_fails2 = 0;

static unsigned long _index = 0UL;

static void TestFail2(const char *expval, const char *val, const char *exp,
              const char *file, int line) {
    TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s (%s) != %s", file, line,
                     exp, expval, val);
    assert(0);
    testing_fails2++;
}

template<typename T, typename U>
static void TestEq(T expval, U val, const char *exp, const char *file, int line) {
    if (U(expval) != val) {
        TEST_OUTPUT_LINE("_index: %lu", _index);
        TestFail2(flatbuffers::NumToString(expval).c_str(),
                 flatbuffers::NumToString(val).c_str(),
                 exp, file, line);
    }
}

#define TEST_EQ(exp, val) TestEq(exp,         val, #exp, __FILE__, __LINE__)

#define check(...) unsigned char expected[] = {__VA_ARGS__}; \
TEST_EQ(sizeof(expected), slb.GetBuffer().size());\
for (_index = 0; _index < sizeof(expected); _index++) { \
    TEST_EQ(data[_index], expected[_index]);\
}\

#define initCheck() _index = 0UL;
#define partCheck(...) {\
unsigned char expected[] = {__VA_ARGS__}; \
for (unsigned long __i = 0; _index + __i < sizeof(expected); __i++) { \
TEST_EQ(data[_index+__i], expected[__i]);\
}\
_index += sizeof(expected);\
}

#define skip(i) _index += i;

#define endCheck() TEST_EQ(_index, slb.GetBuffer().size());

#define start() flexbuffers::Builder slb(512, flexbuffers::BUILDER_FLAG_SHARE_KEYS_AND_STRINGS);
#define finish() slb.Finish(); auto data = slb.GetBuffer().data();
#define dump() for (size_t i = 0; i < slb.GetBuffer().size(); i++)\
printf("%d, ", data[i]);\
printf("\n");


void FlexBuffersEncodingTest() {
    { // encode null
        start()
        slb.Null();
        finish()
        check(0, 0, 1)
    }
    { // enocde true
        start()
        slb.Add(true);
        finish()
        check(1, 4, 1)
    }
    { // encode false
        start()
        slb.Add(false);
        finish()
        check(0, 4, 1)
    }
    { // encode int
        start()
        slb.Add(25);
        finish()
        check(25, 4, 1)
    }
    { // encode negative int
        start()
        slb.Add(-25);
        finish()
        check(231, 4, 1)
    }
    { // encode uint
        start()
        slb.Add((uint8_t)250);
        finish()
        check(250, 8, 1)
    }
    { // encode 2 byte int
        start()
        slb.Add(1025);
        finish()
        check(1, 4, 5, 2)
    }
    { // encode 4 byte int
        start()
        slb.Add(INT_MAX);
        finish()
        check(255, 255, 255, 127, 6, 4)
    }
    { // encode 8 byte int
        start()
        slb.Add(INT_FAST64_MAX);
        finish()
        check(255, 255, 255, 255, 255, 255, 255, 127, 7, 8)
    }
    { // encode 2 byte uint
        start()
        slb.Add((uint16_t)1025);
        finish()
        check(1, 4, 9, 2)
    }
    { // encode 4 byte uint
        start()
        slb.Add(UINT_MAX);
        finish()
        check(255, 255, 255, 255, 10, 4)
    }
    { // encode 8 byte uint
        start()
        slb.Add(UINT_FAST64_MAX);
        finish()
        check(255, 255, 255, 255, 255, 255, 255, 255, 11, 8)
    }
    { // encode float (4byte)
        start()
        slb.Add(4.5);
        finish()
        check(0, 0, 144, 64, 14, 4)
    }
    { // encode double (8byte)
        start()
        slb.Add(0.1);
        finish()
        check(154, 153, 153, 153, 153, 153, 185, 63, 15, 8)
    }
    { // encode string
        start()
        slb.Add("Maxim");
        finish()
        check(5, 77, 97, 120, 105, 109, 0, 6, 20, 1)
    }
    { // encode 1byte vector
        start()
        int8_t arr[] = { 1, 2, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 1, 2, 3, 3, 44, 1)
    }
    { // encode unsigned 2byte int vector
        start()
        int16_t arr[] = { 1, 555, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 0, 1, 0, 43, 2, 3, 0, 6, 45, 1)
    }
    { // encode unsigned 4byte int vector
        start()
        int arr[] = { 1, 55500, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 0, 0, 0, 1, 0, 0, 0, 204, 216, 0, 0, 3, 0, 0, 0, 12, 46, 1)
    }
    { // encode unsigned 8byte int vector
        start()
        int64_t arr[] = { 1, 55555555500, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 172, 128, 94, 239, 12, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 24, 47, 1)
//        initCheck()
//        partCheck(3)
//        skip(7UL)
//        partCheck(1, 0, 0, 0, 0, 0, 0, 0, 172, 128, 94, 239, 12, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 24, 47, 1)
//        endCheck()
    }
    { // encode unsigned byte vector
        start()
        uint8_t arr[] = { 1, 2, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 1, 2, 3, 3, 48, 1)
    }
    { // encode unsigned 2byte vector
        start()
        uint16_t arr[] = { 1, 555, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 0, 1, 0, 43, 2, 3, 0, 6, 49, 1)
    }
    { // encode unsigned 4byte vector
        start()
        uint32_t arr[] = { 1, 55500, 3 };
        slb.Vector(arr, 3);
        finish()
        check(3, 0, 0, 0, 1, 0, 0, 0, 204, 216, 0, 0, 3, 0, 0, 0, 12, 50, 1)
    }
    { // encode unsigned 8byte vector
        start()
        uint64_t arr[] = { 1, 55555555500, 3 };
        slb.Vector(arr, 3);
        finish()
        
        initCheck()
        partCheck(3)
        skip(7UL)
        partCheck(1, 0, 0, 0, 0, 0, 0, 0, 172, 128, 94, 239, 12, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 24, 51, 1)
        endCheck()
    }
    { // encode float vector (4byte)
        start()
        float arr[] = { 1.5, 2.5, 3.5 };
        slb.Vector(arr, 3);
        finish()
        check(3, 0, 0, 0, 0, 0, 192, 63, 0, 0, 32, 64, 0, 0, 96, 64, 12, 54, 1)
    }
    { // encode double vector (8byte)
        start()
        double arr[] = { 1.1, 2.2, 3.3 };
        slb.Vector(arr, 3);
        finish()
        
        initCheck()
        partCheck(3)
        skip(7UL)
        partCheck(154, 153, 153, 153, 153, 153, 241, 63, 154, 153, 153, 153, 153, 153, 1, 64, 102, 102, 102, 102, 102, 102, 10, 64, 24, 55, 1)
        endCheck()
    }
    { // encode bool vector
        start()
        int8_t arr[] = { true, false, true };
        slb.Vector(arr, 3);
        finish()
        check(3, 1, 0, 1, 3, 44, 1)
    }
    { // encode nested vector
        start()
        slb.Vector([&]() {
            slb.Vector([&]() {
                slb.Add(61);
            });
            slb.Add(64);
        });
        finish()
        check(1, 61, 4, 2, 3, 64, 40, 4, 4, 40, 1)
    }
    { // encode vector with one int and multiple nulls
        start()
        slb.Vector([&]() {
            slb.Null();
            slb.Null();
            slb.Add(64);
            slb.Null();
        });
        finish()
        check(4, 0, 0, 64, 0, 0, 0, 4, 0, 8, 40, 1)
    }
    { // encode string vector
        start()
        slb.Vector([&]() {
            slb.Add("foo");
            slb.Add("bar");
            slb.Add("baz");
        });
        finish()
        check(3, 102, 111, 111, 0, 3, 98, 97, 114, 0, 3, 98, 97, 122, 0, 3, 15, 11, 7, 20, 20, 20, 6, 40, 1)
    }
    { // encode typed string vector
        start()
        slb.TypedVector([&]() {
            slb.Add("foo");
            slb.Add("bar");
            slb.Add("baz");
        });
        finish()
        check(3, 102, 111, 111, 0, 3, 98, 97, 114, 0, 3, 98, 97, 122, 0, 3, 15, 11, 7, 3, 60, 1)
    }
    { // encode floats vector not fixed
        start()
        slb.Vector([&]() {
            slb.Add(4.5);
            slb.Add(78.3);
            slb.Add(29.2);
        });
        finish()
        
        initCheck()
        partCheck(3)
        skip(7UL)
        partCheck(0, 0, 0, 0, 0, 0, 18, 64, 51, 51, 51, 51, 51, 147, 83, 64, 51, 51, 51, 51, 51, 51, 61, 64, 15, 15, 15, 27, 43, 1)
        endCheck()
    }
    { // encode int vector not typed
        start()
        slb.Vector([&]() {
            slb.Add(4);
            slb.Add(7);
            slb.Add(29);
        });
        finish()
        check(3, 4, 7, 29, 4, 4, 4, 6, 40, 1)
    }
    { // encode map with one key valeu pair
        start()
        slb.Map([&]() {
            slb.Add("a", 12);
        });
        finish()
        check(97, 0, 1, 3, 1, 1, 1, 12, 4, 2, 36, 1)
    }
    { // encode map with two key value pairs
        start()
        slb.Map([&]() {
            slb.Add("", 45);
            slb.Add("a", 12);
        });
        finish()
        check(0, 97, 0, 2, 4, 4, 2, 1, 2, 45, 12, 4, 4, 4, 36, 1)
    }
    { // encode map with two key value pairs and need for sorting
        start()
        slb.Map([&]() {
            slb.Add("a", 12);
            slb.Add("", 45);
        });
        finish()
        check(97, 0, 0, 2, 2, 5, 2, 1, 2, 45, 12, 4, 4, 4, 36, 1)
    }
    { // encode complex map
        start()
        slb.Map([&]() {
            slb.Add("age", 35);
            slb.Vector("flags",  [&]() {
                slb.Add(true);
                slb.Add(false);
                slb.Add(true);
                slb.Add(true);
            });
            slb.Add("weight", 72.5);
            slb.Add("name", "Maxim");
            slb.Map("address", [&]() {
                slb.Add("city", "Bla");
                slb.Add("zip", "12345");
                slb.Add("countryCode", "XX");
            });
        });
        finish()
        check(97, 103, 101, 0, 102, 108, 97, 103, 115, 0, 4, 1, 0, 1, 1, 4, 4, 4, 4, 119, 101, 105, 103, 104, 116, 0, 110, 97, 109, 101, 0, 5, 77, 97, 120, 105, 109, 0, 97, 100, 100, 114, 101, 115, 115, 0, 99, 105, 116, 121, 0, 3, 66, 108, 97, 0, 122, 105, 112, 0, 5, 49, 50, 51, 52, 53, 0, 99, 111, 117, 110, 116, 114, 121, 67, 111, 100, 101, 0, 2, 88, 88, 0, 3, 38, 18, 30, 3, 1, 3, 38, 11, 31, 20, 20, 20, 5, 59, 98, 95, 74, 82, 0, 0, 7, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 26, 0, 0, 0, 35, 0, 0, 0, 113, 0, 0, 0, 96, 0, 0, 0, 0, 0, 145, 66, 36, 6, 40, 20, 14, 25, 38, 1)
    }
    { // encode map with indirect values
        start()
        slb.Map([&]() {
            slb.IndirectUInt("c", 45);
            slb.IndirectInt("a", -20);
            slb.IndirectFloat("b", 7.5);
            slb.IndirectDouble("d", 56.123);
        });
        finish()
        check(99, 0, 45, 97, 0, 236, 98, 0, 0, 0, 240, 64, 100, 0, 0, 0, 57, 180, 200, 118, 190, 15, 76, 64, 4, 22, 20, 27, 16, 4, 1, 4, 27, 25, 32, 19, 24, 34, 28, 35, 8, 36, 1)
    }
    { // encode map with indirect values
        start()
        slb.Map([&]() {
            slb.IndirectFloat("a", 2.5);
        });
        finish()
        check(97, 0, 0, 0, 0, 0, 32, 64, 1, 9, 1, 1, 1, 9, 34, 2, 36, 1)
        auto map = flexbuffers::GetRoot(slb.GetBuffer()).AsMap();
        TEST_OUTPUT_LINE("%f", map["a"].AsFloat());
    }
    { // encode vector with indirect values
        start()
        slb.Vector([&]() {
            slb.IndirectUInt(45);
            slb.IndirectInt(-20);
            slb.IndirectFloat(7.5);
            slb.IndirectDouble(56.123);
        });
        finish()
        check(45, 236, 0, 0, 0, 0, 240, 64, 57, 180, 200, 118, 190, 15, 76, 64, 4, 17, 17, 15, 12, 28, 24, 34, 35, 8, 40, 1)
    }
    { // encode typed int vector with leght higher than 1byte
        start()
        slb.TypedVector([&]() {
            for (int i = 0; i < 260; i++){
                slb.Add(1);
            }
        });
        finish()
        
        initCheck()
        partCheck(4,1) // vector length
        for (int i = 0; i < 260; i++){
            partCheck(1)
            skip(1) // skip pading
        }
        partCheck(8, 2, 45, 2) // check type and offset to vector start
        endCheck()
    }
    
    assert(testing_fails2 == 0);
}
