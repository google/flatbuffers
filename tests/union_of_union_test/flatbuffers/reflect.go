package flatbuffers

import (
	"errors"
	"math"
	"unicode/utf8"
)

// ReflectionBaseType mirrors the BaseType enum from reflection.fbs.
type ReflectionBaseType byte

const (
	ReflectionBaseTypeNone     ReflectionBaseType = 0
	ReflectionBaseTypeUType    ReflectionBaseType = 1
	ReflectionBaseTypeBool     ReflectionBaseType = 2
	ReflectionBaseTypeByte     ReflectionBaseType = 3
	ReflectionBaseTypeUByte    ReflectionBaseType = 4
	ReflectionBaseTypeShort    ReflectionBaseType = 5
	ReflectionBaseTypeUShort   ReflectionBaseType = 6
	ReflectionBaseTypeInt      ReflectionBaseType = 7
	ReflectionBaseTypeUInt     ReflectionBaseType = 8
	ReflectionBaseTypeLong     ReflectionBaseType = 9
	ReflectionBaseTypeULong    ReflectionBaseType = 10
	ReflectionBaseTypeFloat    ReflectionBaseType = 11
	ReflectionBaseTypeDouble   ReflectionBaseType = 12
	ReflectionBaseTypeString   ReflectionBaseType = 13
	ReflectionBaseTypeVector   ReflectionBaseType = 14
	ReflectionBaseTypeObj      ReflectionBaseType = 15
	ReflectionBaseTypeUnion    ReflectionBaseType = 16
	ReflectionBaseTypeArray    ReflectionBaseType = 17
	ReflectionBaseTypeVector64 ReflectionBaseType = 18
)

// Hardcoded vtable offsets for the reflection schema tables.
// These follow the field declaration order in reflection.fbs.
const (
	// Schema table vtable offsets.
	schemaVOffObjects   VOffsetT = 4
	schemaVOffEnums     VOffsetT = 6
	schemaVOffFileIdent VOffsetT = 8
	schemaVOffFileExt   VOffsetT = 10
	schemaVOffRootTable VOffsetT = 12

	// Object table vtable offsets.
	objectVOffName     VOffsetT = 4
	objectVOffFields   VOffsetT = 6
	objectVOffIsStruct VOffsetT = 8
	objectVOffMinAlign VOffsetT = 10
	objectVOffByteSize VOffsetT = 12

	// Field table vtable offsets.
	fieldVOffName           VOffsetT = 4
	fieldVOffType           VOffsetT = 6
	fieldVOffID             VOffsetT = 8
	fieldVOffOffset         VOffsetT = 10
	fieldVOffDefaultInteger VOffsetT = 12
	fieldVOffDefaultReal    VOffsetT = 14
	fieldVOffDeprecated     VOffsetT = 16
	fieldVOffRequired       VOffsetT = 18

	// Type table vtable offsets.
	typeVOffBaseType VOffsetT = 4
	typeVOffElement  VOffsetT = 6
	typeVOffIndex    VOffsetT = 8
)

// ReflectionSchema represents a parsed binary FlatBuffers schema (.bfbs file).
type ReflectionSchema struct {
	buf []byte
	tab Table
}

// ReflectionObject represents an object (table or struct) in the schema.
type ReflectionObject struct {
	schema *ReflectionSchema
	tab    Table
}

// ReflectionField represents a field within an object.
type ReflectionField struct {
	schema *ReflectionSchema
	tab    Table
}

// ReflectionType represents a field's type information.
type ReflectionType struct {
	schema *ReflectionSchema
	tab    Table
}

// LoadReflectionSchema parses a binary schema (.bfbs) buffer.
// The buffer must begin with a FlatBuffer whose root type is Schema
// (as defined in reflection.fbs).
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

// RootTable returns the root_table object declared in the schema, or nil if
// the field is absent.
func (s *ReflectionSchema) RootTable() *ReflectionObject {
	o := s.tab.Offset(schemaVOffRootTable)
	if o == 0 {
		return nil
	}
	return s.schemaObject(s.tab.Pos + UOffsetT(o))
}

// Objects returns all objects declared in the schema.
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

// ObjectByName finds an object by its fully qualified name.
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

// Name returns the object's fully qualified name.
func (o *ReflectionObject) Name() string {
	slot := o.tab.Offset(objectVOffName)
	if slot == 0 {
		return ""
	}
	return o.tab.String(o.tab.Pos + UOffsetT(slot))
}

// Fields returns all fields of the object, in sorted order.
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

// FieldByName finds a field by name. Returns nil if not found.
func (o *ReflectionObject) FieldByName(name string) *ReflectionField {
	for _, f := range o.Fields() {
		if f.Name() == name {
			return f
		}
	}
	return nil
}

// IsStruct returns true if this object is a fixed-size struct rather than a
// table.
func (o *ReflectionObject) IsStruct() bool {
	return o.tab.GetBoolSlot(objectVOffIsStruct, false)
}

// ByteSize returns the byte size of a struct. Only meaningful for structs;
// for tables the value is 0.
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

// Name returns the field name.
func (f *ReflectionField) Name() string {
	slot := f.tab.Offset(fieldVOffName)
	if slot == 0 {
		return ""
	}
	return f.tab.String(f.tab.Pos + UOffsetT(slot))
}

// Offset returns the vtable offset for this field (used when reading user
// data buffers).
func (f *ReflectionField) Offset() uint16 {
	return uint16(f.tab.GetUint16Slot(fieldVOffOffset, 0))
}

// Required returns true if this field is marked required in the schema.
func (f *ReflectionField) Required() bool {
	return f.tab.GetBoolSlot(fieldVOffRequired, false)
}

// Type returns the type information for this field.
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

// DefaultInteger returns the default integer value for this field.
func (f *ReflectionField) DefaultInteger() int64 {
	return f.tab.GetInt64Slot(fieldVOffDefaultInteger, 0)
}

// DefaultReal returns the default floating-point value for this field.
func (f *ReflectionField) DefaultReal() float64 {
	return f.tab.GetFloat64Slot(fieldVOffDefaultReal, 0.0)
}

// BaseType returns the base type of this type descriptor.
func (t *ReflectionType) BaseType() ReflectionBaseType {
	return ReflectionBaseType(t.tab.GetUint8Slot(typeVOffBaseType, 0))
}

// Element returns the element base type (only meaningful when BaseType is
// Vector or Array).
func (t *ReflectionType) Element() ReflectionBaseType {
	return ReflectionBaseType(t.tab.GetUint8Slot(typeVOffElement, 0))
}

// Index returns the type index.  For Obj types it is the index into the
// schema's objects vector; for Union/enum-derived types it is the index into
// the enums vector. -1 means not applicable.
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

// GetFieldString reads a string field from a data buffer using reflection
// metadata. Returns the default empty string if the field is absent. Returns
// an error if fieldOffset is zero, the buffer is too small, or the string
// bytes are not valid UTF-8.
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

// GetFieldInt reads an integer field (of any integer base type) as int64 from
// a data buffer. Returns the field's default_integer when the field is absent.
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

// GetFieldFloat reads a floating-point field (Float or Double) as float64
// from a data buffer. Returns the field's default_real when the field is
// absent.
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

// GetFieldBool reads a boolean field from a data buffer using reflection
// metadata. Returns false (the FlatBuffers default) when the field is absent.
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
