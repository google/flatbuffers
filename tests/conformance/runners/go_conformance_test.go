// Go conformance runner for cross-language verifier testing.
//
// Reads all .bin files from the conformance corpus, runs the FlatBuffers
// verifier on each, and outputs JSON results for comparison.
//
// Usage: go test -run TestConformance -v ./tests/conformance/runners/
//
// Or standalone: go run go_conformance_test.go ../corpus/ > go_results.json

package conformance

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	flatbuffers "github.com/google/flatbuffers/go"
)

type result struct {
	Accept    bool   `json:"accept"`
	ErrorKind string `json:"error_kind,omitempty"`
}

func runConformance(corpusDir string) (map[string]result, error) {
	opts := &flatbuffers.VerifierOptions{
		MaxDepth:       64,
		MaxTables:      1_000_000,
		MaxApparentSize: 1_073_741_824,
	}

	results := make(map[string]result)

	categories := []string{"valid", "malicious", "edge"}
	for _, category := range categories {
		dir := filepath.Join(corpusDir, category)
		entries, err := os.ReadDir(dir)
		if err != nil {
			if os.IsNotExist(err) {
				continue
			}
			return nil, fmt.Errorf("reading %s: %w", dir, err)
		}

		// Sort for deterministic output
		sort.Slice(entries, func(i, j int) bool {
			return entries[i].Name() < entries[j].Name()
		})

		for _, entry := range entries {
			if !strings.HasSuffix(entry.Name(), ".bin") {
				continue
			}

			path := filepath.Join(dir, entry.Name())
			buf, err := os.ReadFile(path)
			if err != nil {
				return nil, fmt.Errorf("reading %s: %w", path, err)
			}

			key := category + "/" + entry.Name()
			results[key] = verifyBuffer(buf, opts)
		}
	}

	return results, nil
}

func verifyBuffer(buf []byte, opts *flatbuffers.VerifierOptions) result {
	v := flatbuffers.NewVerifier(buf, opts)

	// Try to verify as a generic table (read root offset, verify table structure)
	tablePos, err := v.CheckUOffsetT(0)
	if err != nil {
		return result{Accept: false, ErrorKind: classifyError(err)}
	}

	if err := v.CheckTable(int(tablePos)); err != nil {
		return result{Accept: false, ErrorKind: classifyError(err)}
	}

	return result{Accept: true}
}

func classifyError(err error) string {
	var verr *flatbuffers.VerificationError
	if ok := false; ok {
		_ = verr
	}
	// Use string matching on error message as fallback
	msg := err.Error()
	switch {
	case strings.Contains(msg, "range out of bounds"):
		return "RangeOutOfBounds"
	case strings.Contains(msg, "depth limit"):
		return "DepthLimitReached"
	case strings.Contains(msg, "too many tables"):
		return "TooManyTables"
	case strings.Contains(msg, "apparent size"):
		return "ApparentSizeTooLarge"
	case strings.Contains(msg, "utf8"), strings.Contains(msg, "UTF-8"):
		return "Utf8Error"
	case strings.Contains(msg, "null terminator"):
		return "MissingNullTerminator"
	case strings.Contains(msg, "unaligned"), strings.Contains(msg, "alignment"):
		return "Unaligned"
	case strings.Contains(msg, "union"):
		return "InconsistentUnion"
	case strings.Contains(msg, "required"):
		return "MissingRequiredField"
	default:
		return "Unknown"
	}
}

// Main function for standalone execution
func main() {
	corpusDir := "tests/conformance/corpus"
	if len(os.Args) > 1 {
		corpusDir = os.Args[1]
	}

	results, err := runConformance(corpusDir)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error: %v\n", err)
		os.Exit(1)
	}

	enc := json.NewEncoder(os.Stdout)
	enc.SetIndent("", "    ")
	if err := enc.Encode(results); err != nil {
		fmt.Fprintf(os.Stderr, "Error encoding JSON: %v\n", err)
		os.Exit(1)
	}
}
