FlatBuffers Benchmark code for C++
==================================

This sits in a separate "benchmarks" branch because it is unfinished code that has
several issues:

* It refers to several other serialization projects as submodules, and we don't want to add
  these as dependencies to the main project.
* The code is "thrown together" just to get results out of it, and does not represent the same
  code quality the main project strives for.
* The current code is not portable. It only comes with a Visual Studio 2010 project, and
  makes use of Windows timing functions. This is probably easy to fix.

The code is here such that those wishing to work on benchmarking have something to work with.
Any clean-up of this code would be greatly appreciated.

Notes on building the current implementation:
* After switching to branch "benchmarks" be sure to do a:
  git submodule init
  git submodule update
* Only Release/x64 configuration provided.
