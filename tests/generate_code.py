#!/usr/bin/env python3

import subprocess
import os
from pathlib import Path

script_path = Path(__file__).parent.resolve()
flatc_path = os.path.join(script_path.parent.absolute(), "flatc")

def flatc(*args):
    subprocess.run([flatc_path, args], cwd=script_path)

flatc(["--csharp", "--gen-object-api", "optional_scalars.fbs"])
