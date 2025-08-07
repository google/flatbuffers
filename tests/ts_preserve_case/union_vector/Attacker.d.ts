import * as flatbuffers from 'flatbuffers';
export declare class Attacker implements flatbuffers.IUnpackableObject<AttackerT> {
    bb: flatbuffers.ByteBuffer | null;
    bb_pos: number;
    __init(i: number, bb: flatbuffers.ByteBuffer): Attacker;
    static getRootAsAttacker(bb: flatbuffers.ByteBuffer, obj?: Attacker): Attacker;
    static getSizePrefixedRootAsAttacker(bb: flatbuffers.ByteBuffer, obj?: Attacker): Attacker;
    sword_attack_damage(): number;
    mutate_sword_attack_damage(value: number): boolean;
    static getFullyQualifiedName(): string;
    static startAttacker(builder: flatbuffers.Builder): void;
    static add_sword_attack_damage(builder: flatbuffers.Builder, sword_attack_damage: number): void;
    static endAttacker(builder: flatbuffers.Builder): flatbuffers.Offset;
    static createAttacker(builder: flatbuffers.Builder, sword_attack_damage: number): flatbuffers.Offset;
    unpack(): AttackerT;
    unpackTo(_o: AttackerT): void;
}
export declare class AttackerT implements flatbuffers.IGeneratedObject {
    sword_attack_damage: number;
    constructor(sword_attack_damage?: number);
    pack(builder: flatbuffers.Builder): flatbuffers.Offset;
}
