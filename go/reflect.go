package flatbuffers

import (
	"errors"
	"math"
	"unicode/utf8"
)

// ReflectionBaseType mirrors the BaseType enum defined in reflection.fbs.
// It describes the primitive wire type of a FlatBuffers field or the element
// type of a vector/array field.  The numeric values are stable and match the
// flatc-generated enum exactly, so they can be compared directly against
// values read from a .bfbs binary schema.
type ReflectionBaseType byte

const (
	// ReflectionBaseTypeNone is the zero value; indicates no type / unset.
	ReflectionBaseTypeNone ReflectionBaseType = 0
	// ReflectionBaseTypeUType is the implicit union-type discriminant field
	// that flatc inserts before every union field (always a uint8 on the wire).
	ReflectionBaseTypeUType ReflectionBaseType = 1
	// ReflectionBaseTypeBool is a 1-byte boolean (0 = false, non-zero = true).
	ReflectionBaseTypeBool ReflectionBaseType = 2
	// ReflectionBaseTypeByte is a signed 8-bit integer (int8).
	ReflectionBaseTypeByte ReflectionBaseType = 3
	// ReflectionBaseTypeUByte is an unsigned 8-bit integer (uint8).
	ReflectionBaseTypeUByte ReflectionBaseType = 4
	// ReflectionBaseTypeShort is a signed 16-bit integer (int16), little-endian.
	ReflectionBaseTypeShort ReflectionBaseType = 5
	// ReflectionBaseTypeUShort is an unsigned 16-bit integer (uint16), little-endian.
	ReflectionBaseTypeUShort ReflectionBaseType = 6
	// ReflectionBaseTypeInt is a signed 32-bit integer (int32), little-endian.
	ReflectionBaseTypeInt ReflectionBaseType = 7
	// ReflectionBaseTypeUInt is an unsigned 32-bit integer (uint32), little-endian.
	ReflectionBaseTypeUInt ReflectionBaseType = 8
	// ReflectionBaseTypeLong is a signed 64-bit integer (int64), little-endian.
	ReflectionBaseTypeLong ReflectionBaseType = 9
	// ReflectionBaseTypeULong is an unsigned 64-bit integer (uint64), little-endian.
	ReflectionBaseTypeULong ReflectionBaseType = 10
	// ReflectionBaseTypeFloat is a 32-bit IEEE 754 floating-point value.
	ReflectionBaseTypeFloat ReflectionBaseType = 11
	// ReflectionBaseTypeDouble is a 64-bit IEEE 754 floating-point value.
	ReflectionBaseTypeDouble ReflectionBaseType = 12
	// ReflectionBaseTypeString is a UTF-8 string stored as a FlatBuffers string
	// offset (4-byte relative offset pointing to a length-prefixed byte sequence).
	ReflectionBaseTypeString ReflectionBaseType = 13
	// ReflectionBaseTypeVector is a variable-length vector whose element type is
	// given by [ReflectionType.Element].
	ReflectionBaseTypeVector ReflectionBaseType = 14
	// ReflectionBaseTypeObj is a nested table or struct.  [ReflectionType.Index]
	// gives the index into the schema's objects vector that names the nested type.
	ReflectionBaseTypeObj ReflectionBaseType = 15
	// ReflectionBaseTypeUnion is a union field.  [ReflectionType.Index] gives the
	// index into the schema's enums vector for the union type descriptor.
	ReflectionBaseTypeUnion ReflectionBaseType = 16
	// ReflectionBaseTypeArray is a fixed-length inline array (only valid inside a
	// struct).  [ReflectionType.Element] names the element type.
	ReflectionBaseTypeArray ReflectionBaseType = 17
	// ReflectionBaseTypeVector64 is a 64-bit-offset vector (large vector support).
	// Element type is given by [ReflectionType.Element].
	ReflectionBaseTypeVector64 ReflectionBaseType = 18
)

