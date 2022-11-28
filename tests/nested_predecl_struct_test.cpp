#include "flatbuffers/idl.h"
#include "flatbuffers/flatbuffers.h"
#include "test_assert.h"
#include "nested_predecl_struct_test.h"

namespace flatbuffers {
namespace tests {

void NestedPredeclStructTest()
{
    const char* nested_predecl_struct = R"(
        // This struct contains a "predecl" nested struct
        struct NestedStructTest
        {
            a: Vec3;
        }

        struct Vec3
        {
            x: float;
            y: float;
            z: float;
        }
    )";
    flatbuffers::Parser p;
    TEST_ASSERT(p.Parse(nested_predecl_struct));

    const char* nested_predecl_table = R"(
        // This struct contains a "predecl" nested struct
        struct NestedStructTest
        {
            a: Vec3;
        }

        table Vec3
        {
            x: float;
            y: float;
            z: float;
        }
    )";
    flatbuffers::Parser p2;
    TEST_EQ(p2.Parse(nested_predecl_table), false);
}

}
}
