// automatically generated by the FlatBuffers compiler, do not modify
// swiftlint:disable all
// swiftformat:disable all

import FlatBuffers

public enum optional_scalars_OptionalByte: Int8, Enum, Verifiable {
  public typealias T = Int8
  public static var byteSize: Int { return MemoryLayout<Int8>.size }
  public var value: Int8 { return self.rawValue }
  case none_ = 0
  case one = 1
  case two = 2

  public static var max: optional_scalars_OptionalByte { return .two }
  public static var min: optional_scalars_OptionalByte { return .none_ }
}

extension optional_scalars_OptionalByte: Encodable {
  public func encode(to encoder: Encoder) throws {
    var container = encoder.singleValueContainer()
    switch self {
    case .none_: try container.encode("None")
    case .one: try container.encode("One")
    case .two: try container.encode("Two")
    }
  }
}

public struct optional_scalars_ScalarStuff: FlatBufferObject, Verifiable {

  static func validateVersion() { FlatBuffersVersion_23_5_8() }
  public var __buffer: ByteBuffer! { return _accessor.bb }
  private var _accessor: Table

  public static var id: String { "NULL" } 
  public static func finish(_ fbb: inout FlatBufferBuilder, end: Offset, prefix: Bool = false) { fbb.finish(offset: end, fileId: optional_scalars_ScalarStuff.id, addPrefix: prefix) }
  private init(_ t: Table) { _accessor = t }
  public init(_ bb: ByteBuffer, o: Int32) { _accessor = Table(bb: bb, position: o) }

  private enum VTOFFSET: VOffset {
    case justI8 = 4
    case maybeI8 = 6
    case defaultI8 = 8
    case justU8 = 10
    case maybeU8 = 12
    case defaultU8 = 14
    case justI16 = 16
    case maybeI16 = 18
    case defaultI16 = 20
    case justU16 = 22
    case maybeU16 = 24
    case defaultU16 = 26
    case justI32 = 28
    case maybeI32 = 30
    case defaultI32 = 32
    case justU32 = 34
    case maybeU32 = 36
    case defaultU32 = 38
    case justI64 = 40
    case maybeI64 = 42
    case defaultI64 = 44
    case justU64 = 46
    case maybeU64 = 48
    case defaultU64 = 50
    case justF32 = 52
    case maybeF32 = 54
    case defaultF32 = 56
    case justF64 = 58
    case maybeF64 = 60
    case defaultF64 = 62
    case justBool = 64
    case maybeBool = 66
    case defaultBool = 68
    case justEnum = 70
    case maybeEnum = 72
    case defaultEnum = 74
    var v: Int32 { Int32(self.rawValue) }
    var p: VOffset { self.rawValue }
  }