// Hardcoded vtable offsets for the reflection schema tables.
// These follow the field declaration order in reflection.fbs and are used
// when reading the schema's own FlatBuffer encoding.  There are four groups,
// one per table in reflection.fbs:
//
//   - Schema    — the top-level schema table (root of a .bfbs file)
//   - Object    — a table or struct definition (reflection.fbs: table Object)
//   - Field     — a single field within an Object (reflection.fbs: table Field)
//   - Type      — the type descriptor of a Field (reflection.fbs: table Type)
//
// Each constant is a vtable slot offset: 4 for the first field, 6 for the
// second, and so on (VOffsetT values start at 4 because slots 0 and 2 are
// reserved for vtable size and object size).
const (
	// Schema table vtable offsets.
	schemaVOffObjects   VOffsetT = 4  // objects: [Object]
	schemaVOffEnums     VOffsetT = 6  // enums: [Enum]
	schemaVOffFileIdent VOffsetT = 8  // file_ident: string
	schemaVOffFileExt   VOffsetT = 10 // file_ext: string
	schemaVOffRootTable VOffsetT = 12 // root_table: Object

	// Object table vtable offsets.
	objectVOffName     VOffsetT = 4  // name: string
	objectVOffFields   VOffsetT = 6  // fields: [Field] (sorted by field_id)
	objectVOffIsStruct VOffsetT = 8  // is_struct: bool
	objectVOffMinAlign VOffsetT = 10 // minalign: int
	objectVOffByteSize VOffsetT = 12 // bytesize: int (structs only)

	// Field table vtable offsets.
	fieldVOffName           VOffsetT = 4  // name: string
	fieldVOffType           VOffsetT = 6  // type: Type
	fieldVOffID             VOffsetT = 8  // id: ushort (field_id / vtable slot number)
	fieldVOffOffset         VOffsetT = 10 // offset: ushort (vtable byte offset in data buffers)
	fieldVOffDefaultInteger VOffsetT = 12 // default_integer: long
	fieldVOffDefaultReal    VOffsetT = 14 // default_real: double
	fieldVOffDeprecated     VOffsetT = 16 // deprecated: bool
	fieldVOffRequired       VOffsetT = 18 // required: bool

	// Type table vtable offsets.
	typeVOffBaseType VOffsetT = 4 // base_type: BaseType
	typeVOffElement  VOffsetT = 6 // element: BaseType (vector/array element type)
	typeVOffIndex    VOffsetT = 8 // index: int (objects or enums vector index; -1 = none)
)

// ReflectionSchema wraps a parsed binary FlatBuffers schema (.bfbs file).
// A .bfbs file is itself a FlatBuffer whose root type is the Schema table
// defined in reflection.fbs.  It describes every table, struct, enum, and
// union that was compiled into the schema, along with field names, types,
// default values, and vtable offsets.
//
// Use [LoadReflectionSchema] to construct a ReflectionSchema from a raw .bfbs
// byte slice.  The byte slice is retained by reference and must not be
// modified while any ReflectionSchema, [ReflectionObject], [ReflectionField],
// or [ReflectionType] derived from it is in use.
//
// A typical workflow for dynamic field access in LiftCloud looks like:
//
//	// Load the compiled binary schema once at startup.
//	bfbs, err := os.ReadFile("myschema.bfbs")
//	if err != nil { ... }
//	schema, err := flatbuffers.LoadReflectionSchema(bfbs)
//	if err != nil { ... }
//
//	// Locate the table definition by its fully-qualified FlatBuffers name.
//	obj := schema.ObjectByName("MyNamespace.MyTable")
//	if obj == nil { ... }
//
//	// Look up a specific field.
//	field := obj.FieldByName("device_name")
//	if field == nil { ... }
//
//	// Read the field from a live data buffer (NOT the schema buffer).
//	// dataBuf is a raw FlatBuffer received over the wire or read from disk.
//	rootPos := int(flatbuffers.GetUOffsetT(dataBuf))
//	name, err := flatbuffers.GetFieldString(dataBuf, rootPos, field)
//	if err != nil { ... }
//	fmt.Println("device_name:", name)
type ReflectionSchema struct {
	buf []byte
	tab Table
}

