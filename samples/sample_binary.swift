 // THIS IS JUST TO SHOW THE CODE, PLEASE DO IMPORT FLATBUFFERS WITH SPM..
import Flatbuffers

typealias Monster = MyGame.Sample.Monster
typealias Weapon = MyGame.Sample.Weapon
typealias Color = MyGame.Sample.Color
typealias Vec3 = MyGame.Sample.Vec3

func main() {
    let expectedDMG: [Int16] = [3, 5]
    let expectedNames = ["Sword", "Axe"]

    let builder = FlatBufferBuilder(initialSize: 1024)
    let weapon1Name = builder.create(string: expectedNames[0])
    let weapon2Name = builder.create(string: expectedNames[1])
        
    let weapon1Start = Weapon.startWeapon(builder)
    Weapon.add(name: weapon1Name, builder)
    Weapon.add(damage: expectedDMG[0], builder)
    let sword = Weapon.endWeapon(builder, start: weapon1Start)
    let weapon2Start = Weapon.startWeapon(builder)
    Weapon.add(name: weapon2Name, builder)
    Weapon.add(damage: expectedDMG[1], builder)
    let axe = Weapon.endWeapon(builder, start: weapon2Start)
    
    let name = builder.create(string: "Orc")
    let inventory: [Byte] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    let inventoryOffset = builder.createVector(inventory)

    let weaponsOffset = builder.createVector(ofOffsets: [sword, axe])
    let pos = builder.create(struct: MyGame.Sample.createVec3(x: 1, y: 2, z: 3), type: Vec3.self)
    
    
    let orc = Monster.createMonster(builder, 
                                    offsetOfPos: pos,
                                    hp: 300,
                                    offsetOfName: name,
                                    vectorOfInventory: inventoryOffset,
                                    color: .red,
                                    vectorOfWeapons: weaponsOffset,
                                    equippedType: .weapon,
                                    offsetOfEquipped: axe)
    builder.finish(offset: orc)
    
    var buf = builder.sizedByteArray
    var monster = Monster.getRootAsMonster(bb: ByteBuffer(bytes: buf))

    assert(monster.mana == 150)
    assert(monster.hp == 300)
    assert(monster.name == "Orc")
    assert(monster.color == MyGame.Sample.Color.red)
    assert(monster.pos != nil)
    for i in 0..<monster.inventoryCount {
        assert(i == monster.inventory(at: i))
    }
        
    for i in 0..<monster.weaponsCount {
        let weap = monster.weapons(at: i)
        let index = Int(i)
        assert(weap?.damage == expectedDMG[index])
        assert(weap?.name == expectedNames[index])
    }
    assert(monster.equippedType == .weapon)
    let equipped = monster.equipped(type: Weapon.self)
    assert(equipped?.name == "Axe")
    assert(equipped?.damage == 5)
    print("Monster Object is Verified")
}