package flatbuffers

import (
	"fmt"
	"math"
)

// DeltaKind indicates the type of change detected between two FlatBuffer
// payloads.
type DeltaKind int

const (
	// DeltaChanged means the field is present in both buffers but has different
	// values.
	DeltaChanged DeltaKind = iota
	// DeltaAdded means the field is present in buffer B but absent in buffer A.
	DeltaAdded
	// DeltaRemoved means the field is present in buffer A but absent in buffer B.
	DeltaRemoved
)

// FieldDelta represents a single field-level difference between two FlatBuffer
// payloads of the same root type.
//
// Path uses dot notation to express nested fields:
//   - A top-level field named "hp" becomes "hp".
//   - A nested table field "pos" with sub-field "x" becomes "pos.x".
//   - A vector element at index 2 of field "inventory" becomes "inventory[2]".
type FieldDelta struct {
	// Path is the dot-separated (and bracket-indexed for vectors) field path.
	Path string
	// Kind classifies the change as Changed, Added, or Removed.
	Kind DeltaKind
	// OldValue is the value read from buffer A; nil when Kind == DeltaAdded.
	OldValue any
	// NewValue is the value read from buffer B; nil when Kind == DeltaRemoved.
	NewValue any
}

// DiffBuffers compares two FlatBuffer payloads against a compiled binary schema
// and returns the field-level differences between them.  Both buffers must
// encode the same root table type named by typeName.
//
// schema is obtained by calling [LoadReflectionSchema] on a .bfbs file.
// typeName is the fully qualified FlatBuffers type name
// (e.g. "MyGame.Example.Monster").
// bufA and bufB are raw FlatBuffer data buffers (not schema buffers); each
// must start with a 4-byte root UOffsetT as produced by [Builder.FinishedBytes].
//
// The returned slice contains one [FieldDelta] per differing leaf value,
// ordered by schema field declaration order (depth-first for nested tables,
// element order for vectors).  An empty slice indicates the two buffers are
// semantically identical for all fields described by the schema.
//
// Limitations:
//   - Union fields are not compared; they are silently skipped.
//   - Vectors of nested tables are compared recursively up to min(lenA, lenB)
//     elements; extra elements in either buffer are reported as Added/Removed.
//   - Struct fields inside vectors are not yet supported and are skipped.
//
// Example usage for CDO configuration audit logging:
//
//	schema, err := flatbuffers.LoadReflectionSchema(bfbsBytes)
//	if err != nil { ... }
//
//	deltas, err := flatbuffers.DiffBuffers(schema, "Ln2.ControllerConfig", oldBuf, newBuf)
//	if err != nil { ... }
//	for _, d := range deltas {
//	    log.Printf("field %q: %v -> %v", d.Path, d.OldValue, d.NewValue)
//	}
func DiffBuffers(schema *ReflectionSchema, typeName string, bufA, bufB []byte) ([]FieldDelta, error) {
	obj := schema.ObjectByName(typeName)
	if obj == nil {
		return nil, fmt.Errorf("flatbuffers: type %q not found in schema", typeName)
	}
	if len(bufA) < SizeUOffsetT {
		return nil, fmt.Errorf("flatbuffers: bufA too short (%d bytes)", len(bufA))
	}
	if len(bufB) < SizeUOffsetT {
		return nil, fmt.Errorf("flatbuffers: bufB too short (%d bytes)", len(bufB))
	}
	rootPosA := int(GetUOffsetT(bufA))
	rootPosB := int(GetUOffsetT(bufB))
	var deltas []FieldDelta
	diffTable(bufA, rootPosA, bufB, rootPosB, obj, schema, "", &deltas)
	return deltas, nil
}