// ReflectionObject represents a single table or struct definition within a
// parsed schema.  In FlatBuffers terminology both tables and structs are
// called "objects"; use [ReflectionObject.IsStruct] to distinguish them.
//
// Tables have variable-length encodings with vtable indirection; structs are
// fixed-size and stored inline.  [ReflectionObject.Fields] returns all fields
// declared in the schema, in field-ID order as sorted by flatc.
//
// ReflectionObject values are obtained via [ReflectionSchema.Objects],
// [ReflectionSchema.ObjectByName], or [ReflectionSchema.RootTable].  They
// remain valid as long as the parent [ReflectionSchema]'s buffer is retained.
type ReflectionObject struct {
	schema *ReflectionSchema
	tab    Table
}

// ReflectionField represents a single field declaration within a table or
// struct definition.  It carries the field's name, its vtable offset (used
// when reading live data buffers), type information, default values, and
// flags such as required and deprecated.
//
// The most important accessor for dynamic data reading is [ReflectionField.Offset],
// which returns the vtable byte offset that the FlatBuffers runtime uses to
// locate this field's value inside a data buffer.  Pass a ReflectionField
// directly to [GetFieldString], [GetFieldInt], [GetFieldFloat], or
// [GetFieldBool] to read the field from a live data buffer.
//
// ReflectionField values are obtained via [ReflectionObject.Fields] or
// [ReflectionObject.FieldByName].
type ReflectionField struct {
	schema *ReflectionSchema
	tab    Table
}

// ReflectionType holds the type descriptor for a single field.  A type
// descriptor has three components:
//
//   - [ReflectionType.BaseType]: the primary wire type (e.g. Int, String,
//     Vector, Obj).  For scalar and string fields this fully describes the
//     type.
//
//   - [ReflectionType.Element]: for Vector and Array fields, the element type
//     of the collection.  Unused (zero) for non-collection fields.
//
//   - [ReflectionType.Index]: for Obj, Union, and enum-derived fields, the
//     zero-based index into either the schema's objects vector (Obj/Union) or
//     the schema's enums vector (enum types) that names the referenced type.
//     Returns -1 when the index is not applicable.
//
// A ReflectionType is obtained via [ReflectionField.Type].
type ReflectionType struct {
	schema *ReflectionSchema
	tab    Table
}

// LoadReflectionSchema parses a binary FlatBuffers schema (.bfbs) buffer and
// returns a [ReflectionSchema] that can be used to introspect table and field
// definitions at runtime.
//
// The buf argument must be the raw contents of a .bfbs file produced by flatc
// (e.g. "flatc --binary myschema.fbs").  The buffer begins with a 4-byte root
// offset followed by the encoded Schema table from reflection.fbs.
//
// The returned ReflectionSchema retains a reference to buf; the caller must
// not modify or free buf while the schema or any derived
// [ReflectionObject] / [ReflectionField] / [ReflectionType] is in use.
//
// Returns an error if buf is too short to contain a valid root offset or if
// the root offset points outside buf.
//
// Example — load a schema at startup, then perform dynamic field reads:
//
//	bfbs, err := os.ReadFile("ln2.bfbs")
//	if err != nil {
//	    log.Fatal(err)
//	}
//	schema, err := flatbuffers.LoadReflectionSchema(bfbs)
//	if err != nil {
//	    log.Fatal(err)
//	}
//
//	// Resolve the table definition once and cache it.
//	obj := schema.ObjectByName("LN2.StatusMessage")
//	if obj == nil {
//	    log.Fatal("table not found in schema")
//	}
//	field := obj.FieldByName("device_id")
//	if field == nil {
//	    log.Fatal("field not found")
//	}
//
//	// For each incoming data buffer, locate the root table and read the field.
//	rootPos := int(flatbuffers.GetUOffsetT(dataBuf))
//	deviceID, err := flatbuffers.GetFieldString(dataBuf, rootPos, field)
//	if err != nil {
//	    log.Printf("read error: %v", err)
//	}
func LoadReflectionSchema(bfbs []byte) (*ReflectionSchema, error) {
	if len(bfbs) < SizeUOffsetT {
		return nil, errors.New("flatbuffers: bfbs buffer too short")
	}
	// The bfbs itself is a FlatBuffer — read the root offset.
	rootOffset := GetUOffsetT(bfbs)
	if int(rootOffset)+SizeUOffsetT > len(bfbs) {
		return nil, errors.New("flatbuffers: bfbs root offset out of range")
	}
	s := &ReflectionSchema{buf: bfbs}
	s.tab.Bytes = bfbs
	s.tab.Pos = rootOffset
	return s, nil
}

