import * as flatbuffers from 'flatbuffers';
import { AttackerT } from './Attacker.js';
import { BookReaderT } from './BookReader.js';
import { Character } from './Character.js';
import { RapunzelT } from './Rapunzel.js';
export declare class Movie implements flatbuffers.IUnpackableObject<MovieT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Movie;
    static getRootAsMovie(bb: flatbuffers.ByteBuffer, obj?: Movie): Movie;
    static getSizePrefixedRootAsMovie(bb: flatbuffers.ByteBuffer, obj?: Movie): Movie;
    static bufferHasIdentifier(bb: flatbuffers.ByteBuffer): boolean;
    main_character_type(): Character;
    main_character<T extends flatbuffers.Table>(obj: any | string): any | string | null;
    characters_type(index: number): Character | null;
    characters_type_Length(): number;
    characters_type_Array(): Uint8Array | null;
    characters(index: number, obj: any | string): any | string | null;
    characters_Length(): number;
    static getFullyQualifiedName(): string;
    static startMovie(builder: flatbuffers.Builder): void;
    static add_main_character_type(builder: flatbuffers.Builder, main_character_type: Character): void;
    static add_main_character(builder: flatbuffers.Builder, main_characterOffset: flatbuffers.Offset): void;
    static add_characters_type(builder: flatbuffers.Builder, characters_typeOffset: flatbuffers.Offset): void;
    static create_characters_type_Vector(builder: flatbuffers.Builder, data: Character[]): flatbuffers.Offset;
    static start_characters_type_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static add_characters(builder: flatbuffers.Builder, charactersOffset: flatbuffers.Offset): void;
    static create_characters_Vector(builder: flatbuffers.Builder, data: flatbuffers.Offset[]): flatbuffers.Offset;
    static start_characters_Vector(builder: flatbuffers.Builder, numElems: number): void;
    static endMovie(builder: flatbuffers.Builder): flatbuffers.Offset;
    static finishMovieBuffer(builder: flatbuffers.Builder, offset: flatbuffers.Offset): void;
    static finishSizePrefixedMovieBuffer(builder: flatbuffers.Builder, offset: flatbuffers.Offset): void;
    static createMovie(builder: flatbuffers.Builder, main_character_type: Character, main_characterOffset: flatbuffers.Offset, characters_typeOffset: flatbuffers.Offset, charactersOffset: flatbuffers.Offset): flatbuffers.Offset;
    unpack(): MovieT;
    unpackTo(_o: MovieT): void;
}
export declare class MovieT implements flatbuffers.IGeneratedObject {
    main_character_type: Character;
    main_character: AttackerT | BookReaderT | RapunzelT | string | null;
    characters_type: (Character)[];
    characters: (AttackerT | BookReaderT | RapunzelT | string)[];
    constructor(main_character_type?: Character, main_character?: AttackerT | BookReaderT | RapunzelT | string | null, characters_type?: (Character)[], characters?: (AttackerT | BookReaderT | RapunzelT | string)[]);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