// diffTable compares two table instances (one from bufA at tablePosA and one
// from bufB at tablePosB) against the schema object obj, appending any
// discovered differences to *deltas.  prefix is the dot-separated path of
// parent fields, or "" for the root table.
func diffTable(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	obj *ReflectionObject,
	schema *ReflectionSchema,
	prefix string,
	deltas *[]FieldDelta,
) {
	for _, field := range obj.Fields() {
		fieldName := field.Name()
		path := fieldName
		if prefix != "" {
			path = prefix + "." + fieldName
		}

		ft := field.Type()
		if ft == nil {
			continue
		}

		switch ft.BaseType() {
		case ReflectionBaseTypeBool:
			diffBoolField(bufA, tablePosA, bufB, tablePosB, field, path, deltas)

		case ReflectionBaseTypeUType,
			ReflectionBaseTypeByte,
			ReflectionBaseTypeUByte,
			ReflectionBaseTypeShort,
			ReflectionBaseTypeUShort,
			ReflectionBaseTypeInt,
			ReflectionBaseTypeUInt,
			ReflectionBaseTypeLong,
			ReflectionBaseTypeULong:
			diffIntField(bufA, tablePosA, bufB, tablePosB, field, path, deltas)

		case ReflectionBaseTypeFloat, ReflectionBaseTypeDouble:
			diffFloatField(bufA, tablePosA, bufB, tablePosB, field, path, deltas)

		case ReflectionBaseTypeString:
			diffStringField(bufA, tablePosA, bufB, tablePosB, field, path, deltas)

		case ReflectionBaseTypeObj:
			diffObjField(bufA, tablePosA, bufB, tablePosB, field, ft, schema, path, deltas)

		case ReflectionBaseTypeVector, ReflectionBaseTypeVector64:
			diffVectorField(bufA, tablePosA, bufB, tablePosB, field, ft, schema, path, deltas)

		default:
			// Union and Array types are not compared.
		}
	}
}

// fieldAbsent returns true when the vtable entry for field is missing in buf at
// tablePos (i.e. the field was not written into the buffer).
func fieldAbsent(buf []byte, tablePos int, field *ReflectionField) bool {
	return resolveVtableEntry(buf, tablePos, field.Offset()) == 0
}

// diffBoolField compares a bool field between bufA and bufB.
func diffBoolField(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	field *ReflectionField,
	path string,
	deltas *[]FieldDelta,
) {
	absentA := fieldAbsent(bufA, tablePosA, field)
	absentB := fieldAbsent(bufB, tablePosB, field)
	defaultVal := field.DefaultInteger() != 0

	switch {
	case absentA && absentB:
		return
	case absentA && !absentB:
		valB, err := GetFieldBool(bufB, tablePosB, field)
		if err != nil {
			return
		}
		if valB != defaultVal {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaAdded, OldValue: nil, NewValue: valB})
		}
	case !absentA && absentB:
		valA, err := GetFieldBool(bufA, tablePosA, field)
		if err != nil {
			return
		}
		if valA != defaultVal {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaRemoved, OldValue: valA, NewValue: nil})
		}
	default:
		valA, errA := GetFieldBool(bufA, tablePosA, field)
		valB, errB := GetFieldBool(bufB, tablePosB, field)
		if errA != nil || errB != nil {
			return
		}
		if valA != valB {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
		}
	}
}

// diffIntField compares an integer field between bufA and bufB.
func diffIntField(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	field *ReflectionField,
	path string,
	deltas *[]FieldDelta,
) {
	absentA := fieldAbsent(bufA, tablePosA, field)
	absentB := fieldAbsent(bufB, tablePosB, field)
	defaultVal := field.DefaultInteger()

	switch {
	case absentA && absentB:
		return
	case absentA && !absentB:
		valB, err := GetFieldInt(bufB, tablePosB, field)
		if err != nil {
			return
		}
		if valB != defaultVal {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaAdded, OldValue: nil, NewValue: valB})
		}
	case !absentA && absentB:
		valA, err := GetFieldInt(bufA, tablePosA, field)
		if err != nil {
			return
		}
		if valA != defaultVal {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaRemoved, OldValue: valA, NewValue: nil})
		}
	default:
		valA, errA := GetFieldInt(bufA, tablePosA, field)
		valB, errB := GetFieldInt(bufB, tablePosB, field)
		if errA != nil || errB != nil {
			return
		}
		if valA != valB {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
		}
	}
}