// RootTable returns the root_table object declared in the schema — the table
// type that is the default root type for FlatBuffers encoded with this schema.
// Returns nil if the field is absent (schemas that declare no root_type).
func (s *ReflectionSchema) RootTable() *ReflectionObject {
	o := s.tab.Offset(schemaVOffRootTable)
	if o == 0 {
		return nil
	}
	return s.schemaObject(s.tab.Pos + UOffsetT(o))
}

// Objects returns all table and struct definitions declared in the schema, in
// the order they appear in the schema's objects vector.  Returns nil if the
// schema has no objects.
func (s *ReflectionSchema) Objects() []*ReflectionObject {
	o := s.tab.Offset(schemaVOffObjects)
	if o == 0 {
		return nil
	}
	count := s.tab.VectorLen(UOffsetT(o))
	result := make([]*ReflectionObject, count)
	vec := s.tab.Vector(UOffsetT(o))
	for i := range count {
		elem := vec + UOffsetT(i)*UOffsetT(SizeUOffsetT)
		result[i] = s.schemaObject(elem)
	}
	return result
}

// ObjectByName finds a table or struct definition by its fully qualified
// FlatBuffers name (e.g. "MyNamespace.MyTable").  The name is case-sensitive
// and must match exactly the qualified name flatc writes into the .bfbs file.
// Returns nil if no matching object is found.
func (s *ReflectionSchema) ObjectByName(name string) *ReflectionObject {
	for _, obj := range s.Objects() {
		if obj.Name() == name {
			return obj
		}
	}
	return nil
}

// schemaObject constructs a ReflectionObject whose table resides at the
// indirect offset located at pos in the schema buffer.
func (s *ReflectionSchema) schemaObject(pos UOffsetT) *ReflectionObject {
	obj := &ReflectionObject{schema: s}
	obj.tab.Bytes = s.buf
	obj.tab.Pos = s.tab.Indirect(pos)
	return obj
}

// Name returns the fully qualified FlatBuffers name of this object
// (e.g. "MyNamespace.MyTable").  Returns an empty string if the name field
// is absent in the schema buffer.
func (o *ReflectionObject) Name() string {
	slot := o.tab.Offset(objectVOffName)
	if slot == 0 {
		return ""
	}
	return o.tab.String(o.tab.Pos + UOffsetT(slot))
}

// Fields returns all field definitions for this object, in the sorted order
// that flatc writes them (ascending by field ID).  Returns nil if the object
// has no fields (possible for an empty struct).
func (o *ReflectionObject) Fields() []*ReflectionField {
	slot := o.tab.Offset(objectVOffFields)
	if slot == 0 {
		return nil
	}
	count := o.tab.VectorLen(UOffsetT(slot))
	result := make([]*ReflectionField, count)
	vec := o.tab.Vector(UOffsetT(slot))
	for i := range count {
		elem := vec + UOffsetT(i)*UOffsetT(SizeUOffsetT)
		result[i] = o.schema.schemaField(elem)
	}
	return result
}

// FieldByName finds a field by its exact name as declared in the .fbs source.
// The lookup is linear over [Fields]; for hot paths, cache the returned
// [ReflectionField] rather than calling FieldByName on every message.
// Returns nil if no field with that name exists.
func (o *ReflectionObject) FieldByName(name string) *ReflectionField {
	for _, f := range o.Fields() {
		if f.Name() == name {
			return f
		}
	}
	return nil
}

// IsStruct returns true if this object is a fixed-size, inline struct rather
// than a variable-length table.  Struct fields are always present (no vtable
// absence check), all scalar types, and cannot have defaults.
func (o *ReflectionObject) IsStruct() bool {
	return o.tab.GetBoolSlot(objectVOffIsStruct, false)
}

