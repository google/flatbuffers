import os
import subprocess
import tempfile
import unittest

class TestBfbsGeneratorNamespaceOutput(unittest.TestCase):
    def test_lua_generator_respects_output_path_with_namespace_first_line(self):
        # Schema that starts with a namespace on the first line (no leading comments)
        schema = """namespace My.Game;

table Monster {
  id:int;
}

root_type Monster;
"""
        with tempfile.TemporaryDirectory() as tmpdir:
            schema_path = os.path.join(tmpdir, "schema.fbs")
            with open(schema_path, "w") as f:
                f.write(schema)

                # Run the built flatc binary. Locate repository root relative to
                # this test file so the test works regardless of the current
                # working directory used by the test runner.
                repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
                flatc_bin = os.path.join(repo_root, "build", "flatc")
                cmd = [flatc_bin, "--lua", "-o", tmpdir, schema_path]
            res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(res.returncode, 0, f"flatc failed: {res.stderr.decode()}")

            # Expected generated file should be under tmpdir/My/Game/Monster.lua
            expected_path = os.path.join(tmpdir, "My", "Game", "Monster.lua")
            self.assertTrue(os.path.exists(expected_path), f"Expected file not found: {expected_path}")

    def test_nim_generator_respects_output_path_with_namespace_first_line(self):
        # Same schema, but ensure the Nim bfbs generator writes under the
        # configured output path.
        schema = """namespace My.Game;

table Monster {
  id:int;
}

root_type Monster;
"""
        with tempfile.TemporaryDirectory() as tmpdir:
            schema_path = os.path.join(tmpdir, "schema.fbs")
            with open(schema_path, "w") as f:
                f.write(schema)

            repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
            flatc_bin = os.path.join(repo_root, "build", "flatc")
            cmd = [flatc_bin, "--nim", "-o", tmpdir, schema_path]
            res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            self.assertEqual(res.returncode, 0, f"flatc failed: {res.stderr.decode()}")

            expected_path = os.path.join(tmpdir, "My", "Game", "Monster.nim")
            self.assertTrue(os.path.exists(expected_path), f"Expected file not found: {expected_path}")

if __name__ == '__main__':
    unittest.main()
