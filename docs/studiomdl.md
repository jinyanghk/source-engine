convert smd + qc file to mdl file

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
+               //return false;
        }
 #endif // USE_SDL
 #endif // !DEDICATED
```

```sh
$ LD_LIBRARY_PATH=bin:$LD_LIBRARY_PATH ./studiomdl -game hl2 -nop4  ../..
/Scout.qc
LoadLibrary: path: /home/jin/source-engine/hl2/bin/libfilesystem_stdio.so
LoadLibrary: pModule: vstdlib.dll, path: /home/jin/source-engine/hl2/bin/libvstdlib.so
LoadLibrary: pModule: materialsystem.dll, path: /home/jin/source-engine/hl2/bin/libmaterialsystem.so
LoadLibrary: pModule: studiorender.dll, path: /home/jin/source-engine/hl2/bin/libstudiorender.so
LoadLibrary: pModule: mdllib.dll, path: /home/jin/source-engine/hl2/bin/libmdllib.so
LoadLibrary: pModule: shaderapiempty.dll, path: /home/jin/source-engine/hl2/bin/libshaderapiempty.so
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
qdir:    "/home/jin/"
gamedir: "/home/jin/source-engine/hl2/hl2/"
g_path:  "../../Scout.qc"
Building binary model files...
Working on "Scout.qc"
Segmentation fault (core dumped)
```