// ByteSize returns the total byte size of a struct as laid out in memory.
// This is only meaningful for structs (IsStruct == true); for tables the
// value is always 0 because table size is not fixed.
func (o *ReflectionObject) ByteSize() int {
	return int(o.tab.GetInt32Slot(objectVOffByteSize, 0))
}

// schemaField constructs a ReflectionField from the indirect offset at pos.
func (s *ReflectionSchema) schemaField(pos UOffsetT) *ReflectionField {
	f := &ReflectionField{schema: s}
	f.tab.Bytes = s.buf
	f.tab.Pos = s.tab.Indirect(pos)
	return f
}

// Name returns the field name as declared in the .fbs source file.
// Returns an empty string if the name field is absent in the schema buffer.
func (f *ReflectionField) Name() string {
	slot := f.tab.Offset(fieldVOffName)
	if slot == 0 {
		return ""
	}
	return f.tab.String(f.tab.Pos + UOffsetT(slot))
}

// Offset returns the vtable byte offset for this field.  This is the value
// passed as the VOffsetT argument when looking up a field in a live data
// buffer's vtable.  It is used internally by [GetFieldString], [GetFieldInt],
// [GetFieldFloat], and [GetFieldBool] to locate the field in a data buffer.
// A value of 0 indicates the field has no vtable entry (should not occur for
// fields returned by a valid schema).
func (f *ReflectionField) Offset() uint16 {
	return uint16(f.tab.GetUint16Slot(fieldVOffOffset, 0))
}

// Required returns true if this field is marked "required" in the schema,
// meaning its absence in a data buffer is a protocol error.
func (f *ReflectionField) Required() bool {
	return f.tab.GetBoolSlot(fieldVOffRequired, false)
}

// Type returns the [ReflectionType] descriptor for this field, which carries
// the base type, element type (for collections), and object/enum index.
// Returns nil if the type sub-table is absent in the schema buffer (should
// not occur for a well-formed schema).
func (f *ReflectionField) Type() *ReflectionType {
	slot := f.tab.Offset(fieldVOffType)
	if slot == 0 {
		return nil
	}
	t := &ReflectionType{schema: f.schema}
	t.tab.Bytes = f.schema.buf
	t.tab.Pos = f.tab.Indirect(f.tab.Pos + UOffsetT(slot))
	return t
}

// DefaultInteger returns the default integer value declared for this field in
// the schema (the value used when the field is absent from a data buffer).
// For boolean fields the default is encoded here as 0 (false) or 1 (true).
// Returns 0 if no explicit default was declared.
func (f *ReflectionField) DefaultInteger() int64 {
	return f.tab.GetInt64Slot(fieldVOffDefaultInteger, 0)
}

// DefaultReal returns the default floating-point value declared for this field
// in the schema (the value used when the field is absent from a data buffer).
// Returns 0.0 if no explicit default was declared.
func (f *ReflectionField) DefaultReal() float64 {
	return f.tab.GetFloat64Slot(fieldVOffDefaultReal, 0.0)
}

// BaseType returns the primary wire type of this type descriptor.  For scalar
// and string fields this is the complete type description.  For collection
// fields ([ReflectionBaseTypeVector], [ReflectionBaseTypeArray]) check
// [ReflectionType.Element] for the element type.  For nested object or union
// fields ([ReflectionBaseTypeObj], [ReflectionBaseTypeUnion]) check
// [ReflectionType.Index] for the schema index of the referenced type.
func (t *ReflectionType) BaseType() ReflectionBaseType {
	return ReflectionBaseType(t.tab.GetUint8Slot(typeVOffBaseType, 0))
}

// Element returns the element base type for Vector and Array fields.  For
// example, a field declared as "[float]" in a .fbs file has BaseType ==
// ReflectionBaseTypeVector and Element == ReflectionBaseTypeFloat.
// For non-collection fields this returns ReflectionBaseTypeNone (0).
func (t *ReflectionType) Element() ReflectionBaseType {
	return ReflectionBaseType(t.tab.GetUint8Slot(typeVOffElement, 0))
}

