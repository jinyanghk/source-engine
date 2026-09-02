#!/bin/sh

CRASH=/mnt/c/Users/46172/AppData/Local/Temp/wsl-crashes

DMP=$(ls -t $CRASH | head -1)

#gdb ../../hl2/vrad $CRASH/$DMP
#gdb ../../hl2/vvis $CRASH/$DMP
#gdb ../../hl2/vbsp $CRASH/$DMP
#gdb ../../hl2/studiomdl $CRASH/$DMP
#gdb ../../hl2/hl2_launcher $CRASH/$DMP
gdb ../../hl2/hammer $CRASH/$DMP
