#!/bin/sh

STARTDIR=/mmc/
export DIRNAME=`busybox dirname "$0"`
cd $STARTDIR
exec "$DIRNAME"/filebrowser
