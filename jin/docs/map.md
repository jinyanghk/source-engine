decompile bsp to vmf

```sh
./bspsrc.sh
```

remove `_d` in the file name

```sh
for f in ./*; do mv "$f" "${f%_d*.vmf}.vmf" ; done
```

compile maps

```sh
cd vmf/hl2

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH find ../../maps/hl2 -name *.vmf -exec ./vbsp -game hl2 "{}" \;

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH find ../../maps/hl2 -name *.bsp ! -name background04.bsp -exec ./vvis -game hl2 "{}" \;

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH find ../../maps/hl2 -name *.bsp -exec ./vrad -game hl2 "{}" \;

mv *.bsp ../../bsp/hl2
mv *.prt ../../prt/hl2
mv *.log ../../log/hl2
```

backup existing maps folder

```sh
mv maps maps_backup
```

