
```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vtex
LoadLibrary: pModule: vtex_dll.dll, path: /home/jin/source-engine/hl2/bin/libvtex_dll.so
ERROR: Usage: vtex [-quiet] [-mkdir] [-shader ShaderName] [-vmtparam Param Value] tex1.txt tex2.txt . . .
-quiet            : don't print anything out, don't pause for input
-warningsaserrors : treat warnings as errors
-nomkdir          : don't create destination folder if it doesn't exist
-vmtparam         : adds parameter and value to the .vmt file
-deducepath       : deduce path of sources by target file names
-quickconvert     : use with "-dontusegamedir -quickconvert" to upgrade old .vmt files
-crcvalidate      : validate .vmt against the sources
-crcforce         : generate a new .vmt even if sources crc matches
        eg: -vmtparam $ignorez 1 -vmtparam $translucent 1
Note that you can use wildcards and that you can also chain them
e.g. materialsrc/monster1/*.tga materialsrc/monster2/*.tga
```

```sh
LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./vtex platform/platform_misc_dir./friends/icon_message.tga
LoadLibrary: pModule: vtex_dll.dll, path: /home/jin/source-engine/hl2/bin/libvtex_dll.so
input file: platform/platform_misc_dir./friends/icon_message
no config file for platform/platform_misc_dir./friends/icon_message.tga
SUCCESS: Vtf file created
```