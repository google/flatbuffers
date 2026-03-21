package flatbuffers

import (
	"encoding/json"
	"errors"
	"fmt"
	"math"
)

// ── Enum reflection types ──────────────────────────────────────────────────
// These supplement the existing ReflectionSchema / ReflectionObject API with
// the ability to read enum and union-type descriptors from a binary schema.
// They are needed by ReflectPack / ReflectUnpack for union dispatch.

// Hardcoded vtable offsets for the Enum and EnumVal tables in reflection.fbs.
const (
	// Enum table vtable offsets.
	enumVOffName    VOffsetT = 4 // name: string
	enumVOffValues  VOffsetT = 6 // values: [EnumVal]
	enumVOffIsUnion VOffsetT = 8 // is_union: bool

	// EnumVal table vtable offsets.
	enumValVOffName  VOffsetT = 4 // name: string
	enumValVOffValue VOffsetT = 6 // value: long (int64)

	// reflectBuilderInitialSize is the initial byte size for FlatBuffer builders
	// created by ReflectPack.
	reflectBuilderInitialSize = 1024

	// vtableBaseOffset is the starting byte offset for the first vtable field.
	// FlatBuffers vtable offsets start at 4 (slots 0 and 1 are reserved).
	vtableBaseOffset = 4

	// vtableBytesPerSlot is the number of bytes per vtable slot.
	vtableBytesPerSlot = 2
)

// Sentinel errors for static error messages used in ReflectUnpack and ReflectPack.
var (
	// errBufferTooShort is returned when the FlatBuffer is too short to contain a root offset.
	errBufferTooShort = errors.New("flatbuffers: buffer too short")
	// errVectorOffsetRange is returned when a vector field offset is out of bounds.
	errVectorOffsetRange = errors.New("flatbuffers: vector offset out of range")
	// errVectorStartRange is returned when the computed vector start position is out of bounds.
	errVectorStartRange = errors.New("flatbuffers: vector start out of range")
	// errNotIntegerType is returned when an integer read is attempted on a non-integer field.
	errNotIntegerType = errors.New("flatbuffers: field is not an integer type")
	// errNotFloatType is returned when a float read is attempted on a non-float field.
	errNotFloatType = errors.New("flatbuffers: field is not a float type")
	// errVectorObjBadIndex is returned when a vector of tables has an invalid object index.
	errVectorObjBadIndex = errors.New("flatbuffers: vector Obj has invalid schema index")
	// errVectorObjIndexRange is returned when the vector table index is out of range.
	errVectorObjIndexRange = errors.New("flatbuffers: vector Obj index out of range")
	// errUnsupportedElemType is returned for unsupported vector element types.
	errUnsupportedElemType = errors.New("flatbuffers: unsupported vector element type")
	// errObjIndexNegative is returned when the object schema index is negative.
	errObjIndexNegative = errors.New("flatbuffers: object index is negative")
	// errObjIndexRange is returned when the object schema index is out of range.
	errObjIndexRange = errors.New("flatbuffers: object index out of range")
	// errTypeNotFound is returned when the requested type name is absent from the schema.
	errTypeNotFound = errors.New("flatbuffers: type not found in schema")
	// errUnionVariantNotFound is returned when the named union variant is absent from the schema.
	errUnionVariantNotFound = errors.New("flatbuffers: union variant not found in schema")
	// errExpectedString is returned when a string value is required but not present.
	errExpectedString = errors.New("flatbuffers: expected string value")
	// errExpectedBool is returned when a bool value is required but not present.
	errExpectedBool = errors.New("flatbuffers: expected bool value")
	// errExpectedNumber is returned when a numeric value is required but not present.
	errExpectedNumber = errors.New("flatbuffers: expected numeric value")
	// errExpectedObject is returned when a JSON object is required but not present.
	errExpectedObject = errors.New("flatbuffers: expected object value")
	// errExpectedArray is returned when a JSON array is required but not present.
	errExpectedArray = errors.New("flatbuffers: expected array value")
)

// ReflectionEnum wraps the reflection.Enum table from a binary schema (.bfbs).
// It describes a FlatBuffers enum or union-type enumeration.
//
// Obtain instances via [ReflectionSchema.Enums].
type ReflectionEnum struct {
	schema *ReflectionSchema
	tab    Table
}

// ReflectionEnumVal wraps one declared value within a [ReflectionEnum].
//
// For union-type enums, each value describes one variant; the Value() is the
// integer discriminant placed in the companion "_type" field on the wire.
type ReflectionEnumVal struct {
	schema *ReflectionSchema
	tab    Table
}

// Enums returns all enum definitions (including union-type enumerations) from
// the schema, in the order flatc wrote them.
func (s *ReflectionSchema) Enums() []*ReflectionEnum {
	enumsOffset := s.tab.Offset(schemaVOffEnums)
	if enumsOffset == 0 {
		return nil
	}
	count := s.tab.VectorLen(UOffsetT(enumsOffset))
	result := make([]*ReflectionEnum, count)
	vec := s.tab.Vector(UOffsetT(enumsOffset))

	for index := range count {
		elem := vec + UOffsetT(index)*UOffsetT(SizeUOffsetT)
		entry := &ReflectionEnum{
			schema: s,
			tab:    Table{Bytes: s.buf, Pos: 0},
		}
		entry.tab.Pos = s.tab.Indirect(elem)
		result[index] = entry
	}

	return result
}

// Name returns the fully qualified name of this enum (e.g. "MyGame.Example.Equipment").
// Returns an empty string if the name field is absent.
func (e *ReflectionEnum) Name() string {
	slot := e.tab.Offset(enumVOffName)
	if slot == 0 {
		return ""
	}

	return e.tab.String(e.tab.Pos + UOffsetT(slot))
}

// IsUnion returns true if this enum describes the discriminant of a union type
// rather than a plain integer enum.
func (e *ReflectionEnum) IsUnion() bool {
	return e.tab.GetBoolSlot(enumVOffIsUnion, false)
}

