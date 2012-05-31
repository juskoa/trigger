#!/bin/bash

echo "Running parted ..."
TRIGGER_DIR="$(dirname $(dirname $(readlink -m $0)))"

export VMEBDIR=$TRIGGER_DIR/v/vmeb
export VMECFDIR=$TRIGGER_DIR/v/vme
export dbctp=$TRIGGER_DIR/v/vme/CFG/ctp/DB
export PYTHONPATH="$VMEBDIR"

PARTITION_FILE="example"
test -n "$1" && PARTITION_FILE="$1"

$VMECFDIR/TRG_DBED/parted.py $PARTITION_FILE
