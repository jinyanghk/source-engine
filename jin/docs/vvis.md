
```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vvis -game hl2
LoadLibrary: pModule: vvis_dll.dll, path: /home/jin/source-engine/hl2/bin/libvvis_dll.so
Valve Software - vvis.exe (Sep  7 2026)
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
filesystem BaseDir: /home/jin/source-engine/hl2
Command line: "./vvis" "-game" "hl2"

usage  : vvis [options...] bspfile
example: vvis -fast c:\hl2\hl2\maps\test

Common options:

  -v (or -verbose): Turn on verbose output (also shows more command
  -fast           : Only do first quick pass on vis calculations.
  -mpi            : Use VMPI to distribute computations.
  -low            : Run as an idle-priority process.
                    env_fog_controller specifies one.

  -vproject <directory> : Override the VPROJECT environment variable.
  -game <directory>     : Same as -vproject.

Other options:
  -novconfig      : Don't bring up graphical UI on vproject errors.
  -radius_override: Force a vis radius, regardless of whether an
  -mpi_pw <pw>    : Use a password to choose a specific set of VMPI workers.
  -threads        : Control the number of threads vbsp uses (defaults to the #
                    or processors on your machine).
  -nosort         : Don't sort portals (sorting is an optimization).
  -tmpin          : Make portals come from \tmp\<mapname>.
  -tmpout         : Make portals come from \tmp\<mapname>.
  -trace <start cluster> <end cluster> : Writes a linefile that traces the vis from one cluster to another for debugging map vis.
  -FullMinidumps  : Write large minidumps on crash.
  -x360            : Generate Xbox360 version of vsp
  -nox360                  : Disable generation Xbox360 version of vsp (default)

```


```sh
cd hl2

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vvis -game hl2 ../../d1_town_01_d.bsp
```

taking very long time ...

* hl2/maps/background04.bsp 

slow but able to complete

* hl2/maps/d1_eli_01.bsp
* hl2/maps/d2_coast_12.bsp