// Values returns all declared values (or union variants) for this enum,
// in declaration order.
func (e *ReflectionEnum) Values() []*ReflectionEnumVal {
	slot := e.tab.Offset(enumVOffValues)
	if slot == 0 {
		return nil
	}

	count := e.tab.VectorLen(UOffsetT(slot))
	result := make([]*ReflectionEnumVal, count)
	vec := e.tab.Vector(UOffsetT(slot))

	for index := range count {
		elem := vec + UOffsetT(index)*UOffsetT(SizeUOffsetT)
		entry := &ReflectionEnumVal{
			schema: e.schema,
			tab:    Table{Bytes: e.schema.buf, Pos: 0},
		}
		entry.tab.Pos = e.tab.Indirect(elem)
		result[index] = entry
	}

	return result
}

// Name returns the declared name of this enum value or union variant.
// Returns an empty string if absent.
func (ev *ReflectionEnumVal) Name() string {
	slot := ev.tab.Offset(enumValVOffName)
	if slot == 0 {
		return ""
	}

	return ev.tab.String(ev.tab.Pos + UOffsetT(slot))
}

// Value returns the integer value assigned to this enum value or union discriminant.
// Returns 0 if no explicit value was declared.
func (ev *ReflectionEnumVal) Value() int64 {
	return ev.tab.GetInt64Slot(enumValVOffValue, 0)
}

// ReflectUnpack deserializes a FlatBuffer byte slice into a map[string]any using
// schema reflection from a .bfbs binary schema file.
//
// typeName must be the fully qualified FlatBuffers table name as it appears in
// the binary schema, e.g. "MyGame.Example.Monster" or "Ln2.StatusMessage".
// buf must be a complete, root-offset-prefixed FlatBuffer message — the first
// four bytes are the root UOffsetT that points to the root table.
//
// The returned map contains one key per field present in the buffer.  Fields
// whose vtable entry is absent (i.e. set to the FlatBuffers default) are
// omitted.  Nested tables are returned as nested map[string]any values.
// Vectors of scalars and strings are []any; vectors of tables are []map[string]any.
// Union fields are represented with two keys: "<field>_type" (string variant
// name) and "<field>" (nested map[string]any for the variant's table).
//
// Returns an error if typeName is not found in the schema or if buf is
// structurally invalid.
//
// Example:
//
//	schema, _ := flatbuffers.LoadReflectionSchema(bfbsBytes)
//	m, err := flatbuffers.ReflectUnpack(schema, "MyGame.Example.Monster", buf)
//	if err != nil { log.Fatal(err) }
//	fmt.Println(m["name"]) // "MyMonster"
func ReflectUnpack(schema *ReflectionSchema, typeName string, buf []byte) (map[string]any, error) {
	obj := schema.ObjectByName(typeName)
	if obj == nil {
		return nil, fmt.Errorf("%w: %q", errTypeNotFound, typeName)
	}

	if len(buf) < SizeUOffsetT {
		return nil, errBufferTooShort
	}

	rootPos := int(GetUOffsetT(buf[0:]))

	return reflectUnpackTable(buf, rootPos, obj, schema)
}

// reflectUnpackTable recursively unpacks a FlatBuffers table or struct at
// tablePos within buf, using obj as the schema descriptor.  Returns a
// map[string]any containing all present fields.
//
// For tables, field presence is determined via vtable lookup.  For structs,
// all fields are always present at fixed byte offsets relative to tablePos.
func reflectUnpackTable(
	buf []byte,
	tablePos int,
	obj *ReflectionObject,
	schema *ReflectionSchema,
) (map[string]any, error) {
	isStruct := obj.IsStruct()
	result := make(map[string]any)

	for _, field := range obj.Fields() {
		fieldType := field.Type()
		if fieldType == nil {
			continue
		}

		name := field.Name()

		// Resolve the absolute byte position of the field's data.
		// For tables this goes through the vtable; for structs it's a direct
		// byte offset from the start of the struct.
		var abs UOffsetT
		if isStruct {
			// field.Offset() is the byte offset within the struct.
			abs = UOffsetT(tablePos) + UOffsetT(field.Offset())
		} else {
			abs = resolveVtableEntry(buf, tablePos, field.Offset())
			if abs == 0 {
				// Field absent from vtable — use default and skip.
				continue
			}
		}

		err := unpackFieldIntoMap(buf, tablePos, name, abs, fieldType, obj, schema, result)
		if err != nil {
			return nil, fmt.Errorf("flatbuffers: field %q: %w", name, err)
		}
	}

	return result, nil
}

// unpackFieldIntoMap reads a single field from the buffer and stores it in result.
// It handles all FlatBuffers base types including scalars, strings, nested tables,
// vectors, and unions.
//
//nolint:cyclop // Dispatch on all FlatBuffers scalar/composite types is inherently broad.
func unpackFieldIntoMap(
	buf []byte,
	tablePos int,
	name string,
	abs UOffsetT,
	fieldType *ReflectionType,
	obj *ReflectionObject,
	schema *ReflectionSchema,
	result map[string]any,
) error {
	switch fieldType.BaseType() {
	case ReflectionBaseTypeBool:
		return unpackBoolField(buf, name, abs, result)

	case ReflectionBaseTypeByte, ReflectionBaseTypeUByte,
		ReflectionBaseTypeShort, ReflectionBaseTypeUShort,
		ReflectionBaseTypeInt, ReflectionBaseTypeUInt,
		ReflectionBaseTypeLong, ReflectionBaseTypeULong:
		return unpackIntField(buf, name, abs, fieldType.BaseType(), result)

	case ReflectionBaseTypeFloat, ReflectionBaseTypeDouble:
		return unpackFloatField(buf, name, abs, fieldType.BaseType(), result)

	case ReflectionBaseTypeString:
		tab := Table{Bytes: buf, Pos: UOffsetT(tablePos)}
		result[name] = tab.String(abs)

	case ReflectionBaseTypeObj:
		childMap, err := unpackObjField(buf, abs, fieldType, schema)
		if err != nil {
			return err
		}

		if childMap != nil {
			result[name] = childMap
		}

	case ReflectionBaseTypeVector:
		elems, err := reflectUnpackVector(buf, abs, fieldType, schema)
		if err != nil {
			return err
		}

		result[name] = elems

	case ReflectionBaseTypeUType:
		// UType fields are consumed by the Union case — skip here.

	case ReflectionBaseTypeUnion:
		return unpackUnionIntoMap(buf, tablePos, name, abs, fieldType, obj, schema, result)

	case ReflectionBaseTypeNone, ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return nil
	}

	return nil
}

