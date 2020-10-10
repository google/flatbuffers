// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***
import { flatbuffers } from 'flatbuffers';
import *  as MonsterStorage_fbs from './monster_test_generated';

import * as grpc from 'grpc';

interface IMonsterStorageService extends grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {
  Store: IMonsterStorageService_IStore;
  Retrieve: IMonsterStorageService_IRetrieve;
  GetMaxHitPoint: IMonsterStorageService_IGetMaxHitPoint;
  GetMinMaxHitPoints: IMonsterStorageService_IGetMinMaxHitPoints;
}
interface IMonsterStorageService_IStore extends grpc.MethodDefinition<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat> {
  path: string; // /MyGame.Example.MonsterStorage/Store
  requestStream: boolean; // false
  responseStream: boolean; // false
  requestSerialize: grpc.serialize<MonsterStorage_fbs.Monster>;
  requestDeserialize: grpc.deserialize<MonsterStorage_fbs.Monster>;
  responseSerialize: grpc.serialize<MonsterStorage_fbs.Stat>;
  responseDeserialize: grpc.deserialize<MonsterStorage_fbs.Stat>;
}

interface IMonsterStorageService_IRetrieve extends grpc.MethodDefinition<MonsterStorage_fbs.Stat, MonsterStorage_fbs.Monster> {
  path: string; // /MyGame.Example.MonsterStorage/Retrieve
  requestStream: boolean; // false
  responseStream: boolean; // true
  requestSerialize: grpc.serialize<MonsterStorage_fbs.Stat>;
  requestDeserialize: grpc.deserialize<MonsterStorage_fbs.Stat>;
  responseSerialize: grpc.serialize<MonsterStorage_fbs.Monster>;
  responseDeserialize: grpc.deserialize<MonsterStorage_fbs.Monster>;
}

interface IMonsterStorageService_IGetMaxHitPoint extends grpc.MethodDefinition<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat> {
  path: string; // /MyGame.Example.MonsterStorage/GetMaxHitPoint
  requestStream: boolean; // true
  responseStream: boolean; // false
  requestSerialize: grpc.serialize<MonsterStorage_fbs.Monster>;
  requestDeserialize: grpc.deserialize<MonsterStorage_fbs.Monster>;
  responseSerialize: grpc.serialize<MonsterStorage_fbs.Stat>;
  responseDeserialize: grpc.deserialize<MonsterStorage_fbs.Stat>;
}

interface IMonsterStorageService_IGetMinMaxHitPoints extends grpc.MethodDefinition<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat> {
  path: string; // /MyGame.Example.MonsterStorage/GetMinMaxHitPoints
  requestStream: boolean; // true
  responseStream: boolean; // true
  requestSerialize: grpc.serialize<MonsterStorage_fbs.Monster>;
  requestDeserialize: grpc.deserialize<MonsterStorage_fbs.Monster>;
  responseSerialize: grpc.serialize<MonsterStorage_fbs.Stat>;
  responseDeserialize: grpc.deserialize<MonsterStorage_fbs.Stat>;
}


export const MonsterStorageService: IMonsterStorageService;

export interface IMonsterStorageServer {
  Store: grpc.handleUnaryCall<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
  Retrieve: grpc.handleServerStreamingCall<MonsterStorage_fbs.Stat, MonsterStorage_fbs.Monster>;
  GetMaxHitPoint: grpc.handleClientStreamingCall<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
  GetMinMaxHitPoints: grpc.handleBidiStreamingCall<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
}

export interface IMonsterStorageClient {
  Store(request: MonsterStorage_fbs.Monster, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Stat) => void): grpc.ClientUnaryCall;
  Store(request: MonsterStorage_fbs.Monster, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Stat) => void): grpc.ClientUnaryCall;
  Store(request: MonsterStorage_fbs.Monster, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Stat) => void): grpc.ClientUnaryCall;
  Retrieve(request: MonsterStorage_fbs.Stat, metadata: grpc.Metadata): grpc.ClientReadableStream<MonsterStorage_fbs.Monster>;
  Retrieve(request: MonsterStorage_fbs.Stat, options: Partial<grpc.CallOptions>): grpc.ClientReadableStream<MonsterStorage_fbs.Monster>;
  GetMaxHitPoint(callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  GetMaxHitPoint(metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  GetMaxHitPoint(options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  GetMaxHitPoint(metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  GetMinMaxHitPoints(): grpc.ClientDuplexStream<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
  GetMinMaxHitPoints(options: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
  GetMinMaxHitPoints(metadata: grpc.Metadata, options?: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
}

export class MonsterStorageClient extends grpc.Client implements IMonsterStorageClient {
  constructor(address: string, credentials: grpc.ChannelCredentials, options?: object);  public Store(request: MonsterStorage_fbs.Monster, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Stat) => void): grpc.ClientUnaryCall;
  public Store(request: MonsterStorage_fbs.Monster, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Stat) => void): grpc.ClientUnaryCall;
  public Store(request: MonsterStorage_fbs.Monster, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Stat) => void): grpc.ClientUnaryCall;
  public Retrieve(request: MonsterStorage_fbs.Stat, metadata: grpc.Metadata): grpc.ClientReadableStream<MonsterStorage_fbs.Monster>;
  public Retrieve(request: MonsterStorage_fbs.Stat, options: Partial<grpc.CallOptions>): grpc.ClientReadableStream<MonsterStorage_fbs.Monster>;
  public GetMaxHitPoint(callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  public GetMaxHitPoint(metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  public GetMaxHitPoint(options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  public GetMaxHitPoint(metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: MonsterStorage_fbs.Monster) => void): grpc.ClientWritableStream<MonsterStorage_fbs.Stat>;
  public GetMinMaxHitPoints(): grpc.ClientDuplexStream<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
  public GetMinMaxHitPoints(options: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
  public GetMinMaxHitPoints(metadata: grpc.Metadata, options?: Partial<grpc.CallOptions>): grpc.ClientDuplexStream<MonsterStorage_fbs.Monster, MonsterStorage_fbs.Stat>;
}

