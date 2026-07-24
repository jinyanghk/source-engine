#!/bin/sh

CRASH=/mnt/c/Users/46172/AppData/Local/Temp/wsl-crashes

DMP=$(ls -t $CRASH | head -1)

gdb ../hl2/vbsp $CRASH/$DMP
