
```sh
$ SOURCESDK="$PWD" VPROJECT="$PWD/hl2" STEAMNAME="pi" LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./obj2mdl ../jin/cube.obj
--- OBJ to MDL file conversion helper (Linux) ---
--- Author folder: pi ---
--- Reading MTL file ---
--- Creating VMT and QC files ---
--- Compiling TGAs into a VTEX with vtex ---
Executing System Hook: cd "/media/pi/backup/source-engine/hl2" && LD_LIBRARY_PATH="/media/pi/backup/source-engine/hl2/bin:bin:/home/pi/vulkansdk/1.4.350.1/x86_64/lib/VulkanLoader/lib" ./vtex -nopause "content/hl2/materialsrc/models/contrib/pi/cube/cube.tga"
LoadLibrary: pModule: vtex_dll.dll, path: /media/pi/backup/source-engine/hl2/bin/libvtex_dll.so
input file: content/hl2/materialsrc/models/contrib/pi/cube/cube
no config file for content/hl2/materialsrc/models/contrib/pi/cube/cube.tga
SUCCESS: content/hl2/materialsrc/models/contrib/pi/cube/cube.vtf is up-to-date
--- Compiling OBJs into an MDL with studiomdl ---
Executing System Hook: cd "/media/pi/backup/source-engine/hl2" && LD_LIBRARY_PATH="/media/pi/backup/source-engine/hl2/bin:bin:/home/pi/vulkansdk/1.4.350.1/x86_64/lib/VulkanLoader/lib" ./studiomdl -game hl2 -nop4 "content/hl2/modelsrc/contrib/pi/cube/cube.qc"
LoadLibrary: path: /media/pi/backup/source-engine/hl2/bin/libfilesystem_stdio.so
LoadLibrary: pModule: vstdlib.dll, path: /media/pi/backup/source-engine/hl2/bin/libvstdlib.so
LoadLibrary: pModule: materialsystem.dll, path: /media/pi/backup/source-engine/hl2/bin/libmaterialsystem.so
LoadLibrary: pModule: studiorender.dll, path: /media/pi/backup/source-engine/hl2/bin/libstudiorender.so
LoadLibrary: pModule: mdllib.dll, path: /media/pi/backup/source-engine/hl2/bin/libmdllib.so
LoadLibrary: pModule: shaderapiempty.dll, path: /media/pi/backup/source-engine/hl2/bin/libshaderapiempty.so
WARNING: SDLMgrInterface001 not loaded!
filesystem BaseDir: /media/pi/backup/source-engine/hl2
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/stdshader_dbg.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/libstdshader_dbg.so
WARNING: Can't find module - stdshader_dbg.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/stdshader_dx6.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/libstdshader_dx6.so
WARNING: Can't find module - stdshader_dx6.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/stdshader_dx7.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/libstdshader_dx7.so
WARNING: Can't find module - stdshader_dx7.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/stdshader_dx8.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/libstdshader_dx8.so
WARNING: Can't find module - stdshader_dx8.so
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/stdshader_dx9.so
LoadLibrary: path: /media/pi/backup/source-engine/hl2/bin/libstdshader_dx9.so
WARNING: Convar mat_specular has conflicting FCVAR_CHEAT flags (child: FCVAR_CHEAT, parent: no FCVAR_CHEAT, parent wins)
qdir:    "/media/pi/backup/source-engine/hl2/content/hl2/modelsrc/contrib/pi/cube/"
gamedir: "/media/pi/backup/source-engine/hl2/hl2/"
g_path:  "content/hl2/modelsrc/contrib/pi/cube/cube.qc"
Building binary model files...
Working on "cube.qc"
grabbing cube.obj
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/VPHYSICS.DLL
WARNING: Can't find module - /media/pi/backup/source-engine/hl2/bin/libVPHYSICS.DLL
WARNING: Can't find module - VPHYSICS.DLL
LoadLibrary: pModule: vphysics.dll, path: /media/pi/backup/source-engine/hl2/bin/libvphysics.so
Model has 1 convex sub-parts
Collision model completed.
---------------------
writing /media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.mdl:
Loading preexisting model: /media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.mdl
   Found 1 preexisting sequences.
bones          964 bytes (1)
WriteAnimations: pBlockData is NULL or invalid, allocating new buffer
$BoneSaveFrame "default" position rotation
animations     152 bytes (1 anims) (30 frames) [0:01]
sequences      212 bytes (1 seq) 
WriteModelFiles: Detected bodyparts, calling WriteModel
textures       260 bytes
keyvalues        0 bytes
bone transforms        0 bytes
bone flex driver       0 bytes
Collision model volume 8.00 in^3
collision        0 bytes
total         1668
WriteModelFiles: Writing MDL file: /media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.mdl, size=1668 bytes
WriteModelFiles: MDL write completed
---------------------
writing :
blocks	         3
total           16
---------------------
writing /media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.vvd:
vertices      1152 bytes (24 vertices)
tangents   -1954545792 bytes (24 vertices)
total         1600 bytes
---------------------
Generating optimized mesh "/media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.sw.vtx":
body parts:         8 bytes
models:            20 bytes
model LODs:        12 bytes
meshes:             0 bytes
strip groups:       0 bytes
strips:             0 bytes
verts:              0 bytes
indices:            0 bytes
bone changes:       8 bytes
everything:        72 bytes
---------------------
Generating optimized mesh "/media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.dx80.vtx":
body parts:         8 bytes
models:            20 bytes
model LODs:        12 bytes
meshes:             0 bytes
strip groups:       0 bytes
strips:             0 bytes
verts:              0 bytes
indices:            0 bytes
bone changes:       8 bytes
everything:        72 bytes
---------------------
Generating optimized mesh "/media/pi/backup/source-engine/hl2/hl2/models/contrib/pi/cube.dx90.vtx":
body parts:         8 bytes
models:            20 bytes
model LODs:        12 bytes
meshes:             0 bytes
strip groups:       0 bytes
strips:             0 bytes
verts:              0 bytes
indices:            0 bytes
bone changes:       8 bytes
everything:        72 bytes
Simple mesh detected (no LOD pools). Skipping vertex sort fixup phase.

Completed "cube.qc"

```