package flatbuffers

import (
	"os"
	"path/filepath"
	"testing"
)

// FuzzVerifier feeds arbitrary byte sequences to the verifier.
// The verifier must never panic regardless of input.
func FuzzVerifier(f *testing.F) {
	// Seed with the golden monster test buffer
	goldenPath := filepath.Join("..", "tests", "monsterdata_test.golden")
	if golden, err := os.ReadFile(goldenPath); err == nil {
		f.Add(golden)
	}

	// Seed with known-bad patterns
	f.Add([]byte{})                                               // empty
	f.Add([]byte{0x00, 0x00, 0x00, 0x00})                         // zero root offset
	f.Add([]byte{0xFF, 0xFF, 0xFF, 0xFF})                         // max root offset
	f.Add([]byte{0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) // minimal valid-ish

	// Seed with corpus files if available
	corpusDir := filepath.Join("..", "tests", "conformance", "corpus")
	for _, category := range []string{"valid", "malicious", "edge"} {
		dir := filepath.Join(corpusDir, category)
		entries, err := os.ReadDir(dir)
		if err != nil {
			continue
		}
		for _, entry := range entries {
			if filepath.Ext(entry.Name()) == ".bin" {
				if data, err := os.ReadFile(filepath.Join(dir, entry.Name())); err == nil {
					f.Add(data)
				}
			}
		}
	}

	f.Fuzz(func(t *testing.T, data []byte) {
		opts := &VerifierOptions{
			MaxDepth:        64,
			MaxTables:       1000,
			MaxApparentSize: len(data) + 1024,
		}
		v := NewVerifier(data, opts)

		// Must not panic
		tablePos, err := v.CheckUOffsetT(0)
		if err != nil {
			return
		}
		_ = v.CheckTable(int(tablePos))
	})
}
