// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***
import { flatbuffers } from 'flatbuffers';
import *  as MonsterStorage_fbs from './monster_test_generated';

var grpc = require('grpc');

function serialize_Stat(buffer_args) {
  if (!(buffer_args instanceof MonsterStorage_fbs.Stat)) {
    throw new Error('Expected argument of type MonsterStorage_fbs.Stat');
  }
  return buffer_args.serialize();
}

function deserialize_Stat(buffer) {
  return MonsterStorage_fbs.Stat.getRootAsStat(new flatbuffers.ByteBuffer(buffer))
}


function serialize_Monster(buffer_args) {
  if (!(buffer_args instanceof MonsterStorage_fbs.Monster)) {
    throw new Error('Expected argument of type MonsterStorage_fbs.Monster');
  }
  return buffer_args.serialize();
}

function deserialize_Monster(buffer) {
  return MonsterStorage_fbs.Monster.getRootAsMonster(new flatbuffers.ByteBuffer(buffer))
}




var MonsterStorageService = exports.MonsterStorageService = {
  Store: {
    path: '/MyGame.Example.MonsterStorage/Store',
    requestStream: false,
    responseStream: false,
    requestType: flatbuffers.ByteBuffer,
    responseType: MonsterStorage_fbs.Stat,
    requestSerialize: serialize_Monster,
    requestDeserialize: deserialize_Monster,
    responseSerialize: serialize_Stat,
    responseDeserialize: deserialize_Stat,
  },
  Retrieve: {
    path: '/MyGame.Example.MonsterStorage/Retrieve',
    requestStream: false,
    responseStream: true,
    requestType: flatbuffers.ByteBuffer,
    responseType: MonsterStorage_fbs.Monster,
    requestSerialize: serialize_Stat,
    requestDeserialize: deserialize_Stat,
    responseSerialize: serialize_Monster,
    responseDeserialize: deserialize_Monster,
  },
  GetMaxHitPoint: {
    path: '/MyGame.Example.MonsterStorage/GetMaxHitPoint',
    requestStream: true,
    responseStream: false,
    requestType: flatbuffers.ByteBuffer,
    responseType: MonsterStorage_fbs.Stat,
    requestSerialize: serialize_Monster,
    requestDeserialize: deserialize_Monster,
    responseSerialize: serialize_Stat,
    responseDeserialize: deserialize_Stat,
  },
  GetMinMaxHitPoints: {
    path: '/MyGame.Example.MonsterStorage/GetMinMaxHitPoints',
    requestStream: true,
    responseStream: true,
    requestType: flatbuffers.ByteBuffer,
    responseType: MonsterStorage_fbs.Stat,
    requestSerialize: serialize_Monster,
    requestDeserialize: deserialize_Monster,
    responseSerialize: serialize_Stat,
    responseDeserialize: deserialize_Stat,
  },
};
exports.MonsterStorageClient = grpc.makeGenericClientConstructor(MonsterStorageService);
