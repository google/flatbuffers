from __future__ import print_function

import os
import sys
from concurrent import futures
import time
import grpc

import flatbuffers
import MyGame.Sample.Color
import MyGame.Sample.Equipment
import MyGame.Sample.Monster
import MyGame.Sample.Vec3
import MyGame.Sample.Weapon

import MyGame.Sample.monster_grpc_fb as monster_grpc_fb

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

def build():
  builder = flatbuffers.Builder(0)

  # Create a weapon for our Monster ('Sword').
  weapon = builder.CreateString('Sword')
  MyGame.Sample.Weapon.WeaponStart(builder)
  MyGame.Sample.Weapon.WeaponAddName(builder, weapon)
  MyGame.Sample.Weapon.WeaponAddDamage(builder, 5)
  sword = MyGame.Sample.Weapon.WeaponEnd(builder)

  name = builder.CreateString('Orc')

  MyGame.Sample.Monster.MonsterStartInventoryVector(builder, 10)

  # Note: Since we prepend the bytes, this loop iterates in reverse order.
  for i in reversed(range(0, 10)):
    builder.PrependByte(i)
  inv = builder.EndVector(10)

  MyGame.Sample.Monster.MonsterStartWeaponsVector(builder, 1)
  builder.PrependUOffsetTRelative(sword)
  weapons = builder.EndVector(1)

  pos = MyGame.Sample.Vec3.CreateVec3(builder, 1.0, 2.0, 3.0)

  MyGame.Sample.Monster.MonsterStart(builder)
  MyGame.Sample.Monster.MonsterAddPos(builder, pos)
  MyGame.Sample.Monster.MonsterAddHp(builder, 300)
  MyGame.Sample.Monster.MonsterAddName(builder, name)
  MyGame.Sample.Monster.MonsterAddInventory(builder, inv)
  MyGame.Sample.Monster.MonsterAddColor(builder,
                                        MyGame.Sample.Color.Color().Red)
  MyGame.Sample.Monster.MonsterAddWeapons(builder, weapons)
  MyGame.Sample.Monster.MonsterAddEquippedType(
      builder, MyGame.Sample.Equipment.Equipment().Weapon)
  MyGame.Sample.Monster.MonsterAddEquipped(builder, sword)
  orc = MyGame.Sample.Monster.MonsterEnd(builder)

  builder.Finish(orc)
  return builder

class MonsterStorage(monster_grpc_fb.MonsterStorageServicer):

  def Store(self, request, context):
    return monster_grpc_fb.Weapon(name='Hello, %s!' % request.name)

def serve(name):
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
  monster_grpc_fb.add_MonsterStorageServicer_to_server(MonsterStorage(), server)
  server.add_insecure_port('[::]:50051')
  server.start()
  run(name)

def run(name):
  channel = grpc.insecure_channel('localhost:50051')
  stub = monster_grpc_fb.MonsterStorageStub(channel)
  response = stub.Store(monster_grpc_fb.Monster(name=name))
  print("Greeter client received: " + response.message)


if __name__ == '__main__':
  builder = build()
  buf = builder.Output()
  monster = MyGame.Sample.Monster.Monster.GetRootAsMonster(buf, 0)
  serve(monster.Name())