// unpackBoolField reads a bool field from buf at abs and stores it in result.
func unpackBoolField(buf []byte, name string, abs UOffsetT, result map[string]any) error {
	if int(abs) >= len(buf) {
		return nil
	}

	result[name] = GetBool(buf[abs:])

	return nil
}

// unpackIntField reads a scalar integer field and stores it as int64 in result.
func unpackIntField(
	buf []byte,
	name string,
	abs UOffsetT,
	baseType ReflectionBaseType,
	result map[string]any,
) error {
	val, err := readScalarInt(buf, abs, baseType)
	if err != nil {
		return err
	}

	result[name] = val

	return nil
}

// unpackFloatField reads a float field and stores it as float64 in result.
func unpackFloatField(
	buf []byte,
	name string,
	abs UOffsetT,
	baseType ReflectionBaseType,
	result map[string]any,
) error {
	val, err := readScalarFloat(buf, abs, baseType)
	if err != nil {
		return err
	}

	result[name] = val

	return nil
}

// unpackUnionIntoMap handles unpacking a union field, finding its companion
// _type discriminant field and resolving the variant object.
func unpackUnionIntoMap(
	buf []byte,
	tablePos int,
	name string,
	abs UOffsetT,
	fieldType *ReflectionType,
	obj *ReflectionObject,
	schema *ReflectionSchema,
	result map[string]any,
) error {
	// Find the companion _type field (flatc inserts "<field_name>_type"
	// before every union field).
	typeField := obj.FieldByName(name + "_type")
	if typeField == nil {
		return nil
	}

	typeAbs := resolveVtableEntry(buf, tablePos, typeField.Offset())
	if typeAbs == 0 {
		return nil
	}

	discriminant := int(GetUint8(buf[typeAbs:]))
	if discriminant == 0 {
		// Variant 0 is always NONE.
		return nil
	}

	variantName, childObj := resolveUnionVariant(schema, fieldType.Index(), discriminant)
	if childObj == nil {
		return nil
	}

	if int(abs)+SizeUOffsetT > len(buf) {
		return nil
	}

	childPos := int(abs) + int(GetUOffsetT(buf[abs:]))

	child, err := reflectUnpackTable(buf, childPos, childObj, schema)
	if err != nil {
		return fmt.Errorf("union field %q: %w", name, err)
	}

	result[name+"_type"] = variantName
	result[name] = child

	return nil
}

// unpackObjField reads a nested table or struct at the absolute field position
// abs and returns the decoded map.  Returns (nil, nil) if the field can be
// safely skipped.
func unpackObjField(buf []byte, abs UOffsetT, ft *ReflectionType, schema *ReflectionSchema) (map[string]any, error) {
	idx := ft.Index()
	if idx < 0 {
		return nil, errObjIndexNegative
	}

	objs := schema.Objects()
	if int(idx) >= len(objs) {
		return nil, errObjIndexRange
	}

	childObj := objs[idx]

	var childPos int
	if childObj.IsStruct() {
		// Structs are stored inline: abs is the absolute byte position.
		childPos = int(abs)
	} else {
		// Tables use an additional UOffset indirection.
		if int(abs)+SizeUOffsetT > len(buf) {
			return nil, errVectorOffsetRange
		}

		childPos = int(abs) + int(GetUOffsetT(buf[abs:]))
	}

	return reflectUnpackTable(buf, childPos, childObj, schema)
}

// reflectUnpackVector reads the FlatBuffers vector at abs (an absolute position
// within buf whose content is the UOffset to the vector) and returns all elements
// as []any.  Supports scalar, string, and table element types.
func reflectUnpackVector(buf []byte, abs UOffsetT, fieldType *ReflectionType, schema *ReflectionSchema) ([]any, error) {
	if int(abs)+SizeUOffsetT > len(buf) {
		return nil, errVectorOffsetRange
	}

	vecStart := int(abs) + int(GetUOffsetT(buf[abs:]))
	if vecStart+SizeUOffsetT > len(buf) {
		return nil, errVectorStartRange
	}

	count := int(GetUint32(buf[vecStart:]))
	dataStart := vecStart + SizeUOffsetT
	elem := fieldType.Element()
	result := make([]any, 0, count)

	switch elem {
	case ReflectionBaseTypeBool, ReflectionBaseTypeByte, ReflectionBaseTypeUByte,
		ReflectionBaseTypeShort, ReflectionBaseTypeUShort, ReflectionBaseTypeInt,
		ReflectionBaseTypeUInt, ReflectionBaseTypeLong, ReflectionBaseTypeULong:
		return unpackVectorIntegers(buf, dataStart, count, elem, result), nil

	case ReflectionBaseTypeFloat, ReflectionBaseTypeDouble:
		return unpackVectorFloats(buf, dataStart, count, elem, result), nil

	case ReflectionBaseTypeString:
		for index := range count {
			elemPos := UOffsetT(dataStart + index*4)
			tab := Table{Bytes: buf, Pos: elemPos}
			result = append(result, tab.String(elemPos))
		}

	case ReflectionBaseTypeObj:
		var err error

		result, err = unpackVectorOfTables(buf, dataStart, count, fieldType, schema, result)
		if err != nil {
			return nil, err
		}

	case ReflectionBaseTypeNone, ReflectionBaseTypeUType, ReflectionBaseTypeVector,
		ReflectionBaseTypeUnion, ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return result, nil
	}

	return result, nil
}

