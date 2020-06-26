#if !canImport(ObjectiveC)
import XCTest

extension FlatBuffersDoubleTests {
    // DO NOT MODIFY: This is autogenerated, use:
    //   `swift test --generate-linuxmain`
    // to regenerate.
    static let __allTests__FlatBuffersDoubleTests = [
        ("testCreateFinish", testCreateFinish),
        ("testCreateFinishWithPrefix", testCreateFinishWithPrefix),
    ]
}

extension FlatBuffersMonsterWriterTests {
    // DO NOT MODIFY: This is autogenerated, use:
    //   `swift test --generate-linuxmain`
    // to regenerate.
    static let __allTests__FlatBuffersMonsterWriterTests = [
        ("testCreateMonster", testCreateMonster),
        ("testCreateMonsterPrefixed", testCreateMonsterPrefixed),
        ("testCreateMonsterResizedBuffer", testCreateMonsterResizedBuffer),
        ("testCreateMonsterUsingCreateMonsterMethodWithNilPos", testCreateMonsterUsingCreateMonsterMethodWithNilPos),
        ("testCreateMonsterUsingCreateMonsterMethodWithPosX", testCreateMonsterUsingCreateMonsterMethodWithPosX),
        ("testData", testData),
        ("testReadFromOtherLanguages", testReadFromOtherLanguages),
        ("testReadMonsterFromUnsafePointerWithoutCopying", testReadMonsterFromUnsafePointerWithoutCopying),
    ]
}

extension FlatBuffersStructsTests {
    // DO NOT MODIFY: This is autogenerated, use:
    //   `swift test --generate-linuxmain`
    // to regenerate.
    static let __allTests__FlatBuffersStructsTests = [
        ("testCreatingEnums", testCreatingEnums),
        ("testCreatingStruct", testCreatingStruct),
        ("testCreatingVectorStruct", testCreatingVectorStruct),
        ("testCreatingVectorStructWithForcedDefaults", testCreatingVectorStructWithForcedDefaults),
        ("testReadingStruct", testReadingStruct),
        ("testReadingStructWithEnums", testReadingStructWithEnums),
        ("testWritingAndMutatingBools", testWritingAndMutatingBools),
    ]
}

extension FlatBuffersTests {
    // DO NOT MODIFY: This is autogenerated, use:
    //   `swift test --generate-linuxmain`
    // to regenerate.
    static let __allTests__FlatBuffersTests = [
        ("testCreateFinish", testCreateFinish),
        ("testCreateFinishWithPrefix", testCreateFinishWithPrefix),
        ("testCreateString", testCreateString),
        ("testEndian", testEndian),
        ("testOffset", testOffset),
        ("testReadCountry", testReadCountry),
        ("testStartTable", testStartTable),
    ]
}

extension FlatBuffersUnionTests {
    // DO NOT MODIFY: This is autogenerated, use:
    //   `swift test --generate-linuxmain`
    // to regenerate.
    static let __allTests__FlatBuffersUnionTests = [
        ("testCreateMonstor", testCreateMonstor),
        ("testEndTableFinish", testEndTableFinish),
        ("testEnumVector", testEnumVector),
        ("testUnionVector", testUnionVector),
    ]
}

extension FlatBuffersVectors {
    // DO NOT MODIFY: This is autogenerated, use:
    //   `swift test --generate-linuxmain`
    // to regenerate.
    static let __allTests__FlatBuffersVectors = [
        ("testCreateEmptyIntArray", testCreateEmptyIntArray),
        ("testCreateIntArray", testCreateIntArray),
        ("testCreateSharedStringVector", testCreateSharedStringVector),
        ("testCreateVectorOfStrings", testCreateVectorOfStrings),
        ("testCreatingTwoCountries", testCreatingTwoCountries),
        ("testReadDoubleArray", testReadDoubleArray),
        ("testReadInt32Array", testReadInt32Array),
    ]
}

public func __allTests() -> [XCTestCaseEntry] {
    return [
        testCase(FlatBuffersDoubleTests.__allTests__FlatBuffersDoubleTests),
        testCase(FlatBuffersMonsterWriterTests.__allTests__FlatBuffersMonsterWriterTests),
        testCase(FlatBuffersStructsTests.__allTests__FlatBuffersStructsTests),
        testCase(FlatBuffersTests.__allTests__FlatBuffersTests),
        testCase(FlatBuffersUnionTests.__allTests__FlatBuffersUnionTests),
        testCase(FlatBuffersVectors.__allTests__FlatBuffersVectors),
    ]
}
#endif
