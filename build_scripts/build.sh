#!/usr/bin/bash
set -x

if [[ ! -v OS ]]; then
  echo "ERROR: OS undefined"
  exit 1
fi

if [ $OS == "Windows" ]; then
CMAKE=cmake.exe
else
CMAKE=cmake
fi

$CMAKE --build cmake-build-debug --config Debug -j8
