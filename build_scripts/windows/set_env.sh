#!/usr/bin/bash

if [[ ! -v VCPKG_DIR ]]; then
  export VCPKG_DIR='C:\vcpkg'
fi
CURRENT_DIR=`wslpath -wa .`
export OS=Windows
export PKG_CONFIG=$CURRENT_DIR'\external\windows\bin\pkgconf.exe'

export PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
export PKG_CONFIG_ALLOW_SYSTEM_LIBS=1
export PKG_CONFIG_DONT_DEFINE_PREFIX=1
export PKG_CONFIG_PATH=$VCPKG_DIR'\installed\x64-windows\lib\pkgconfig;'$CURRENT_DIR'\external\windows\lib\pkgconfig'

export WSLENV=PKG_CONFIG/w:PKG_CONFIG_PATH/w:PKG_CONFIG_ALLOW_SYSTEM_LIBS/w:PKG_CONFIG_ALLOW_SYSTEM_CFLAGS/w:PKG_CONFIG_DONT_DEFINE_PREFIX/w
