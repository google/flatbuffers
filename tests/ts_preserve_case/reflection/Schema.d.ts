import * as flatbuffers from 'flatbuffers';
import { Enum, EnumT } from '../reflection/Enum.js';
import { Object_, Object_T } from '../reflection/Object.js';
import { SchemaFile, SchemaFileT } from '../reflection/SchemaFile.js';
import { Service, ServiceT } from '../reflection/Service.js';
export declare class Schema implements flatbuffers.IUnpackableObject<SchemaT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Schema;
    static getRootAsSchema(bb: flatbuffers.ByteBuffer, obj?: Schema): Schema;
    static getSizePrefixedRootAsSchema(bb: flatbuffers.ByteBuffer, obj?: Schema): Schema;
    static bufferHasIdentifier(bb: flatbuffers.ByteBuffer): boolean;
    objects(index: number, obj?: Object_): Object_ | null;
    objects_Length(): number;
    enums(index: number, obj?: Enum): Enum | null;
    enums_Length(): number;
    file_ident(): string | null;
    file_ident(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    file_ext(): string | null;
    file_ext(optionalEncoding: flatbuffers.Encoding): string | Uint8Array | null;
    root_table(obj?: Object_): Object_ | null;
    services(index: number, obj?: Service): Service | null;
    services_Length(): number;
    advanced_features(): bigint;
    mutate_advanced_features(value: bigint): boolean;
    /**
     * All the files used in this compilation. Files are relative to where
     * flatc was invoked.
     */
    fbs_files(index: number, obj?: SchemaFile): SchemaFile | null;
    fbs_files_Length(): number;
    static getFullyQualifiedName(): string;
    static startSchema(builder: flatbuffers.Builder): void;
    static add_objects(builder: flatbuffers.Builder, objectsOffset: flatbuffers.Offset): void;
    static create_objects_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_objects_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_enums(builder: flatbuffers.Builder, enumsOffset: flatbuffers.Offset): void;
    static create_enums_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_enums_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_file_ident(builder: flatbuffers.Builder, file_identOffset: flatbuffers.Offset): void;
    static add_file_ext(builder: flatbuffers.Builder, file_extOffset: flatbuffers.Offset): void;
    static add_root_table(builder: flatbuffers.Builder, root_tableOffset: flatbuffers.Offset): void;
    static add_services(builder: flatbuffers.Builder, servicesOffset: flatbuffers.Offset): void;
    static create_services_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_services_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_advanced_features(builder: flatbuffers.Builder, advanced_features: bigint): void;
    static add_fbs_files(builder: flatbuffers.Builder, fbs_filesOffset: flatbuffers.Offset): void;
    static create_fbs_files_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_fbs_files_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static endSchema(builder: flatbuffers.Builder): flatbuffers.Offset;
    static finishSchemaBuffer(builder: flatbuffers.Builder, offset: flatbuffers.Offset): void;
    static finishSizePrefixedSchemaBuffer(builder: flatbuffers.Builder, offset: flatbuffers.Offset): void;
    unpack(): SchemaT;
    unpackTo(_o: SchemaT): void;
}
export declare class SchemaT implements flatbuffers.IGeneratedObject {
    objects: (Object_T)[];
    enums: (EnumT)[];
    file_ident: string | Uint8Array | null;
    file_ext: string | Uint8Array | null;
    root_table: Object_T | null;
    services: (ServiceT)[];
    advanced_features: bigint;
    fbs_files: (SchemaFileT)[];
    constructor(objects?: (Object_T)[], enums?: (EnumT)[], file_ident?: string | Uint8Array | null, file_ext?: string | Uint8Array | null, root_table?: Object_T | null, services?: (ServiceT)[], advanced_features?: bigint, fbs_files?: (SchemaFileT)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
