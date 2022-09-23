import glob
import os
import platform
import shutil
import subprocess
from pathlib import Path

test_nim_dir = Path(__file__).parent
test_dir = test_nim_dir.parent
generated_dir = test_nim_dir / "generated"
# Get the root path as an absolute path, so all derived paths are absolute.
root_path = test_dir.parent.absolute()

# Windows works with subprocess.run a bit differently.
is_windows = platform.system() == "Windows"
# Get the location of the flatc executable
flatc_exe = Path("flatc.exe" if is_windows else "flatc")
# Find and assert flatc compiler is present.
if root_path in flatc_exe.parents:
    flatc_exe = flatc_exe.relative_to(root_path)
flatc_path = Path(root_path, flatc_exe)
assert flatc_path.exists(), "Cannot find the flatc compiler " + str(flatc_path)


def check_call(args, cwd=test_nim_dir):
    subprocess.check_call(args, cwd=str(cwd), shell=is_windows)


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
