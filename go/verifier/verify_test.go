package verifier

import (
	"testing"

	"github.com/stretchr/testify/assert"

	flatbuffers "github.com/google/flatbuffers/go"

	"github.com/google/flatbuffers/go/verifier/generated"
)

func TestVerify_ParseVTable_Empty(t *testing.T) {
	as := assert.New(t)
	e := generated.EmptyT{}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	// initial a new verifier
	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 12)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_Int8(t *testing.T) {
	as := assert.New(t)
	e := generated.Int8T{
		Field: 1,
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 16)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_Int16(t *testing.T) {
	as := assert.New(t)
	e := generated.Int16T{
		Field: 1,
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 16)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_Int32(t *testing.T) {
	as := assert.New(t)
	e := generated.Int32T{
		Field: 1,
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 16)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_Int64(t *testing.T) {
	as := assert.New(t)
	e := generated.Int64T{
		Field: 1,
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 16)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_String(t *testing.T) {
	as := assert.New(t)
	e := generated.StringT{
		Field: "1234567",
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 16)
	as.Equal(v.Size(), len(buf))
}
func TestVerify_ParseVTable_Complex(t *testing.T) {
	as := assert.New(t)
	e := generated.ComplexT{
		Field1: 1,
		Field2: 2,
		Field3: 3,
		Field4: 4,
		Field6: []byte("123"),
		Field7: generated.EnumTypeFemale,
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 32)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_Monster1(t *testing.T) {
	as := assert.New(t)
	e := generated.MonsterT{
		Pos: &generated.Vec3T{
			X: 1.0,
			Y: 2.0,
			Z: 3.0,
		},
		Mana:      123,
		Hp:        123,
		Name:      "name",
		Inventory: []byte("123"),
		Color:     generated.ColorBlue,
		Weapons: []*generated.WeaponT{
			&generated.WeaponT{
				Name:   "weapon",
				Damage: 1,
			},
		},
		Equipped: &generated.EquipmentT{
			Type: generated.EquipmentWeapon,
			Value: &generated.WeaponT{
				Name:   "weapon",
				Damage: 1.0,
			},
		},
		Path: []*generated.Vec3T{
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
		},
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 36)
	as.Equal(v.Size(), len(buf))
}

func TestVerify_ParseVTable_Monster2(t *testing.T) {
	as := assert.New(t)
	wp := &generated.GunT{
		Name:   "gun",
		Damage: 1.0,
		Time:   365,
	}
	e := generated.MonsterT{
		Pos: &generated.Vec3T{
			X: 1.0,
			Y: 2.0,
			Z: 3.0,
		},
		Mana:      123,
		Hp:        123,
		Name:      "name",
		Inventory: []byte("123"),
		Color:     generated.ColorBlue,
		Weapons: []*generated.WeaponT{
			&generated.WeaponT{
				Name:   "weapon",
				Damage: 1,
			},
		},
		Equipped: &generated.EquipmentT{
			Type:  generated.EquipmentGun,
			Value: wp,
		},
		Path: []*generated.Vec3T{
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
		},
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	n, err := v.Parse(buf, 0)
	as.NoError(err)
	as.Equal(n, 36)
	as.Equal(v.Size(), len(buf))

	mt := new(generated.MonsterT)
	generated.GetRootAsMonster(buf, 0).UnPackTo(mt)

	as.Equal(mt.Equipped.Type, generated.EquipmentGun)
	as.Equal(mt.Equipped.Value.(*generated.GunT).Time, wp.Time)
}

func TestVerify_ParseVTable_Monster3(t *testing.T) {
	as := assert.New(t)
	e := generated.MonsterT{
		Pos: &generated.Vec3T{
			X: 1.0,
			Y: 2.0,
			Z: 3.0,
		},
		Mana:      123,
		Hp:        123,
		Name:      "name",
		Inventory: []byte("123"),
		Color:     generated.ColorBlue,
		Weapons: []*generated.WeaponT{
			&generated.WeaponT{
				Name:   "weapon",
				Damage: 1,
			},
		},
		Equipped: &generated.EquipmentT{
			Type: generated.EquipmentGun,
			Value: &generated.GunT{
				Name:   "weapon",
				Damage: 1.0,
				Time:   365,
			},
		},
		Path: []*generated.Vec3T{
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
			&generated.Vec3T{
				X: 1.0,
				Y: 2.0,
				Z: 3.0,
			},
		},
	}
	b := flatbuffers.NewBuilder(0)
	b.Finish(e.Pack(b))
	buf := b.FinishedBytes()

	v := NewVerify()

	//   changed buf,  will be error cause
	_, err := v.Parse(buf[1:], 0)
	as.Error(err)
}
