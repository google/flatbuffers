import sys

import flatbuffers

import MyGame  # refers to generated code
import MyGame.Example  # refers to generated code
import MyGame.Example.Any  # refers to generated code
import MyGame.Example.Color  # refers to generated code
import MyGame.Example.Monster  # refers to generated code
import MyGame.Example.Test  # refers to generated code
import MyGame.Example.Stat  # refers to generated code
import MyGame.Example.Vec3  # refers to generated code

b = flatbuffers.Builder(0)
string = b.CreateString("MyMonster")
test1 = b.CreateString("test1")
test2 = b.CreateString("test2")
fred = b.CreateString("Fred")

N = int(sys.argv[1])
MyGame.Example.Monster.MonsterStartInventoryVector(b, N)

for i in range(N):
    b.PrependByte(i % 125)

inv = b.EndVector(N)

MyGame.Example.Monster.MonsterStart(b)
MyGame.Example.Monster.MonsterAddName(b, fred)
mon2 = MyGame.Example.Monster.MonsterEnd(b)

MyGame.Example.Monster.MonsterStartTest4Vector(b, 2)
MyGame.Example.Test.CreateTest(b, 10, 20)
MyGame.Example.Test.CreateTest(b, 30, 40)
test4 = b.EndVector(2)

MyGame.Example.Monster.MonsterStartTestarrayofstringVector(b, 2)
b.PrependUOffsetTRelative(test2)
b.PrependUOffsetTRelative(test1)
testArrayOfString = b.EndVector(2)

MyGame.Example.Monster.MonsterStart(b)

pos = MyGame.Example.Vec3.CreateVec3(b, 1.0, 2.0, 3.0, 3.0, 2, 5, 6)
MyGame.Example.Monster.MonsterAddPos(b, pos)

MyGame.Example.Monster.MonsterAddHp(b, 80)
MyGame.Example.Monster.MonsterAddName(b, string)
MyGame.Example.Monster.MonsterAddInventory(b, inv)
MyGame.Example.Monster.MonsterAddTestType(b, 1)
MyGame.Example.Monster.MonsterAddTest(b, mon2)
MyGame.Example.Monster.MonsterAddTest4(b, test4)
MyGame.Example.Monster.MonsterAddTestarrayofstring(b, testArrayOfString)
mon = MyGame.Example.Monster.MonsterEnd(b)

b.Finish(mon)

gen_buf, gen_off = b.Bytes, b.Head()

monster = MyGame.Example.Monster.Monster.GetRootAsMonster(gen_buf, gen_off)

import time
start = time.time()
for counter in xrange(100):
    inventory = monster.InventoryAsNumpy()

numpy_time = (time.time() - start)
print ' secs to retrieve full inventory 100 times, numpy:', numpy_time

start = time.time()
for counter in xrange(100):
    inventory_old = []
    for i in xrange(monster.InventoryLength()):
        inventory_old.append(int(monster.Inventory(i)))

list_time = (time.time() - start)
print ' secs to retrieve full inventory 100 times, list :', list_time

print ' list time / numpy time :', list_time / numpy_time
