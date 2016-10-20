"use strict";
var flatbuffers_1 = require("./flatbuffers");
var test_generated_1 = require("./test_generated");
var builder = new flatbuffers_1["default"].Builder(1);
var weaponOne = builder.createString('Sword');
var weaponTwo = builder.createString('Axe');
// Create the first `Weapon` ('Sword').
test_generated_1["default"].Sample.Weapon.startWeapon(builder);
test_generated_1["default"].Sample.Weapon.addName(builder, weaponOne);
test_generated_1["default"].Sample.Weapon.addDamage(builder, 3);
var sword = test_generated_1["default"].Sample.Weapon.endWeapon(builder);
// Create the second `Weapon` ('Axe').
test_generated_1["default"].Sample.Weapon.startWeapon(builder);
test_generated_1["default"].Sample.Weapon.addName(builder, weaponTwo);
test_generated_1["default"].Sample.Weapon.addDamage(builder, 5);
var axe = test_generated_1["default"].Sample.Weapon.endWeapon(builder);
test_generated_1["default"].Sample.Monster.addEquippedType(builder, test_generated_1["default"].Sample.Equipment.Weapon); // Union type
test_generated_1["default"].Sample.Monster.addEquipped(builder, axe); // Union data
// Serialize a name for our monster, called 'Orc'.
var name = builder.createString('Orc');
// Create a `vector` representing the inventory of the Orc. Each number
// could correspond to an item that can be claimed after he is slain.
var treasure = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
var inv = test_generated_1["default"].Sample.Monster.createInventoryVector(builder, treasure);
// Create an array from the two `Weapon`s and pass it to the
// `createWeaponsVector()` method to create a FlatBuffer vector.
var weaps = [sword, axe];
var weapons = test_generated_1["default"].Sample.Monster.createWeaponsVector(builder, weaps);
// Create a `Vec3`, representing the Orc's position in 3-D space.
var pos = test_generated_1["default"].Sample.Vec3.createVec3(builder, 1.0, 2.0, 3.0);
// Create our monster by using `startMonster()` and `endMonster()`.
test_generated_1["default"].Sample.Monster.startMonster(builder);
test_generated_1["default"].Sample.Monster.addPos(builder, pos);
test_generated_1["default"].Sample.Monster.addHp(builder, 300);
test_generated_1["default"].Sample.Monster.addColor(builder, test_generated_1["default"].Sample.Color.Red);
test_generated_1["default"].Sample.Monster.addName(builder, name);
test_generated_1["default"].Sample.Monster.addInventory(builder, inv);
test_generated_1["default"].Sample.Monster.addWeapons(builder, weapons);
test_generated_1["default"].Sample.Monster.addEquippedType(builder, test_generated_1["default"].Sample.Equipment.Weapon);
test_generated_1["default"].Sample.Monster.addEquipped(builder, axe);
var orc = test_generated_1["default"].Sample.Monster.endMonster(builder);
builder.finish(orc);
// This must be called after `finish()`.
var buf1 = builder.asUint8Array(); // Of type `Uint8Array`.
var bytes = buf1; /* the data you just read, in an object of type "Uint8Array" */
var buf = new flatbuffers_1["default"].ByteBuffer(bytes);
// Get an accessor to the root object inside the buffer.
var monster = test_generated_1["default"].Sample.Monster.getRootAsMonster(buf);
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
if (unionType == test_generated_1["default"].Sample.Equipment.Weapon) {
    var weapon_name = monster.equipped(new test_generated_1["default"].Sample.Weapon()).name(); // 'Axe'
    var weapon_damage = monster.equipped(new test_generated_1["default"].Sample.Weapon()).damage(); // 5
}
