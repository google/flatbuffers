import std.stdio;
import std.file;
import mygame.example;

void main()
{
	assert(std.file.exists("monsterdata_test.mon"));
	auto rdata = cast(ubyte[])(std.file.read("monsterdata_test.mon"));
	testBuffer(new ByteBuffer(rdata));
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
