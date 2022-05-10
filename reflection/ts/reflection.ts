// This library provides a few basic reflection utilities for Flatbuffers.
// Currently, this only really supports the level of reflection that would
// be necessary to convert a serialized flatbuffer to JSON using just a
// reflection.Schema flatbuffer describing the type.
// Note that this currently assumes that reflection.fbs is codegen'd to
// typescript using --ts-flat-file and made available at
// 'flatbuffers_refleciton/reflection_generated'. If you are using the
// provided bazel rules, this is managed for you.
// See tests/reflection_test.ts for sample usage.

import {ByteBuffer} from 'flatbuffers';
import * as reflection from 'flatbuffers_reflection/reflection_generated';

// Returns the size, in bytes, of the given type. For vectors/strings/etc.
// returns the size of the offset.
function typeSize(baseType: reflection.BaseType): number {
  switch (baseType) {
    case reflection.BaseType.None:
    case reflection.BaseType.UType:
    case reflection.BaseType.Bool:
    case reflection.BaseType.Byte:
    case reflection.BaseType.UByte:
      return 1;
    case reflection.BaseType.Short:
    case reflection.BaseType.UShort:
      return 2;
    case reflection.BaseType.Int:
    case reflection.BaseType.UInt:
      return 4;
    case reflection.BaseType.Long:
    case reflection.BaseType.ULong:
      return 8;
    case reflection.BaseType.Float:
      return 4;
    case reflection.BaseType.Double:
      return 8;
    case reflection.BaseType.String:
    case reflection.BaseType.Vector:
    case reflection.BaseType.Obj:
    case reflection.BaseType.Union:
    case reflection.BaseType.Array:
      return 4;
  }
  return NaN;
}

// Returns whether the given type is a scalar type.
function isScalar(baseType: reflection.BaseType): boolean {
  switch (baseType) {
    case reflection.BaseType.UType:
    case reflection.BaseType.Bool:
    case reflection.BaseType.Byte:
    case reflection.BaseType.UByte:
    case reflection.BaseType.Short:
    case reflection.BaseType.UShort:
    case reflection.BaseType.Int:
    case reflection.BaseType.UInt:
    case reflection.BaseType.Long:
    case reflection.BaseType.ULong:
    case reflection.BaseType.Float:
    case reflection.BaseType.Double:
      return true;
    case reflection.BaseType.None:
    case reflection.BaseType.String:
    case reflection.BaseType.Vector:
    case reflection.BaseType.Obj:
    case reflection.BaseType.Union:
    case reflection.BaseType.Array:
      return false;
  }
  return false;
}

// Returns whether the given type is integer or not.
function isInteger(baseType: reflection.BaseType): boolean {
  switch (baseType) {
    case reflection.BaseType.UType:
    case reflection.BaseType.Bool:
    case reflection.BaseType.Byte:
    case reflection.BaseType.UByte:
    case reflection.BaseType.Short:
    case reflection.BaseType.UShort:
    case reflection.BaseType.Int:
    case reflection.BaseType.UInt:
    case reflection.BaseType.Long:
    case reflection.BaseType.ULong:
      return true;
    case reflection.BaseType.Float:
    case reflection.BaseType.Double:
    case reflection.BaseType.None:
    case reflection.BaseType.String:
    case reflection.BaseType.Vector:
    case reflection.BaseType.Obj:
    case reflection.BaseType.Union:
    case reflection.BaseType.Array:
      return false;
  }
  return false;
}

// Returns whether the given type is a long--this is needed to know whether it
// can be represented by the normal javascript number (8-byte integers require a
// special type, since the native number type is an 8-byte double, which won't
// represent 8-byte integers to full precision).
function isLong(baseType: reflection.BaseType): boolean {
  return isInteger(baseType) && (typeSize(baseType) > 4);
}

