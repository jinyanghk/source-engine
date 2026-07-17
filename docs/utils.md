
### offical doc for map compiling

https://developer.valvesoftware.com/wiki/Map_Compiling_Theory


### sortie's fixes

https://github.com/ValveSoftware/source-sdk-2013/pull/160


### build

```sh
python3 ./waf configure -T debug --prefix=hl2 --utils --disable-warns

python3 ./waf build -v 

python3 ./waf install
```

### wsl core dump

location: %TEMP%\wsl-crashes

### some files need to be updated from official sdk, e.g. staticprop.cpp

https://github.com/ValveSoftware/source-sdk-2013

### strlwr

`_strlwr` is used in many places, need to implement linux version 

```sh
g++ -c utils/common/strlwr.cpp -MMD -U_FORTIFY_SOURCE -fno-strict-aliasing -fvisibility=hidden -g -O0 -pipe -fPIC -pthread -march=core2 -mfpmath=sse -std=c++11 -fpermissive -w -o build/utils/common/strlwr.o
```
