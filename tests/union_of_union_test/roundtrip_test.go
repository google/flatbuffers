package union_of_union_test

import (
	"testing"

	flatbuffers "union_of_union_test/flatbuffers"
	"union_of_union_test/Test"
)

func TestUnionOfUnionRoundTrip_TableA(t *testing.T) {
	// Build a Root with content = TableA (from Inner union, flattened into Outer)
	builder := flatbuffers.NewBuilder(256)

	Test.TableAStart(builder)
	Test.TableAAddX(builder, 42)
	tableAOff := Test.TableAEnd(builder)

	Test.RootStart(builder)
	Test.RootAddContentType(builder, Test.OuterTableA)
	Test.RootAddContent(builder, tableAOff)
	rootOff := Test.RootEnd(builder)
	builder.Finish(rootOff)

	buf := builder.FinishedBytes()

	root := Test.GetRootAsRoot(buf, 0)
	if root.ContentType() != Test.OuterTableA {
		t.Fatalf("ContentType = %d, want %d (TableA)", root.ContentType(), Test.OuterTableA)
	}

	// Read the union value as a Table, then init TableA from it
	var tbl flatbuffers.Table
	if !root.Content(&tbl) {
		t.Fatal("Content returned false")
	}
	var tableA Test.TableA
	tableA.Init(tbl.Bytes, tbl.Pos)
	if tableA.X() != 42 {
		t.Errorf("TableA.X = %d, want 42", tableA.X())
	}
}

func TestUnionOfUnionRoundTrip_TableC(t *testing.T) {
	// Build a Root with content = TableC (direct member of Outer)
	builder := flatbuffers.NewBuilder(256)

	Test.TableCStart(builder)
	Test.TableCAddZ(builder, 3.14)
	tableCOff := Test.TableCEnd(builder)

	Test.RootStart(builder)
	Test.RootAddContentType(builder, Test.OuterTableC)
	Test.RootAddContent(builder, tableCOff)
	rootOff := Test.RootEnd(builder)
	builder.Finish(rootOff)

	buf := builder.FinishedBytes()

	root := Test.GetRootAsRoot(buf, 0)
	if root.ContentType() != Test.OuterTableC {
		t.Fatalf("ContentType = %d, want %d (TableC)", root.ContentType(), Test.OuterTableC)
	}

	var tbl flatbuffers.Table
	if !root.Content(&tbl) {
		t.Fatal("Content returned false")
	}
	var tableC Test.TableC
	tableC.Init(tbl.Bytes, tbl.Pos)
	if tableC.Z() != float32(3.14) {
		t.Errorf("TableC.Z = %f, want 3.14", tableC.Z())
	}
}

func TestUnionOfUnionRoundTrip_ObjectAPI(t *testing.T) {
	// Use the Object API to pack and unpack
	builder := flatbuffers.NewBuilder(256)

	rootT := &Test.RootT{
		Content: &Test.OuterT{
			Type: Test.OuterTableB,
			Value: &Test.TableBT{
				Y: "hello from nested union",
			},
		},
	}

	off := rootT.Pack(builder)
	builder.Finish(off)
	buf := builder.FinishedBytes()

	// Unpack
	root := Test.GetRootAsRoot(buf, 0)
	unpacked := root.UnPack()

	if unpacked.Content == nil {
		t.Fatal("Unpacked Content is nil")
	}
	if unpacked.Content.Type != Test.OuterTableB {
		t.Fatalf("Unpacked Content.Type = %d, want %d (TableB)", unpacked.Content.Type, Test.OuterTableB)
	}

	tableB, ok := unpacked.Content.Value.(*Test.TableBT)
	if !ok {
		t.Fatalf("Unpacked Content.Value type = %T, want *TableBT", unpacked.Content.Value)
	}
	if tableB.Y != "hello from nested union" {
		t.Errorf("TableB.Y = %q, want %q", tableB.Y, "hello from nested union")
	}
}