// Stores the data associated with a Table within a given buffer.
export class Table {
  // Wrapper to represent an object (Table or Struct) within a ByteBuffer.
  // The ByteBuffer is the raw data associated with the object.
  // typeIndex is an index into the schema object vector for the parser
  // object that this is associated with.
  // offset is the absolute location within the buffer where the root of the
  // object is.
  // Note that a given Table assumes that it is being used with a particular
  // Schema object.
  // External users should generally not be using this constructor directly.
  constructor(
      public readonly bb: ByteBuffer, public readonly typeIndex: number,
      public readonly offset: number) {}
  // Constructs a Table object for the root of a ByteBuffer--this assumes that
  // the type of the Table is the root table of the Parser that you are using.
  static getRootTable(bb: ByteBuffer): Table {
    return new Table(bb, -1, bb.readInt32(bb.position()) + bb.position());
  }
  // Constructs a table from a type name instead of from a type index.
  static getNamedTable(
      bb: ByteBuffer, schema: reflection.Schema, type: string,
      offset?: number): Table {
    for (let ii = 0; ii < schema.objectsLength(); ++ii) {
      const schemaObject = schema.objects(ii);
      if (schemaObject !== null && schemaObject.name() === type) {
        return new Table(
            bb, ii,
            (offset === undefined) ?
                bb.readInt32(bb.position()) + bb.position() :
                offset);
      }
    }
    throw new Error('Unable to find type ' + type + ' in schema.');
  }
  // Reads a scalar of a given type at a given offset.
  readScalar(fieldType: reflection.BaseType, offset: number) {
    switch (fieldType) {
      case reflection.BaseType.UType:
      case reflection.BaseType.Bool:
        return this.bb.readUint8(offset);
      case reflection.BaseType.Byte:
        return this.bb.readInt8(offset);
      case reflection.BaseType.UByte:
        return this.bb.readUint8(offset);
      case reflection.BaseType.Short:
        return this.bb.readInt16(offset);
      case reflection.BaseType.UShort:
        return this.bb.readUint16(offset);
      case reflection.BaseType.Int:
        return this.bb.readInt32(offset);
      case reflection.BaseType.UInt:
        return this.bb.readUint32(offset);
      case reflection.BaseType.Long:
        return this.bb.readInt64(offset);
      case reflection.BaseType.ULong:
        return this.bb.readUint64(offset);
      case reflection.BaseType.Float:
        return this.bb.readFloat32(offset);
      case reflection.BaseType.Double:
        return this.bb.readFloat64(offset);
    }
    throw new Error('Unsupported message type ' + fieldType);
  }
};

// The Parser class uses a Schema to provide all the utilities required to
// parse flatbuffers that have a type that is the same as the root_type defined
// by the Schema.
// The classical usage would be to, e.g., be reading a channel with a type of
// "foo.Bar". At startup, you would construct a Parser from the channel's
// Schema. When a message is received on the channel , you would then use
// Table.getRootTable() on the received buffer to construct the Table, and
// then access the members using the various methods of the Parser (or just
// convert the entire object to a javascript Object/JSON using toObject()).
// There are three basic ways to access fields in a Table:
// 1) Call toObject(), which turns the entire table into a javascript object.
//    This is not meant to be particularly fast, but is useful to, e.g.,
//    convert something to JSON, or as a debugging tool.
// 2) Use the read*Lambda() accessors: These return a function that lets you
//    access the specified field given a table. This is used by the plotter
//    to repeatedly access the same field on a bunch of tables of the same type,
//    without having to redo all the reflection-related work on every access.
// 3) Use the read*() accessors: These just call the lambda returned by
//    read*Lambda() for you, as a convenience. This is cleaner to use, but for
//    repeated lookups on tables of the same type, this may be inefficient.
export class Parser {
  constructor(private readonly schema: reflection.Schema) {}

