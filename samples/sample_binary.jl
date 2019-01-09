import FlatBuffers

include("monster_generated.jl")

# require the files generated from the schema
import .MyGame.Sample.Weapon
import .MyGame.Sample.Monster
import .MyGame.Sample.Vec3
import .MyGame.Sample.Color
import .MyGame.Sample.Equipment

sword = Weapon(;name="Sword", damage=3)
axe = Weapon(;name="Axe", damage=5)
orc = Monster(;
    name="Orc",
    pos=Vec3(1.0, 2.0, 3.0),
    hp=300,
    inventory=collect(1:10),
    color=MyGame.Sample.ColorRed,
    weapons=[sword, axe],
    equipped=axe
)

# Get the flatbuffer as a string containing the binary data
io = IOBuffer()
FlatBuffers.serialize(io, orc)
bytes = take!(io)

# Build the Monster from the raw bytes
mon = Monster(bytes)

@assert mon.mana == 150
@assert mon.hp == 300
@assert mon.name == "Orc"
@assert mon.color == MyGame.Sample.ColorRed
@assert mon.pos.x == 1.0
@assert mon.pos.y == 2.0
@assert mon.pos.z == 3.0
@assert mon.inventory == collect(1:10)

@assert mon.equipped_type == FlatBuffers.typeorder(Equipment, Weapon)
@assert mon.equipped.name == "Axe"
@assert mon.equipped.damage == 5

@assert mon.weapons[1].name == "Sword"
@assert mon.weapons[1].damage == 3

@assert mon.weapons[2].name == "Axe"
@assert mon.weapons[2].damage == 5
