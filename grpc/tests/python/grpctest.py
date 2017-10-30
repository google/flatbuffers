from __future__ import print_function

import os
import sys
from concurrent import futures
import time
import grpc

import flatbuffers
import MyGame.Sample.Color as Color
import MyGame.Sample.Equipment as Equipment
import MyGame.Sample.Monster as Monster
import MyGame.Sample.Vec3 as Vec3
import MyGame.Sample.Weapon as Weapon

import MyGame.Sample.monster_grpc_fb as monster_grpc_fb

def build():
  builder = flatbuffers.Builder(0)

  # Create a weapon for our Monster ('Sword').
  weapon = builder.CreateString('Sword')
  Weapon.WeaponStart(builder)
  Weapon.WeaponAddName(builder, weapon)
  Weapon.WeaponAddDamage(builder, 5)
  sword = Weapon.WeaponEnd(builder)

  name = builder.CreateString('Orc')

  Monster.MonsterStartInventoryVector(builder, 10)

  # Note: Since we prepend the bytes, this loop iterates in reverse order.
  for i in reversed(range(0, 10)):
    builder.PrependByte(i)
  inv = builder.EndVector(10)

  Monster.MonsterStartWeaponsVector(builder, 1)
  builder.PrependUOffsetTRelative(sword)
  weapons = builder.EndVector(1)

  pos = Vec3.CreateVec3(builder, 1.0, 2.0, 3.0)

  Monster.MonsterStart(builder)
  Monster.MonsterAddPos(builder, pos)
  Monster.MonsterAddHp(builder, 300)
  Monster.MonsterAddName(builder, name)
  Monster.MonsterAddInventory(builder, inv)
  Monster.MonsterAddColor(builder, Color.Color().Red)
  Monster.MonsterAddWeapons(builder, weapons)
  Monster.MonsterAddEquippedType(builder, Equipment.Equipment().Weapon)
  Monster.MonsterAddEquipped(builder, sword)
  orc = Monster.MonsterEnd(builder)

  builder.Finish(orc)
  return builder

builder = build()
buf = builder.Output()
monster = None

class MonsterStorage(monster_grpc_fb.MonsterStorageServicer):

  def Store(self, request, context):
    monster = request.GetRootAsMonster(buf, 0)
    monster_name = monster.Name().decode('utf-8')
    print("Monster's name: " + monster_name)
    return (monster_grpc_fb.Weapon.Weapon(), monster)

def serve():
  server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
  monster_grpc_fb.add_MonsterStorageServicer_to_server(MonsterStorage(), server)
  server.add_insecure_port('[::]:50051')
  server.start()
  print("Server started!")
  run()

def run():
  channel = grpc.insecure_channel('localhost:50051')
  stub = monster_grpc_fb.MonsterStorageStub(channel)
  response = stub.Store(monster_grpc_fb.Monster.Monster())
  weapon = response[0]
  monster = response[1]
  name = weapon.Init(monster.Equipped().Bytes, monster.Equipped().Pos)
  weapon_name = weapon.Name().decode('utf-8')
  print("Monster is equipped with a " + weapon_name + "!")


if __name__ == '__main__':
  serve()
