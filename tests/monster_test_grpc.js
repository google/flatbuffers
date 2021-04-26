// Generated GRPC code for FlatBuffers TS *** DO NOT EDIT ***
import * as flatbuffers from 'flatbuffers';
import { Stat as MyGame_Example_Stat } from './my-game/example/stat';
import { Monster as MyGame_Example_Monster } from './my-game/example/monster';

var grpc = require('grpc');

function serialize_MyGame_Example_Stat(buffer_args) {
  if (!(buffer_args instanceof MyGame_Example_Stat)) {
    throw new Error('Expected argument of type Stat');
  }
  return buffer_args.serialize();
}

function deserialize_MyGame_Example_Stat(buffer) {
  return MyGame_Example_Stat.getRootAsStat(new flatbuffers.ByteBuffer(buffer))
}


function serialize_MyGame_Example_Monster(buffer_args) {
  if (!(buffer_args instanceof MyGame_Example_Monster)) {
    throw new Error('Expected argument of type Monster');
  }
  return buffer_args.serialize();
}

function deserialize_MyGame_Example_Monster(buffer) {
  return MyGame_Example_Monster.getRootAsMonster(new flatbuffers.ByteBuffer(buffer))
}




var MonsterStorageService = exports.MonsterStorageService = {
  Store: {
    path: '/MyGame.Example.MonsterStorage/Store',
    requestStream: false,
    responseStream: false,
    requestType: flatbuffers.ByteBuffer,
    responseType: MyGame_Example_Stat,
    requestSerialize: serialize_MyGame_Example_Monster,
    requestDeserialize: deserialize_MyGame_Example_Monster,
    responseSerialize: serialize_MyGame_Example_Stat,
    responseDeserialize: deserialize_MyGame_Example_Stat,
  },
  Retrieve: {
    path: '/MyGame.Example.MonsterStorage/Retrieve',
    requestStream: false,
    responseStream: true,
    requestType: flatbuffers.ByteBuffer,
    responseType: MyGame_Example_Monster,
    requestSerialize: serialize_MyGame_Example_Stat,
    requestDeserialize: deserialize_MyGame_Example_Stat,
    responseSerialize: serialize_MyGame_Example_Monster,
    responseDeserialize: deserialize_MyGame_Example_Monster,
  },
  GetMaxHitPoint: {
    path: '/MyGame.Example.MonsterStorage/GetMaxHitPoint',
    requestStream: true,
    responseStream: false,
    requestType: flatbuffers.ByteBuffer,
    responseType: MyGame_Example_Stat,
    requestSerialize: serialize_MyGame_Example_Monster,
    requestDeserialize: deserialize_MyGame_Example_Monster,
    responseSerialize: serialize_MyGame_Example_Stat,
    responseDeserialize: deserialize_MyGame_Example_Stat,
  },
  GetMinMaxHitPoints: {
    path: '/MyGame.Example.MonsterStorage/GetMinMaxHitPoints',
    requestStream: true,
    responseStream: true,
    requestType: flatbuffers.ByteBuffer,
    responseType: MyGame_Example_Stat,
    requestSerialize: serialize_MyGame_Example_Monster,
    requestDeserialize: deserialize_MyGame_Example_Monster,
    responseSerialize: serialize_MyGame_Example_Stat,
    responseDeserialize: deserialize_MyGame_Example_Stat,
  },
};
exports.MonsterStorageClient = grpc.makeGenericClientConstructor(MonsterStorageService);
