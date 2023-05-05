#!/usr/bin/env python3

from cpp.generate import GenerateCpp
from csharp.generate import GenerateCSharp

# Run each language generation logic
GenerateCpp()
GenerateCSharp()

# TODO add other languages