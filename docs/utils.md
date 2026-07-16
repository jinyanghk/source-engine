
https://developer.valvesoftware.com/wiki/Map_Compiling_Theory



some files need to be updated from official sdk, e.g. staticprop.cpp

https://github.com/ValveSoftware/source-sdk-2013

### strlwr

`_strlwr` is used in many places, need to implement linux version 

```sh
g++ -c utils/common/strlwr.cpp -MMD -U_FORTIFY_SOURCE -fno-strict-aliasing -fvisibility=hidden -g -O0 -pipe -fPIC -pthread -march=core2 -mfpmath=sse -std=c++11 -fpermissive -w -o build/utils/common/strlwr.o
```
