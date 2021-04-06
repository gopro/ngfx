#!/usr/bin/bash
set -x

if [[ ! -v EXTERNAL_DIR ]]; then
  export EXTERNAL_DIR=external
fi
SCRIPT_DIR=$(dirname $(realpath $0))
echo $SCRIPT_DIR

mkdir -p $EXTERNAL_DIR
cd $EXTERNAL_DIR

make -f $SCRIPT_DIR/install_deps.in
