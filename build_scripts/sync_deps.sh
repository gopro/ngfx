#!/bin/bash
set -x
OS=$1
if [[ -z "$2" ]]; then
    EXTERNAL_DIR=$(dirname $0)/../external
else
    EXTERNAL_DIR=$2
fi
echo "OS: $OS"
echo "EXTERNAL DIR: $EXTERNAL_DIR"
mkdir -p $EXTERNAL_DIR
cd $EXTERNAL_DIR
TMP_DIR=/tmp/ngfx/external

wget -nc -q https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -P $TMP_DIR/json
mkdir -p json/include && cp -rf $TMP_DIR/json/* json/include/.

wget -nc -q https://raw.githubusercontent.com/nothings/stb/master/stb_image.h -P $TMP_DIR/stb
mkdir -p stb && cp $TMP_DIR/stb/stb_image.h stb/.

if [ $OS == "Windows" ]; then
    mkdir -p windows

    wget -nc -q https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip -P $TMP_DIR/glm
    (cd windows && unzip -q -o -d glm-0.9.9.8 $TMP_DIR/glm/glm-0.9.9.8.zip)

    wget -nc -q https://raw.githubusercontent.com/microsoft/DirectX-Graphics-Samples/v10.0.17763.0/Libraries/D3DX12/d3dx12.h -P $TMP_DIR/d3dx12
    mkdir -p windows/d3dx12 && cp $TMP_DIR/d3dx12/d3dx12.h windows/d3dx12/.

    wget -nc -q https://renderdoc.org/stable/1.11/RenderDoc_1.11_64.zip -P $TMP_DIR/renderdoc
    (cd windows && unzip -q -o $TMP_DIR/renderdoc/RenderDoc_1.11_64.zip)

    wget -nc -q https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip -P $TMP_DIR
    (cd windows && unzip -q -o $TMP_DIR/glfw-3.3.2.bin.WIN64.zip)

fi
