import std.stdio;
import myGame.sample;
import std.file;

void main()
{
	writeln("Edit source/app.d to start your project.");
	
	auto builder = new FlatBufferBuilder(64);
	
	auto name = builder.createString("MyMonster");
	
	name = builder.createString("hello world");
	
	ubyte[] invData = cast(ubyte[])("MyMonster");
	writeln("invdata : ",invData);
	auto inventory = Monster.createInventoryVector(builder, invData); //todo：数组错误
	//Create monster:
	Monster.startMonster(builder);
	Monster.addBy(builder,'a');
	Monster.addPos(builder, Vec3.createVec3(builder, 1, 2, 3));
	Monster.addMana(builder, 150);
	Monster.addHp(builder, 80);
	Monster.addName(builder, name);
	Monster.addInventory(builder, inventory);
	Monster.addColor(builder, Color.blue);
	auto mloc = Monster.endMonster(builder);
	
	builder.finish(mloc);
	//We now have a FlatBuffer we can store or send somewhere.
	
	//** file/network code goes here :) **
	//access builder.sizedByteArray() for builder.sizedByteArray().length bytes
	
	//Instead, we're going to access it straight away.
	//Get access to the root:
	auto data = builder.sizedByteArray();
	auto monster = Monster.getRootAsMonster(new ByteBuffer(data));
	
	writeln("monster.by " , monster.by, "  'a' = ", cast(ubyte)('a'));
	assert(monster.by == 'a');
	assert(monster.hp == 80);
	assert(monster.mana == 150); //default
	//assert(monster.name == "MyMonster");
	
	auto pos = monster.pos();
	assert(!pos.isNull());
	assert(pos.z == 3);
	auto hh = monster.inventory();
	foreach (i;hh){
            writeln(i);
	}
	writeln("\n\n");
	 int len = monster.inventoryLength;
	for (int i = 0; i < len; ++i) {
	writeln(monster.inventory(i));
	}
}
