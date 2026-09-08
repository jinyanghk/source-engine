

```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vbsp -game hl2
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
filesystem BaseDir: /home/jin/source-engine/hl2
Valve Software - vbsp.exe (Sep  7 2026)
Command line: "./vbsp" "-game" "hl2"

usage  : vbsp [options...] mapfile
example: vbsp -onlyents c:\hl2\hl2\maps\test

Common options (use -v to see all options):

  -v (or -verbose): Turn on verbose output (also shows more command
                    line options).

  -onlyents   : This option causes vbsp only import the entities from the .vmf
                file. -onlyents won't reimport brush models.
  -onlyprops  : Only update the static props and detail props.
  -glview     : Writes .gl files in the current directory that can be viewed
                with glview.exe. If you use -tmpout, it will write the files
                into the \tmp folder.
  -nodetail   : Get rid of all detail geometry. The geometry left over is
                what affects visibility.
  -nowater    : Get rid of water brushes.
  -low        : Run as an idle-priority process.
  -embed <directory>  : Use <directory> as an additional search path for assets
                        and embed all assets in this directory into the compiled
                        map

  -vproject <directory> : Override the VPROJECT environment variable.
  -game <directory>     : Same as -vproject.
```

### BSPSrc

https://github.com/ata4/bspsrc

need to use bspsrc to generate vmf from bsp file

### need to merge sortie's fixes for linux build

https://github.com/ValveSoftware/source-sdk-2013/pull/160


```sh
cd hl2

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vbsp -game hl2 ../../d1_town_01_d.vmf
```