// diffFloatField compares a float/double field between bufA and bufB.
// NaN values compare as equal to suppress false positives from uninitialized
// float fields that are NaN in both buffers.
func diffFloatField(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	field *ReflectionField,
	path string,
	deltas *[]FieldDelta,
) {
	absentA := fieldAbsent(bufA, tablePosA, field)
	absentB := fieldAbsent(bufB, tablePosB, field)
	defaultVal := field.DefaultReal()

	switch {
	case absentA && absentB:
		return
	case absentA && !absentB:
		valB, err := GetFieldFloat(bufB, tablePosB, field)
		if err != nil {
			return
		}
		if valB != defaultVal && !(math.IsNaN(valB) && math.IsNaN(defaultVal)) {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaAdded, OldValue: nil, NewValue: valB})
		}
	case !absentA && absentB:
		valA, err := GetFieldFloat(bufA, tablePosA, field)
		if err != nil {
			return
		}
		if valA != defaultVal && !(math.IsNaN(valA) && math.IsNaN(defaultVal)) {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaRemoved, OldValue: valA, NewValue: nil})
		}
	default:
		valA, errA := GetFieldFloat(bufA, tablePosA, field)
		valB, errB := GetFieldFloat(bufB, tablePosB, field)
		if errA != nil || errB != nil {
			return
		}
		// Treat NaN == NaN as equal (both are "not a number" — semantically same).
		if valA != valB && !(math.IsNaN(valA) && math.IsNaN(valB)) {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
		}
	}
}

// diffStringField compares a string field between bufA and bufB.
func diffStringField(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	field *ReflectionField,
	path string,
	deltas *[]FieldDelta,
) {
	absentA := fieldAbsent(bufA, tablePosA, field)
	absentB := fieldAbsent(bufB, tablePosB, field)

	switch {
	case absentA && absentB:
		return
	case absentA && !absentB:
		valB, err := GetFieldString(bufB, tablePosB, field)
		if err != nil {
			return
		}
		if valB != "" {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaAdded, OldValue: nil, NewValue: valB})
		}
	case !absentA && absentB:
		valA, err := GetFieldString(bufA, tablePosA, field)
		if err != nil {
			return
		}
		if valA != "" {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaRemoved, OldValue: valA, NewValue: nil})
		}
	default:
		valA, errA := GetFieldString(bufA, tablePosA, field)
		valB, errB := GetFieldString(bufB, tablePosB, field)
		if errA != nil || errB != nil {
			return
		}
		if valA != valB {
			*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
		}
	}
}

// diffObjField handles a nested table/struct field.  For structs, the field
// offset points directly to the inline struct data; for tables, it is a
// relative offset to the child table.
func diffObjField(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	field *ReflectionField,
	ft *ReflectionType,
	schema *ReflectionSchema,
	path string,
	deltas *[]FieldDelta,
) {
	idx := ft.Index()
	if idx < 0 {
		return
	}
	objects := schema.Objects()
	if int(idx) >= len(objects) {
		return
	}
	nested := objects[idx]

	absentA := fieldAbsent(bufA, tablePosA, field)
	absentB := fieldAbsent(bufB, tablePosB, field)

	if absentA && absentB {
		return
	}

	if nested.IsStruct() {
		// Structs are inline; the vtable entry is a direct byte offset from tablePos.
		if absentA || absentB {
			// Treat absent struct as all-zero; produce deltas for each scalar field.
			// For simplicity, mark the whole object as added/removed rather than
			// enumerating individual fields when only one side is present.
			if absentA {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaAdded, OldValue: nil, NewValue: "<struct>"})
			} else {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaRemoved, OldValue: "<struct>", NewValue: nil})
			}
			return
		}
		// Both present: compare inline struct fields.
		absA := int(resolveVtableEntry(bufA, tablePosA, field.Offset()))
		absB := int(resolveVtableEntry(bufB, tablePosB, field.Offset()))
		diffStructInline(bufA, absA, bufB, absB, nested, path, deltas)
		return
	}

	// Table: follow the UOffset.
	if absentA {
		*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaAdded, OldValue: nil, NewValue: "<table>"})
		return
	}
	if absentB {
		*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaRemoved, OldValue: "<table>", NewValue: nil})
		return
	}

	// Resolve the child table positions by following the UOffset stored at absA/absB.
	absA := resolveVtableEntry(bufA, tablePosA, field.Offset())
	if int(absA)+SizeUOffsetT > len(bufA) {
		return
	}
	childPosA := int(UOffsetT(absA) + GetUOffsetT(bufA[absA:]))

	absB := resolveVtableEntry(bufB, tablePosB, field.Offset())
	if int(absB)+SizeUOffsetT > len(bufB) {
		return
	}
	childPosB := int(UOffsetT(absB) + GetUOffsetT(bufB[absB:]))

	// Bounds-check child positions before recursing.
	if childPosA < 0 || childPosA >= len(bufA) || childPosB < 0 || childPosB >= len(bufB) {
		return
	}

	diffTable(bufA, childPosA, bufB, childPosB, nested, schema, path, deltas)
}

