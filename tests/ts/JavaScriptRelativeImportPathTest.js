import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, resolve } from "node:path";

const here = dirname(fileURLToPath(import.meta.url));
const headerPath = resolve(here, "relative_imports/transit/three/header.ts");

const contents = readFileSync(headerPath, "utf8");

const expectedImports = [
  "from '../one/info.js';",
  "from '../two/identity.js';",
];

for (const expected of expectedImports) {
  if (!contents.includes(expected)) {
    throw new Error(`Missing relative import "${expected}" in ${headerPath}`);
  }
}

const forbidden = "../transit/";
if (contents.includes(forbidden)) {
  throw new Error(
    `Found unexpected namespace segment in import path within ${headerPath}`
  );
}

console.log("JavaScriptRelativeImportPathTest: OK");