  public var justI8: Int8 { let o = _accessor.offset(VTOFFSET.justI8.v); return o == 0 ? 0 : _accessor.readBuffer(of: Int8.self, at: o) }
  public var maybeI8: Int8? { let o = _accessor.offset(VTOFFSET.maybeI8.v); return o == 0 ? nil : _accessor.readBuffer(of: Int8.self, at: o) }
  public var defaultI8: Int8 { let o = _accessor.offset(VTOFFSET.defaultI8.v); return o == 0 ? 42 : _accessor.readBuffer(of: Int8.self, at: o) }
  public var justU8: UInt8 { let o = _accessor.offset(VTOFFSET.justU8.v); return o == 0 ? 0 : _accessor.readBuffer(of: UInt8.self, at: o) }
  public var maybeU8: UInt8? { let o = _accessor.offset(VTOFFSET.maybeU8.v); return o == 0 ? nil : _accessor.readBuffer(of: UInt8.self, at: o) }
  public var defaultU8: UInt8 { let o = _accessor.offset(VTOFFSET.defaultU8.v); return o == 0 ? 42 : _accessor.readBuffer(of: UInt8.self, at: o) }
  public var justI16: Int16 { let o = _accessor.offset(VTOFFSET.justI16.v); return o == 0 ? 0 : _accessor.readBuffer(of: Int16.self, at: o) }
  public var maybeI16: Int16? { let o = _accessor.offset(VTOFFSET.maybeI16.v); return o == 0 ? nil : _accessor.readBuffer(of: Int16.self, at: o) }
  public var defaultI16: Int16 { let o = _accessor.offset(VTOFFSET.defaultI16.v); return o == 0 ? 42 : _accessor.readBuffer(of: Int16.self, at: o) }
  public var justU16: UInt16 { let o = _accessor.offset(VTOFFSET.justU16.v); return o == 0 ? 0 : _accessor.readBuffer(of: UInt16.self, at: o) }
  public var maybeU16: UInt16? { let o = _accessor.offset(VTOFFSET.maybeU16.v); return o == 0 ? nil : _accessor.readBuffer(of: UInt16.self, at: o) }
  public var defaultU16: UInt16 { let o = _accessor.offset(VTOFFSET.defaultU16.v); return o == 0 ? 42 : _accessor.readBuffer(of: UInt16.self, at: o) }
  public var justI32: Int32 { let o = _accessor.offset(VTOFFSET.justI32.v); return o == 0 ? 0 : _accessor.readBuffer(of: Int32.self, at: o) }
  public var maybeI32: Int32? { let o = _accessor.offset(VTOFFSET.maybeI32.v); return o == 0 ? nil : _accessor.readBuffer(of: Int32.self, at: o) }
  public var defaultI32: Int32 { let o = _accessor.offset(VTOFFSET.defaultI32.v); return o == 0 ? 42 : _accessor.readBuffer(of: Int32.self, at: o) }
  public var justU32: UInt32 { let o = _accessor.offset(VTOFFSET.justU32.v); return o == 0 ? 0 : _accessor.readBuffer(of: UInt32.self, at: o) }
  public var maybeU32: UInt32? { let o = _accessor.offset(VTOFFSET.maybeU32.v); return o == 0 ? nil : _accessor.readBuffer(of: UInt32.self, at: o) }
  public var defaultU32: UInt32 { let o = _accessor.offset(VTOFFSET.defaultU32.v); return o == 0 ? 42 : _accessor.readBuffer(of: UInt32.self, at: o) }
  public var justI64: Int64 { let o = _accessor.offset(VTOFFSET.justI64.v); return o == 0 ? 0 : _accessor.readBuffer(of: Int64.self, at: o) }
  public var maybeI64: Int64? { let o = _accessor.offset(VTOFFSET.maybeI64.v); return o == 0 ? nil : _accessor.readBuffer(of: Int64.self, at: o) }
  public var defaultI64: Int64 { let o = _accessor.offset(VTOFFSET.defaultI64.v); return o == 0 ? 42 : _accessor.readBuffer(of: Int64.self, at: o) }
  public var justU64: UInt64 { let o = _accessor.offset(VTOFFSET.justU64.v); return o == 0 ? 0 : _accessor.readBuffer(of: UInt64.self, at: o) }
  public var maybeU64: UInt64? { let o = _accessor.offset(VTOFFSET.maybeU64.v); return o == 0 ? nil : _accessor.readBuffer(of: UInt64.self, at: o) }
  public var defaultU64: UInt64 { let o = _accessor.offset(VTOFFSET.defaultU64.v); return o == 0 ? 42 : _accessor.readBuffer(of: UInt64.self, at: o) }
  public var justF32: Float32 { let o = _accessor.offset(VTOFFSET.justF32.v); return o == 0 ? 0.0 : _accessor.readBuffer(of: Float32.self, at: o) }
  public var maybeF32: Float32? { let o = _accessor.offset(VTOFFSET.maybeF32.v); return o == 0 ? nil : _accessor.readBuffer(of: Float32.self, at: o) }
  public var defaultF32: Float32 { let o = _accessor.offset(VTOFFSET.defaultF32.v); return o == 0 ? 42.0 : _accessor.readBuffer(of: Float32.self, at: o) }
  public var justF64: Double { let o = _accessor.offset(VTOFFSET.justF64.v); return o == 0 ? 0.0 : _accessor.readBuffer(of: Double.self, at: o) }
  public var maybeF64: Double? { let o = _accessor.offset(VTOFFSET.maybeF64.v); return o == 0 ? nil : _accessor.readBuffer(of: Double.self, at: o) }
  public var defaultF64: Double { let o = _accessor.offset(VTOFFSET.defaultF64.v); return o == 0 ? 42.0 : _accessor.readBuffer(of: Double.self, at: o) }
  public var justBool: Bool { let o = _accessor.offset(VTOFFSET.justBool.v); return o == 0 ? false : _accessor.readBuffer(of: Bool.self, at: o) }
  public var maybeBool: Bool? { let o = _accessor.offset(VTOFFSET.maybeBool.v); return o == 0 ? nil : _accessor.readBuffer(of: Bool.self, at: o) }
  public var defaultBool: Bool { let o = _accessor.offset(VTOFFSET.defaultBool.v); return o == 0 ? true : _accessor.readBuffer(of: Bool.self, at: o) }
  public var justEnum: optional_scalars_OptionalByte { let o = _accessor.offset(VTOFFSET.justEnum.v); return o == 0 ? .none_ : optional_scalars_OptionalByte(rawValue: _accessor.readBuffer(of: Int8.self, at: o)) ?? .none_ }
  public var maybeEnum: optional_scalars_OptionalByte? { let o = _accessor.offset(VTOFFSET.maybeEnum.v); return o == 0 ? nil : optional_scalars_OptionalByte(rawValue: _accessor.readBuffer(of: Int8.self, at: o)) ?? nil }
  public var defaultEnum: optional_scalars_OptionalByte { let o = _accessor.offset(VTOFFSET.defaultEnum.v); return o == 0 ? .one : optional_scalars_OptionalByte(rawValue: _accessor.readBuffer(of: Int8.self, at: o)) ?? .one }
  public static func startScalarStuff(_ fbb: inout FlatBufferBuilder) -> UOffset { fbb.startTable(with: 36) }
  public static func add(justI8: Int8, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justI8, def: 0, at: VTOFFSET.justI8.p) }
  public static func add(maybeI8: Int8?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeI8, at: VTOFFSET.maybeI8.p) }
  public static func add(defaultI8: Int8, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultI8, def: 42, at: VTOFFSET.defaultI8.p) }
  public static func add(justU8: UInt8, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justU8, def: 0, at: VTOFFSET.justU8.p) }
  public static func add(maybeU8: UInt8?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeU8, at: VTOFFSET.maybeU8.p) }
  public static func add(defaultU8: UInt8, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultU8, def: 42, at: VTOFFSET.defaultU8.p) }
  public static func add(justI16: Int16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justI16, def: 0, at: VTOFFSET.justI16.p) }
  public static func add(maybeI16: Int16?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeI16, at: VTOFFSET.maybeI16.p) }
  public static func add(defaultI16: Int16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultI16, def: 42, at: VTOFFSET.defaultI16.p) }
  public static func add(justU16: UInt16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justU16, def: 0, at: VTOFFSET.justU16.p) }
  public static func add(maybeU16: UInt16?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeU16, at: VTOFFSET.maybeU16.p) }
  public static func add(defaultU16: UInt16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultU16, def: 42, at: VTOFFSET.defaultU16.p) }
  public static func add(justI32: Int32, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justI32, def: 0, at: VTOFFSET.justI32.p) }
  public static func add(maybeI32: Int32?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeI32, at: VTOFFSET.maybeI32.p) }
  public static func add(defaultI32: Int32, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultI32, def: 42, at: VTOFFSET.defaultI32.p) }
  public static func add(justU32: UInt32, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justU32, def: 0, at: VTOFFSET.justU32.p) }
  public static func add(maybeU32: UInt32?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeU32, at: VTOFFSET.maybeU32.p) }
  public static func add(defaultU32: UInt32, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultU32, def: 42, at: VTOFFSET.defaultU32.p) }
  public static func add(justI64: Int64, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justI64, def: 0, at: VTOFFSET.justI64.p) }
  public static func add(maybeI64: Int64?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeI64, at: VTOFFSET.maybeI64.p) }
  public static func add(defaultI64: Int64, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultI64, def: 42, at: VTOFFSET.defaultI64.p) }
  public static func add(justU64: UInt64, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justU64, def: 0, at: VTOFFSET.justU64.p) }
  public static func add(maybeU64: UInt64?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeU64, at: VTOFFSET.maybeU64.p) }
  public static func add(defaultU64: UInt64, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultU64, def: 42, at: VTOFFSET.defaultU64.p) }
  public static func add(justF32: Float32, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justF32, def: 0.0, at: VTOFFSET.justF32.p) }
  public static func add(maybeF32: Float32?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeF32, at: VTOFFSET.maybeF32.p) }
  public static func add(defaultF32: Float32, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultF32, def: 42.0, at: VTOFFSET.defaultF32.p) }
  public static func add(justF64: Double, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justF64, def: 0.0, at: VTOFFSET.justF64.p) }
  public static func add(maybeF64: Double?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeF64, at: VTOFFSET.maybeF64.p) }
  public static func add(defaultF64: Double, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultF64, def: 42.0, at: VTOFFSET.defaultF64.p) }
  public static func add(justBool: Bool, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justBool, def: false,
   at: VTOFFSET.justBool.p) }
  public static func add(maybeBool: Bool?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeBool, at: VTOFFSET.maybeBool.p) }
  public static func add(defaultBool: Bool, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultBool, def: true,
   at: VTOFFSET.defaultBool.p) }
  public static func add(justEnum: optional_scalars_OptionalByte, _ fbb: inout FlatBufferBuilder) { fbb.add(element: justEnum.rawValue, def: 0, at: VTOFFSET.justEnum.p) }
  public static func add(maybeEnum: optional_scalars_OptionalByte?, _ fbb: inout FlatBufferBuilder) { fbb.add(element: maybeEnum?.rawValue, at: VTOFFSET.maybeEnum.p) }
  public static func add(defaultEnum: optional_scalars_OptionalByte, _ fbb: inout FlatBufferBuilder) { fbb.add(element: defaultEnum.rawValue, def: 1, at: VTOFFSET.defaultEnum.p) }
  public static func endScalarStuff(_ fbb: inout FlatBufferBuilder, start: UOffset) -> Offset { let end = Offset(offset: fbb.endTable(at: start)); return end }
  public static func createScalarStuff(
    _ fbb: inout FlatBufferBuilder,
    justI8: Int8 = 0,
    maybeI8: Int8? = nil,
    defaultI8: Int8 = 42,
    justU8: UInt8 = 0,
    maybeU8: UInt8? = nil,
    defaultU8: UInt8 = 42,
    justI16: Int16 = 0,
    maybeI16: Int16? = nil,
    defaultI16: Int16 = 42,
    justU16: UInt16 = 0,
    maybeU16: UInt16? = nil,
    defaultU16: UInt16 = 42,
    justI32: Int32 = 0,
    maybeI32: Int32? = nil,
    defaultI32: Int32 = 42,
    justU32: UInt32 = 0,
    maybeU32: UInt32? = nil,
    defaultU32: UInt32 = 42,
    justI64: Int64 = 0,
    maybeI64: Int64? = nil,
    defaultI64: Int64 = 42,
    justU64: UInt64 = 0,
    maybeU64: UInt64? = nil,
    defaultU64: UInt64 = 42,
    justF32: Float32 = 0.0,
    maybeF32: Float32? = nil,
    defaultF32: Float32 = 42.0,
    justF64: Double = 0.0,
    maybeF64: Double? = nil,
    defaultF64: Double = 42.0,
    justBool: Bool = false,
    maybeBool: Bool? = nil,
    defaultBool: Bool = true,
    justEnum: optional_scalars_OptionalByte = .none_,
    maybeEnum: optional_scalars_OptionalByte? = nil,
    defaultEnum: optional_scalars_OptionalByte = .one
  ) -> Offset {
    let __start = optional_scalars_ScalarStuff.startScalarStuff(&fbb)
    optional_scalars_ScalarStuff.add(justI8: justI8, &fbb)
    optional_scalars_ScalarStuff.add(maybeI8: maybeI8, &fbb)
    optional_scalars_ScalarStuff.add(defaultI8: defaultI8, &fbb)
    optional_scalars_ScalarStuff.add(justU8: justU8, &fbb)
    optional_scalars_ScalarStuff.add(maybeU8: maybeU8, &fbb)
    optional_scalars_ScalarStuff.add(defaultU8: defaultU8, &fbb)
    optional_scalars_ScalarStuff.add(justI16: justI16, &fbb)
    optional_scalars_ScalarStuff.add(maybeI16: maybeI16, &fbb)
    optional_scalars_ScalarStuff.add(defaultI16: defaultI16, &fbb)
    optional_scalars_ScalarStuff.add(justU16: justU16, &fbb)
    optional_scalars_ScalarStuff.add(maybeU16: maybeU16, &fbb)
    optional_scalars_ScalarStuff.add(defaultU16: defaultU16, &fbb)
    optional_scalars_ScalarStuff.add(justI32: justI32, &fbb)
    optional_scalars_ScalarStuff.add(maybeI32: maybeI32, &fbb)
    optional_scalars_ScalarStuff.add(defaultI32: defaultI32, &fbb)
    optional_scalars_ScalarStuff.add(justU32: justU32, &fbb)
    optional_scalars_ScalarStuff.add(maybeU32: maybeU32, &fbb)
    optional_scalars_ScalarStuff.add(defaultU32: defaultU32, &fbb)
    optional_scalars_ScalarStuff.add(justI64: justI64, &fbb)
    optional_scalars_ScalarStuff.add(maybeI64: maybeI64, &fbb)
    optional_scalars_ScalarStuff.add(defaultI64: defaultI64, &fbb)
    optional_scalars_ScalarStuff.add(justU64: justU64, &fbb)
    optional_scalars_ScalarStuff.add(maybeU64: maybeU64, &fbb)
    optional_scalars_ScalarStuff.add(defaultU64: defaultU64, &fbb)
    optional_scalars_ScalarStuff.add(justF32: justF32, &fbb)
    optional_scalars_ScalarStuff.add(maybeF32: maybeF32, &fbb)
    optional_scalars_ScalarStuff.add(defaultF32: defaultF32, &fbb)
    optional_scalars_ScalarStuff.add(justF64: justF64, &fbb)
    optional_scalars_ScalarStuff.add(maybeF64: maybeF64, &fbb)
    optional_scalars_ScalarStuff.add(defaultF64: defaultF64, &fbb)
    optional_scalars_ScalarStuff.add(justBool: justBool, &fbb)
    optional_scalars_ScalarStuff.add(maybeBool: maybeBool, &fbb)
    optional_scalars_ScalarStuff.add(defaultBool: defaultBool, &fbb)
    optional_scalars_ScalarStuff.add(justEnum: justEnum, &fbb)
    optional_scalars_ScalarStuff.add(maybeEnum: maybeEnum, &fbb)
    optional_scalars_ScalarStuff.add(defaultEnum: defaultEnum, &fbb)
    return optional_scalars_ScalarStuff.endScalarStuff(&fbb, start: __start)
  }

  public static func verify<T>(_ verifier: inout Verifier, at position: Int, of type: T.Type) throws where T: Verifiable {
    var _v = try verifier.visitTable(at: position)
    try _v.visit(field: VTOFFSET.justI8.p, fieldName: "justI8", required: false, type: Int8.self)
    try _v.visit(field: VTOFFSET.maybeI8.p, fieldName: "maybeI8", required: false, type: Int8.self)
    try _v.visit(field: VTOFFSET.defaultI8.p, fieldName: "defaultI8", required: false, type: Int8.self)
    try _v.visit(field: VTOFFSET.justU8.p, fieldName: "justU8", required: false, type: UInt8.self)
    try _v.visit(field: VTOFFSET.maybeU8.p, fieldName: "maybeU8", required: false, type: UInt8.self)
    try _v.visit(field: VTOFFSET.defaultU8.p, fieldName: "defaultU8", required: false, type: UInt8.self)
    try _v.visit(field: VTOFFSET.justI16.p, fieldName: "justI16", required: false, type: Int16.self)
    try _v.visit(field: VTOFFSET.maybeI16.p, fieldName: "maybeI16", required: false, type: Int16.self)
    try _v.visit(field: VTOFFSET.defaultI16.p, fieldName: "defaultI16", required: false, type: Int16.self)
    try _v.visit(field: VTOFFSET.justU16.p, fieldName: "justU16", required: false, type: UInt16.self)
    try _v.visit(field: VTOFFSET.maybeU16.p, fieldName: "maybeU16", required: false, type: UInt16.self)
    try _v.visit(field: VTOFFSET.defaultU16.p, fieldName: "defaultU16", required: false, type: UInt16.self)
    try _v.visit(field: VTOFFSET.justI32.p, fieldName: "justI32", required: false, type: Int32.self)
    try _v.visit(field: VTOFFSET.maybeI32.p, fieldName: "maybeI32", required: false, type: Int32.self)
    try _v.visit(field: VTOFFSET.defaultI32.p, fieldName: "defaultI32", required: false, type: Int32.self)
    try _v.visit(field: VTOFFSET.justU32.p, fieldName: "justU32", required: false, type: UInt32.self)
    try _v.visit(field: VTOFFSET.maybeU32.p, fieldName: "maybeU32", required: false, type: UInt32.self)
    try _v.visit(field: VTOFFSET.defaultU32.p, fieldName: "defaultU32", required: false, type: UInt32.self)
    try _v.visit(field: VTOFFSET.justI64.p, fieldName: "justI64", required: false, type: Int64.self)
    try _v.visit(field: VTOFFSET.maybeI64.p, fieldName: "maybeI64", required: false, type: Int64.self)
    try _v.visit(field: VTOFFSET.defaultI64.p, fieldName: "defaultI64", required: false, type: Int64.self)
    try _v.visit(field: VTOFFSET.justU64.p, fieldName: "justU64", required: false, type: UInt64.self)
    try _v.visit(field: VTOFFSET.maybeU64.p, fieldName: "maybeU64", required: false, type: UInt64.self)
    try _v.visit(field: VTOFFSET.defaultU64.p, fieldName: "defaultU64", required: false, type: UInt64.self)
    try _v.visit(field: VTOFFSET.justF32.p, fieldName: "justF32", required: false, type: Float32.self)
    try _v.visit(field: VTOFFSET.maybeF32.p, fieldName: "maybeF32", required: false, type: Float32.self)
    try _v.visit(field: VTOFFSET.defaultF32.p, fieldName: "defaultF32", required: false, type: Float32.self)
    try _v.visit(field: VTOFFSET.justF64.p, fieldName: "justF64", required: false, type: Double.self)
    try _v.visit(field: VTOFFSET.maybeF64.p, fieldName: "maybeF64", required: false, type: Double.self)
    try _v.visit(field: VTOFFSET.defaultF64.p, fieldName: "defaultF64", required: false, type: Double.self)
    try _v.visit(field: VTOFFSET.justBool.p, fieldName: "justBool", required: false, type: Bool.self)
    try _v.visit(field: VTOFFSET.maybeBool.p, fieldName: "maybeBool", required: false, type: Bool.self)
    try _v.visit(field: VTOFFSET.defaultBool.p, fieldName: "defaultBool", required: false, type: Bool.self)
    try _v.visit(field: VTOFFSET.justEnum.p, fieldName: "justEnum", required: false, type: optional_scalars_OptionalByte.self)
    try _v.visit(field: VTOFFSET.maybeEnum.p, fieldName: "maybeEnum", required: false, type: optional_scalars_OptionalByte.self)
    try _v.visit(field: VTOFFSET.defaultEnum.p, fieldName: "defaultEnum", required: false, type: optional_scalars_OptionalByte.self)
    _v.finish()
  }
}