  // Parse a Table to a javascript object. This is can be used, e.g., to convert
  // a flatbuffer Table to JSON.
  // If readDefaults is set to true, then scalar fields will be filled out with
  // their default values if not populated; if readDefaults is false and the
  // field is not populated, the resulting object will not populate the field.
  toObject(table: Table, readDefaults: boolean = false): Record<string, any> {
    const result: Record<string, any> = {};
    const schema = this.getType(table.typeIndex);
    const numFields = schema.fieldsLength();
    for (let ii = 0; ii < numFields; ++ii) {
      const field = schema.fields(ii);
      if (field === null) {
        throw new Error(
            'Malformed schema: field at index ' + ii + ' not populated.');
      }
      const fieldType = field.type();
      if (fieldType === null) {
        throw new Error(
            'Malformed schema: "type" field of Field not populated.');
      }
      const fieldName = field.name();
      if (fieldName === null) {
        throw new Error(
            'Malformed schema: "name" field of Field not populated.');
      }
      const baseType = fieldType.baseType();
      let fieldValue = null;
      if (isScalar(baseType)) {
        fieldValue = this.readScalar(table, fieldName, readDefaults);
      } else if (baseType === reflection.BaseType.String) {
        fieldValue = this.readString(table, fieldName);
      } else if (baseType === reflection.BaseType.Obj) {
        const subTable = this.readTable(table, fieldName);
        if (subTable !== null) {
          fieldValue = this.toObject(subTable, readDefaults);
        }
      } else if (baseType === reflection.BaseType.Vector) {
        const elementType = fieldType.element();
        if (isScalar(elementType)) {
          fieldValue = this.readVectorOfScalars(table, fieldName);
        } else if (elementType === reflection.BaseType.String) {
          fieldValue = this.readVectorOfStrings(table, fieldName);
        } else if (elementType === reflection.BaseType.Obj) {
          const tables = this.readVectorOfTables(table, fieldName);
          if (tables !== null) {
            fieldValue = [];
            for (const table of tables) {
              fieldValue.push(this.toObject(table, readDefaults));
            }
          }
        } else {
          throw new Error('Vectors of Unions and Arrays are not supported.');
        }
      } else {
        throw new Error(
            'Unions and Arrays are not supported in field ' + field.name());
      }
      if (fieldValue !== null) {
        result[fieldName] = fieldValue;
      }
    }
    return result;
  }

  // Returns the Object definition associated with the given type index.
  getType(typeIndex: number): reflection.Object_ {
    if (typeIndex === -1) {
      const rootTable = this.schema.rootTable();
      if (rootTable === null) {
        throw new Error('Malformed schema: No root table.');
      }
      return rootTable;
    }
    if (typeIndex < 0 || typeIndex > this.schema.objectsLength()) {
      throw new Error('Type index out-of-range.');
    }
    const table = this.schema.objects(typeIndex);
    if (table === null) {
      throw new Error(
          'Malformed schema: No object at index ' + typeIndex + '.');
    }
    return table;
  }

  // Retrieves the Field schema for the given field name within a given
  // type index.
  getField(fieldName: string, typeIndex: number): reflection.Field {
    const schema: reflection.Object_ = this.getType(typeIndex);
    const numFields = schema.fieldsLength();
    for (let ii = 0; ii < numFields; ++ii) {
      const field = schema.fields(ii);
      if (field === null) {
        throw new Error(
            'Malformed schema: Missing Field table at index ' + ii + '.');
      }
      const name = field.name();
      if (fieldName === name) {
        return field;
      }
    }
    throw new Error(
        'Couldn\'t find field ' + fieldName + ' in object ' + schema.name() +
        '.');
  }

