#!/bin/sh

# Remove files and directory that are needed to build and run the .NET tests.
# The script NetTest.sh installs these as needed.

[ -d .dotnet_tmp ] && rm -rf .dotnet_tmp
[ -d .tmp ] && rm -rf .tmp
[ -d bin ] && rm -rf bin
[ -d obj ] && rm -rf obj
[ -f dotnet-install.sh ] && rm dotnet-install.sh
