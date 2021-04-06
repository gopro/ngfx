#!/usr/bin/bash
set -x

if [[ ! -v EXTERNAL_DIR ]]; then
  export EXTERNAL_DIR=external
fi

mkdir $EXTERNAL_DIR
cd $EXTERNAL_DIR

make -f ../build_scripts/install_deps.in