// unpackVectorIntegers appends integer-typed elements to result.
//
//nolint:cyclop // All integer subtypes must be dispatched here.
func unpackVectorIntegers(buf []byte, dataStart, count int, elem ReflectionBaseType, result []any) []any {
	switch elem {
	case ReflectionBaseTypeBool:
		for index := range count {
			result = append(result, buf[dataStart+index] != 0)
		}

	case ReflectionBaseTypeByte:
		for index := range count {
			result = append(result, int64(int8(buf[dataStart+index]))) //nolint:gosec // deliberate narrowing
		}

	case ReflectionBaseTypeUByte:
		for index := range count {
			result = append(result, int64(buf[dataStart+index]))
		}

	case ReflectionBaseTypeShort:
		for index := range count {
			result = append(result, int64(GetInt16(buf[dataStart+index*2:])))
		}

	case ReflectionBaseTypeUShort:
		for index := range count {
			result = append(result, int64(GetUint16(buf[dataStart+index*2:])))
		}

	case ReflectionBaseTypeInt:
		for index := range count {
			result = append(result, int64(GetInt32(buf[dataStart+index*4:])))
		}

	case ReflectionBaseTypeUInt:
		for index := range count {
			result = append(result, int64(GetUint32(buf[dataStart+index*4:])))
		}

	case ReflectionBaseTypeLong:
		for index := range count {
			result = append(result, GetInt64(buf[dataStart+index*8:]))
		}

	case ReflectionBaseTypeULong:
		for index := range count {
			result = append(result, int64(GetUint64(buf[dataStart+index*8:]))) //nolint:gosec // deliberate narrowing
		}

	case ReflectionBaseTypeNone, ReflectionBaseTypeUType, ReflectionBaseTypeFloat,
		ReflectionBaseTypeDouble, ReflectionBaseTypeString, ReflectionBaseTypeVector,
		ReflectionBaseTypeObj, ReflectionBaseTypeUnion, ReflectionBaseTypeArray,
		ReflectionBaseTypeVector64:
		return result // Non-integer types: caller should not invoke this function for these.
	}

	return result
}

// unpackVectorFloats appends float-typed elements to result.
func unpackVectorFloats(buf []byte, dataStart, count int, elem ReflectionBaseType, result []any) []any {
	switch elem {
	case ReflectionBaseTypeFloat:
		for index := range count {
			bits := GetUint32(buf[dataStart+index*4:])
			result = append(result, float64(math.Float32frombits(bits)))
		}

	case ReflectionBaseTypeDouble:
		for index := range count {
			bits := GetUint64(buf[dataStart+index*8:])
			result = append(result, math.Float64frombits(bits))
		}

	case ReflectionBaseTypeNone, ReflectionBaseTypeUType, ReflectionBaseTypeBool,
		ReflectionBaseTypeByte, ReflectionBaseTypeUByte, ReflectionBaseTypeShort,
		ReflectionBaseTypeUShort, ReflectionBaseTypeInt, ReflectionBaseTypeUInt,
		ReflectionBaseTypeLong, ReflectionBaseTypeULong, ReflectionBaseTypeString,
		ReflectionBaseTypeVector, ReflectionBaseTypeObj, ReflectionBaseTypeUnion,
		ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return result // Non-float types: caller should not invoke this function for these.
	}

	return result
}

// unpackVectorOfTables appends decoded table entries from a vector of tables
// into result and returns the extended slice.
func unpackVectorOfTables(
	buf []byte,
	dataStart int,
	count int,
	fieldType *ReflectionType,
	schema *ReflectionSchema,
	result []any,
) ([]any, error) {
	idx := fieldType.Index()
	if idx < 0 {
		return nil, errVectorObjBadIndex
	}

	objs := schema.Objects()
	if int(idx) >= len(objs) {
		return nil, errVectorObjIndexRange
	}

	childObj := objs[idx]

	for index := range count {
		elemPos := UOffsetT(dataStart + index*4)

		var childPos int
		if childObj.IsStruct() {
			childPos = int(elemPos)
		} else {
			childPos = int(elemPos) + int(GetUOffsetT(buf[elemPos:]))
		}

		child, err := reflectUnpackTable(buf, childPos, childObj, schema)
		if err != nil {
			return nil, err
		}

		result = append(result, child)
	}

	return result, nil
}

// readScalarInt reads a scalar integer value from buf[abs:] according to the
// given base type and returns it as int64.
func readScalarInt(buf []byte, abs UOffsetT, baseType ReflectionBaseType) (int64, error) {
	switch baseType {
	case ReflectionBaseTypeByte:
		return int64(GetInt8(buf[abs:])), nil
	case ReflectionBaseTypeUByte:
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
		return int64(GetUint64(buf[abs:])), nil //nolint:gosec // deliberate widening; ULong > MaxInt64 wraps
	case ReflectionBaseTypeNone, ReflectionBaseTypeUType, ReflectionBaseTypeBool,
		ReflectionBaseTypeFloat, ReflectionBaseTypeDouble, ReflectionBaseTypeString,
		ReflectionBaseTypeVector, ReflectionBaseTypeObj, ReflectionBaseTypeUnion,
		ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return 0, errNotIntegerType
	}

	return 0, errNotIntegerType
}

// readScalarFloat reads a float32 or float64 from buf[abs:] and returns float64.
func readScalarFloat(buf []byte, abs UOffsetT, baseType ReflectionBaseType) (float64, error) {
	switch baseType {
	case ReflectionBaseTypeFloat:
		bits := GetUint32(buf[abs:])
		return float64(math.Float32frombits(bits)), nil
	case ReflectionBaseTypeDouble:
		bits := GetUint64(buf[abs:])
		return math.Float64frombits(bits), nil
	case ReflectionBaseTypeNone, ReflectionBaseTypeUType, ReflectionBaseTypeBool,
		ReflectionBaseTypeByte, ReflectionBaseTypeUByte, ReflectionBaseTypeShort,
		ReflectionBaseTypeUShort, ReflectionBaseTypeInt, ReflectionBaseTypeUInt,
		ReflectionBaseTypeLong, ReflectionBaseTypeULong, ReflectionBaseTypeString,
		ReflectionBaseTypeVector, ReflectionBaseTypeObj, ReflectionBaseTypeUnion,
		ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return 0, errNotFloatType
	}

	return 0, errNotFloatType
}

