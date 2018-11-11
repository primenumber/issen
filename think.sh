#!/bin/sh

SCRIPT_DIR=$(cd $(dirname $0); pwd)
export VAL_PATH=$SCRIPT_DIR/lsval
cd $SCRIPT_DIR
src/issen --think "$1" "$2" "$3"
