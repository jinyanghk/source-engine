//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: OBJ -> MDL conversion helper (Linux port)
//
//=============================================================================//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>
#include <fnmatch.h>
#include <vector>
#include <string>
#include <cstdlib>

#include "tier0/dbg.h"
#include "strtools.h"
#include "utlvector.h"

extern char **environ;

#define BINEXT ""

bool verbose = false;

SpewRetval_t SpewFunc(SpewType_t type, char const *pMsg)
{
    printf("%s", pMsg);
    fflush(stdout);

    if (type == SPEW_ERROR)
    {
        printf("\n");
    }

    return SPEW_CONTINUE;
}

void printusage()
{
    printf("usage:  obj2mdl <modelname.obj file> \n\n");
    printf("The directory containing the <modelname>.obj file must also contain a\n");
    printf("<modelname>.mtl file.\n");
    printf("\nEnvironment variables:\n");
    printf("  VPROJECT   - game/mod directory (e.g. /path/to/tf)\n");
    printf("  SOURCESDK  - Source SDK root (contains bin/, content/)\n");
    printf("  STEAMNAME  - optional, author folder name under models/contrib (default: 'user')\n");
}

static bool DirectoryExists(const char *dirName)
{
    struct stat st;
    if (stat(dirName, &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

static bool FileExists(const char *filePathName)
{
    struct stat st;
    if (stat(filePathName, &st) != 0)
        return false;
    return S_ISREG(st.st_mode);
}

static bool MakeDirs(const char *path)
{
    if (DirectoryExists(path))
        return true;

    char tmp[MAX_PATH];
    V_strncpy(tmp, path, sizeof(tmp));

    size_t len = V_strlen(tmp);
    if (len == 0)
        return false;

    while (len > 1 && tmp[len - 1] == '/')
        tmp[--len] = 0;

    for (char *p = tmp + 1; *p; ++p)
    {
        if (*p == '/')
        {
            *p = 0;
            if (!DirectoryExists(tmp))
                mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);

    return DirectoryExists(tmp);
}

static bool CopyOneFile(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in)
        return false;

    FILE *out = fopen(dst, "wb");
    if (!out)
    {
        fclose(in);
        return false;
    }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
    {
        if (fwrite(buf, 1, n, out) != n)
        {
            fclose(in);
            fclose(out);
            return false;
        }
    }

    fclose(in);
    fclose(out);
    return true;
}

static bool CopyFiles( const char *pcSourceDir, const char *pcPattern, const char *pcDestDir )
{
    DIR *d = opendir( pcSourceDir );
    if ( !d )
        return false;

    bool bAllSucceeded = true;
    struct dirent *ent;

    // Normalize source string to ensure it preserves trailing slashes cleanly on UNIX paths
    std::string srcBase(pcSourceDir);
    if (!srcBase.empty() && srcBase.back() != '/')
    {
        srcBase += '/';
    }

    while ( ( ent = readdir( d ) ) != NULL )
    {
        if ( ent->d_name[0] == '.' )
            continue;

        if ( fnmatch( pcPattern, ent->d_name, 0 ) != 0 )
            continue;

        char szSrcPath[MAX_PATH];
        char szDestPath[MAX_PATH];

        V_snprintf( szSrcPath, sizeof( szSrcPath ), "%s%s", srcBase.c_str(), ent->d_name );
        V_snprintf( szDestPath, sizeof( szDestPath ), "%s/%s", pcDestDir, ent->d_name );

        if ( FileExists( szDestPath ) )
            unlink( szDestPath );

        if ( !CopyOneFile( szSrcPath, szDestPath ) )
            bAllSucceeded = false;
    }

    closedir( d );
    return bAllSucceeded;
}

void RunCommandLine( const char *pCmdLine, const char *pWorkingDir )
{
    char szFullSystemCmd[MAX_PATH * 3];
    
    // Ensure the working directory stays exactly where the utilities reside
    const char *pRealCWD = pWorkingDir ? pWorkingDir : ".";

    // Build a precise absolute path to the engine libraries folder
    char szAbsoluteBinPath[MAX_PATH];
    V_snprintf( szAbsoluteBinPath, sizeof( szAbsoluteBinPath ), "%s/bin", pRealCWD );

    const char* old_ld = getenv("LD_LIBRARY_PATH");
    std::string ldPath = szAbsoluteBinPath;
    if (old_ld && *old_ld)
    {
        ldPath += ":";
        ldPath += old_ld;
    }

    // Execute directly from the tool engine root folder, injecting the absolute library path
    V_snprintf( szFullSystemCmd, sizeof( szFullSystemCmd ),
                "cd \"%s\" && LD_LIBRARY_PATH=\"%s\" %s",
                pRealCWD,
                ldPath.c_str(),
                pCmdLine );

    if (verbose || true) // Trace the string compilation hook exactly
    {
        printf("Executing System Hook: %s\n", szFullSystemCmd);
    }

    int rc = system( szFullSystemCmd );
    if ( rc != 0 )
    {
        printf( "--- Command returned status code: %d ---\n", rc );
    }
}

bool GetSteamUserName(char *pcSteamName, size_t nSteamNameBufSize)
{
    pcSteamName[0] = 0;
    const char *env = getenv("STEAMNAME");
    if (env && *env)
    {
        V_strncpy(pcSteamName, env, nSteamNameBufSize);
    }
    else
    {
        const char *user = getenv("USER");
        if (!user || !*user)
            user = "user";
        V_strncpy(pcSteamName, user, nSteamNameBufSize);
    }
    return (V_strlen(pcSteamName) > 0);
}

bool GetSDKBinDirectory(char *pcSDKBinDir, size_t nBuffSize)
{
    *pcSDKBinDir = 0;
    const char *sdk = getenv("SOURCESDK");
    if (sdk && *sdk)
    {
        V_snprintf(pcSDKBinDir, nBuffSize, "%s", sdk);
        if (DirectoryExists(pcSDKBinDir))
            return true;

        V_snprintf(pcSDKBinDir, nBuffSize, "%s", sdk);
        return DirectoryExists(pcSDKBinDir);
    }

    const char *vproj = getenv("VPROJECT");
    if (vproj && *vproj)
    {
        char tmp[MAX_PATH];
        V_strncpy(tmp, vproj, sizeof(tmp));
        V_StripFilename(tmp);
        V_snprintf(pcSDKBinDir, nBuffSize, "%s", tmp);
        if (DirectoryExists(pcSDKBinDir))
            return true;
    }
    return false;
}

bool GetSDKSourcesDirectory(char *pcSDKSourcesDir, size_t nBufSize)
{
    *pcSDKSourcesDir = 0;
    const char *sdk = getenv("SOURCESDK");
    const char *vproj = getenv("VPROJECT");

    if (sdk && *sdk && vproj && *vproj)
    {
        char base[MAX_PATH];
        V_FileBase(vproj, base, sizeof(base));
        V_snprintf(pcSDKSourcesDir, nBufSize, "%s/content/%s", sdk, base);
        return true;
    }
    return false;
}

bool GetVProjectDirectory(char *pcVProjectPath, size_t nBufSize)
{
    pcVProjectPath[0] = 0;
    const char *vproj = getenv("VPROJECT");
    if (vproj && *vproj)
    {
        V_strncpy(pcVProjectPath, vproj, nBufSize);
        return true;
    }
    return false;
}

void CreateTemplateQC( const char *pcDirectory, const char *pcBaseName, const char *pcSteamName )
{
    char sz_Buffer[MAX_PATH];
    V_snprintf( sz_Buffer, MAX_PATH, "%s%s.qc", pcDirectory, pcBaseName );

    FILE *fp = fopen( sz_Buffer, "w" );
    if ( !fp )
        return;

    fprintf( fp, "$modelname \"contrib/%s/%s.mdl\"\n", pcSteamName, pcBaseName );
    fputs( "$upaxis Y\n",  fp );
    fputs( "$scale 1.00\n",  fp );
    fprintf( fp, "$body \"Body\" \"%s.obj\"\n", pcBaseName );
    fputs( "$surfaceprop \"cloth\"\n", fp );
    fprintf( fp, "$cdmaterials \"models/contrib/%s/%s\"\n", pcSteamName, pcBaseName );
    fprintf( fp, "$sequence \"idle\" \"%s.obj\" fps 30 numframes 30 loop\n", pcBaseName );
    fprintf( fp, "$collisionmodel \"%s.obj\" {\n", pcBaseName );
    fputs( "$mass 5.0\n",  fp );
    fputs( "}\n",  fp );

    fclose( fp );
}

void CreateTemplateVMT( const char *pcDirectory, const char *pcBaseName,
                        const char *pcTGAColorFile, const char *pcTGANormalFile,
                        const char *pcSteamName )
{
    char sz_Buffer[MAX_PATH];
    char szTGAColorFileBase[MAX_PATH];
    char szTGANormalFileBase[MAX_PATH];

    V_snprintf( sz_Buffer, MAX_PATH, "%s%s.vmt", pcDirectory, pcBaseName );

    FILE *fp = fopen( sz_Buffer, "w" );
    if ( !fp )
        return;

    fputs( "\"VertexlitGeneric\"\n",  fp );
    fputs( "{\n",  fp );
    V_StripExtension( pcTGAColorFile, szTGAColorFileBase, sizeof( szTGAColorFileBase ) );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"$baseTexture\" \"models/contrib/%s/%s/%s\"\n",
                pcSteamName, pcBaseName, szTGAColorFileBase );
    fputs( sz_Buffer,  fp );

    if ( pcTGANormalFile && ( V_strlen( pcTGANormalFile ) > 0 ) )
    {
        V_StripExtension( pcTGANormalFile, szTGANormalFileBase, sizeof( szTGANormalFileBase ) );
        V_snprintf( sz_Buffer, MAX_PATH, "\t\"$bumpmap\" \"models/contrib/%s/%s/%s\"\n",
                    pcSteamName, pcBaseName, szTGANormalFileBase );
        fputs( sz_Buffer,  fp );
    }

    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$detail", "effects/tiledfire/fireLayeredSlowTiled512.vtf" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$detailscale", "5" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" %s\n", "$detailblendfactor", ".01" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" %s\n", "$detailblendmode", "6" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$yellow", "0" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$phong", "1" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$phongexponent", "25" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$phongboost", "5" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$lightwarptexture", "models\\lightwarps\\weapon_lightwarp" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$phongfresnelranges", "[.25 .5 1]" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$basemapalphaphongmask", "1" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$rimlight", "1" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$rimlightexponent", "4" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$rimlightboost", "2" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\"%s\" \"%s\"\n", "$cloakPassEnabled", "1" );
    fputs( sz_Buffer,  fp );

    fputs( "\t\"Proxies\"\n",  fp );
    fputs( "\t{\n",  fp );
    fputs( "\t\t\"weapon_invis\"\n",  fp );
    fputs( "\t\t{\n",  fp );
    fputs( "\t\t}\n",  fp );
    fputs( "\t\t\"AnimatedTexture\"\n",  fp );
    fputs( "\t\t{\n",  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" \"%s\"\n", "animatedtexturevar", "$detail" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" \"%s\"\n", "animatedtextureframenumvar", "$detailframe" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" %s\n", "animatedtextureframerate", "30" );
    fputs( sz_Buffer,  fp );
    fputs( "\t\t}\n",  fp );
    fputs( "\t\t\"BurnLevel\"\n",  fp );
    fputs( "\t\t{\n",  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" \"%s\"\n", "resultVar", "$detailblendfactor" );
    fputs( sz_Buffer,  fp );
    fputs( "\t\t}\n",  fp );
    fputs( "\t\t\"YellowLevel\"\n",  fp );
    fputs( "\t\t{\n",  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" \"%s\"\n", "resultVar", "$yellow" );
    fputs( sz_Buffer,  fp );
    fputs( "\t\t}\n",  fp );
    fputs( "\t\t\"Equals\"\n",  fp );
    fputs( "\t\t{\n",  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" \"%s\"\n", "srcVar1", "$yellow" );
    fputs( sz_Buffer,  fp );
    V_snprintf( sz_Buffer, MAX_PATH, "\t\t\t\"%s\" \"%s\"\n", "resultVar", "$color2" );
    fputs( sz_Buffer,  fp );
    fputs( "\t\t}\n",  fp );
    fputs( "\t}\n",  fp );
    fputs( "}\n",  fp );

    fclose( fp );
}

bool CheckFilesExist(const char *pcDirectory, const char *pcOBJFile, const char *pcMTLFile)
{
    char szPath[MAX_PATH];
    V_snprintf(szPath, MAX_PATH, "%s%s", pcDirectory, pcOBJFile);
    if (!FileExists(szPath))
        return false;
    V_snprintf(szPath, MAX_PATH, "%s%s", pcDirectory, pcMTLFile);
    if (!FileExists(szPath))
        return false;
    return true;
}
bool ParseMTL(const char *pcDirectory, const char *pcOBJFile, const char *pcMTLFile,
              char *pcTGAColorFile, size_t nTGAColorBufSize,
              char *pcTGASpecularFile, size_t nTGASpecularBufSize)
{
    char szMTLFile[MAX_PATH];
    V_snprintf(szMTLFile, MAX_PATH, "%s%s", pcDirectory, pcMTLFile);
    FILE *fp = fopen(szMTLFile, "r");
    if (!fp)
        return false;
    char szLine[MAX_PATH];
    while (fgets(szLine, sizeof(szLine), fp))
    {
        char szToken[MAX_PATH];
        char szValue[MAX_PATH];
        char cTab;
        if (sscanf(szLine, "%s%c%s", szToken, &cTab, szValue) >= 3)
        {
            if (V_stristr(szToken, "map_Kd"))
            {
                V_strncpy(pcTGAColorFile, V_UnqualifiedFileName(szValue), nTGAColorBufSize);
            }
            else if (V_stristr(szToken, "map_Ks"))
            {
                V_strncpy(pcTGASpecularFile, V_UnqualifiedFileName(szValue), nTGASpecularBufSize);
            }
        }
    }
    fclose(fp);
    return true;
}
bool ParseArguments(int argc, char *argv[],
                    char *pcDirectory, size_t nDirectoryBufSize,
                    char *pcBaseName, size_t nBaseNameBufSize,
                    char *pcOBJFile, size_t nOBJFileBufSize,
                    char *pcMTLFile, size_t nMTLFileBufSize)
{
    if (argc < 2)
    {
        printusage();
        return false;
    }
    char szFile[MAX_PATH];
    V_ExtractFilePath(argv[1], pcDirectory, nDirectoryBufSize);
    V_strncpy(szFile, V_UnqualifiedFileName(argv[1]), MAX_PATH);
    if (V_stristr(szFile, ".obj"))
    {
        V_FileBase(szFile, pcBaseName, nBaseNameBufSize);
        V_strncpy(pcOBJFile, szFile, nOBJFileBufSize);
        V_snprintf(pcMTLFile, nMTLFileBufSize, "%s.mtl", pcBaseName);
    }
    else
    {
        printusage();
        return false;
    }
    return true;
}

void CompileVTEX( const char *pcDirectory, const char *pcTGAColorFile,
                  const char *pcSteamName, const char *pcBaseName )
{
    char szCmdLine[MAX_PATH];
    char szSDKBinDir[MAX_PATH];
    char szSDKSourcesDir[MAX_PATH];

    GetSDKBinDirectory( szSDKBinDir, sizeof( szSDKBinDir ) );
    GetSDKSourcesDirectory( szSDKSourcesDir, sizeof( szSDKSourcesDir ) );

    V_snprintf( szCmdLine, sizeof( szCmdLine ),
                "./vtex -nopause \"content/hl2/materialsrc/models/contrib/%s/%s/%s\"",
                pcSteamName, pcBaseName, pcTGAColorFile );
    RunCommandLine( szCmdLine, szSDKBinDir );
}

void CompileMDL( const char *pcObjFile, const char *pcSteamName, const char *pcBaseName )
{
    char szCmdLine[MAX_PATH];
    char szSDKBinDir[MAX_PATH];
    char szSDKSourcesDir[MAX_PATH];
    char szGameName[MAX_PATH];

    GetSDKBinDirectory( szSDKBinDir, sizeof( szSDKBinDir ) );
    GetSDKSourcesDirectory( szSDKSourcesDir, sizeof( szSDKSourcesDir ) );

    const char *vproj = getenv( "VPROJECT" );
    if ( vproj && *vproj )
        V_FileBase( vproj, szGameName, sizeof( szGameName ) );
    else
        V_strncpy( szGameName, "hl2", sizeof( szGameName ) );

    // Since our CWD is source-engine/hl2, the content directory is sitting right there natively!
    char szRelativeQCPath[MAX_PATH];
    V_snprintf( szRelativeQCPath, sizeof( szRelativeQCPath ), 
                "content/%s/modelsrc/contrib/%s/%s/%s.qc", 
                szGameName, pcSteamName, pcBaseName, pcBaseName );

    V_snprintf( szCmdLine, sizeof( szCmdLine ),
                "./studiomdl -game %s -nop4 \"%s\"",
                szGameName, szRelativeQCPath );

    RunCommandLine( szCmdLine, szSDKBinDir );
}

void CopyVMT(const char *pcDirectory, const char *pcBaseName, const char *pcSteamName)
{
    char szVProjectPath[MAX_PATH];
    char szSourceVMT[MAX_PATH];
    char szDestVMT[MAX_PATH];
    char szDestDir[MAX_PATH];
    GetVProjectDirectory(szVProjectPath, sizeof(szVProjectPath));
    V_snprintf(szSourceVMT, sizeof(szSourceVMT), "%s%s.vmt", pcDirectory, pcBaseName);
    V_snprintf(szDestDir, sizeof(szDestDir),
               "%s/materials/models/contrib/%s/%s", szVProjectPath, pcSteamName, pcBaseName);
    V_snprintf(szDestVMT, sizeof(szDestVMT), "%s/%s.vmt", szDestDir, pcBaseName);
    MakeDirs(szDestDir);
    if (FileExists(szDestVMT))
        unlink(szDestVMT);
    CopyOneFile(szSourceVMT, szDestVMT);
}
bool CopyMaterialSourcesToSrcTree(const char *pcDirectory, const char *pcSteamName, const char *pcBaseName)
{
    char szSDKSourcesDir[MAX_PATH];
    char szBuffer[MAX_PATH];
    bool bAllSucceeded = true;
    GetSDKSourcesDirectory(szSDKSourcesDir, sizeof(szSDKSourcesDir));
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/materialsrc", szSDKSourcesDir);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/materialsrc/models", szSDKSourcesDir);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/materialsrc/models/contrib", szSDKSourcesDir);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/materialsrc/models/contrib/%s", szSDKSourcesDir, pcSteamName);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/materialsrc/models/contrib/%s/%s", szSDKSourcesDir, pcSteamName, pcBaseName);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    if (bAllSucceeded)
    {
        CopyFiles(pcDirectory, "*.tga", szBuffer);
    }
    return bAllSucceeded;
}
bool ReplaceLineInTXTFile(const char pcFilePath, const char pcFirstChars, const char *pcReplacement)
{
    char sz_Buffer[MAX_PATH];
    CUtlStringList fileContents;
    FILE *fp = fopen(pcFilePath, "r");
    if (!fp)
        return false;
    while (fgets(sz_Buffer, sizeof(sz_Buffer), fp))
    {
        if (!V_strncmp(sz_Buffer, pcFirstChars, V_strlen(pcFirstChars)))
        {
            size_t replLen = V_strlen(pcReplacement);
            V_strncpy(sz_Buffer, pcReplacement, replLen + 1);
            if (replLen == 0 || sz_Buffer[replLen - 1] != '\n')
            {
                if (replLen + 1 < sizeof(sz_Buffer))
                {
                    sz_Buffer[replLen] = '\n';
                    sz_Buffer[replLen + 1] = 0;
                }
            }
        }
        fileContents.CopyAndAddToTail(sz_Buffer);
    }
    fclose(fp);
    fp = fopen(pcFilePath, "w");
    if (!fp)
    {
        fileContents.PurgeAndDeleteElements();
        return false;
    }
    FOR_EACH_VEC(fileContents, i)
    {
        fputs(fileContents[i], fp);
    }
    fclose(fp);
    fileContents.PurgeAndDeleteElements();
    return true;
}
bool CopyModelSourcesToSrcTree(const char *pcDirectory, const char *pcSteamName, const char *pcBaseName)
{
    char szSDKSourcesDir[MAX_PATH];
    char szBuffer[MAX_PATH];
    bool bAllSucceeded = true;
    GetSDKSourcesDirectory(szSDKSourcesDir, sizeof(szSDKSourcesDir));
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/modelsrc", szSDKSourcesDir);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/modelsrc/contrib", szSDKSourcesDir);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/modelsrc/contrib/%s", szSDKSourcesDir, pcSteamName);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    V_snprintf(szBuffer, sizeof(szBuffer), "%s/modelsrc/contrib/%s/%s", szSDKSourcesDir, pcSteamName, pcBaseName);
    MakeDirs(szBuffer);
    bAllSucceeded &= DirectoryExists(szBuffer);
    if (bAllSucceeded)
    {
        CopyFiles(pcDirectory, ".obj", szBuffer);
        CopyFiles(pcDirectory, ".mtl", szBuffer);
        CopyFiles(pcDirectory, "*.qc", szBuffer);
    }
    char szFileToChange[MAX_PATH];
    char szReplacementLine[MAX_PATH];
    V_snprintf(szFileToChange, sizeof(szFileToChange), "%s/%s.mtl", szBuffer, pcBaseName);
    V_snprintf(szReplacementLine, sizeof(szReplacementLine), "newmtl %s\n", pcBaseName);
    ReplaceLineInTXTFile(szFileToChange, "newmtl", szReplacementLine);
    V_snprintf(szReplacementLine, sizeof(szReplacementLine), "map_Kd %s.tga\n", pcBaseName);
    ReplaceLineInTXTFile(szFileToChange, "map_Kd", szReplacementLine);
    V_snprintf(szFileToChange, sizeof(szFileToChange), "%s/%s.obj", szBuffer, pcBaseName);
    V_snprintf(szReplacementLine, sizeof(szReplacementLine), "g %s\n", pcBaseName);
    ReplaceLineInTXTFile(szFileToChange, "g ", szReplacementLine);
    V_snprintf(szReplacementLine, sizeof(szReplacementLine), "usemtl %s\n", pcBaseName);
    ReplaceLineInTXTFile(szFileToChange, "usemtl ", szReplacementLine);
    return bAllSucceeded;
}
int main(int argc, char *argv[])
{
    char szDirectory[MAX_PATH];
    char szOBJFile[MAX_PATH];
    char szMTLFile[MAX_PATH];
    char szTGAColorFile[MAX_PATH];
    char szTGANormalFile[MAX_PATH];
    char szTGATransparencyFile[MAX_PATH];
    char szTGASpecularFile[MAX_PATH];
    char szBaseName[MAX_PATH];
    char szSteamName[64];
    SpewOutputFunc(SpewFunc);
    szDirectory[0] = szMTLFile[0] = szOBJFile[0] = szSteamName[0] =
        szTGAColorFile[0] = szTGANormalFile[0] = szTGATransparencyFile[0] = szTGASpecularFile[0] = 0;
    if (ParseArguments(argc, argv,
                       szDirectory, sizeof(szDirectory),
                       szBaseName, sizeof(szBaseName),
                       szOBJFile, sizeof(szOBJFile),
                       szMTLFile, sizeof(szMTLFile)))
    {
        if (CheckFilesExist(szDirectory, szOBJFile, szMTLFile))
        {
            printf("--- OBJ to MDL file conversion helper (Linux) ---\n");
            if (!GetSteamUserName(szSteamName, sizeof(szSteamName)))
            {
                printf("--- Unable to determine user name. Exiting. ---\n");
                return 1;
            }
            printf("--- Author folder: %s ---\n", szSteamName);
            printf("--- Reading MTL file ---\n");
            ParseMTL(szDirectory, szOBJFile, szMTLFile,
                     szTGAColorFile, sizeof(szTGAColorFile),
                     szTGASpecularFile, sizeof(szTGASpecularFile));
            printf("--- Creating VMT and QC files ---\n");
            CreateTemplateVMT(szDirectory, szBaseName, szTGAColorFile, szTGANormalFile, szSteamName);
            CreateTemplateQC(szDirectory, szBaseName, szSteamName);
            CopyMaterialSourcesToSrcTree(szDirectory, szSteamName, szBaseName);
            printf("--- Compiling TGAs into a VTEX with vtex ---\n");
            CompileVTEX(szDirectory, szTGAColorFile, szSteamName, szBaseName);
            CopyVMT(szDirectory, szBaseName, szSteamName);
            printf("--- Compiling OBJs into an MDL with studiomdl ---\n");
            CopyModelSourcesToSrcTree(szDirectory, szSteamName, szBaseName);
            CompileMDL(szOBJFile, szSteamName, szBaseName);
            return 0;
        }
    }
    return 1;
}
