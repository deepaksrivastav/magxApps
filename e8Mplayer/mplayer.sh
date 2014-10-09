#!/bin/bash
mypath=${0%/*}
source /etc/profile
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$mypath

cd $mypath
./e8mplayer