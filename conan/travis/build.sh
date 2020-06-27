#!/bin/bash

set -e
set -x

if [[ "$(uname -s)" == 'Darwin' ]]; then
    if which pyenv > /dev/null; then
        eval "$(pyenv init -)"
    fi
    pyenv activate conan
fi

pip install flake8  # lint Python code for syntax errors and undefined names
flake8 . --count --select=E9,F63,F7,F82 --show-source --statistics
conan user
python conan/build.py
