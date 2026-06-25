package flatbuffers

import (
	"os"
	"path/filepath"
	"testing"
)

func loadGoldenBuffer(b *testing.B) []byte {
	b.Helper()
	path := filepath.Join("..", "tests", "monsterdata_test.golden")
	data, err := os.ReadFile(path)
	if err != nil {
		b.Skipf("Golden buffer not found: %v", err)
	}
	return data
}

func BenchmarkVerifierValidBuffer(b *testing.B) {
	buf := loadGoldenBuffer(b)
	opts := &VerifierOptions{}

	b.ResetTimer()
	b.ReportAllocs()
	for b.Loop() {
		v := NewVerifier(buf, opts)
		tablePos, err := v.CheckUOffsetT(0)
		if err != nil {
			b.Fatal(err)
		}
		if err := v.CheckTable(int(tablePos)); err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkVerifierInvalidImmediate(b *testing.B) {
	buf := []byte{0xFF, 0xFF, 0xFF, 0xFF}
	opts := &VerifierOptions{}

	b.ResetTimer()
	b.ReportAllocs()
	for b.Loop() {
		v := NewVerifier(buf, opts)
		_, _ = v.CheckUOffsetT(0)
	}
}

func BenchmarkVerifierZeroAlloc(b *testing.B) {
	buf := loadGoldenBuffer(b)
	opts := &VerifierOptions{}

	allocs := testing.AllocsPerRun(100, func() {
		v := NewVerifier(buf, opts)
		tablePos, _ := v.CheckUOffsetT(0)
		_ = v.CheckTable(int(tablePos))
	})

	if allocs > 0 {
		b.Errorf("Happy path allocated %v times — should be zero", allocs)
	}
}
