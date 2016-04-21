import std.stdio;
import mygame.sample;
//import std.file;
import std.conv;

void main()
{
	writeln("Edit source/app.d to start your project.");
	
	auto builder = new FlatBufferBuilder(512);
	
	auto name = builder.createString("MyMonster");
	
	int[] weaps;
	foreach ( i ; 0..3){
            weaps ~= Weapon.createWeapon(builder,builder.createString("Weapon." ~ i.to!string()),cast(short)i);
	}
	int t = Monster.createWeaponsVector(builder,weaps);
	
	ubyte[] invData = cast(ubyte[])("MyMonster");	
	auto inventory = Monster.createInventoryVector(builder, invData); 
	//Create monster:
	Monster.startMonster(builder);
	Monster.addPos(builder, Vec3.createVec3(builder, 1, 2, 3));
	Monster.addMana(builder, 150);
	Monster.addHp(builder, 80);
	Monster.addName(builder, name);
	Monster.addInventory(builder, inventory);
	Monster.addColor(builder, Color.Blue);
	Monster.addWeapons(builder,t);
	auto mloc = Monster.endMonster(builder);
	
	builder.finish(mloc);
	//We now have a FlatBuffer we can store or send somewhere.
	
	//** file/network code goes here :) **
	//access builder.sizedByteArray() for builder.sizedByteArray().length bytes
	
	//Instead, we're going to access it straight away.
	//Get access to the root:
	auto data = builder.sizedByteArray();
	//std.file.write("data",data);
	//writeln("write serialized data in file data!");
	
	//auto rdata = cast(ubyte[])(std.file.read("data"));
	auto monster = Monster.getRootAsMonster(new ByteBuffer(data));
	
	assert(monster.hp == 80);
	assert(monster.mana == 150); //default
	assert(monster.name == "MyMonster");
	
	assert(monster.color == Color.Blue);
	
	auto pos = monster.pos();
	assert(!pos.isNull());
	assert(pos.z == 3);
	
	auto hh = monster.inventory();
	assert(hh.length ==  9);
	assert(hh[2] == 'M');
	
	
	auto weapons = monster.weapons();
	assert(weapons.length ==  3);
	Nullable!Weapon weap = weapons[1];
	assert(!weap.isNull());
	assert(weap.damage == 1);
	assert(weap.name == "Weapon.1");
	writeln("un serialize data over!");
}


