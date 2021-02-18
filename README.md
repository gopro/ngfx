# NGFX Graphics Framework

NGFX is a low level graphics framework, providing an abstraction API on top of Vulkan, DirectX12, and Metal.

It exposes the benefits of next-generation graphics technology via a common platform abstraction API.  

It also supports optional access to the backend data structures, enabling platform specific optimizations.

## Build Instructions

### 1) Install Dependencies

#### 1.1) Install Platform-Specific Dependencies

*On Windows 10 (or newer):*

  Install Microsoft Visual Studio 2019 (or newer)

  Install Windows Subsystem for Linux (Ubuntu 20.04.1 LTS or newer)
  - Note: Git Bash or MSYS2 should work too

*On Linux (Ubuntu 20.04.1 or newer, other Linux distros should work too):*

  Install GNU Toolchain (apt install build-essential)

*On Mac:*

  Install XCode

#### 1.2) Install Common System Dependencies

Install CMake 3.19.1 (or newer)

#### 1.3) Install Third-Party Dependencies

*ngfx has the following dependencies:*

- nlohmann/json 3.9.1 (or newer)
- stb_image 2.26 (or newer)
- glm 0.9.9.8 (or newer)
- RenderDoc 1.11_64 (or newer, on Windows and Linux)
- d3dx12.h v10.0.17763.0 (or newer, on Windows)
- glfw 3.3.2 (or newer, on Windows and Linux)
- shaderc v2020.4 (or newer)
- spirv-cross 2021-01-15 (or newer)
- vulkan sdk 1.2.162.1 (or newer, if using Vulkan backend)

We provide a helper script to help install 3rd-party dependencies

bash build_scripts/sync_deps.sh *platform*
- *platform*: Windows, Linux, or Darwin (Mac OS)

### 2) Configure

cmake.exe -H. -B*build_folder* -DCMAKE_BUILD_TYPE=*build_type* -D*ngfx_graphics_backend*=ON -G *cmake_generator*
- *build_folder*: the build output folder (e.g. cmake-build-debug, or cmake-build-release)
- *build type*: Debug or Release
- *ngfx_graphics_backend*: the graphics backend (NGFX_GRAPHICS_BACKEND_VULKAN, NGFX_GRAPHICS_BACKEND_METAL, or NGFX_GRAPHICS_BACKEND_DIRECT3D12)
- *cmake_generator*: CMake generator (e.g. Visual Studio 16 2019)

Example:

cmake.exe -H. -Bcmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DNGFX_GRAPHICS_BACKEND_DIRECT3D12=ON

### 3) Build

From command line: 

cmake.exe --build *build_folder*

Alternatively, can build from IDE

### 4) Install

cmake.exe --install *build_folder* --config *build_type* --prefix *install_prefix*
- *install_prefix*: The install prefix

## Contributing

### Coding Style Guidelines

We plan to use clang-format to automatically enforce coding style.
We will adopt llvm coding style (the default option in clang-format), with one difference:
variable names should begin with lowercase.
The benefit is that we can reuse the type name as a variable name, 
(for example: Matrix matrix).  

The LLVM coding style can be referenced here: See https://llvm.org/docs/CodingStandards.html  
There is also a proposal to modify the variable naming rule in llvm coding style 
to support lowercase, see https://www.llvm.org/docs/Proposals/VariableNames.html#lldb

We plan to use doxygen annotations to generate API documentation.