// resolveUnionVariant looks up a union variant by its discriminant value in the
// schema's enums vector.  It returns the variant name and the corresponding
// ReflectionObject, or ("", nil) if not found.
//
// FlatBuffers unions are encoded as an enum in the schema.  The enum's values
// vector stores (name, value) pairs; the matching entry's name is the fully
// qualified object name that corresponds to the variant.
func resolveUnionVariant(schema *ReflectionSchema, enumIdx int32, discriminant int) (string, *ReflectionObject) {
	if enumIdx < 0 {
		return "", nil
	}

	enums := schema.Enums()
	if int(enumIdx) >= len(enums) {
		return "", nil
	}

	enumDef := enums[enumIdx]

	for _, enumVal := range enumDef.Values() {
		if int(enumVal.Value()) == discriminant {
			variantName := enumVal.Name()
			obj := resolveUnionObject(schema, enumDef.Name(), variantName)

			return variantName, obj
		}
	}

	return "", nil
}

// resolveUnionObject tries to find the ReflectionObject for a union variant.
// FlatBuffers stores union variant objects under their fully qualified name, so
// we try a few heuristics:
//  1. Exact match on variantName.
//  2. Prefix with the enum's namespace (strip trailing enum name) + variantName.
func resolveUnionObject(schema *ReflectionSchema, enumName, variantName string) *ReflectionObject {
	if obj := schema.ObjectByName(variantName); obj != nil {
		return obj
	}

	// Derive namespace from enumName (everything up to the last ".").
	namespace := ""

	for pos := len(enumName) - 1; pos >= 0; pos-- {
		if enumName[pos] == '.' {
			namespace = enumName[:pos+1]

			break
		}
	}

	if namespace != "" {
		if obj := schema.ObjectByName(namespace + variantName); obj != nil {
			return obj
		}
	}

	return nil
}

// ReflectPack serializes a JSON object into a FlatBuffer using schema reflection.
//
// typeName must be the fully qualified FlatBuffers table name (e.g.
// "MyGame.Example.Monster").  jsonData is a JSON object whose keys are field
// names as declared in the .fbs source.
//
// Value representations:
//   - Bool: JSON bool.
//   - Scalars (integers/floats): JSON number.
//   - Strings: JSON string.
//   - Nested tables: JSON object (recursive).
//   - Vectors: JSON array.
//   - Unions: JSON object with a companion "<field>_type" key whose value is
//     the string variant name, and a "<field>" key whose value is the nested
//     table object.
//
// The function builds the FlatBuffer in the required order (strings and nested
// objects first, then the root table) and calls Finish before returning the
// finished bytes.
//
// Returns an error if typeName is not found in the schema, if jsonData is not
// a valid JSON object, or if a field value cannot be coerced to the declared
// schema type.
//
// Example:
//
//	jsonData := []byte(`{"name":"Orc","hp":300,"mana":100}`)
//	buf, err := flatbuffers.ReflectPack(schema, "MyGame.Example.Monster", jsonData)
//	if err != nil { log.Fatal(err) }
func ReflectPack(schema *ReflectionSchema, typeName string, jsonData []byte) ([]byte, error) {
	obj := schema.ObjectByName(typeName)
	if obj == nil {
		return nil, fmt.Errorf("%w: %q", errTypeNotFound, typeName)
	}

	var inputMap map[string]any

	err := json.Unmarshal(jsonData, &inputMap)
	if err != nil {
		return nil, fmt.Errorf("flatbuffers: JSON unmarshal: %w", err)
	}

	builder := NewBuilder(reflectBuilderInitialSize)

	rootOffset, err := reflectPackTable(builder, inputMap, obj, schema)
	if err != nil {
		return nil, err
	}

	builder.Finish(rootOffset)

	return builder.FinishedBytes(), nil
}

// reflectPackTable recursively packs a map[string]any into a FlatBuffers table
// using the given object schema descriptor.  Returns the UOffsetT of the
// written table.
//
// FlatBuffers requires that all non-inline data (strings, nested tables,
// vectors) be written before StartObject is called for the parent.  This
// function performs a two-pass approach:
//  1. Pre-build all offset-valued fields (strings, nested tables, vectors).
//  2. Call StartObject / Prepend* / EndObject to write the table itself.
//
// The vtable slot index for each field is derived from the field's vtable byte
// offset: slot = (voffset - 4) / 2.  This matches the slot numbering that
// flatc-generated Pack() functions use with PrependXxxSlot.
func reflectPackTable(
	builder *Builder,
	inputMap map[string]any,
	obj *ReflectionObject,
	schema *ReflectionSchema,
) (UOffsetT, error) {
	fields := obj.Fields()

	// Compute the maximum vtable slot index to size StartObject correctly.
	maxSlot := 0

	for _, field := range fields {
		if slot := vtableSlot(field.Offset()); slot > maxSlot {
			maxSlot = slot
		}
	}

	// prebuiltOffsets maps vtable slot -> pre-built UOffsetT for offset fields.
	prebuiltOffsets := make(map[int]UOffsetT, len(fields))

	// Pass 1: build all offset-valued sub-objects before opening the table.
	err := prepackOffsetFields(builder, inputMap, fields, schema, prebuiltOffsets)
	if err != nil {
		return 0, err
	}

	// Pass 2: write the table.  StartObject must be called with enough slots
	// to cover the highest vtable slot index used by any field.
	builder.StartObject(maxSlot + 1)

	for _, field := range fields {
		fieldType := field.Type()
		if fieldType == nil {
			continue
		}

		name := field.Name()
		slot := vtableSlot(field.Offset())

		slotErr := packFieldSlot(builder, inputMap, name, slot, field, fieldType, obj, schema, prebuiltOffsets)
		if slotErr != nil {
			return 0, fmt.Errorf("flatbuffers: field %q: %w", name, slotErr)
		}
	}

	return builder.EndObject(), nil
}