  // Reads a scalar with the given field name from a Table. If readDefaults
  // is set to false and the field is unset, we will return null. If
  // readDefaults is true and the field is unset, we will look-up the default
  // value for the field and return that.
  // For 64-bit fields, returns a flatbuffer Long rather than a standard number.
  readScalar(table: Table, fieldName: string, readDefaults: boolean = false):
      number|BigInt|null {
    return this.readScalarLambda(
        table.typeIndex, fieldName, readDefaults)(table);
  }
  // Like readScalar(), except that this returns an accessor for the specified
  // field, rather than the value of the field itself.
  // Note that the *Lambda() methods take a typeIndex instead of a Table, which
  // can be obtained using table.typeIndex.
  readScalarLambda(
      typeIndex: number, fieldName: string,
      readDefaults: boolean = false): (t: Table) => number | BigInt | null {
    const field = this.getField(fieldName, typeIndex);
    const fieldType = field.type();
    if (fieldType === null) {
      throw new Error('Malformed schema: "type" field of Field not populated.');
    }
    const isStruct = this.getType(typeIndex).isStruct();
    if (!isScalar(fieldType.baseType())) {
      throw new Error('Field ' + fieldName + ' is not a scalar type.');
    }

    if (isStruct) {
      const baseType = fieldType.baseType();
      return (t: Table) => {
        return t.readScalar(baseType, t.offset + field.offset());
      };
    }

    return (t: Table) => {
      const offset = t.offset + t.bb.__offset(t.offset, field.offset());
      if (offset === t.offset) {
        if (!readDefaults) {
          return null;
        }
        if (isInteger(fieldType.baseType())) {
          return field.defaultInteger();
        } else {
          return field.defaultReal();
        }
      }
      return t.readScalar(fieldType.baseType(), offset);
    };
  }
  // Reads a string with the given field name from the provided Table.
  // If the field is unset, returns null.
  readString(table: Table, fieldName: string): string|null {
    return this.readStringLambda(table.typeIndex, fieldName)(table);
  }

  readStringLambda(typeIndex: number, fieldName: string):
      (t: Table) => string | null {
    const field = this.getField(fieldName, typeIndex);
    const fieldType = field.type();
    if (fieldType === null) {
      throw new Error('Malformed schema: "type" field of Field not populated.');
    }
    if (fieldType.baseType() !== reflection.BaseType.String) {
      throw new Error('Field ' + fieldName + ' is not a string.');
    }


    return (t: Table) => {
      const offsetToOffset = t.offset + t.bb.__offset(t.offset, field.offset());
      if (offsetToOffset === t.offset) {
        return null;
      }
      return t.bb.__string(offsetToOffset) as string;
    };
  }
  // Reads a sub-message from the given Table. The sub-message may either be
  // a struct or a Table. Returns null if the sub-message is not set.
  readTable(table: Table, fieldName: string): Table|null {
    return this.readTableLambda(table.typeIndex, fieldName)(table);
  }
  readTableLambda(typeIndex: number, fieldName: string):
      (t: Table) => Table | null {
    const field = this.getField(fieldName, typeIndex);
    const fieldType = field.type();
    if (fieldType === null) {
      throw new Error('Malformed schema: "type" field of Field not populated.');
    }
    const parentIsStruct = this.getType(typeIndex).isStruct();
    if (fieldType.baseType() !== reflection.BaseType.Obj) {
      throw new Error('Field ' + fieldName + ' is not an object type.');
    }

    if (parentIsStruct) {
      return (t: Table) => {
        return new Table(t.bb, fieldType.index(), t.offset + field.offset());
      };
    }

    const elementIsStruct = this.getType(fieldType.index()).isStruct();

    return (table: Table) => {
      const offsetToOffset =
          table.offset + table.bb.__offset(table.offset, field.offset());
      if (offsetToOffset === table.offset) {
        return null;
      }

      const objectStart = elementIsStruct ? offsetToOffset :
                                            table.bb.__indirect(offsetToOffset);
      return new Table(table.bb, fieldType.index(), objectStart);
    };
  }
  // Reads a vector of scalars (like readScalar, may return a vector of BigInt's
  // instead). Also, will return null if the vector is not set.
  // TODO(jkuszmaul): Allow this to return a slice into the underlying
  // ByteBuffer to avoid excessive memory allocation (at least for vectors of
  // bytes, which is a common way to transmit images).
  readVectorOfScalars(table: Table, fieldName: string): (number|BigInt)[]|null {
    return this.readVectorOfScalarsLambda(table.typeIndex, fieldName)(table);
  }

