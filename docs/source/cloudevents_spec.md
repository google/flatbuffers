# FlatBuffers Event Format for CloudEvents - Version 1.0.0-wip

## Abstract

The FlatBuffers Format for [CloudEvents](https://cloudevents.io) defines how events are expressed in the FlatBuffers Schema Language.

## Table of Contents

1. [Introduction](#1-introduction)
2. [CloudEvent Attributes](#2-cloudevent-attributes)
3. [Data](#3-data)
4. [Transport](#4-transport)
5. [Schema Evolution](#5-schema-evolution)
6. [Examples](#6-examples)

## 1. Introduction

CloudEvents is a standardized and protocol-agnostic definition of the structure and metadata description of events. This specification defines how CloudEvents are to be represented using FlatBuffers.

The CloudEvent Attributes section describes the naming conventions and data type mappings for CloudEvents attributes.

This specification does not define an envelope format. The FlatBuffers type system's intent is primarily to provide a consistent type system for structured data serialization.

The FlatBuffers event format does not currently define a batch mode format.

### 1.1. Conformance

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC2119.

## 2. CloudEvent Attributes

This section defines how CloudEvents attributes are mapped to the FlatBuffers type system. This specification explicitly maps each attribute.

### 2.1 Type System Mapping

The CloudEvents type system MUST be mapped to FlatBuffers types as follows:

| CloudEvents    | FlatBuffers                              |
|----------------|------------------------------------------|
| Boolean        | bool                                     |
| Integer        | int                                      |
| String         | string                                   |
| Binary         | [ubyte] (vector of unsigned bytes)       |
| URI            | string following RFC 3986 §4.3           |
| URI-reference  | string following RFC 3986 §4.1           |
| Timestamp      | string following RFC 3339 (ISO 8601)     |

Extension specifications MAY define secondary mapping rules for the values of attributes they define, but MUST also include the previously defined primary mapping.

### 2.2 REQUIRED Attributes

The following attributes are REQUIRED to be present in all CloudEvents:

- **id**: string
- **source**: string 
- **specversion**: string
- **type**: string

### 2.3 OPTIONAL Attributes

CloudEvents Spec defines OPTIONAL attributes. In FlatBuffers, optional fields are nullable by default and do not require special union syntax.

The following OPTIONAL attributes are defined:

- **datacontenttype**: string
- **dataschema**: string
- **subject**: string
- **time**: string

### 2.4 Extension Attributes

CloudEvents allows arbitrary extension attributes. These are represented using a key-value table structure in FlatBuffers.

The `type` field in `ExtensionAttributes` MUST be one of: `Boolean`, `Integer`, `String`, `Binary`, `URI`, `URI-reference`, Timestamp.

| Extension Type  | Serialization                          |
|-----------------|----------------------------------------|
| Boolean         | 1 byte (0x00 = false, 0x01 = true)     |
| Integer         | 4 bytes, little-endian                 |
| String          | UTF-8 encoded bytes                    |
| Binary          | Raw bytes                              |
| URI             | UTF-8 encoded bytes (RFC 3986 §4.3)    |
| URI-reference   | UTF-8 encoded bytes (RFC 3986 §4.1)    |
| Timestamp       | UTF-8 encoded bytes (RFC 3339)         |

### 2.5 Schema Definition

Users of FlatBuffers MUST use a schema compatible with the CloudEvent FlatBuffers Schema.

The CloudEvent envelope schema (io.cloudevents.CloudEvent) MUST be known in advance by both producer and consumer. The dataschema attribute applies only to the data payload, not to the envelope itself.

```fbs
namespace io.cloudevents;


// Enumerates the type of the extension attribute value
enum ExtensionType:byte {
  BOOLEAN = 0,
  INTEGER = 1,
  STRING = 2,
  BINARY = 3,
  URI = 4,
  URI_REFERENCE = 5,
  TIMESTAMP = 6
}

// Key-value pair for extension attributes
table ExtensionAttributes {
  key: string (required);
  type: ExtensionType;
  value: [ubyte] (required);
}

// Main CloudEvent record
table CloudEvent {
  // REQUIRED attributes
  id: string (required);
  source: string (required);
  specversion: string (required);
  type: string (required);
  
  // OPTIONAL attributes
  datacontenttype: string;
  dataschema: string;
  subject: string;
  time: string;
  
  // Extension attributes
  extensions: [ExtensionAttributes];
  
  // Event data payload
  data: [ubyte];
}

root_type CloudEvent;
```

## 3. Data

In the FlatBuffers event format, data is always stored as a vector of unsigned bytes (`[ubyte]`). This differs from formats like `JSON` or `protobuf Any` that support inline structured data. Consumers SHOULD consult the datacontenttype attribute to determine how to interpret the byte payload.

The `data` field can store:

- Binary data directly
- UTF-8 encoded strings
- JSON serialized as UTF-8 bytes
- Any other serialized format

The implementation MUST store the data value in the `data` field as a byte vector.

## 4. Transport

Transports that support content identification MUST use the following designation:

```
application/cloudevents+flatbuffers
```

## 5. Schema Evolution

FlatBuffers provides robust schema evolution capabilities that are essential for event-driven architectures where producers and consumers evolve independently.

### 5.1 Compatibility Guarantees

FlatBuffers achieves forward and backward compatibility through an offset-based vtable (virtual table) system:

- **Forward compatibility**: Readers using older schemas can read data written with newer schemas by ignoring unknown fields
- **Backward compatibility**: Readers using newer schemas can read data written with older schemas by treating missing fields as unset

This is accomplished through:

1. **Vtable indirection**: Fields are accessed via offset lookups in a vtable rather than fixed positions
2. **Field IDs**: Fields are identified by their declaration order, not their position in the buffer
3. **Optional field detection**: Readers detect missing fields when vtable entries are absent (offset = 0)

### 5.2 CloudEvent Envelope Schema Evolution

The CloudEvent envelope has required and optional fields as defined in section 2.5.

**Required fields** (id, source, specversion, type):
- Must always be present in all CloudEvents
- Cannot be removed or made optional

**Optional fields** (datacontenttype, dataschema, subject, time):
- May be present or absent
- Readers treat missing optional fields as unset/null
- Can be added or removed without breaking compatibility

**Extension attributes:**
- Dynamic key-value pairs in the extensions array
- Keys can be added or removed freely without schema changes
- Readers process only recognized keys and ignore unknown keys

### 5.3 Implementation Considerations

**Buffer structure:**
- FlatBuffers serializes tables with vtable headers containing field offset mappings
- Optional fields that are not set occupy no space in the buffer
- Field access requires vtable lookup followed by offset-based memory access

**Performance characteristics:**
- Zero-copy deserialization enables direct buffer access
- Optional field access has constant-time overhead (vtable lookup)
- Missing field detection is O(1) via vtable entry check

**Migration best practices:**
- Deploy consumers before producers when adding optional fields
- Maintain compatibility windows when deprecating fields
- Use feature flags or configuration to control field usage during transitions
- Test compatibility with both old and new schema versions

### 5.4 Cross-Architecture Compatibility

FlatBuffers guarantees binary compatibility across heterogeneous architectures through its wire format specification and the `dataschema` field mechanism.

**Wire format guarantee:**
- All FlatBuffers data MUST be serialized in little-endian byte order
- FlatBuffers implementations handle endianness conversion transparently
- Consumers using the schema from `dataschema` can deserialize data regardless of architecture

**Schema evolution across architectures:**
- Adding or removing optional fields is safe across all architectures
- Field presence is determined by schema-defined vtable entries, not byte patterns
- The `dataschema` URI ensures both producer and consumer use compatible schemas
- Endianness conversion is handled by FlatBuffers runtime, not application code

**Key principle:**
When both producer and consumer obtain their schema from the same `dataschema` URI, FlatBuffers guarantees correct deserialization regardless of underlying processor architecture (x86-64, ARM, SPARC, PowerPC) or endianness (little-endian or big-endian). Schema evolution operations (adding/removing optional fields) maintain this guarantee.

## 6. Examples

### 6.1 Using dataschema for Dynamic Deserialization

The `dataschema` field enables dynamic schema resolution and runtime compilation for FlatBuffers data payloads:

**Producer workflow:**
1. Compile domain schema (e.g., `UserCreated.fbs`) using `flatc`
2. Serialize data structure into FlatBuffers binary format
3. Set `dataschema` to URI hosting the `.fbs` schema file
   - Example: `https://schemas.example.com/events/v1/UserCreated.fbs`
   - Alternative: `https://registry.example.com/schemas/v2/UserCreated.fbs`
   - Alternative: `fbs://internal-registry/com.example.events.UserCreated/v1`
4. Populate `data` field with serialized bytes
5. Set `datacontenttype` to `application/cloudevents+flatbuffers`

**Alternative producer workflow (JSON data):**
1. Serialize data structure into JSON format
2. Set `dataschema` to URI hosting the JSON Schema file
   - Example: `https://schemas.example.com/events/v1/UserCreated.json`
3. Populate `data` field with UTF-8 encoded JSON bytes
4. Set `datacontenttype` to `application/json`

**Consumer workflow (first encounter):**
1. Extract `dataschema` URI from CloudEvent
2. Fetch `.fbs` schema file via HTTP GET
3. Compile schema dynamically using `flatc` or FlatBuffers reflection API
4. Cache compiled schema artifacts keyed by URI
5. Deserialize `data` bytes using compiled schema
6. Process deserialized structure

**Consumer workflow (cached schema):**
1. Extract `dataschema` URI
2. Lookup cached compiled schema
3. Deserialize `data` bytes directly
4. Process deserialized structure

This approach enables schema evolution without requiring consumers to update code or redeploy when new event types are introduced. The reflection API allows runtime buffer navigation without code generation, while dynamic `flatc` invocation provides strongly-typed accessors.

### 6.2 Exemplary Mappings

The following table shows exemplary mappings:

| CloudEvents Attribute | Type   | Exemplary FlatBuffers Value                      |
|-----------------------|--------|--------------------------------------------------|
| id                    | string | "7a0dc520-c870-4193c8"                           |
| source                | string | "https://github.com/cloudevents"                 |
| specversion           | string | "1.0"                                            |
| type                  | string | "com.example.object.user.created.v2"                  |
| datacontenttype       | string | "application/cloudevents+flatbuffers"                        |
| dataschema            | string | "https://schemas.example.com/events/v2/UserCreated.fbs" |
| subject               | string | "user-created-123456789.fbs"                                  |
| time                  | string | "2019-06-05T23:45:00Z"                           |
| data                  | [ubyte]| [FlatBuffers serialized bytes]                   |

## References

- [Flatbuffers](https://github.com/google/flatbuffers/)
- [Google PubSub Spec](https://github.com/googleapis/google-cloudevents/blob/main/docs/spec/pubsub.md)
- [CloudEvents Specification v1.0](https://github.com/cloudevents/spec/blob/v1.0/spec.md)
- [Avro Compact CloudEvents](https://github.com/cloudevents/spec/blob/main/cloudevents/working-drafts/cloudevents-compact.avsc)
- [FlatBuffers Documentation](https://google.github.io/flatbuffers/)
- [RFC 3339 - Date and Time on the Internet: Timestamps](https://tools.ietf.org/html/rfc3339)
- [RFC 3986 - Uniform Resource Identifier (URI): Generic Syntax](https://tools.ietf.org/html/rfc3986)

## License

This specification follows the same license as the CloudEvents project (Apache 2.0).

## Contributing

This is a work-in-progress specification. Feedback and contributions are welcome through the CloudEvents working group.
