convert smd + qc file to mdl file

studiomdl doesn't need SDLMgrInterface001

```sh
diff --git a/materialsystem/cmaterialsystem.cpp b/materialsystem/cmaterialsystem.cpp
index e7870712..b77cb060 100644
--- a/materialsystem/cmaterialsystem.cpp
+++ b/materialsystem/cmaterialsystem.cpp
@@ -672,7 +672,7 @@ bool CMaterialSystem::Connect( CreateInterfaceFn factory )
        g_pLauncherMgr = (ILauncherMgr *)factory( "SDLMgrInterface001" /*SDL_MGR_INTERFACE_VERSION*/, NULL );   
        if ( !g_pLauncherMgr )
        {
-               return false;
+               Warning("SDLMgrInterface001 not loaded!");
        }
 #endif // USE_SDL
 #endif // !DEDICATED
```

```sh
rm hl2/models/props_junk/box.mdl

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./studiomdl -game hl2 -nop4 -nomerge box.qc
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
LoadLibrary: pModule: vstdlib.dll, path: /home/jin/source-engine/hl2/bin/libvstdlib.so
LoadLibrary: pModule: materialsystem.dll, path: /home/jin/source-engine/hl2/bin/libmaterialsystem.so
LoadLibrary: pModule: studiorender.dll, path: /home/jin/source-engine/hl2/bin/libstudiorender.so
LoadLibrary: pModule: mdllib.dll, path: /home/jin/source-engine/hl2/bin/libmdllib.so
LoadLibrary: pModule: shaderapiempty.dll, path: /home/jin/source-engine/hl2/bin/libshaderapiempty.so
WARNING: SDLMgrInterface001 not loaded!
filesystem BaseDir: /home/jin/source-engine/hl2
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dbg.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dbg.so
WARNING: Can't find module - stdshader_dbg.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx6.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dx6.so
WARNING: Can't find module - stdshader_dx6.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx7.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dx7.so
WARNING: Can't find module - stdshader_dx7.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx8.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dx8.so
WARNING: Can't find module - stdshader_dx8.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx9.so
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libstdshader_dx9.so
WARNING: Convar mat_specular has conflicting FCVAR_CHEAT flags (child: FCVAR_CHEAT, parent: no FCVAR_CHEAT, parent wins)
qdir:    "/home/jin/source-engine/hl2/"
gamedir: "/home/jin/source-engine/hl2/hl2/"
g_path:  "box.qc"
Building binary model files...
Working on "box.qc"
SMD MODEL box.smd
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/VPHYSICS.DLL
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libVPHYSICS.DLL
WARNING: Can't find module - VPHYSICS.DLL
LoadLibrary: pModule: vphysics.dll, path: /home/jin/source-engine/hl2/bin/libvphysics.so
Model has 1 convex sub-parts
Collision model completed.
---------------------
writing /home/jin/source-engine/hl2/hl2/models/props_junk/box.mdl:
bones          964 bytes (1)
WriteAnimations: pBlockData is NULL or invalid, allocating new buffer
WriteAnimations: pData became invalid after allocating animdesc
WriteAnimations: pBlockEnd invalid at 0x0xffffffffb84ac010, allocating
WriteAnimations: pData invalid before WriteAnimationData, reallocating
WriteAnimationData: pData invalid after processing section 0
WriteAnimations: pData invalid after processing animation 0
$BoneSaveFrame "static_prop" position rotation
WriteModelFiles: WriteAnimations returned invalid, using pStart + usedBeforeAnim
animations       0 bytes (1 anims) (1 frames) [0:01]
sequences      212 bytes (1 seq)
WriteModelFiles: Detected bodyparts, calling WriteModel
WriteModel: pData invalid after bodypart loop
WriteTextures: pData invalid after texture loop
textures         0 bytes
keyvalues        0 bytes
bone transforms        0 bytes
bone flex driver       0 bytes
WriteModelFiles: pData invalid before WriteStringTable, resetting
WriteModelFiles: WriteStringTable returned invalid, using pStart + usedBeforeString
Collision model volume 32768.00 in^3
collision        0 bytes
total          664
WriteModelFiles: Skipping LoadMaterials (numtextures=0)
WriteModelFiles: Writing MDL file: /home/jin/source-engine/hl2/hl2/models/props_junk/box.mdl, size=664 bytes
WriteModelFiles: MDL write completed
WriteModelFiles: Invalid block length: -91942912, skipping
---------------------
writing : SKIPPED

Completed "box.qc"
```

