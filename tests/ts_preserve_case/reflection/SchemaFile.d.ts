import * as flatbuffers from 'flatbuffers';
/**
 * File specific information.
 * Symbols declared within a file may be recovered by iterating over all
 * symbols and examining the `declaration_file` field.
 */
export declare class SchemaFile implements flatbuffers.IUnpackableObject<SchemaFileT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): SchemaFile;
    static getRootAsSchemaFile(bb: flatbuffers.ByteBuffer, obj?: SchemaFile): SchemaFile;
    static getSizePrefixedRootAsSchemaFile(bb: flatbuffers.ByteBuffer, obj?: SchemaFile): SchemaFile;
    /**
     * Filename, relative to project root.
     */
    filename(): string | null;
    filename(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    /**
     * Names of included files, relative to project root.
     */
    included_filenames(index: number): string;
    included_filenames(index: number, optionalEncoding: flatbuffers.Encoding): string | Uint8Array;
    included_filenames_Length(): number;
    static getFullyQualifiedName(): string;
    static startSchemaFile(builder: flatbuffers.Builder): void;
    static add_filename(builder: flatbuffers.Builder, filenameOffset: flatbuffers.Offset): void;
    static add_included_filenames(builder: flatbuffers.Builder, included_filenamesOffset: flatbuffers.Offset): void;
    static create_included_filenames_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_included_filenames_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static endSchemaFile(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createSchemaFile(builder: flatbuffers.Builder, filenameOffset: flatbuffers.Offset, included_filenamesOffset: flatbuffers.Offset): flatbuffers.Offset;
    unpack(): SchemaFileT;
    unpackTo(_o: SchemaFileT): void;
}
export declare class SchemaFileT implements flatbuffers.IGeneratedObject {
    filename: string | Uint8Array | null;
    included_filenames: (string)[];
    constructor(filename?: string | Uint8Array | null, included_filenames?: (string)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
