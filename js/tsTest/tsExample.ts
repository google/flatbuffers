import fb from "./flatbuffers"
import MyGame from "./test_generated"

var builder=new fb.Builder(1)

var weaponOne = builder.createString('Sword');
var weaponTwo = builder.createString('Axe');
// Create the first `Weapon` ('Sword').
MyGame.Sample.Weapon.startWeapon(builder);
MyGame.Sample.Weapon.addName(builder, weaponOne);
MyGame.Sample.Weapon.addDamage(builder, 3);
var sword = MyGame.Sample.Weapon.endWeapon(builder);
// Create the second `Weapon` ('Axe').
MyGame.Sample.Weapon.startWeapon(builder);
MyGame.Sample.Weapon.addName(builder, weaponTwo);
MyGame.Sample.Weapon.addDamage(builder, 5);
var axe = MyGame.Sample.Weapon.endWeapon(builder);

MyGame.Sample.Monster.addEquippedType(builder, MyGame.Sample.Equipment.Weapon); // Union type
MyGame.Sample.Monster.addEquipped(builder, axe); // Union data

// Serialize a name for our monster, called 'Orc'.
var name = builder.createString('Orc');
// Create a `vector` representing the inventory of the Orc. Each number
// could correspond to an item that can be claimed after he is slain.
var treasure = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
var inv = MyGame.Sample.Monster.createInventoryVector(builder, treasure);

// Create an array from the two `Weapon`s and pass it to the
// `createWeaponsVector()` method to create a FlatBuffer vector.
var weaps = [sword, axe];
var weapons = MyGame.Sample.Monster.createWeaponsVector(builder, weaps);

// Create a `Vec3`, representing the Orc's position in 3-D space.
var pos = MyGame.Sample.Vec3.createVec3(builder, 1.0, 2.0, 3.0);

// Create our monster by using `startMonster()` and `endMonster()`.
MyGame.Sample.Monster.startMonster(builder);
MyGame.Sample.Monster.addPos(builder, pos);
MyGame.Sample.Monster.addHp(builder, 300);
MyGame.Sample.Monster.addColor(builder, MyGame.Sample.Color.Red)
MyGame.Sample.Monster.addName(builder, name);
MyGame.Sample.Monster.addInventory(builder, inv);
MyGame.Sample.Monster.addWeapons(builder, weapons);
MyGame.Sample.Monster.addEquippedType(builder, MyGame.Sample.Equipment.Weapon);
MyGame.Sample.Monster.addEquipped(builder, axe);
var orc = MyGame.Sample.Monster.endMonster(builder);

builder.finish(orc);

// This must be called after `finish()`.
var buf1 = builder.asUint8Array(); // Of type `Uint8Array`.

var bytes = buf1/* the data you just read, in an object of type "Uint8Array" */
var buf = new fb.ByteBuffer(bytes);
// Get an accessor to the root object inside the buffer.
var monster = MyGame.Sample.Monster.getRootAsMonster(buf);
var hp = monster.hp();
var mana = monster.mana();
var name2 = monster.name();
var pos2 = monster.pos();
var x = pos2.x();
var y = pos2.y();
var z = pos2.z();
var invLength = monster.inventoryLength();
var thirdItem = monster.inventory(2);
var weaponsLength = monster.weaponsLength();
var secondWeaponName = monster.weapons(1).name();
var secondWeaponDamage = monster.weapons(1).damage();
var unionType = monster.equippedType();
if (unionType == MyGame.Sample.Equipment.Weapon) {
  var weapon_name = monster.equipped(new MyGame.Sample.Weapon()).name();     // 'Axe'
  var weapon_damage = monster.equipped(new MyGame.Sample.Weapon()).damage(); // 5
}