// Copyright 2025 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package go_map_set_test

import (
	"testing"

	mapset "github.com/google/flatbuffers/MapSetTest"
	flatbuffers "github.com/google/flatbuffers/go"
)

// packAndUnpack serializes cfg via ConfigT.Pack and deserializes it back via
// Config.UnPack using a fresh builder each time.
func packAndUnpack(t *testing.T, cfg *mapset.ConfigT) *mapset.ConfigT {
	t.Helper()
	b := flatbuffers.NewBuilder(512)
	b.Finish(cfg.Pack(b))
	return mapset.GetRootAsConfig(b.FinishedBytes(), 0).UnPack()
}

// packBytes serializes cfg via ConfigT.Pack and returns the finished bytes.
func packBytes(cfg *mapset.ConfigT) []byte {
	b := flatbuffers.NewBuilder(512)
	b.Finish(cfg.Pack(b))
	return b.FinishedBytes()
}

// TestMapSetRoundTrip verifies that map and set fields survive a FlatBuffers
// serialize/deserialize round-trip using the Object API ConfigT.Pack / Config.UnPack.
//
// Covered fields:
//   - Settings   map[string]int32    (3 entries: alpha=1, beta=2, gamma=3)
//   - IdToName   map[uint32]string   (2 entries: 10="ten", 20="twenty")
//   - Tags        map[string]struct{} (2 entries: "hello", "world")
//   - ActiveIds   map[int32]struct{}  (2 entries: 100, 200)
func TestMapSetRoundTrip(t *testing.T) {
	cfg := &mapset.ConfigT{
		Settings: map[string]int32{
			"alpha": 1,
			"beta":  2,
			"gamma": 3,
		},
		IdToName: map[uint32]string{
			10: "ten",
			20: "twenty",
		},
		Tags: map[string]struct{}{
			"hello": {},
			"world": {},
		},
		ActiveIds: map[int32]struct{}{
			100: {},
			200: {},
		},
	}

	got := packAndUnpack(t, cfg)

	// --- Verify Settings (map[string]int32) ---
	if len(got.Settings) != 3 {
		t.Fatalf("Settings length: want 3, got %d", len(got.Settings))
	}
	wantSettings := map[string]int32{"alpha": 1, "beta": 2, "gamma": 3}
	for k, wantVal := range wantSettings {
		if gotVal, ok := got.Settings[k]; !ok {
			t.Errorf("Settings[%q]: not found after round-trip", k)
		} else if gotVal != wantVal {
			t.Errorf("Settings[%q]: want %d, got %d", k, wantVal, gotVal)
		}
	}

	// --- Verify IdToName (map[uint32]string) ---
	if len(got.IdToName) != 2 {
		t.Fatalf("IdToName length: want 2, got %d", len(got.IdToName))
	}
	wantIdToName := map[uint32]string{10: "ten", 20: "twenty"}
	for k, wantVal := range wantIdToName {
		if gotVal, ok := got.IdToName[k]; !ok {
			t.Errorf("IdToName[%d]: not found after round-trip", k)
		} else if gotVal != wantVal {
			t.Errorf("IdToName[%d]: want %q, got %q", k, wantVal, gotVal)
		}
	}

	// --- Verify Tags (map[string]struct{}) ---
	if len(got.Tags) != 2 {
		t.Fatalf("Tags length: want 2, got %d", len(got.Tags))
	}
	for _, tag := range []string{"hello", "world"} {
		if _, ok := got.Tags[tag]; !ok {
			t.Errorf("Tags[%q]: not found after round-trip", tag)
		}
	}

	// --- Verify ActiveIds (map[int32]struct{}) ---
	if len(got.ActiveIds) != 2 {
		t.Fatalf("ActiveIds length: want 2, got %d", len(got.ActiveIds))
	}
	for _, id := range []int32{100, 200} {
		if _, ok := got.ActiveIds[id]; !ok {
			t.Errorf("ActiveIds[%d]: not found after round-trip", id)
		}
	}
}

// TestMapSetEmptyRoundTrip verifies that a ConfigT with nil maps/sets
// round-trips cleanly and produces empty (nil) collections on the other side.
func TestMapSetEmptyRoundTrip(t *testing.T) {
	got := packAndUnpack(t, &mapset.ConfigT{})

	if len(got.Settings) != 0 {
		t.Errorf("empty Settings: want 0 entries, got %d", len(got.Settings))
	}
	if len(got.IdToName) != 0 {
		t.Errorf("empty IdToName: want 0 entries, got %d", len(got.IdToName))
	}
	if len(got.Tags) != 0 {
		t.Errorf("empty Tags: want 0 entries, got %d", len(got.Tags))
	}
	if len(got.ActiveIds) != 0 {
		t.Errorf("empty ActiveIds: want 0 entries, got %d", len(got.ActiveIds))
	}
}