// Index returns the schema index associated with this type:
//   - For [ReflectionBaseTypeObj]: index into [ReflectionSchema.Objects] that
//     names the nested table or struct type.
//   - For [ReflectionBaseTypeUnion]: index into the schema's enums vector that
//     names the union type descriptor.
//   - For enum-typed fields: index into the schema's enums vector.
//   - For all other types: -1 (not applicable).
func (t *ReflectionType) Index() int32 {
	return t.tab.GetInt32Slot(typeVOffIndex, -1)
}

// resolveVtableEntry looks up the vtable entry for fieldOffset in the data
// buffer at tablePos and returns the absolute position of the field data.
// Returns 0 when the field is absent.
func resolveVtableEntry(buf []byte, tablePos int, fieldOffset uint16) UOffsetT {
	tab := Table{Bytes: buf, Pos: UOffsetT(tablePos)}
	entry := tab.Offset(VOffsetT(fieldOffset))
	if entry == 0 {
		return 0
	}
	return tab.Pos + UOffsetT(entry)
}

// GetFieldString reads a UTF-8 string field from a data buffer using
// reflection metadata obtained from a [ReflectionSchema].
//
// The buf parameter is the raw FlatBuffer data buffer — for example, bytes
// received over the network or read from disk.  It is NOT the .bfbs schema
// buffer.  The tablePos parameter is the absolute byte position of the root
// table within buf, typically obtained by calling
// flatbuffers.GetUOffsetT(buf) on a freshly received message.
//
// The field parameter must be a [ReflectionField] whose [ReflectionField.Offset]
// returns the vtable byte offset for the string field within the target table.
//
// Return values:
//   - If the field is absent from the data buffer (vtable entry missing), the
//     FlatBuffers default for string fields ("") is returned with a nil error.
//   - If the field is present but the bytes are not valid UTF-8, an error is
//     returned.
//   - Returns an error if field is nil or if buf is structurally invalid.
func GetFieldString(buf []byte, tablePos int, field *ReflectionField) (string, error) {
	if field == nil {
		return "", errors.New("flatbuffers: nil field")
	}
	abs := resolveVtableEntry(buf, tablePos, field.Offset())
	if abs == 0 {
		return "", nil
	}
	tab := Table{Bytes: buf, Pos: UOffsetT(tablePos)}
	s := tab.String(abs)
	if !utf8.ValidString(s) {
		return "", errors.New("flatbuffers: string field contains invalid UTF-8")
	}
	return s, nil
}

// GetFieldInt reads an integer field (of any integer [ReflectionBaseType]) as
// int64 from a data buffer using reflection metadata obtained from a
// [ReflectionSchema].
//
// The buf parameter is the raw FlatBuffer data buffer — NOT the .bfbs schema
// buffer.  The tablePos parameter is the absolute byte position of the root
// table within buf, obtained via flatbuffers.GetUOffsetT(buf).
//
// The following base types are supported: UType, Byte, UByte, Bool, Short,
// UShort, Int, UInt, Long, ULong.  All values are widened to int64; unsigned
// types that exceed math.MaxInt64 will wrap to negative values.
//
// Return values:
//   - If the field is absent from the data buffer (vtable entry missing), the
//     field's [ReflectionField.DefaultInteger] value is returned with a nil error.
//   - If the field's base type is not an integer type, an error is returned.
//   - Returns an error if field is nil, if the field has no type descriptor,
//     or if the resolved offset falls outside buf.
func GetFieldInt(buf []byte, tablePos int, field *ReflectionField) (int64, error) {
	if field == nil {
		return 0, errors.New("flatbuffers: nil field")
	}
	ft := field.Type()
	if ft == nil {
		return 0, errors.New("flatbuffers: field has no type")
	}
	abs := resolveVtableEntry(buf, tablePos, field.Offset())
	if abs == 0 {
		return field.DefaultInteger(), nil
	}
	if int(abs) >= len(buf) {
		return 0, errors.New("flatbuffers: field offset out of range")
	}
	switch ft.BaseType() {
	case ReflectionBaseTypeUType, ReflectionBaseTypeByte:
		return int64(GetInt8(buf[abs:])), nil
	case ReflectionBaseTypeBool, ReflectionBaseTypeUByte:
		return int64(GetUint8(buf[abs:])), nil
	case ReflectionBaseTypeShort:
		return int64(GetInt16(buf[abs:])), nil
	case ReflectionBaseTypeUShort:
		return int64(GetUint16(buf[abs:])), nil
	case ReflectionBaseTypeInt:
		return int64(GetInt32(buf[abs:])), nil
	case ReflectionBaseTypeUInt:
		return int64(GetUint32(buf[abs:])), nil
	case ReflectionBaseTypeLong:
		return GetInt64(buf[abs:]), nil
	case ReflectionBaseTypeULong:
		return int64(GetUint64(buf[abs:])), nil
	default:
		return 0, errors.New("flatbuffers: field is not an integer type")
	}
}

