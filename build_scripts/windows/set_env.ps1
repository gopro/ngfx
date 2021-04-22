#Set-PSDebug -Trace 1
if (-not (Test-Path env:VCPKG_DIR)) {
    $env:VCPKG_DIR = 'C:\vcpkg'
}

$env:OS='Windows'
$CURRENT_DIR=$PWD.tostring()
$env:PKG_CONFIG=$CURRENT_DIR+'\external\windows\bin\pkgconf.exe'

$env:PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
$env:PKG_CONFIG_ALLOW_SYSTEM_LIBS=1
$env:PKG_CONFIG_DONT_DEFINE_PREFIX=1
$env:PKG_CONFIG_PATH=$VCPKG_DIR+'\installed\x64-windows\lib\pkgconfig;'+$CURRENT_DIR+'\external\windows\lib\pkgconfig'