// prepackOffsetFields writes all offset-typed fields (strings, tables, vectors,
// unions) into the builder before the parent table is opened.  Results are stored
// in prebuiltOffsets keyed by vtable slot number.
func prepackOffsetFields(
	builder *Builder,
	inputMap map[string]any,
	fields []*ReflectionField,
	schema *ReflectionSchema,
	prebuiltOffsets map[int]UOffsetT,
) error {
	for _, field := range fields {
		fieldType := field.Type()
		if fieldType == nil {
			continue
		}

		name := field.Name()
		slot := vtableSlot(field.Offset())

		val, present := inputMap[name]
		if !present {
			continue
		}

		off, err := buildOffsetField(builder, inputMap, name, val, fieldType, schema)
		if err != nil {
			return fmt.Errorf("flatbuffers: field %q: %w", name, err)
		}

		if off != 0 {
			prebuiltOffsets[slot] = off
		}
	}

	return nil
}

// buildOffsetField constructs a pre-built offset for a single offset-typed field
// (String, Obj, Vector, or Union).  Returns 0 for scalar and unsupported types.
func buildOffsetField(
	builder *Builder,
	inputMap map[string]any,
	name string,
	val any,
	fieldType *ReflectionType,
	schema *ReflectionSchema,
) (UOffsetT, error) {
	switch fieldType.BaseType() {
	case ReflectionBaseTypeString:
		strVal, err := coerceString(val)
		if err != nil {
			return 0, err
		}

		return builder.CreateString(strVal), nil

	case ReflectionBaseTypeObj:
		return packObjField(builder, val, fieldType, schema)

	case ReflectionBaseTypeVector:
		arrVal, err := coerceSlice(val)
		if err != nil {
			return 0, err
		}

		return reflectPackVector(builder, arrVal, fieldType, schema)

	case ReflectionBaseTypeUnion:
		return packUnionField(builder, inputMap, name, val, fieldType, schema)

	case ReflectionBaseTypeBool, ReflectionBaseTypeByte, ReflectionBaseTypeUByte,
		ReflectionBaseTypeShort, ReflectionBaseTypeUShort, ReflectionBaseTypeInt,
		ReflectionBaseTypeUInt, ReflectionBaseTypeLong, ReflectionBaseTypeULong,
		ReflectionBaseTypeFloat, ReflectionBaseTypeDouble, ReflectionBaseTypeUType,
		ReflectionBaseTypeNone, ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		// Scalar types are written in Pass 2; None/Array/Vector64 are unsupported.
		return 0, nil
	}

	return 0, nil
}

// packFieldSlot writes a single field into the open table.  Offset-typed
// fields (strings, tables, vectors, unions) look up their pre-built offset
// in prebuiltOffsets; scalar fields are prepended directly.
//
//nolint:cyclop // Dispatch on all scalar types is inherently wide.
func packFieldSlot(
	builder *Builder,
	inputMap map[string]any,
	name string,
	slot int,
	field *ReflectionField,
	fieldType *ReflectionType,
	obj *ReflectionObject,
	schema *ReflectionSchema,
	prebuiltOffsets map[int]UOffsetT,
) error {
	switch fieldType.BaseType() {
	case ReflectionBaseTypeBool:
		return packBoolSlot(builder, inputMap, name, slot)

	case ReflectionBaseTypeByte:
		return packInt8Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeUByte:
		return packUint8Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeShort:
		return packInt16Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeUShort:
		return packUint16Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeInt:
		return packInt32Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeUInt:
		return packUint32Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeLong:
		return packInt64Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeULong:
		return packUint64Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeFloat:
		return packFloat32Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeDouble:
		return packFloat64Slot(builder, inputMap, name, slot)

	case ReflectionBaseTypeString, ReflectionBaseTypeObj,
		ReflectionBaseTypeVector, ReflectionBaseTypeUnion:
		if off, found := prebuiltOffsets[slot]; found && off != 0 {
			builder.PrependUOffsetTSlot(slot, off, 0)
		}

	case ReflectionBaseTypeUType:
		// The UType discriminant companion field for a union.  Derive the
		// union field name by stripping the "_type" suffix.
		return packUTypeField(builder, inputMap, name, slot, field, obj, schema)

	case ReflectionBaseTypeNone, ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return nil
	}

	return nil
}

// ── Scalar slot helpers ────────────────────────────────────────────────────
// One function per scalar type keeps packFieldSlot readable and keeps each
// helper below the funlen threshold.

func packBoolSlot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	boolVal, err := coerceBool(val)
	if err != nil {
		return err
	}

	builder.PrependBoolSlot(slot, boolVal, false)

	return nil
}