extension optional_scalars_ScalarStuff: Encodable {

  enum CodingKeys: String, CodingKey {
    case justI8 = "just_i8"
    case maybeI8 = "maybe_i8"
    case defaultI8 = "default_i8"
    case justU8 = "just_u8"
    case maybeU8 = "maybe_u8"
    case defaultU8 = "default_u8"
    case justI16 = "just_i16"
    case maybeI16 = "maybe_i16"
    case defaultI16 = "default_i16"
    case justU16 = "just_u16"
    case maybeU16 = "maybe_u16"
    case defaultU16 = "default_u16"
    case justI32 = "just_i32"
    case maybeI32 = "maybe_i32"
    case defaultI32 = "default_i32"
    case justU32 = "just_u32"
    case maybeU32 = "maybe_u32"
    case defaultU32 = "default_u32"
    case justI64 = "just_i64"
    case maybeI64 = "maybe_i64"
    case defaultI64 = "default_i64"
    case justU64 = "just_u64"
    case maybeU64 = "maybe_u64"
    case defaultU64 = "default_u64"
    case justF32 = "just_f32"
    case maybeF32 = "maybe_f32"
    case defaultF32 = "default_f32"
    case justF64 = "just_f64"
    case maybeF64 = "maybe_f64"
    case defaultF64 = "default_f64"
    case justBool = "just_bool"
    case maybeBool = "maybe_bool"
    case defaultBool = "default_bool"
    case justEnum = "just_enum"
    case maybeEnum = "maybe_enum"
    case defaultEnum = "default_enum"
  }
  public func encode(to encoder: Encoder) throws {
    var container = encoder.container(keyedBy: CodingKeys.self)
    if justI8 != 0 {
      try container.encodeIfPresent(justI8, forKey: .justI8)
    }
    try container.encodeIfPresent(maybeI8, forKey: .maybeI8)
    if defaultI8 != 42 {
      try container.encodeIfPresent(defaultI8, forKey: .defaultI8)
    }
    if justU8 != 0 {
      try container.encodeIfPresent(justU8, forKey: .justU8)
    }
    try container.encodeIfPresent(maybeU8, forKey: .maybeU8)
    if defaultU8 != 42 {
      try container.encodeIfPresent(defaultU8, forKey: .defaultU8)
    }
    if justI16 != 0 {
      try container.encodeIfPresent(justI16, forKey: .justI16)
    }
    try container.encodeIfPresent(maybeI16, forKey: .maybeI16)
    if defaultI16 != 42 {
      try container.encodeIfPresent(defaultI16, forKey: .defaultI16)
    }
    if justU16 != 0 {
      try container.encodeIfPresent(justU16, forKey: .justU16)
    }
    try container.encodeIfPresent(maybeU16, forKey: .maybeU16)
    if defaultU16 != 42 {
      try container.encodeIfPresent(defaultU16, forKey: .defaultU16)
    }
    if justI32 != 0 {
      try container.encodeIfPresent(justI32, forKey: .justI32)
    }
    try container.encodeIfPresent(maybeI32, forKey: .maybeI32)
    if defaultI32 != 42 {
      try container.encodeIfPresent(defaultI32, forKey: .defaultI32)
    }
    if justU32 != 0 {
      try container.encodeIfPresent(justU32, forKey: .justU32)
    }
    try container.encodeIfPresent(maybeU32, forKey: .maybeU32)
    if defaultU32 != 42 {
      try container.encodeIfPresent(defaultU32, forKey: .defaultU32)
    }
    if justI64 != 0 {
      try container.encodeIfPresent(justI64, forKey: .justI64)
    }
    try container.encodeIfPresent(maybeI64, forKey: .maybeI64)
    if defaultI64 != 42 {
      try container.encodeIfPresent(defaultI64, forKey: .defaultI64)
    }
    if justU64 != 0 {
      try container.encodeIfPresent(justU64, forKey: .justU64)
    }
    try container.encodeIfPresent(maybeU64, forKey: .maybeU64)
    if defaultU64 != 42 {
      try container.encodeIfPresent(defaultU64, forKey: .defaultU64)
    }
    if justF32 != 0.0 {
      try container.encodeIfPresent(justF32, forKey: .justF32)
    }
    try container.encodeIfPresent(maybeF32, forKey: .maybeF32)
    if defaultF32 != 42.0 {
      try container.encodeIfPresent(defaultF32, forKey: .defaultF32)
    }
    if justF64 != 0.0 {
      try container.encodeIfPresent(justF64, forKey: .justF64)
    }
    try container.encodeIfPresent(maybeF64, forKey: .maybeF64)
    if defaultF64 != 42.0 {
      try container.encodeIfPresent(defaultF64, forKey: .defaultF64)
    }
    if justBool != false {
      try container.encodeIfPresent(justBool, forKey: .justBool)
    }
    try container.encodeIfPresent(maybeBool, forKey: .maybeBool)
    if defaultBool != true {
      try container.encodeIfPresent(defaultBool, forKey: .defaultBool)
    }
    if justEnum != .none_ {
      try container.encodeIfPresent(justEnum, forKey: .justEnum)
    }
    try container.encodeIfPresent(maybeEnum, forKey: .maybeEnum)
    if defaultEnum != .one {
      try container.encodeIfPresent(defaultEnum, forKey: .defaultEnum)
    }
  }
}

