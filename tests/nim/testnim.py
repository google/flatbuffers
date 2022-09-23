import glob
import os
import platform
import shutil
import subprocess
from pathlib import Path

test_nim_dir = Path(__file__).absolute().parent
test_dir = test_nim_dir.parent
generated_dir = test_nim_dir / "generated"
root_path = test_dir.parent
flatc_exe = Path("flatc.exe" if  platform.system() == "Windows" else "flatc")
flatc_path = root_path / flatc_exe
assert flatc_path.exists(), "Cannot find the flatc compiler " + str(flatc_path)

# Execute the flatc compiler with the specified parameters
def flatc(options, schema, prefix=None, include=None, data=None, cwd=root_path):
    cmd = [str(flatc_path)] + options
    if prefix:
        cmd += ["-o"] + [prefix]
    if include:
        cmd += ["-I"] + [include]
    if isinstance(schema, Path):
        cmd += [str(schema)]
    elif isinstance(schema, str):
        cmd += [schema]
    else:
        cmd += schema
    if data:
        cmd += [data] if isinstance(data, str) else data
    return subprocess.check_call(cmd, cwd=str(cwd))


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