```sh
rm hl2/models/player/custom_player.mdl

LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./studiomdl -game hl2 -nop4 -nomerge player/player.qc
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
LoadLibrary: pModule: vstdlib.dll, path: /home/jin/source-engine/hl2/bin/libvstdlib.so
LoadLibrary: pModule: materialsystem.dll, path: /home/jin/source-engine/hl2/bin/libmaterialsystem.so
LoadLibrary: pModule: studiorender.dll, path: /home/jin/source-engine/hl2/bin/libstudiorender.so
LoadLibrary: pModule: mdllib.dll, path: /home/jin/source-engine/hl2/bin/libmdllib.so
LoadLibrary: pModule: shaderapiempty.dll, path: /home/jin/source-engine/hl2/bin/libshaderapiempty.so
WARNING: SDLMgrInterface001 not loaded!
filesystem BaseDir: /home/jin/source-engine/hl2
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dbg.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dbg.so
WARNING: Can't find module - stdshader_dbg.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx6.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dx6.so
WARNING: Can't find module - stdshader_dx6.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx7.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dx7.so
WARNING: Can't find module - stdshader_dx7.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx8.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libstdshader_dx8.so
WARNING: Can't find module - stdshader_dx8.so
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/stdshader_dx9.so
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libstdshader_dx9.so
WARNING: Convar mat_specular has conflicting FCVAR_CHEAT flags (child: FCVAR_CHEAT, parent: no FCVAR_CHEAT, parent wins)
qdir:    "/home/jin/source-engine/hl2/player/"
gamedir: "/home/jin/source-engine/hl2/hl2/"
g_path:  "player/player.qc"
Building binary model files...
Working on "player.qc"
SMD MODEL body_reference.smd
SMD MODEL head_reference.smd
SMD MODEL idle.smd
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/VPHYSICS.DLL
WARNING: Can't find module - /home/jin/source-engine/hl2/bin/libVPHYSICS.DLL
WARNING: Can't find module - VPHYSICS.DLL
LoadLibrary: pModule: vphysics.dll, path: /home/jin/source-engine/hl2/bin/libvphysics.so
Collision model completed.
---------------------
writing /home/jin/source-engine/hl2/hl2/models/player/custom_player.mdl:
bones          964 bytes (1)
WriteAnimations: pBlockData is NULL or invalid, allocating new buffer
WriteAnimations: pData became invalid after allocating animdesc
WriteAnimations: pBlockEnd invalid at 0x0xffffffff900d1010, allocating
WriteAnimations: pData invalid before WriteAnimationData, reallocating
WriteAnimationData: pData invalid after processing section 0
WriteAnimations: pData invalid after processing animation 0
WriteAnimations: pBlockEnd invalid at 0x0xffffffff88aff010, allocating
WriteAnimations: pData invalid before WriteAnimationData, reallocating
WriteAnimationData: pData invalid after processing section 0
WriteAnimations: pData invalid after processing animation 1
WriteAnimations: pBlockEnd invalid at 0x0xffffffff887fc010, allocating
WriteAnimations: pData invalid before WriteAnimationData, reallocating
WriteAnimationData: pData invalid after processing section 0
WriteAnimations: pData invalid after processing animation 2
$BoneSaveFrame "root" position rotation
WriteModelFiles: WriteAnimations returned invalid, using pStart + usedBeforeAnim
animations       0 bytes (3 anims) (3 frames) [0:03]
sequences      636 bytes (3 seq)
WriteModelFiles: Detected bodyparts, calling WriteModel
WriteModel: pData invalid after bodypart loop
WriteTextures: pData invalid after texture loop
textures         0 bytes
keyvalues        0 bytes
bone transforms        0 bytes
bone flex driver       0 bytes
WriteModelFiles: pData invalid before WriteStringTable, resetting
WriteModelFiles: WriteStringTable returned invalid, using pStart + usedBeforeString
collision        0 bytes
total          664
WriteModelFiles: Skipping LoadMaterials (numtextures=0)
WriteModelFiles: Writing MDL file: /home/jin/source-engine/hl2/hl2/models/player/custom_player.mdl, size=664 bytes
WriteModelFiles: MDL write completed
WriteModelFiles: Invalid block length: -129859584, skipping
---------------------
writing : SKIPPED

Completed "player.qc"
```