  readVectorOfScalarsLambda(typeIndex: number, fieldName: string):
      (t: Table) => (number | BigInt)[] | null {
    const field = this.getField(fieldName, typeIndex);
    const fieldType = field.type();
    if (fieldType === null) {
      throw new Error('Malformed schema: "type" field of Field not populated.');
    }
    if (fieldType.baseType() !== reflection.BaseType.Vector) {
      throw new Error('Field ' + fieldName + ' is not an vector.');
    }
    if (!isScalar(fieldType.element())) {
      throw new Error('Field ' + fieldName + ' is not an vector of scalars.');
    }

    return (table: Table) => {
      const offsetToOffset =
          table.offset + table.bb.__offset(table.offset, field.offset());
      if (offsetToOffset === table.offset) {
        return null;
      }
      const numElements = table.bb.__vector_len(offsetToOffset);
      const result = [];
      const baseOffset = table.bb.__vector(offsetToOffset);
      const scalarSize = typeSize(fieldType.element());
      for (let ii = 0; ii < numElements; ++ii) {
        result.push(table.readScalar(
            fieldType.element(), baseOffset + scalarSize * ii));
      }
      return result;
    };
  }
  // Reads a vector of tables. Returns null if vector is not set.
  readVectorOfTables(table: Table, fieldName: string): Table[]|null {
    return this.readVectorOfTablesLambda(table.typeIndex, fieldName)(table);
  }
  readVectorOfTablesLambda(typeIndex: number, fieldName: string):
      (t: Table) => Table[] | null {
    const field = this.getField(fieldName, typeIndex);
    const fieldType = field.type();
    if (fieldType === null) {
      throw new Error('Malformed schema: "type" field of Field not populated.');
    }
    if (fieldType.baseType() !== reflection.BaseType.Vector) {
      throw new Error('Field ' + fieldName + ' is not an vector.');
    }
    if (fieldType.element() !== reflection.BaseType.Obj) {
      throw new Error('Field ' + fieldName + ' is not an vector of objects.');
    }

    const elementSchema = this.getType(fieldType.index());
    const elementIsStruct = elementSchema.isStruct();
    const elementSize = elementIsStruct ? elementSchema.bytesize() :
                                          typeSize(fieldType.element());

    return (table: Table) => {
      const offsetToOffset =
          table.offset + table.bb.__offset(table.offset, field.offset());
      if (offsetToOffset === table.offset) {
        return null;
      }
      const numElements = table.bb.__vector_len(offsetToOffset);
      const result = [];
      const baseOffset = table.bb.__vector(offsetToOffset);
      for (let ii = 0; ii < numElements; ++ii) {
        const elementOffset = baseOffset + elementSize * ii;
        result.push(new Table(
            table.bb, fieldType.index(),
            elementIsStruct ? elementOffset :
                              table.bb.__indirect(elementOffset)));
      }
      return result;
    };
  }
  // Reads a vector of strings. Returns null if not set.
  readVectorOfStrings(table: Table, fieldName: string): string[]|null {
    return this.readVectorOfStringsLambda(table.typeIndex, fieldName)(table);
  }
  readVectorOfStringsLambda(typeIndex: number, fieldName: string):
      (t: Table) => string[] | null {
    const field = this.getField(fieldName, typeIndex);
    const fieldType = field.type();
    if (fieldType === null) {
      throw new Error('Malformed schema: "type" field of Field not populated.');
    }
    if (fieldType.baseType() !== reflection.BaseType.Vector) {
      throw new Error('Field ' + fieldName + ' is not an vector.');
    }
    if (fieldType.element() !== reflection.BaseType.String) {
      throw new Error('Field ' + fieldName + ' is not an vector of strings.');
    }

    return (table: Table) => {
      const offsetToOffset =
          table.offset + table.bb.__offset(table.offset, field.offset());
      if (offsetToOffset === table.offset) {
        return null;
      }
      const numElements = table.bb.__vector_len(offsetToOffset);
      const result = [];
      const baseOffset = table.bb.__vector(offsetToOffset);
      const offsetSize = typeSize(fieldType.element());
      for (let ii = 0; ii < numElements; ++ii) {
        result.push(table.bb.__string(baseOffset + offsetSize * ii) as string);
      }
      return result;
    };
  }
}
