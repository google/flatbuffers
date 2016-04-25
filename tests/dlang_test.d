import std.stdio;
import std.file;
import MyGame.Example;

void main()
{
	assert(std.file.exists("monsterdata_test.mon"));
	auto rdata = cast(ubyte[])(std.file.read("monsterdata_test.mon"));
	testBuffer(new ByteBuffer(rdata));

	FlatBufferBuilder fbb = new FlatBufferBuilder(1);

	// We set up the same values as monsterdata.json:
	
	int str = fbb.createString("MyMonster");
	
	int inv = Monster.createInventoryVector(fbb, cast(byte[]) [0, 1, 2, 3, 4]);
	
	int fred = fbb.createString("Fred");
	Monster.startMonster(fbb);
	Monster.addName(fbb, fred);
	int mon2 = Monster.endMonster(fbb);
	
	Monster.startTest4Vector(fbb, 2);
	Test.createTest(fbb, cast(short) 10, cast(short) 20);
	Test.createTest(fbb, cast(short) 30, cast(short) 40);
	int test4 = fbb.endVector();
	
	int testArrayOfString = Monster.createTestarrayofstringVector(fbb, [
		fbb.createString("test1"),
		fbb.createString("test2")
	]);
	
	Monster.startMonster(fbb);
	Monster.addPos(fbb, Vec3.createVec3(fbb, 1.0f, 2.0f, 3.0f, 3.0, Color.Green, cast(short) 5, cast(byte) 6));
	Monster.addHp(fbb, cast(short) 80);
	Monster.addName(fbb, str);
	Monster.addInventory(fbb, inv);
	Monster.addTestType(fbb, cast(byte) Any.Monster);
	Monster.addTest(fbb, mon2);
	Monster.addTest4(fbb, test4);
	Monster.addTestarrayofstring(fbb, testArrayOfString);
	Monster.addTestbool(fbb, false);
	Monster.addTesthashu32Fnv1(fbb, uint.max);
	int mon = Monster.endMonster(fbb);
	
	Monster.finishMonsterBuffer(fbb, mon);
	std.file.write("monsterdata_d_wire.mon", fbb.sizedByteArray());
	
	testExtendedBuffer(fbb.dataBuffer);
	
	// test enums
	static assert(__traits(hasMember, Color, "Red"));
	static assert(__traits(hasMember, Color, "Blue"));
	static assert(__traits(hasMember, Any, "NONE"));
	static assert(__traits(hasMember, Any, "Monster"));
}

void testBuffer(ByteBuffer buf)
{
	auto monster = Monster.getRootAsMonster(buf);

	assert(monster.hp() == 80);
	assert(monster.mana() == 150);  // default
	assert(monster.name() == "MyMonster");

	Nullable!Vec3 pos = monster.pos;
	assert(!pos.isNull);
	assert(pos.x == 1.0f);
	assert(pos.y == 2.0f);
	assert(pos.z == 3.0f);
	assert(pos.test1 == 3.0);
	assert(pos.test2 == Color.Green);
	Nullable!Test t = pos.test3;
	assert(!t.isNull);
	assert(t.a == 5);
	assert(t.b == 6);

	assert(monster.testType == cast(ubyte)Any.Monster);
	//Monster monster2 = new Monster();
	Monster monster2 = monster.test!Monster();
	assert(monster2.name == "Fred");

	assert(monster.inventoryLength == 5);

	int invsum = 0;
	for (int i = 0; i < 5; i++)
		invsum += monster.inventory(i);
	assert(invsum == 10);

	invsum = 0;
	auto inv = monster.inventory();
	foreach(u;inv)
		invsum += u;
	assert(invsum == 10);

	Nullable!Test  test_0 = monster.test4(0);
	Nullable!Test  test_1 = monster.test4(1);

	assert(!test_0.isNull);
	assert(!test_1.isNull);

	assert(monster.test4Length == 2);
	assert(test_0.a() + test_0.b() + test_1.a() + test_1.b() == 100);


	assert(monster.testarrayofstringLength() == 2);
	assert(monster.testarrayofstring(0) == "test1");
	assert(monster.testarrayofstring(1) == "test2");

	assert(monster.testbool() == false);
}

void testExtendedBuffer(ByteBuffer bb)
{
	testBuffer(bb);
	
	Monster monster = Monster.getRootAsMonster(bb);
	
	assert(monster.testhashu32Fnv1 == uint.max);
}
