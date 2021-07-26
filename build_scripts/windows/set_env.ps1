#Set-PSDebug -Trace 1
$env:OS='Windows'
$CURRENT_DIR=$PWD.tostring()
$env:PKG_CONFIG=$CURRENT_DIR+'\external\windows\bin\pkgconf.exe'

$env:PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
$env:PKG_CONFIG_ALLOW_SYSTEM_LIBS=1
$env:PKG_CONFIG_DONT_DEFINE_PREFIX=1
$env:PKG_CONFIG_PATH=$CURRENT_DIR+'\external\windows\lib\pkgconfig'