// GetFieldFloat reads a floating-point field ([ReflectionBaseTypeFloat] or
// [ReflectionBaseTypeDouble]) as float64 from a data buffer using reflection
// metadata obtained from a [ReflectionSchema].
//
// The buf parameter is the raw FlatBuffer data buffer — NOT the .bfbs schema
// buffer.  The tablePos parameter is the absolute byte position of the root
// table within buf, obtained via flatbuffers.GetUOffsetT(buf).
//
// A Float (32-bit) field is widened to float64 without loss of value range;
// a Double (64-bit) field is returned directly.
//
// Return values:
//   - If the field is absent from the data buffer (vtable entry missing), the
//     field's [ReflectionField.DefaultReal] value is returned with a nil error.
//   - If the field's base type is neither Float nor Double, an error is returned.
//   - Returns an error if field is nil, if the field has no type descriptor,
//     or if the resolved offset falls outside buf.
func GetFieldFloat(buf []byte, tablePos int, field *ReflectionField) (float64, error) {
	if field == nil {
		return 0, errors.New("flatbuffers: nil field")
	}
	ft := field.Type()
	if ft == nil {
		return 0, errors.New("flatbuffers: field has no type")
	}
	abs := resolveVtableEntry(buf, tablePos, field.Offset())
	if abs == 0 {
		return field.DefaultReal(), nil
	}
	if int(abs) >= len(buf) {
		return 0, errors.New("flatbuffers: field offset out of range")
	}
	switch ft.BaseType() {
	case ReflectionBaseTypeFloat:
		bits := GetUint32(buf[abs:])
		return float64(math.Float32frombits(bits)), nil
	case ReflectionBaseTypeDouble:
		bits := GetUint64(buf[abs:])
		return math.Float64frombits(bits), nil
	default:
		return 0, errors.New("flatbuffers: field is not a float type")
	}
}

// GetFieldBool reads a boolean field ([ReflectionBaseTypeBool]) from a data
// buffer using reflection metadata obtained from a [ReflectionSchema].
//
// The buf parameter is the raw FlatBuffer data buffer — NOT the .bfbs schema
// buffer.  The tablePos parameter is the absolute byte position of the root
// table within buf, obtained via flatbuffers.GetUOffsetT(buf).
//
// Return values:
//   - If the field is absent from the data buffer (vtable entry missing), the
//     default is derived from [ReflectionField.DefaultInteger]: non-zero means
//     true, zero means false (the FlatBuffers convention for boolean defaults).
//   - If the field's base type is not Bool, an error is returned.
//   - Returns an error if field is nil, if the field has no type descriptor,
//     or if the resolved offset falls outside buf.
func GetFieldBool(buf []byte, tablePos int, field *ReflectionField) (bool, error) {
	if field == nil {
		return false, errors.New("flatbuffers: nil field")
	}
	ft := field.Type()
	if ft == nil {
		return false, errors.New("flatbuffers: field has no type")
	}
	if ft.BaseType() != ReflectionBaseTypeBool {
		return false, errors.New("flatbuffers: field is not a bool type")
	}
	abs := resolveVtableEntry(buf, tablePos, field.Offset())
	if abs == 0 {
		return field.DefaultInteger() != 0, nil
	}
	if int(abs) >= len(buf) {
		return false, errors.New("flatbuffers: field offset out of range")
	}
	return GetBool(buf[abs:]), nil
}
