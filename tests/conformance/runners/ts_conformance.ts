/**
 * TypeScript conformance runner for cross-language verifier testing.
 *
 * Reads all .bin files from the conformance corpus, runs the FlatBuffers
 * verifier on each, and outputs JSON results for comparison.
 *
 * Usage: npx ts-node tests/conformance/runners/ts_conformance.ts [corpus_dir]
 */

import * as fs from 'fs';
import * as path from 'path';
import {Verifier, VerifierOptions, VerificationError, ErrorKind} from '../../../ts/verifier.js';

interface Result {
  accept: boolean;
  error_kind?: string;
}

const DEFAULT_OPTS: VerifierOptions = {
  maxDepth: 64,
  maxTables: 1_000_000,
  maxApparentSize: 1_073_741_824,
};

function verifyBuffer(buf: Uint8Array): Result {
  try {
    const view = new DataView(buf.buffer, buf.byteOffset, buf.byteLength);
    const verifier = new Verifier(view, DEFAULT_OPTS);

    // Read root offset and verify basic table structure
    const tablePos = verifier.readUint32(0);
    verifier.checkTable(tablePos);

    return {accept: true};
  } catch (e: unknown) {
    const err = e as VerificationError;
    return {
      accept: false,
      error_kind: err.kind ?? 'Unknown',
    };
  }
}

function runConformance(corpusDir: string): Record<string, Result> {
  const results: Record<string, Result> = {};
  const categories = ['valid', 'malicious', 'edge'];

  for (const category of categories) {
    const dir = path.join(corpusDir, category);
    if (!fs.existsSync(dir)) continue;

    const entries = fs.readdirSync(dir)
      .filter(f => f.endsWith('.bin'))
      .sort();

    for (const entry of entries) {
      const filePath = path.join(dir, entry);
      const buf = new Uint8Array(fs.readFileSync(filePath));
      const key = `${category}/${entry}`;
      results[key] = verifyBuffer(buf);
    }
  }

  return results;
}

// Main
const corpusDir = process.argv[2] ?? 'tests/conformance/corpus';
const results = runConformance(corpusDir);
console.log(JSON.stringify(results, null, 4));
