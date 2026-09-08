
```sh
LD_LIBRARY_PATH ./glview
Usage: glview [-portal] [-disp] <filename.gl>
```


```sh
LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vbsp -game hl2 -glview ../../d1_town_01_d.vmf

cp ../../d1_town_01_d.gl .
cp ../../d1_town_01_d.prt .

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./glview -game hl2 d1_town_01_d.gl

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./glview -game hl2 -portal d1_town_01_d.gl
```