#!/bin/bash
set -x

if [[ ! -v OS ]]; then
  echo "ERROR: OS undefined"
  exit 1
fi

echo "OS: $OS"

if [ $OS == "Windows" ]; then
cmake.exe -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -G"Visual Studio 16 2019" -DNGFX_GRAPHICS_BACKEND_DIRECT3D12=ON -DVCPKG_DIR='C:\vcpkg' -DCMAKE_SYSTEM_VERSION=10.0.18362.0 -DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreadedDLL
elif [ $OS == "Linux" ]; then
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -GNinja -DNGFX_GRAPHICS_BACKEND_VULKAN=ON
elif [ $OS == "Darwin" ]; then
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -GXcode -DNGFX_GRAPHICS_BACKEND_METAL=ON
fi

