#ifndef COMPILE_SETTINGS_H
#define COMPILE_SETTINGS_H

struct CompileSettings
{
    char mapName[64]   = "test";
    char gameDir[64]   = "hl2";

    bool bRunVbsp      = true;
    bool bRunVvis      = true;
    bool bRunVrad      = true;
    bool bCopyToMaps   = true;
    bool bLaunchGame   = true;

    char vbspArgs[128] = "";
    char vvisArgs[128] = "-fast";
    char vradArgs[128] = "-fast";
};

// 全局唯一实例
extern CompileSettings g_compileSettings;

#endif // COMPILE_SETTINGS_H