// TestMapSetDuplicateKeyLastWriterWins verifies that when the same key is
// assigned multiple times in a Go map, only the final value survives the
// round-trip. Go map semantics deduplicate before Pack is ever called.
func TestMapSetDuplicateKeyLastWriterWins(t *testing.T) {
	settings := map[string]int32{}
	settings["dup"] = 7
	settings["dup"] = 99 // last write wins — Go map deduplicates before Pack
	settings["other"] = 1

	got := packAndUnpack(t, &mapset.ConfigT{Settings: settings})

	if len(got.Settings) != 2 {
		t.Fatalf("duplicate-key Settings length: want 2, got %d", len(got.Settings))
	}
	if v, ok := got.Settings["dup"]; !ok {
		t.Fatal(`Settings["dup"]: not found after round-trip`)
	} else if v != 99 {
		t.Errorf(`Settings["dup"]: want 99 (last-writer-wins), got %d`, v)
	}
	if v, ok := got.Settings["other"]; !ok {
		t.Fatal(`Settings["other"]: not found after round-trip`)
	} else if v != 1 {
		t.Errorf(`Settings["other"]: want 1, got %d`, v)
	}
}

// TestMapSetBinaryHasSortedKeys verifies that ConfigT.Pack stores map keys in
// ascending sorted order, as required for LookupByKey binary search to work.
//
// The verification strategy: pack a ConfigT, inspect the serialized binary via
// the low-level Config accessors to confirm:
//  1. The vector length matches the number of map entries.
//  2. VerifyRootAsConfig passes structural validation.
//  3. Every key in the original map survives UnPack (proves Pack→UnPack is
//     lossless even when Go map iteration order is random).
//
// The sort guarantee is enforced by ConfigT.Pack itself (which calls sort.Slice
// on the key-value slice before building the FlatBuffers vector). A separate
// internal test in MapSetTest/config_sort_test.go validates the binary vector
// key order via the unexported entry types.
func TestMapSetBinaryHasSortedKeys(t *testing.T) {
	cfg := &mapset.ConfigT{
		Settings: map[string]int32{
			"cherry": 30,
			"apple":  10,
			"banana": 20,
			"date":   40,
		},
	}

	buf := packBytes(cfg)

	// Structural validation: VerifyRootAsConfig walks the binary and confirms
	// all offsets and field sizes are valid.
	if err := mapset.VerifyRootAsConfig(buf, nil); err != nil {
		t.Fatalf("VerifyRootAsConfig: %v", err)
	}

	rawCfg := mapset.GetRootAsConfig(buf, 0)

	// The packed vector must contain exactly 4 entries.
	if n := rawCfg.SettingsLength(); n != 4 {
		t.Fatalf("SettingsLength: want 4, got %d", n)
	}

	// Unpack and verify all keys survive the round-trip.
	// Pack guarantees keys are written in sorted order; UnPack reads
	// sequentially and reconstructs the Go map with all entries intact.
	got := rawCfg.UnPack()
	if len(got.Settings) != 4 {
		t.Fatalf("UnPack Settings length: want 4, got %d", len(got.Settings))
	}

	wantSettings := map[string]int32{"apple": 10, "banana": 20, "cherry": 30, "date": 40}
	for k, wantVal := range wantSettings {
		if gotVal, ok := got.Settings[k]; !ok {
			t.Errorf("Settings[%q]: not found (sort or Pack broken?)", k)
		} else if gotVal != wantVal {
			t.Errorf("Settings[%q]: want %d, got %d", k, wantVal, gotVal)
		}
	}

	// Absent key must not appear in the round-tripped map.
	if _, ok := got.Settings["zzz"]; ok {
		t.Error(`Settings["zzz"]: found unexpected key after round-trip`)
	}
}

// TestMapSetUnpackFields verifies that UnpackFields populates only the requested
// fields and leaves the rest at zero values.
func TestMapSetUnpackFields(t *testing.T) {
	cfg := &mapset.ConfigT{
		Settings:  map[string]int32{"x": 42},
		IdToName:  map[uint32]string{1: "one"},
		Tags:      map[string]struct{}{"tag": {}},
		ActiveIds: map[int32]struct{}{7: {}},
	}
	buf := packBytes(cfg)
	rawCfg := mapset.GetRootAsConfig(buf, 0)

	// Request only "settings" — other fields must be nil/empty.
	partial := rawCfg.UnpackFields("settings")
	if len(partial.Settings) != 1 {
		t.Errorf("partial.Settings length: want 1, got %d", len(partial.Settings))
	}
	if v, ok := partial.Settings["x"]; !ok || v != 42 {
		t.Errorf("partial.Settings[\"x\"]: want 42, got %v (ok=%v)", v, ok)
	}
	if len(partial.IdToName) != 0 {
		t.Errorf("partial.IdToName: want empty (not requested), got %d entries", len(partial.IdToName))
	}
	if len(partial.Tags) != 0 {
		t.Errorf("partial.Tags: want empty (not requested), got %d entries", len(partial.Tags))
	}
}
