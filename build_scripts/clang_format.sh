#!/usr/bin/bash
set -x
find src -name '*.c' -o -name '*.cpp' -o -name '*.h' | xargs -I{} clang-format -i {}
