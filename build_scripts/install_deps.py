#!/usr/bin/python3
import os
import os.path as op
import subprocess
import sys

def err(msg):
    print('ERROR: ' + msg, file=sys.stderr)

def shell(cmd):
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    output = ''
    for line in iter(p.stdout.readline, b''):
        line = line.decode('utf-8')
        print(line.rstrip())
        output += line
    return output

def read_file(path):
    fp = open(path)
    contents = fp.read()
    fp.close()
    return contents

def write_file(path, contents, mode = 'w+'):
    fp = open(path, mode)
    fp.write(contents)
    fp.close()

env_params = {}

def env(key, default = None):
    output = os.getenv(key, default)
    env_params[key] = output
    return output

OS = env('OS')
if not OS:
    err('OS is undefined')
OS_LOWER_CASE = OS.lower()

EXTERNAL_DIR = env('EXTERNAL_DIR', 'external')

SCRIPT_DIR = sys.path[0]

if not op.isdir(EXTERNAL_DIR):
    os.mkdir(EXTERNAL_DIR)
os.chdir(EXTERNAL_DIR)

if OS == 'Windows':
    env('CMAKE_GENERATOR', '"Visual Studio 16 2019"')
    env('CMAKE_SYSTEM_VERSION', '10.0.18362.0')
    env('SHADERC_VERSION', '2020.3')
    VCPKG_DIR = env('VCPKG_DIR', 'C:\\vcpkg')
    VCPKG_DIR_WSL = shell(f'wslpath -a "{VCPKG_DIR}"').rstrip()
    env_params['VCPKG_DIR_WSL'] = VCPKG_DIR_WSL
    if not op.isdir(VCPKG_DIR_WSL):
        err('{VCPKG_DIR_WSL} doesn\'t exist')
    env('VCPKG_TRIPLET', 'x64-windows')
    env('VULKAN_SDK_VERSION', '1.2.162.1')
    env('PKGCONF_VERSION', '1.7.4')
    PREFIX = shell('wslpath -wa .').rstrip() + '\windows'
    env_params['PREFIX'] = PREFIX
elif OS == 'Linux':
    env('VULKAN_SDK_VERSION', '1.2.170')
elif OS == 'Darwin':
    env('SHADERC_VERSION', '2020.3')

PKGS = env('PKGS', 'all')

env_params['UNZIP'] = 'unzip -q -n'
env_params['WGET'] = 'wget -nc -q'

ninja_template = read_file(f'{SCRIPT_DIR}/install_deps_{OS_LOWER_CASE}.ninja.in')
write_file(f'install_deps_{OS_LOWER_CASE}.ninja', ninja_template.format(**env_params))
for key, val in env_params.items():
    print(f'{key} = {val}')
shell(f'ninja -j 1 -f install_deps_{OS_LOWER_CASE}.ninja {PKGS}')
