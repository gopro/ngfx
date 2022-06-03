import os
import os.path as op
import subprocess
import sys

ENV_PARAMS = {}

def env(key, default = None):
    output = os.getenv(key, default)
    ENV_PARAMS[key] = output
    return output

def err(msg, returncode = 1):
    print('ERROR: ' + msg, file=sys.stderr)
    exit(returncode)

class shell:
    def __init__(self, cmd):
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        output = ''
        for line in iter(p.stdout.readline, b''):
            line = line.decode('utf-8')
            print(line.rstrip())
            output += line
        p.wait()
        if p.returncode != 0:
            err(cmd, p.returncode)
        self.output = output
        self.p = p

def read_file(path):
    fp = open(path)
    contents = fp.read()
    fp.close()
    return contents

def write_file(path, contents, mode = 'w+'):
    fp = open(path, mode)
    fp.write(contents)
    fp.close()

OS = env('OS')
if not OS:
    err('OS is undefined')
OS_LOWER_CASE = OS.lower()

EXTERNAL_DIR = env('EXTERNAL_DIR', 'external')
if not op.isdir(EXTERNAL_DIR):
    os.mkdir(EXTERNAL_DIR)
SCRIPT_DIR = sys.path[0]
BUILD_TYPE = env('BUILD_TYPE', 'Debug')
BUILDDIR = env('BUILDDIR', op.join(os.getcwd(), 'builddir'))
INSTALL_PREFIX = env('INSTALL_PREFIX', op.join(os.getcwd(), 'install'))
INSTALL_INCLUDEDIR = env('INSTALL_INCLUDEDIR', 'include')
INSTALL_LIBDIR = env('INSTALL_LIBDIR', 'lib')

CMAKE_GENERATOR = env('CMAKE_GENERATOR', { 'Windows': 'Visual Studio 16 2019', 'Linux': 'Ninja', 'Darwin': 'Xcode' }[OS])
NGFX_GRAPHICS_BACKEND = env('NGFX_GRAPHICS_BACKEND', { 'Windows': 'Direct3D12', 'Linux': 'Vulkan', 'Darwin': 'Metal' }[OS].upper())
CMAKE = 'cmake.exe' if OS == 'Windows' else 'cmake'
if OS == 'Windows':
    env('CMAKE_GENERATOR', 'Visual Studio 16 2019')
    CMAKE_SYSTEM_VERSION = env('CMAKE_SYSTEM_VERSION', '10.0.22000.0')
    env('SHADERC_VERSION', '2020.3')
    VCPKG_DIR = env('VCPKG_DIR', 'C:\\vcpkg')
    if not op.isdir(VCPKG_DIR):
        err('{VCPKG_DIR} doesn\'t exist')
    env('VCPKG_TRIPLET', 'x64-windows')
    env('VULKAN_SDK_VERSION', '1.2.162.1')
    env('PKGCONF_VERSION', '1.7.4')
    CMAKE_MSVC_RUNTIME_LIBRARY = env('CMAKE_MSVC_RUNTIME_LIBRARY', 'MultiThreadedDLL')
    PREFIX = op.join(os.getcwd(), EXTERNAL_DIR, 'windows')
    env('PREFIX', PREFIX)
elif OS == 'Linux':
    env('VULKAN_SDK_VERSION', '1.2.170')
elif OS == 'Darwin':
    env('SHADERC_VERSION', '2020.3')
env('UNZIP', 'unzip -q -n')
env('WGET', 'iwr' if OS == 'Windows' else 'wget -nc -q')
env('MKDIR', 'ni -ea 0 -ItemType Directory' if OS == 'Windows' else 'mkdir')