// diffStructInline compares two inline (non-table) structs field-by-field.
// posA and posB are the absolute byte positions of the struct data in their
// respective buffers.
func diffStructInline(
	bufA []byte, posA int,
	bufB []byte, posB int,
	obj *ReflectionObject,
	prefix string,
	deltas *[]FieldDelta,
) {
	byteSize := obj.ByteSize()
	if byteSize <= 0 {
		return
	}
	// Verify bounds before reading.
	if posA+byteSize > len(bufA) || posB+byteSize > len(bufB) {
		return
	}

	// Struct fields use their vtable offset as a direct byte offset from the
	// struct start (no vtable indirection for structs).
	for _, field := range obj.Fields() {
		ft := field.Type()
		if ft == nil {
			continue
		}
		fieldOffset := int(field.Offset())
		if fieldOffset == 0 {
			continue
		}
		fieldName := field.Name()
		path := fieldName
		if prefix != "" {
			path = prefix + "." + fieldName
		}

		absA := posA + fieldOffset
		absB := posB + fieldOffset

		switch ft.BaseType() {
		case ReflectionBaseTypeBool:
			if absA >= len(bufA) || absB >= len(bufB) {
				continue
			}
			valA := GetBool(bufA[absA:])
			valB := GetBool(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeByte:
			if absA >= len(bufA) || absB >= len(bufB) {
				continue
			}
			valA := GetInt8(bufA[absA:])
			valB := GetInt8(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeUByte:
			if absA >= len(bufA) || absB >= len(bufB) {
				continue
			}
			valA := GetUint8(bufA[absA:])
			valB := GetUint8(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeShort:
			if absA+SizeInt16 > len(bufA) || absB+SizeInt16 > len(bufB) {
				continue
			}
			valA := GetInt16(bufA[absA:])
			valB := GetInt16(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeUShort:
			if absA+SizeUint16 > len(bufA) || absB+SizeUint16 > len(bufB) {
				continue
			}
			valA := GetUint16(bufA[absA:])
			valB := GetUint16(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeInt:
			if absA+SizeInt32 > len(bufA) || absB+SizeInt32 > len(bufB) {
				continue
			}
			valA := GetInt32(bufA[absA:])
			valB := GetInt32(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeUInt:
			if absA+SizeUint32 > len(bufA) || absB+SizeUint32 > len(bufB) {
				continue
			}
			valA := GetUint32(bufA[absA:])
			valB := GetUint32(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeLong:
			if absA+SizeInt64 > len(bufA) || absB+SizeInt64 > len(bufB) {
				continue
			}
			valA := GetInt64(bufA[absA:])
			valB := GetInt64(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeULong:
			if absA+SizeUint64 > len(bufA) || absB+SizeUint64 > len(bufB) {
				continue
			}
			valA := GetUint64(bufA[absA:])
			valB := GetUint64(bufB[absB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: int64(valA), NewValue: int64(valB)})
			}
		case ReflectionBaseTypeFloat:
			if absA+SizeFloat32 > len(bufA) || absB+SizeFloat32 > len(bufB) {
				continue
			}
			valA := GetFloat32(bufA[absA:])
			valB := GetFloat32(bufB[absB:])
			fa, fb := float64(valA), float64(valB)
			if fa != fb && !(math.IsNaN(fa) && math.IsNaN(fb)) {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: fa, NewValue: fb})
			}
		case ReflectionBaseTypeDouble:
			if absA+SizeFloat64 > len(bufA) || absB+SizeFloat64 > len(bufB) {
				continue
			}
			valA := GetFloat64(bufA[absA:])
			valB := GetFloat64(bufB[absB:])
			if valA != valB && !(math.IsNaN(valA) && math.IsNaN(valB)) {
				*deltas = append(*deltas, FieldDelta{Path: path, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		}
	}
}

// diffVectorField compares a vector field between bufA and bufB.
// Elements beyond min(lenA, lenB) are reported as Added or Removed.
// Supported element types: scalars, strings, and nested tables.
func diffVectorField(
	bufA []byte, tablePosA int,
	bufB []byte, tablePosB int,
	field *ReflectionField,
	ft *ReflectionType,
	schema *ReflectionSchema,
	path string,
	deltas *[]FieldDelta,
) {
	absentA := fieldAbsent(bufA, tablePosA, field)
	absentB := fieldAbsent(bufB, tablePosB, field)

	// Helper to read vector length and data start from buf at tablePos.
	// Returns (length, dataStart, ok).
	readVecMeta := func(buf []byte, tablePos int) (int, UOffsetT, bool) {
		abs := resolveVtableEntry(buf, tablePos, field.Offset())
		if abs == 0 {
			return 0, 0, true // absent == empty vector
		}
		// abs points to a UOffset that leads to the vector header.
		if int(abs)+SizeUOffsetT > len(buf) {
			return 0, 0, false
		}
		vecBase := UOffsetT(abs) + GetUOffsetT(buf[abs:])
		if int(vecBase)+SizeUOffsetT > len(buf) {
			return 0, 0, false
		}
		count := int(GetUOffsetT(buf[vecBase:]))
		dataStart := vecBase + UOffsetT(SizeUOffsetT)
		return count, dataStart, true
	}

	var lenA, lenB int
	var dataA, dataB UOffsetT
	var ok bool

	if !absentA {
		lenA, dataA, ok = readVecMeta(bufA, tablePosA)
		if !ok {
			return
		}
	}
	if !absentB {
		lenB, dataB, ok = readVecMeta(bufB, tablePosB)
		if !ok {
			return
		}
	}

	elemType := ft.Element()
	minLen := lenA
	if lenB < minLen {
		minLen = lenB
	}

	// Compare element-by-element up to min length.
	for i := range minLen {
		elemPath := fmt.Sprintf("%s[%d]", path, i)
		switch elemType {
		case ReflectionBaseTypeBool:
			if int(dataA)+i >= len(bufA) || int(dataB)+i >= len(bufB) {
				continue
			}
			valA := GetBool(bufA[dataA+UOffsetT(i):])
			valB := GetBool(bufB[dataB+UOffsetT(i):])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeByte:
			if int(dataA)+i >= len(bufA) || int(dataB)+i >= len(bufB) {
				continue
			}
			valA := int64(GetInt8(bufA[dataA+UOffsetT(i):]))
			valB := int64(GetInt8(bufB[dataB+UOffsetT(i):]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeUByte:
			if int(dataA)+i >= len(bufA) || int(dataB)+i >= len(bufB) {
				continue
			}
			valA := int64(GetUint8(bufA[dataA+UOffsetT(i):]))
			valB := int64(GetUint8(bufB[dataB+UOffsetT(i):]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeShort:
			posA := int(dataA) + i*SizeInt16
			posB := int(dataB) + i*SizeInt16
			if posA+SizeInt16 > len(bufA) || posB+SizeInt16 > len(bufB) {
				continue
			}
			valA := int64(GetInt16(bufA[posA:]))
			valB := int64(GetInt16(bufB[posB:]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeUShort:
			posA := int(dataA) + i*SizeUint16
			posB := int(dataB) + i*SizeUint16
			if posA+SizeUint16 > len(bufA) || posB+SizeUint16 > len(bufB) {
				continue
			}
			valA := int64(GetUint16(bufA[posA:]))
			valB := int64(GetUint16(bufB[posB:]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeInt:
			posA := int(dataA) + i*SizeInt32
			posB := int(dataB) + i*SizeInt32
			if posA+SizeInt32 > len(bufA) || posB+SizeInt32 > len(bufB) {
				continue
			}
			valA := int64(GetInt32(bufA[posA:]))
			valB := int64(GetInt32(bufB[posB:]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeUInt:
			posA := int(dataA) + i*SizeUint32
			posB := int(dataB) + i*SizeUint32
			if posA+SizeUint32 > len(bufA) || posB+SizeUint32 > len(bufB) {
				continue
			}
			valA := int64(GetUint32(bufA[posA:]))
			valB := int64(GetUint32(bufB[posB:]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeLong:
			posA := int(dataA) + i*SizeInt64
			posB := int(dataB) + i*SizeInt64
			if posA+SizeInt64 > len(bufA) || posB+SizeInt64 > len(bufB) {
				continue
			}
			valA := GetInt64(bufA[posA:])
			valB := GetInt64(bufB[posB:])
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeULong:
			posA := int(dataA) + i*SizeUint64
			posB := int(dataB) + i*SizeUint64
			if posA+SizeUint64 > len(bufA) || posB+SizeUint64 > len(bufB) {
				continue
			}
			valA := int64(GetUint64(bufA[posA:]))
			valB := int64(GetUint64(bufB[posB:]))
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeFloat:
			posA := int(dataA) + i*SizeFloat32
			posB := int(dataB) + i*SizeFloat32
			if posA+SizeFloat32 > len(bufA) || posB+SizeFloat32 > len(bufB) {
				continue
			}
			fa := float64(GetFloat32(bufA[posA:]))
			fb := float64(GetFloat32(bufB[posB:]))
			if fa != fb && !(math.IsNaN(fa) && math.IsNaN(fb)) {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: fa, NewValue: fb})
			}
		case ReflectionBaseTypeDouble:
			posA := int(dataA) + i*SizeFloat64
			posB := int(dataB) + i*SizeFloat64
			if posA+SizeFloat64 > len(bufA) || posB+SizeFloat64 > len(bufB) {
				continue
			}
			fa := GetFloat64(bufA[posA:])
			fb := GetFloat64(bufB[posB:])
			if fa != fb && !(math.IsNaN(fa) && math.IsNaN(fb)) {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: fa, NewValue: fb})
			}
		case ReflectionBaseTypeString:
			// Each element is a UOffset to a string.
			posA := int(dataA) + i*SizeUOffsetT
			posB := int(dataB) + i*SizeUOffsetT
			if posA+SizeUOffsetT > len(bufA) || posB+SizeUOffsetT > len(bufB) {
				continue
			}
			tabA := Table{Bytes: bufA, Pos: UOffsetT(posA)}
			tabB := Table{Bytes: bufB, Pos: UOffsetT(posB)}
			strOffA := UOffsetT(posA)
			strOffB := UOffsetT(posB)
			valA := tabA.String(strOffA)
			valB := tabB.String(strOffB)
			if valA != valB {
				*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaChanged, OldValue: valA, NewValue: valB})
			}
		case ReflectionBaseTypeObj:
			// Vector of tables: each element is a 4-byte UOffset to a child
			// table; the UOffset is relative to the slot position itself.
			idx := ft.Index()
			if idx < 0 {
				continue
			}
			objects := schema.Objects()
			if int(idx) >= len(objects) {
				continue
			}
			nested := objects[idx]
			posA := int(dataA) + i*SizeUOffsetT
			posB := int(dataB) + i*SizeUOffsetT
			if posA+SizeUOffsetT > len(bufA) || posB+SizeUOffsetT > len(bufB) {
				continue
			}
			childPosA := posA + int(GetUOffsetT(bufA[posA:]))
			childPosB := posB + int(GetUOffsetT(bufB[posB:]))
			// Bounds-check: child table must lie within the buffer.
			if childPosA < 0 || childPosA >= len(bufA) || childPosB < 0 || childPosB >= len(bufB) {
				continue
			}
			diffTable(bufA, childPosA, bufB, childPosB, nested, schema, elemPath, deltas)
		}
	}

	// Extra elements in A are removed; extra elements in B are added.
	for i := minLen; i < lenA; i++ {
		elemPath := fmt.Sprintf("%s[%d]", path, i)
		*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaRemoved, OldValue: "<element>", NewValue: nil})
	}
	for i := minLen; i < lenB; i++ {
		elemPath := fmt.Sprintf("%s[%d]", path, i)
		*deltas = append(*deltas, FieldDelta{Path: elemPath, Kind: DeltaAdded, OldValue: nil, NewValue: "<element>"})
	}
}