func packInt8Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependInt8Slot(slot, int8(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packUint8Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependUint8Slot(slot, uint8(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packInt16Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependInt16Slot(slot, int16(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packUint16Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependUint16Slot(slot, uint16(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packInt32Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependInt32Slot(slot, int32(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packUint32Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependUint32Slot(slot, uint32(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packInt64Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependInt64Slot(slot, intVal, 0)

	return nil
}

func packUint64Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	intVal, err := coerceInt64(val)
	if err != nil {
		return err
	}

	builder.PrependUint64Slot(slot, uint64(intVal), 0) //nolint:gosec // deliberate narrowing

	return nil
}

func packFloat32Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	floatVal, err := coerceFloat64(val)
	if err != nil {
		return err
	}

	builder.PrependFloat32Slot(slot, float32(floatVal), 0)

	return nil
}

func packFloat64Slot(builder *Builder, inputMap map[string]any, name string, slot int) error {
	val, present := inputMap[name]
	if !present {
		return nil
	}

	floatVal, err := coerceFloat64(val)
	if err != nil {
		return err
	}

	builder.PrependFloat64Slot(slot, floatVal, 0)

	return nil
}

// packUTypeField writes the uint8 discriminant for a union's companion _type
// field.  The field name is expected to end in "_type".
func packUTypeField(
	builder *Builder,
	inputMap map[string]any,
	name string,
	slot int,
	_ *ReflectionField,
	obj *ReflectionObject,
	schema *ReflectionSchema,
) error {
	unionFieldName := name

	const typeSuffix = "_type"
	if len(unionFieldName) > len(typeSuffix) && unionFieldName[len(unionFieldName)-len(typeSuffix):] == typeSuffix {
		unionFieldName = unionFieldName[:len(unionFieldName)-len(typeSuffix)]
	}

	unionField := obj.FieldByName(unionFieldName)
	if unionField == nil {
		return nil
	}

	typeVal, typeOk := inputMap[name]
	if !typeOk {
		return nil
	}

	variantName, err := coerceString(typeVal)
	if err != nil {
		return nil //nolint:nilerr // bad input silently skipped
	}

	uft := unionField.Type()
	if uft == nil {
		return nil
	}

	discriminant := resolveUnionDiscriminant(schema, uft.Index(), variantName)
	builder.PrependUint8Slot(slot, uint8(discriminant), 0) //nolint:gosec // discriminant <= 255 by spec

	return nil
}

// packObjField packs a nested table or struct field value.
func packObjField(
	builder *Builder,
	val any,
	fieldType *ReflectionType,
	schema *ReflectionSchema,
) (UOffsetT, error) {
	childMap, err := coerceMap(val)
	if err != nil {
		return 0, err
	}

	idx := fieldType.Index()
	if idx < 0 {
		return 0, nil
	}

	objs := schema.Objects()
	if int(idx) >= len(objs) {
		return 0, nil
	}

	return reflectPackTable(builder, childMap, objs[idx], schema)
}

// packUnionField packs a union field from JSON using the companion _type key
// to identify the variant.
func packUnionField(
	builder *Builder,
	inputMap map[string]any,
	name string,
	val any,
	fieldType *ReflectionType,
	schema *ReflectionSchema,
) (UOffsetT, error) {
	typeVal, typeOk := inputMap[name+"_type"]
	if !typeOk {
		return 0, nil
	}

	variantName, err := coerceString(typeVal)
	if err != nil {
		return 0, fmt.Errorf("union type key: %w", err)
	}

	childMap, err := coerceMap(val)
	if err != nil {
		return 0, fmt.Errorf("union value: %w", err)
	}

	childObj := resolveUnionObject(schema, resolveEnumName(schema, fieldType.Index()), variantName)
	if childObj == nil {
		return 0, fmt.Errorf("%w: %q", errUnionVariantNotFound, variantName)
	}

	return reflectPackTable(builder, childMap, childObj, schema)
}

// reflectPackVector writes a FlatBuffers vector from a []any slice.
// Returns the UOffsetT of the written vector, or 0 if the slice is empty.
//
//nolint:cyclop // Dispatch on all FlatBuffers scalar element types is inherently broad.
func reflectPackVector(
	builder *Builder,
	elems []any,
	fieldType *ReflectionType,
	schema *ReflectionSchema,
) (UOffsetT, error) {
	elem := fieldType.Element()
	count := len(elems)

	if count == 0 {
		builder.StartVector(0, 0, 1)

		return builder.EndVector(0), nil
	}

	switch elem {
	case ReflectionBaseTypeString:
		return packVectorOfStrings(builder, elems)

	case ReflectionBaseTypeObj:
		return packVectorOfTables(builder, elems, fieldType, schema)

	case ReflectionBaseTypeBool:
		return packVectorBool(builder, elems, count)

	case ReflectionBaseTypeByte, ReflectionBaseTypeUByte:
		return packVectorByte(builder, elems, count)

	case ReflectionBaseTypeShort, ReflectionBaseTypeUShort:
		return packVectorShort(builder, elems, count)

	case ReflectionBaseTypeInt, ReflectionBaseTypeUInt:
		return packVectorInt(builder, elems, count)

	case ReflectionBaseTypeLong, ReflectionBaseTypeULong:
		return packVectorLong(builder, elems, count)

	case ReflectionBaseTypeFloat:
		return packVectorFloat32(builder, elems, count)

	case ReflectionBaseTypeDouble:
		return packVectorFloat64(builder, elems, count)

	case ReflectionBaseTypeNone, ReflectionBaseTypeUType, ReflectionBaseTypeVector,
		ReflectionBaseTypeUnion, ReflectionBaseTypeArray, ReflectionBaseTypeVector64:
		return 0, errUnsupportedElemType
	}

	return 0, errUnsupportedElemType
}

// ── Vector packing helpers ─────────────────────────────────────────────────

func packVectorBool(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeBool, count, SizeBool)

	for index := count - 1; index >= 0; index-- {
		boolVal, err := coerceBool(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependByte(boolByte(boolVal))
	}

	return builder.EndVector(count), nil
}

func packVectorByte(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeInt8, count, SizeInt8)

	for index := count - 1; index >= 0; index-- {
		intVal, err := coerceInt64(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependByte(byte(intVal)) //nolint:gosec // deliberate narrowing
	}

	return builder.EndVector(count), nil
}

func packVectorShort(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeInt16, count, SizeInt16)

	for index := count - 1; index >= 0; index-- {
		intVal, err := coerceInt64(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependUint16(uint16(intVal)) //nolint:gosec // deliberate narrowing
	}

	return builder.EndVector(count), nil
}

func packVectorInt(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeInt32, count, SizeInt32)

	for index := count - 1; index >= 0; index-- {
		intVal, err := coerceInt64(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependUint32(uint32(intVal)) //nolint:gosec // deliberate narrowing
	}

	return builder.EndVector(count), nil
}

func packVectorLong(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeInt64, count, SizeInt64)

	for index := count - 1; index >= 0; index-- {
		intVal, err := coerceInt64(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependUint64(uint64(intVal)) //nolint:gosec // deliberate narrowing
	}

	return builder.EndVector(count), nil
}

func packVectorFloat32(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeFloat32, count, SizeFloat32)

	for index := count - 1; index >= 0; index-- {
		floatVal, err := coerceFloat64(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependUint32(math.Float32bits(float32(floatVal)))
	}

	return builder.EndVector(count), nil
}

func packVectorFloat64(builder *Builder, elems []any, count int) (UOffsetT, error) {
	builder.StartVector(SizeFloat64, count, SizeFloat64)

	for index := count - 1; index >= 0; index-- {
		floatVal, err := coerceFloat64(elems[index])
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		builder.PrependUint64(math.Float64bits(floatVal))
	}

	return builder.EndVector(count), nil
}

// packVectorOfStrings writes a vector of FlatBuffers strings and returns its
// UOffsetT.
func packVectorOfStrings(builder *Builder, elems []any) (UOffsetT, error) {
	count := len(elems)
	offsets := make([]UOffsetT, count)

	for index, val := range elems {
		strVal, err := coerceString(val)
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		offsets[index] = builder.CreateString(strVal)
	}

	builder.StartVector(SizeUOffsetT, count, SizeUOffsetT)

	for index := count - 1; index >= 0; index-- {
		builder.PrependUOffsetT(offsets[index])
	}

	return builder.EndVector(count), nil
}

// packVectorOfTables writes a vector of nested tables and returns its UOffsetT.
func packVectorOfTables(
	builder *Builder,
	elems []any,
	ft *ReflectionType,
	schema *ReflectionSchema,
) (UOffsetT, error) {
	count := len(elems)
	idx := ft.Index()

	if idx < 0 {
		return 0, errVectorObjBadIndex
	}

	objs := schema.Objects()
	if int(idx) >= len(objs) {
		return 0, errVectorObjIndexRange
	}

	childObj := objs[idx]
	offsets := make([]UOffsetT, count)

	for index, val := range elems {
		childMap, err := coerceMap(val)
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		off, err := reflectPackTable(builder, childMap, childObj, schema)
		if err != nil {
			return 0, fmt.Errorf("vector element %d: %w", index, err)
		}

		offsets[index] = off
	}

	builder.StartVector(SizeUOffsetT, count, SizeUOffsetT)

	for index := count - 1; index >= 0; index-- {
		builder.PrependUOffsetT(offsets[index])
	}

	return builder.EndVector(count), nil
}

// vtableSlot converts a FlatBuffers vtable byte offset (voffset) to a Builder
// slot index.  FlatBuffers vtable offsets start at vtableBaseOffset (4) for
// slot 0, and each slot occupies vtableBytesPerSlot (2) bytes.
// Formula: slot = (voffset - 4) / 2.
func vtableSlot(voffset uint16) int {
	if voffset < vtableBaseOffset {
		return 0
	}

	return int((voffset - vtableBaseOffset) / vtableBytesPerSlot)
}

// resolveEnumName returns the name of the enum at index enumIdx in the
// schema's enums vector, or "" if out of range.
func resolveEnumName(schema *ReflectionSchema, enumIdx int32) string {
	if enumIdx < 0 {
		return ""
	}

	enums := schema.Enums()
	if int(enumIdx) >= len(enums) {
		return ""
	}

	return enums[enumIdx].Name()
}

// resolveUnionDiscriminant finds the integer discriminant for a given variant
// name within the schema enum at enumIdx.  Returns 0 (NONE) if not found.
func resolveUnionDiscriminant(schema *ReflectionSchema, enumIdx int32, variantName string) int {
	if enumIdx < 0 {
		return 0
	}

	enums := schema.Enums()
	if int(enumIdx) >= len(enums) {
		return 0
	}

	for _, enumVal := range enums[enumIdx].Values() {
		if enumVal.Name() == variantName {
			return int(enumVal.Value())
		}
	}

	return 0
}

// ── JSON coercion helpers ──────────────────────────────────────────────────

// coerceString coerces an any value to string.
func coerceString(val any) (string, error) {
	switch typed := val.(type) {
	case string:
		return typed, nil
	case nil:
		return "", nil
	default:
		return "", fmt.Errorf("%w, got %T", errExpectedString, val)
	}
}

// coerceBool coerces an any value to bool.
func coerceBool(val any) (bool, error) {
	switch typed := val.(type) {
	case bool:
		return typed, nil
	case float64:
		return typed != 0, nil
	case json.Number:
		floatVal, err := typed.Float64()
		return floatVal != 0, err
	default:
		return false, fmt.Errorf("%w, got %T", errExpectedBool, val)
	}
}

// coerceInt64 coerces an any value to int64.  json.Unmarshal produces
// float64 for all JSON numbers, so we round-trip through float64.
func coerceInt64(val any) (int64, error) {
	switch typed := val.(type) {
	case float64:
		return int64(typed), nil
	case int64:
		return typed, nil
	case int:
		return int64(typed), nil
	case json.Number:
		intVal, err := typed.Int64()
		if err != nil {
			return 0, fmt.Errorf("coercing json.Number to int64: %w", err)
		}

		return intVal, nil
	default:
		return 0, fmt.Errorf("%w, got %T", errExpectedNumber, val)
	}
}

// coerceFloat64 coerces an any value to float64.
func coerceFloat64(val any) (float64, error) {
	switch typed := val.(type) {
	case float64:
		return typed, nil
	case json.Number:
		floatVal, err := typed.Float64()
		if err != nil {
			return 0, fmt.Errorf("coercing json.Number to float64: %w", err)
		}

		return floatVal, nil
	default:
		return 0, fmt.Errorf("%w, got %T", errExpectedNumber, val)
	}
}

// coerceMap coerces an any value to map[string]any.
func coerceMap(val any) (map[string]any, error) {
	if mapVal, ok := val.(map[string]any); ok {
		return mapVal, nil
	}

	return nil, fmt.Errorf("%w, got %T", errExpectedObject, val)
}

// coerceSlice coerces an any value to []any.
func coerceSlice(val any) ([]any, error) {
	if sliceVal, ok := val.([]any); ok {
		return sliceVal, nil
	}

	return nil, fmt.Errorf("%w, got %T", errExpectedArray, val)
}

// boolByte converts bool to byte for vector element writes.
func boolByte(boolVal bool) byte {
	if boolVal {
		return 1
	}

	return 0
}
