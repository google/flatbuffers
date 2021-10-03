import Foundation

public protocol FlatbuffersJSONDecodable {
  static func decode(decoder: DecoderContainer, builder: inout FlatBufferBuilder) throws -> Offset
}

public protocol FlatbuffersDecodable: NativeStruct {
  init(decoder: DecoderContainer) throws
}

public protocol FlatbuffersEnumDecodable: Enum {
  init?(value: String?)
}

extension String: FlatbuffersJSONDecodable {
  public static func decode(decoder: DecoderContainer, builder: inout FlatBufferBuilder) throws -> Offset {
    let container = decoder.singleValueContainer()
    let str: String? = container.decode(type: String.self)
    return builder.create(string: str)
  }
}

public struct UnkeyedDecoderContainer {

  private let array: NSArray

  init (array: NSArray) {
    self.array = array
  }

  public func encodeAsString(to builder: inout FlatBufferBuilder) throws -> Offset {
    guard let array = array as? [String] else {
      throw FlatbuffersErrors.couldNotCastArray(to: "String")
    }
    return builder.createVector(ofStrings: array)
  }

  public func encodeAs<T>(type: T.Type, to builder: inout FlatBufferBuilder) throws -> Offset where T: FlatbuffersEnumDecodable {
    guard let array = array as? [String] else {
      throw FlatbuffersErrors.couldNotCastArray(to: "String")
    }
    return builder.createVector(array.compactMap({ T.init(value: $0) }))
  }

  public func values<T>(type: T.Type) throws -> [T]? where T: FlatbuffersEnumDecodable {
    guard let array = array as? [String] else {
      return nil
    }
    return array.compactMap({ T.init(value: $0) })
  }

  public func encodeAs<T>(type: T.Type, to builder: inout FlatBufferBuilder) throws -> Offset where T: Scalar {
    guard let array = array as? [T] else {
      throw FlatbuffersErrors.couldNotCastArray(to: String(describing: T.self))
    }
    return builder.createVector(array)
  }

  public func encodeAsStruct<T>(type: T.Type, to builder: inout FlatBufferBuilder) throws -> Offset where T: FlatbuffersDecodable {
    var items: [T] = []
    for item in array {
      guard let item = item as? [String: Any] else { continue }
      items.append(try T.init(decoder: DecoderContainer(data: item)))
    }
    return builder.createVector(ofStructs: items)
  }

  public func encodeAs<T>(type: T.Type, to builder: inout FlatBufferBuilder) throws -> Offset where T: FlatbuffersJSONDecodable {
    var offsets: [Offset] = []
    for item in array {
      guard let item = item as? [String: Any] else { continue }
      let off = try T.decode(decoder: DecoderContainer(data: item), builder: &builder)
      offsets.append(off)
    }
    return builder.createVector(ofOffsets: offsets)
  }

  public func value<Element>(at index: Int, builder: inout FlatBufferBuilder, type: Element.Type) throws -> Offset? where Element: FlatbuffersJSONDecodable {
    let item = array[index]
    if let item = item as? [String: Any] {
      return try Element.decode(decoder: DecoderContainer(data: item), builder: &builder)
    } else {
      return try Element.decode(decoder: DecoderContainer(data: ["i": item]), builder: &builder)
    }
  }

}

public struct UniqueValueDecoderContainer {
  init(value: Any?) {
    self.value = value
  }

  public func decode<T>(type: T.Type) -> T? {
    return value as? T
  }

  public func decode<T>(type: T.Type) throws -> T {
    guard let value = decode(type: T.self) else {
      throw FlatbuffersErrors.couldNotDecodeValue
    }
    return value
  }

  private let value: Any?
}

public struct KeyedDecoderContainer<T> where T: CodingKey {

  private let data: [String: Any]

  init(data: [String: Any]) {
    self.data = data
  }

  // MARK: - Decoding JSON

  public func object<Element>(
    for key: T,
    builder: inout FlatBufferBuilder,
    type: Element.Type)
  throws -> Offset where Element: FlatbuffersJSONDecodable
  {
    guard let element = data[key.stringValue] as? [String: Any] else {
      return Offset()
    }
    return try Element.decode(decoder: DecoderContainer(data: element), builder: &builder)
  }

