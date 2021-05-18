// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***
import * as flatbuffers from 'flatbuffers';
import { Stat as MyGame_Example_Stat } from './my-game/example/stat';
import { Monster as MyGame_Example_Monster } from './my-game/example/monster';

import * as grpc from '@grpc/grpc-js';

interface IMonsterStorageService extends grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {
  Store: IMonsterStorageService_IStore;
  Retrieve: IMonsterStorageService_IRetrieve;
  GetMaxHitPoint: IMonsterStorageService_IGetMaxHitPoint;
  GetMinMaxHitPoints: IMonsterStorageService_IGetMinMaxHitPoints;
}
interface IMonsterStorageService_IStore extends grpc.MethodDefinition<MyGame_Example_Monster, MyGame_Example_Stat> {
  path: string; // /MyGame.Example.MonsterStorage/Store
  requestStream: boolean; // false
  responseStream: boolean; // false
  requestSerialize: grpc.serialize<MyGame_Example_Monster>;
  requestDeserialize: grpc.deserialize<MyGame_Example_Monster>;
  responseSerialize: grpc.serialize<MyGame_Example_Stat>;
  responseDeserialize: grpc.deserialize<MyGame_Example_Stat>;
}

interface IMonsterStorageService_IRetrieve extends grpc.MethodDefinition<MyGame_Example_Stat, MyGame_Example_Monster> {
  path: string; // /MyGame.Example.MonsterStorage/Retrieve
  requestStream: boolean; // false
  responseStream: boolean; // true
  requestSerialize: grpc.serialize<MyGame_Example_Stat>;
  requestDeserialize: grpc.deserialize<MyGame_Example_Stat>;
  responseSerialize: grpc.serialize<MyGame_Example_Monster>;
  responseDeserialize: grpc.deserialize<MyGame_Example_Monster>;
}

interface IMonsterStorageService_IGetMaxHitPoint extends grpc.MethodDefinition<MyGame_Example_Monster, MyGame_Example_Stat> {
  path: string; // /MyGame.Example.MonsterStorage/GetMaxHitPoint
  requestStream: boolean; // true
  responseStream: boolean; // false
  requestSerialize: grpc.serialize<MyGame_Example_Monster>;
  requestDeserialize: grpc.deserialize<MyGame_Example_Monster>;
  responseSerialize: grpc.serialize<MyGame_Example_Stat>;
  responseDeserialize: grpc.deserialize<MyGame_Example_Stat>;
}

interface IMonsterStorageService_IGetMinMaxHitPoints extends grpc.MethodDefinition<MyGame_Example_Monster, MyGame_Example_Stat> {
  path: string; // /MyGame.Example.MonsterStorage/GetMinMaxHitPoints
  requestStream: boolean; // true
  responseStream: boolean; // true
  requestSerialize: grpc.serialize<MyGame_Example_Monster>;
  requestDeserialize: grpc.deserialize<MyGame_Example_Monster>;
  responseSerialize: grpc.serialize<MyGame_Example_Stat>;
  responseDeserialize: grpc.deserialize<MyGame_Example_Stat>;
}


export const MonsterStorageService: IMonsterStorageService;

export interface IMonsterStorageServer extends grpc.UntypedServiceImplementation {
  Store: grpc.handleUnaryCall<MyGame_Example_Monster, MyGame_Example_Stat>;
  Retrieve: grpc.handleServerStreamingCall<MyGame_Example_Stat, MyGame_Example_Monster>;
  GetMaxHitPoint: grpc.handleClientStreamingCall<MyGame_Example_Monster, MyGame_Example_Stat>;
  GetMinMaxHitPoints: grpc.handleBidiStreamingCall<MyGame_Example_Monster, MyGame_Example_Stat>;
}

export interface IMonsterStorageClient {
  Store(request: MyGame_Example_Monster, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Stat) => void): grpc.ClientUnaryCall;
  Store(request: MyGame_Example_Monster, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Stat) => void): grpc.ClientUnaryCall;
  Store(request: MyGame_Example_Monster, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Stat) => void): grpc.ClientUnaryCall;
  Retrieve(request: MyGame_Example_Stat, metadata: grpc.Metadata): grpc.ClientReadableStream<MyGame_Example_Monster>;
  Retrieve(request: MyGame_Example_Stat, options: Partial<grpc.CallOptions>): grpc.ClientReadableStream<MyGame_Example_Monster>;
  GetMaxHitPoint(callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  GetMaxHitPoint(metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  GetMaxHitPoint(options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  GetMaxHitPoint(metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  GetMinMaxHitPoints(): grpc.ClientDuplexStream<MyGame_Example_Monster, MyGame_Example_Stat>;
  GetMinMaxHitPoints(options: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MyGame_Example_Monster, MyGame_Example_Stat>;
  GetMinMaxHitPoints(metadata: grpc.Metadata, options?: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MyGame_Example_Monster, MyGame_Example_Stat>;
}

export class MonsterStorageClient extends grpc.Client implements IMonsterStorageClient {
  constructor(address: string, credentials: grpc.ChannelCredentials, options?: object);
  public Store(request: MyGame_Example_Monster, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Stat) => void): grpc.ClientUnaryCall;
  public Store(request: MyGame_Example_Monster, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Stat) => void): grpc.ClientUnaryCall;
  public Store(request: MyGame_Example_Monster, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Stat) => void): grpc.ClientUnaryCall;
  public Retrieve(request: MyGame_Example_Stat, metadata: grpc.Metadata): grpc.ClientReadableStream<MyGame_Example_Monster>;
  public Retrieve(request: MyGame_Example_Stat, options: Partial<grpc.CallOptions>): grpc.ClientReadableStream<MyGame_Example_Monster>;
  public GetMaxHitPoint(callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  public GetMaxHitPoint(metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  public GetMaxHitPoint(options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  public GetMaxHitPoint(metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MyGame_Example_Monster) => void): grpc.ClientWritableStream<MyGame_Example_Stat>;
  public GetMinMaxHitPoints(): grpc.ClientDuplexStream<MyGame_Example_Monster, MyGame_Example_Stat>;
  public GetMinMaxHitPoints(options: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MyGame_Example_Monster, MyGame_Example_Stat>;
  public GetMinMaxHitPoints(metadata: grpc.Metadata, options?: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MyGame_Example_Monster, MyGame_Example_Stat>;
}

