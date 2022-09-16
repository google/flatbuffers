import glob
import os
import shutil
import subprocess
import sys
from pathlib import Path

test_nim_dir = Path(__file__).parent
test_dir = test_nim_dir.parent
generated_dir = test_nim_dir / "generated"
scripts_dir = test_dir.parent / "scripts"
sys.path.insert(0, str(scripts_dir))
from util import flatc


def generate_mygame():
    flatc(
        [
            "--nim",
            "--gen-mutable",
        ],
        include=str(test_dir / "include_test"),
        schema=test_dir / "monster_test.fbs",
        cwd=Path(generated_dir),
    )


def generate_optional_scalars():
    flatc(
        [
            "--nim",
        ],
        schema=test_dir / "optional_scalars.fbs",
        cwd=Path(generated_dir),
    )


def generate_moredefaults():
    flatc(
        [
            "--nim",
        ],
        schema=test_dir / "more_defaults.fbs",
        cwd=Path(generated_dir),
    )


def generate_mutatingbool():
    flatc(
        [
            "--nim",
            "--gen-mutable",
        ],
        schema=test_dir / "MutatingBool.fbs",
        cwd=Path(generated_dir),
    )


def main():
    generated_dir.mkdir(exist_ok=True)
    try:
        generate_mygame()
        generate_optional_scalars()
        generate_moredefaults()
        generate_mutatingbool()
        subprocess.check_call("testament --megatest:off all".split())
    finally:
        shutil.rmtree(generated_dir)
        shutil.rmtree(test_nim_dir / "nimcache")
        shutil.rmtree(test_nim_dir / "testresults")
        for f in glob.glob(str(test_nim_dir / "tests" / "*" / "test")):
            os.remove(f)


if __name__ == "__main__":
    main()