  // MARK: - Structs and enums

  public func value<Element>(for key: T, type: Element.Type) throws -> Element? where Element: FlatbuffersEnumDecodable {
    Element.init(value: data[key.stringValue] as? String)
  }

  public func value<Element>(for key: T, type: Element.Type) throws -> Element where Element : FlatbuffersEnumDecodable {
    guard let value = try value(for: key, type: type) else {
      throw FlatbuffersErrors.valueNotFoundJSON(fieldName: key.stringValue)
    }
    return value
  }

  public func value<Element>(for key: T, builder: inout FlatBufferBuilder, type: Element.Type) throws -> Offset? where Element: FlatbuffersDecodable {
    guard let element = data[key.stringValue] as? [String: Any],
            let _object = try? Element.init(decoder: DecoderContainer(data: element)) else {
      return nil
    }
    return builder.create(struct: _object)
  }

  public func value<Element>(for key: T, type: Element.Type) throws -> Element? where Element: FlatbuffersDecodable {
    guard let element = data[key.stringValue] as? [String: Any] else {
      return nil
    }
    return try Element.init(decoder: DecoderContainer(data: element))
  }

  public func value<Element>(for key: T, type: Element.Type) throws -> Element where Element: FlatbuffersDecodable {
    guard let value = try value(for: key, type: type) else {
      throw FlatbuffersErrors.valueNotFoundJSON(fieldName: key.stringValue)
    }
    return value
  }

  // MARK: - Array

  public func values(for key: T) throws -> UnkeyedDecoderContainer? {
    guard let array = data[key.stringValue] as? NSArray else {
      return nil
    }
    return UnkeyedDecoderContainer(array: array)
  }

  public func values(for key: T) throws -> UnkeyedDecoderContainer {
    guard let value = try values(for: key) else {
      throw FlatbuffersErrors.valueNotFoundJSON(fieldName: key.stringValue)
    }
    return value
  }

  // MARK: - Types

  public func value(for key: T, builder: inout FlatBufferBuilder) throws -> Offset? {
    guard let str = data[key.stringValue] as? String else {
      return nil
    }
    return builder.create(string: str)
  }

  public func value<Element>(for key: T, type: Element.Type) throws -> Element? where Element: Scalar {
    return data[key.stringValue] as? Element
  }

  public func value<Element>(for key: T, type: Element.Type) throws -> Element where Element : Scalar {
    guard let value = try value(for: key, type: type) else {
      throw FlatbuffersErrors.valueNotFoundJSON(fieldName: key.stringValue)
    }
    return value
  }

}

public struct DecoderContainer {
  
  let data: [String: Any]

  init(data: [String: Any]) {
    self.data = data
  }

  public func keyedContainer<T>(codingKey: T.Type) -> KeyedDecoderContainer<T> {
    return KeyedDecoderContainer(data: data)
  }

  public func singleValueContainer() -> UniqueValueDecoderContainer {
    return UniqueValueDecoderContainer(value: data.first?.value)
  }
}

public class FlatbuffersJSONDecoder {

  private lazy var builder = FlatBufferBuilder(initialSize: 64)
  private let data: [String: Any]

  init(data: [String: Any]) {
    self.data = data
    self.builder = builder
  }

  public func decode<T>(type: T.Type) throws -> ByteBuffer where T: FlatbuffersJSONDecodable {
    let offset = try T.decode(decoder: DecoderContainer(data: data), builder: &builder)
    builder.finish(offset: offset)
    return builder.sizedBuffer
  }

  public static func build(with data: Data) throws -> FlatbuffersJSONDecoder {
    let _data = try JSONSerialization.jsonObject(
      with: data,
      options: .fragmentsAllowed)
    guard let data = _data as? [String: Any], JSONSerialization.isValidJSONObject(_data) else {
      throw FlatbuffersErrors.notValidJSONObject
    }
    return FlatbuffersJSONDecoder(data: data)
  }

}

