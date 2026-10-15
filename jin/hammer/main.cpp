#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>

#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "appframework/AppFramework.h"
#include "tier0/dbg.h"
#include "vstdlib/cvar.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "materialsystem/materialsystem_config.h"
#include "istudiorender.h"
#include "filesystem_init.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "vphysics_interface.h"
#include "tier0/icommandline.h"
#include "appframework/ilaunchermgr.h"
#include "mathlib/vmatrix.h"
#include "mathlib/mathlib.h"

#include "inputsystem/iinputsystem.h"

#include "gizmo/GizmoHit.h"
#include "gizmo/GizmoDraw.h"
#include "gizmo/GizmoPickTarget.h"
#include "render/TextureManager.h"
#include "scene/Brush.h"
#include "scene/Entity.h"
#include "scene/SceneGlobals.h"
#include "scene/EntityRender.h"
#include "scene/EntityPick.h"
#include "render/BoxRender.h"
#include "app/ImGuiLayer.h"
#include "app/EditorLayout.h"
#include "ui/EditorPanels.h"
#include "render/BoxRender.h"
#include "scene/Brush.h"
#include "app/FgdManager.h"
#include "ui/EntityPalette.h"
#include "map/MapConverter.h"
#include "map/MapDocument.h"
#include "map/VmfIO.h"
#include "gizmo/GizmoDraw.h"   // DrawScaleGizmo
#include "render/MaterialList.h"
#include "app/CompileSettings.h"
#include "app/LogBuffer.h"
#include "app/UndoStack.h"

extern IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
extern IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

extern void *CreateSDLMgr();

// 全局：face / vertex gizmo 的 hover / active 状态
int g_iHoverFace = -1;
int g_iActiveFace = -1;
// 全局：网格吸附
float g_flGridSize = 16.0f;   // 默认 16 单位

//-----------------------------------------------------------------------------

class CHammerApp : public CAppSystemGroup
{
public:
    virtual bool Create();
    virtual bool PreInit();
    virtual int Main();
    virtual void PostShutdown();
    virtual void Destroy();
};

CHammerApp g_ApplicationObject;

int main(int argc, char *argv[])
{
    CommandLine()->CreateCmdLine(argc, argv);
    return g_ApplicationObject.Run();
}

bool CHammerApp::Create()
{
    CommandLine()->AppendParm("-hammer", NULL);
    IAppSystem *pSystem;
    AppModule_t cvarModule = LoadModule(VStdLib_GetICVarFactory());
    pSystem = AddSystem(cvarModule, CVAR_INTERFACE_VERSION);
    if (!pSystem) return false;
    bool bSteam;
    char pFileSystemDLL[MAX_PATH];
    if (FileSystem_GetFileSystemDLLName(pFileSystemDLL, MAX_PATH, bSteam) != FS_OK) return false;
    AppModule_t fileSystemModule = LoadModule(pFileSystemDLL);
    g_pFileSystem = (IFileSystem *)AddSystem(fileSystemModule, FILESYSTEM_INTERFACE_VERSION);
    FileSystem_SetBasePaths(g_pFileSystem);
    AppSystemInfo_t appSystems[] = {
        {"materialsystem.dll", MATERIAL_SYSTEM_INTERFACE_VERSION},
        {"inputsystem.dll", INPUTSYSTEM_INTERFACE_VERSION},
        {"studiorender.dll", STUDIO_RENDER_INTERFACE_VERSION},
        {"vphysics.dll", VPHYSICS_INTERFACE_VERSION},
        {"datacache.dll", DATACACHE_INTERFACE_VERSION},
        {"datacache.dll", MDLCACHE_INTERFACE_VERSION},
        {"datacache.dll", STUDIO_DATA_CACHE_INTERFACE_VERSION},
        {"", ""}};
    AddSystem((IAppSystem *)CreateSDLMgr(), SDLMGR_INTERFACE_VERSION);
    if (!AddSystems(appSystems)) return false;
    g_pMaterialSystem = (IMaterialSystem *)FindSystem(MATERIAL_SYSTEM_INTERFACE_VERSION);
    g_pDataCache = (IDataCache *)FindSystem(DATACACHE_INTERFACE_VERSION);
    g_pInputSystem = (IInputSystem *)FindSystem(INPUTSYSTEM_INTERFACE_VERSION);
    g_pStudioRender = (IStudioRender *)FindSystem(STUDIO_RENDER_INTERFACE_VERSION);
    g_pMDLCache = (IMDLCache *)FindSystem(MDLCACHE_INTERFACE_VERSION);
    g_pMaterialSystem->SetShaderAPI("shaderapidx9.dll");
    return true;
}

void CHammerApp::Destroy() {}

SpewRetval_t HammerSpewFunc(SpewType_t type, tchar const *pMsg)
{
    if (type == SPEW_ASSERT) return SPEW_DEBUGGER;
    if (type == SPEW_ERROR) { Msg("Hammer Error %s\n", pMsg); return SPEW_ABORT; }
    return SPEW_CONTINUE;
}

bool CHammerApp::PreInit()
{
    SpewOutputFunc(HammerSpewFunc);
    CFSSearchPathsInit initInfo;
    initInfo.m_pFileSystem = g_pFileSystem;
    initInfo.m_pDirectoryName = "hl2";
    if (FileSystem_LoadSearchPaths(initInfo) != FS_OK) Error("Unable to load search paths!\n");
    if (g_pFileSystem)
    {
        g_pFileSystem->AddSearchPath("hl2/hl2_textures.vpk", "GAME");
        g_pFileSystem->AddSearchPath("hl2/hl2_misc.vpk", "GAME");
        g_pFileSystem->AddSearchPath("hl2", "GAME");
    }
    g_pMaterialSystem->EnableEditorMaterials();
    g_pMaterialSystem->SetAdapter(0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE);
    return true;
}

void CHammerApp::PostShutdown() {}

//-----------------------------------------------------------------------------

enum GizmoMode { 
    GIZMO_NONE = 0, 
    GIZMO_TRANSLATE, 
    GIZMO_ROTATE, 
    GIZMO_SCALE, 
    GIZMO_FACE,
    GIZMO_VERTEX
};

//-----------------------------------------------------------------------------
// 执行 shell 命令，返回是否成功
//-----------------------------------------------------------------------------
static bool RunCommand(const char* cmd)
{
    {
        char buf[1024];
        snprintf(buf, sizeof(buf), "[COMPILE] $ %s", cmd);
        LogBuffer::Instance().Add(buf);
        printf("%s\n", buf);
        fflush(stdout);
    }

    int ret = system(cmd);

    {
        char buf[64];
        snprintf(buf, sizeof(buf), "[COMPILE] exit code = %d", ret);
        LogBuffer::Instance().Add(buf);
        printf("%s\n", buf);
        fflush(stdout);
    }

    return ret == 0;
}

//-----------------------------------------------------------------------------
// 一键编译 + 启动 HL2
//-----------------------------------------------------------------------------
static void CompileAndRun()
{
    char cmd[1024];
    const char* mapName = g_compileSettings.mapName;
    const char* gameDir = g_compileSettings.gameDir;

    char vmfPath[128];
    char bspPath[128];
    snprintf(vmfPath, sizeof(vmfPath), "%s.vmf", mapName);
    snprintf(bspPath, sizeof(bspPath), "%s.bsp", mapName);

    // 1. 导出 VMF
    MapDocument doc;
    ConvertEditorToDocument(doc);
    if (!VmfIO::SaveToFile(vmfPath, doc))
    {
        LogBuffer::Instance().Add("[COMPILE] VMF export FAILED");
        printf("[COMPILE] VMF export FAILED\n");
        return;
    }
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "[COMPILE] VMF export OK: %s", vmfPath);
        LogBuffer::Instance().Add(buf);
        printf("%s\n", buf);
    }

    // 2. VBSP
    if (g_compileSettings.bRunVbsp)
    {
        snprintf(cmd, sizeof(cmd), "./vbsp -game %s %s %s",
                 gameDir, g_compileSettings.vbspArgs, vmfPath);
        if (!RunCommand(cmd))
        {
            LogBuffer::Instance().Add("[COMPILE] VBSP FAILED");
            printf("[COMPILE] VBSP FAILED\n");
            return;
        }
    }

    // 3. VVIS
    if (g_compileSettings.bRunVvis)
    {
        snprintf(cmd, sizeof(cmd), "./vvis -game %s %s %s",
                 gameDir, g_compileSettings.vvisArgs, bspPath);
        if (!RunCommand(cmd))
        {
            LogBuffer::Instance().Add("[COMPILE] VVIS FAILED");
            printf("[COMPILE] VVIS FAILED\n");
            return;
        }
    }

    // 4. VRAD
    if (g_compileSettings.bRunVrad)
    {
        snprintf(cmd, sizeof(cmd), "./vrad -game %s %s %s",
                 gameDir, g_compileSettings.vradArgs, bspPath);
        if (!RunCommand(cmd))
        {
            LogBuffer::Instance().Add("[COMPILE] VRAD FAILED");
            printf("[COMPILE] VRAD FAILED\n");
            return;
        }
    }

    // 5. 复制 BSP
    if (g_compileSettings.bCopyToMaps)
    {
        snprintf(cmd, sizeof(cmd), "cp %s %s/maps/%s", bspPath, gameDir, bspPath);
        if (!RunCommand(cmd))
        {
            LogBuffer::Instance().Add("[COMPILE] cp FAILED");
            printf("[COMPILE] cp FAILED\n");
            return;
        }
    }

    // 6. 启动 HL2
    if (g_compileSettings.bLaunchGame)
    {
        snprintf(cmd, sizeof(cmd), "./hl2_launcher -game %s -console +map %s",
                 gameDir, mapName);
        RunCommand(cmd);
    }

    LogBuffer::Instance().Add("[COMPILE] Done.");
    printf("[COMPILE] Done.\n");
}

// 把 v 吸附到最近网格
inline float SnapToGrid(float v)
{
    if (g_flGridSize <= 0.0f) return v;
    return roundf(v / g_flGridSize) * g_flGridSize;
}

inline Vector SnapToGrid(const Vector& v)
{
    if (g_flGridSize <= 0.0f) return v;
    return Vector(
        roundf(v.x / g_flGridSize) * g_flGridSize,
        roundf(v.y / g_flGridSize) * g_flGridSize,
        roundf(v.z / g_flGridSize) * g_flGridSize
    );
}

//-----------------------------------------------------------------------------

int CHammerApp::Main()
{
    bool bFullscreen = (CommandLine()->CheckParm("-f") != nullptr);
    int w = 1280, h = 720;
    if (bFullscreen)
    {
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0) { w = dm.w; h = dm.h; }
    }
    else
    {
        w = CommandLine()->ParmValue("-w", 1280);
        h = CommandLine()->ParmValue("-h", 720);
    }
    SDL_Window *pWindow = SDL_GL_GetCurrentWindow();
    if (pWindow)
    {
        SDL_SetWindowTitle(pWindow, "Hammer");
        if (bFullscreen)
        {
            SDL_SetWindowSize(pWindow, w, h);
            SDL_SetWindowFullscreen(pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else
        {
            SDL_SetWindowFullscreen(pWindow, 0);
            SDL_SetWindowSize(pWindow, w, h);
            SDL_SetWindowPosition(pWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }
    else
    {
        printf("[DEBUG ERROR] SDL_GL_GetCurrentWindow returned NULL!\n");
        return -1;
    }
    MaterialVideoMode_t mode;
    mode.m_Width = w; mode.m_Height = h; mode.m_Format = IMAGE_FORMAT_RGBA8888; mode.m_RefreshRate = 60;
    MaterialSystem_Config_t config;
    config.m_VideoMode = mode;
    config.SetFlag(MATSYS_VIDCFG_FLAGS_WINDOWED, !bFullscreen);
    if (!g_pMaterialSystem->SetMode((void *)pWindow, config))
        Warning("[HAMMER] Material System SetMode tracking failure.\n");
    pWindow = SDL_GL_GetCurrentWindow();
    if (pWindow) { SDL_ShowWindow(pWindow); SDL_RaiseWindow(pWindow); }

    SDL_GLContext glContext = SDL_GL_GetCurrentContext();
    if (!ImGuiLayer::Init(pWindow, glContext))
        Warning("[HAMMER] ImGuiLayer::Init failed.\n");
    ImGuiIO &io = ImGui::GetIO();

    CreatePlaceholderTextures();
    // ★★★ 加这一行 ★★★
    glActiveTexture(GL_TEXTURE0);

    // 加载 FGD。文件路径按你项目实际位置调整。
    if (!FgdManager::Instance().LoadFgdFile("halflife2.fgd"))
    {
        Warning("[HAMMER] Failed to load halflife2.fgd — Entity Palette will be empty.\n");
    }

    if (!MaterialList::Instance().LoadFromFile("materials.txt"))
    {
        Warning("[HAMMER] Failed to load materials.txt\n");
    }
    else
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "[MAT] Loaded %d materials", 
                (int)MaterialList::Instance().GetAll().size());
        LogBuffer::Instance().Add(buf);
    }

    GizmoPick::Init(w, h);

    // 初始场景：1024×1024 的房间，高 256，带光照 + 出生点
    {
        const float HALF_W   = 512.0f;
        const float WALL_TH  = 16.0f;
        const float FLOOR_H  = 16.0f;
        const float CEIL_H   = 16.0f;
        const float ROOM_H   = 256.0f;

        const float WALL_Z_CENTER = (ROOM_H + CEIL_H - FLOOR_H) * 0.5f;   // 128
        const float WALL_Z_HALF   = (ROOM_H + CEIL_H + FLOOR_H) * 0.5f;   // 144

        const char* MAT_FLOOR = "CONCRETE/CONCRETEFLOOR001A";
        const char* MAT_WALL  = "BRICK/BRICKWALL001A";
        const char* MAT_CEIL  = "CONCRETE/CONCRETECEILING001A";

        // 地板
        {
            CBrush floor;
            floor.m_vecPos  = Vector(0, 0, -FLOOR_H * 0.5f);
            floor.m_vecSize = Vector(HALF_W, HALF_W, FLOOR_H * 0.5f);
            floor.m_szMaterial = MAT_FLOOR;
            floor.RebuildPrimitiveMesh();
            g_brushes.push_back(floor);
        }

        // 天花板
        {
            CBrush ceiling;
            ceiling.m_vecPos  = Vector(0, 0, ROOM_H + CEIL_H * 0.5f);
            ceiling.m_vecSize = Vector(HALF_W, HALF_W, CEIL_H * 0.5f);
            ceiling.m_szMaterial = MAT_CEIL;
            ceiling.RebuildPrimitiveMesh();
            g_brushes.push_back(ceiling);
        }

        // 北墙（+Y）
        {
            CBrush wallN;
            wallN.m_vecPos  = Vector(0, HALF_W + WALL_TH * 0.5f, WALL_Z_CENTER);
            wallN.m_vecSize = Vector(HALF_W + WALL_TH, WALL_TH * 0.5f, WALL_Z_HALF);
            wallN.m_szMaterial = MAT_WALL;
            wallN.RebuildPrimitiveMesh();
            g_brushes.push_back(wallN);
        }

        // 南墙（-Y）
        {
            CBrush wallS;
            wallS.m_vecPos  = Vector(0, -(HALF_W + WALL_TH * 0.5f), WALL_Z_CENTER);
            wallS.m_vecSize = Vector(HALF_W + WALL_TH, WALL_TH * 0.5f, WALL_Z_HALF);
            wallS.m_szMaterial = MAT_WALL;
            wallS.RebuildPrimitiveMesh();
            g_brushes.push_back(wallS);
        }

        // 西墙（-X）
        {
            CBrush wallW;
            wallW.m_vecPos  = Vector(-(HALF_W + WALL_TH * 0.5f), 0, WALL_Z_CENTER);
            wallW.m_vecSize = Vector(WALL_TH * 0.5f, HALF_W + WALL_TH, WALL_Z_HALF);
            wallW.m_szMaterial = MAT_WALL;
            wallW.RebuildPrimitiveMesh();
            g_brushes.push_back(wallW);
        }

        // 东墙（+X）
        {
            CBrush wallE;
            wallE.m_vecPos  = Vector(HALF_W + WALL_TH * 0.5f, 0, WALL_Z_CENTER);
            wallE.m_vecSize = Vector(WALL_TH * 0.5f, HALF_W + WALL_TH, WALL_Z_HALF);
            wallE.m_szMaterial = MAT_WALL;
            wallE.RebuildPrimitiveMesh();
            g_brushes.push_back(wallE);
        }
    }

    // 初始 entity：吸顶灯 + 出生点
    {
        // 吸顶灯：房顶中间偏下，不贴天花板
        {
            CEntity light;
            light.m_iType = ENTITY_MODEL;
            light.m_szName = "light";
            light.m_vecPos = Vector(0, 0, 240);   // 天花板底面 256，light 放 240
            light.m_angRot = QAngle(0, 0, 0);
            light.m_hMdl = MDLHANDLE_INVALID;

            light.m_properties["_light"] = "255 240 200";
            light.m_properties["_distance"] = "1024";
            light.m_properties["_fifty_percent_distance"] = "512";

            g_entities.push_back(light);
        }

        // 出生点：地板正中间偏上
        {
            CEntity spawn;
            spawn.m_iType = ENTITY_PLAYER_START;
            spawn.m_szName = "info_player_start";
            spawn.m_vecPos = Vector(0, 0, 16);   // 地板顶面 0，玩家身高 72，放 z=16
            spawn.m_angRot = QAngle(0, 0, 0);

            std::string modelPath = "models/editor/playerstart.mdl";
            spawn.m_hMdl = g_pMDLCache->FindMDL(modelPath.c_str());

            g_entities.push_back(spawn);
        }
    }

    // 初始没有 entity 被选中。
    g_iSelectedEntity = -1;
    g_iSelectedBrush = -1;

    IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();
    bool bRunning = true;
    SDL_Event event;

    Vector m_camTarget(0, 0, 40);
    float m_camYaw = 90.0f;
    float m_camPitch = -15.0f;
    float m_camDistance = 150.0f;

    Vector g_vecEye(0,0,0), g_vecAt(0,0,0);
    Vector g_camForward(0,0,0), g_camLeft(0,0,0), g_camUp(0,0,0);
    float g_camTanHalfFov = 0.0f, g_camAspect = 1.0f;

    int g_iGizmoMode = GIZMO_TRANSLATE;
    bool g_bShowGizmo = true;
    bool g_bXRayGizmo = false;
    int g_iActiveAxis = -1;
    int g_iHoverAxis = -1;
    float g_flDragStartWorldT = 0.0f;
    //float g_flDragStartAngle = 0.0f;
    Vector g_vecDragStartPos;
    //QAngle g_angDragStartRot;
    Vector g_vecFrozenGizmoCenterAtClick;
    float g_flDragStartSize = 0.0f;   // ★ scale 拖动时保存起始半尺寸
    //int g_iHoverFace = -1;  // Tracks which brush face is hovered
    //int g_iActiveFace = -1; // Tracks which face is actively clicked and dragged    
    int g_iHoverVertex = -1;  // Tracks which of the 8 vertices is hovered (-1 to 7)
    int g_iActiveVertex = -1; // Tracks which vertex is locked during an active drag
    //Vector g_vecDragStartVertexPos; // Caches the individual point's starting position

    bool g_bLeftMouseDown = false;
    bool g_bRightMouseDown = false;
    bool g_bMiddleMouseDown = false;
    bool g_bDraggingGizmo = false;
    int g_iMouseX = 0, g_iMouseY = 0;
    int g_iLastMouseX = 0, g_iLastMouseY = 0;

    // 保存当前帧的投影 / 视图矩阵，供 pick target 使用。
    float g_savedProj[16];
    float g_savedView[16];
    memset(g_savedProj, 0, sizeof(g_savedProj));
    memset(g_savedView, 0, sizeof(g_savedView));

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    uint32_t lastTicks = SDL_GetTicks();

    auto ExportVMF = []()
    {
        MapDocument doc;
        ConvertEditorToDocument(doc);
        char vmfPath[128];
        snprintf(vmfPath, sizeof(vmfPath), "%s.vmf", g_compileSettings.mapName);
        bool ok = VmfIO::SaveToFile(vmfPath, doc);

        char buf[256];
        snprintf(buf, sizeof(buf), "[VMF] Export %s: %s", ok ? "OK" : "FAILED", vmfPath);
        LogBuffer::Instance().Add(buf);
        printf("%s\n", buf);
    };

    SetExportVMFHandler(ExportVMF);
    SetCompileAndRunHandler(CompileAndRun);

    while (bRunning)
    {
        while (SDL_PollEvent(&event))
        {
            ImGuiLayer::ProcessEvent(event);
            switch (event.type)
            {
            case SDL_QUIT: bRunning = false; break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    w = event.window.data1;
                    h = event.window.data2;
                    GizmoPick::Resize(w, h);
                }
                break;
            case SDL_MOUSEMOTION:
                g_iMouseX = event.motion.x; g_iMouseY = event.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                { g_bLeftMouseDown = true; g_iMouseX = event.button.x; g_iMouseY = event.button.y; }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                { g_bRightMouseDown = true; g_iMouseX = event.button.x; g_iMouseY = event.button.y; }
                else if (event.button.button == SDL_BUTTON_MIDDLE)
                { g_bMiddleMouseDown = true; g_iMouseX = event.button.x; g_iMouseY = event.button.y; }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                { 
                    g_bLeftMouseDown = false; 
                    g_bDraggingGizmo = false; 
                    g_iActiveAxis = -1; 
                    g_iActiveFace = -1;
                    g_iActiveVertex = -1;
                }
                else if (event.button.button == SDL_BUTTON_RIGHT) g_bRightMouseDown = false;
                else if (event.button.button == SDL_BUTTON_MIDDLE) g_bMiddleMouseDown = false;
                break;
            case SDL_KEYDOWN:
                if (!io.WantCaptureKeyboard)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE) bRunning = false;
                    else if (event.key.keysym.sym == SDLK_1) g_iGizmoMode = GIZMO_TRANSLATE;
                    else if (event.key.keysym.sym == SDLK_2) g_iGizmoMode = GIZMO_ROTATE;
                    else if (event.key.keysym.sym == SDLK_3) g_iGizmoMode = GIZMO_SCALE;
                    else if (event.key.keysym.sym == SDLK_4) g_iGizmoMode = GIZMO_FACE;
                    else if (event.key.keysym.sym == SDLK_5) g_iGizmoMode = GIZMO_VERTEX;
                    else if (event.key.keysym.sym == SDLK_0) g_iGizmoMode = GIZMO_NONE;
                    else if (event.key.keysym.sym == SDLK_r)
                    {
                        m_camTarget = Vector(0, 0, 40);
                        m_camYaw = 90.0f;
                        m_camPitch = -15.0f;
                        m_camDistance = 450.0f;
                        Msg("[HAMMER] Camera reset\n");
                    }
                    else if (event.key.keysym.sym == SDLK_z && !(event.key.keysym.mod & KMOD_SHIFT) && !(event.key.keysym.mod & KMOD_CTRL))
                    { m_camDistance *= 0.85f; if (m_camDistance < 20.0f) m_camDistance = 20.0f; }
                    else if (event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & KMOD_SHIFT))
                    { m_camDistance *= 1.15f; if (m_camDistance > 3000.0f) m_camDistance = 3000.0f; }
                    else if (event.key.keysym.sym == SDLK_LEFTBRACKET)
                    {
                        // 减小网格
                        if (g_flGridSize <= 1.0f) g_flGridSize = 0.0f;
                        else if (g_flGridSize <= 2.0f) g_flGridSize = 1.0f;
                        else if (g_flGridSize <= 4.0f) g_flGridSize = 2.0f;
                        else if (g_flGridSize <= 8.0f) g_flGridSize = 4.0f;
                        else if (g_flGridSize <= 16.0f) g_flGridSize = 8.0f;
                        else if (g_flGridSize <= 32.0f) g_flGridSize = 16.0f;
                        else if (g_flGridSize <= 64.0f) g_flGridSize = 32.0f;
                        else g_flGridSize = 64.0f;

                        char buf[64];
                        snprintf(buf, sizeof(buf), "[GRID] Grid size = %.0f", g_flGridSize);
                        LogBuffer::Instance().Add(buf);
                    }
                    else if (event.key.keysym.sym == SDLK_RIGHTBRACKET)
                    {
                        // 增大网格
                        if (g_flGridSize < 1.0f) g_flGridSize = 1.0f;
                        else if (g_flGridSize < 2.0f) g_flGridSize = 2.0f;
                        else if (g_flGridSize < 4.0f) g_flGridSize = 4.0f;
                        else if (g_flGridSize < 8.0f) g_flGridSize = 8.0f;
                        else if (g_flGridSize < 16.0f) g_flGridSize = 16.0f;
                        else if (g_flGridSize < 32.0f) g_flGridSize = 32.0f;
                        else if (g_flGridSize < 64.0f) g_flGridSize = 64.0f;
                        else g_flGridSize = 128.0f;

                        char buf[64];
                        snprintf(buf, sizeof(buf), "[GRID] Grid size = %.0f", g_flGridSize);
                        LogBuffer::Instance().Add(buf);
                    }    
                    else if (event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & KMOD_CTRL))
                    {
                        if (UndoStack::Instance().Undo())
                        {
                            LogBuffer::Instance().Add("[UNDO] Undo");
                        }
                        else
                        {
                            LogBuffer::Instance().Add("[UNDO] Nothing to undo");
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_y && (event.key.keysym.mod & KMOD_CTRL))
                    {
                        if (UndoStack::Instance().Redo())
                        {
                            LogBuffer::Instance().Add("[UNDO] Redo");
                        }
                        else
                        {
                            LogBuffer::Instance().Add("[UNDO] Nothing to redo");
                        }
                    }                                    
                }
                break;
            }
        }

        uint32_t currentTicks = SDL_GetTicks();
        float frameTime = (currentTicks - lastTicks) / 1000.0f;
        if (frameTime == 0.0f) frameTime = 0.01f;
        lastTicks = currentTicks;

        // 每帧检查当前是否有有效选中的 entity。
        // 相机 orbit、gizmo hover / drag / draw、pick target、UI 都用它。
        const bool bHasSelection = (!g_entities.empty() &&
                                    g_iSelectedEntity >= 0 &&
                                    g_iSelectedEntity < (int)g_entities.size());

        const bool bHasBrushSelection = (!g_brushes.empty() &&
                                        g_iSelectedBrush >= 0 &&
                                        g_iSelectedBrush < (int)g_brushes.size());

        // gizmo 优先操作 brush（brush 被选中时），否则操作 entity
        const bool bGizmoOnBrush = bHasBrushSelection;
        const bool bGizmoActive  = bGizmoOnBrush || bHasSelection;

        int mouseDX = g_iMouseX - g_iLastMouseX;
        int mouseDY = g_iMouseY - g_iLastMouseY;

        if (!io.WantCaptureKeyboard)
        {
            float flySpeed = m_camDistance * 1.2f * frameTime;
            float rp = m_camPitch * (float)M_PI / 180.0f;
            float ry = m_camYaw * (float)M_PI / 180.0f;
            Vector offset(std::cos(rp) * std::cos(ry), std::cos(rp) * std::sin(ry), std::sin(rp));
            offset *= -m_camDistance;
            Vector eye = m_camTarget + offset;
            Vector forward = m_camTarget - eye;
            VectorNormalize(forward);
            Vector forwardH = forward; forwardH.z = 0.0f;
            if (forwardH.LengthSqr() > 1e-6f) VectorNormalize(forwardH);
            else forwardH = Vector(1, 0, 0);
            Vector worldUp(0, 0, 1);
            Vector leftH;
            CrossProduct(forwardH, worldUp, leftH);
            VectorNormalize(leftH);
            if (keystate[SDL_SCANCODE_W]) m_camTarget += forwardH * flySpeed;
            if (keystate[SDL_SCANCODE_S]) m_camTarget -= forwardH * flySpeed;
            if (keystate[SDL_SCANCODE_A]) m_camTarget += leftH * flySpeed;
            if (keystate[SDL_SCANCODE_D]) m_camTarget -= leftH * flySpeed;
            if (keystate[SDL_SCANCODE_E]) m_camTarget.z += flySpeed;
            if (keystate[SDL_SCANCODE_Q]) m_camTarget.z -= flySpeed;
        }

        if (!io.WantCaptureMouse && !g_bDraggingGizmo)
        {
            if (bHasSelection && g_bLeftMouseDown && g_iHoverAxis < 0)
            {
                m_camYaw -= mouseDX * 0.25f;
                m_camPitch += mouseDY * 0.25f;
                if (m_camPitch > 89.0f) m_camPitch = 89.0f;
                if (m_camPitch < -89.0f) m_camPitch = -89.0f;
            }
            else if (g_bRightMouseDown || g_bMiddleMouseDown)
            {
                float panSpeed = m_camDistance * 0.0018f;
                float ry = m_camYaw * (float)M_PI / 180.0f;
                Vector panLeft(std::sin(ry + (float)M_PI), -std::cos(ry + (float)M_PI), 0);
                Vector panUp(0, 0, 1);
                m_camTarget += panLeft * (mouseDX * panSpeed);
                m_camTarget += panUp * (-mouseDY * panSpeed);
            }
            if (io.MouseWheel != 0.0f)
            {
                m_camDistance *= (1.0f - io.MouseWheel * 0.12f);
                if (m_camDistance < 20.0f) m_camDistance = 20.0f;
                if (m_camDistance > 3000.0f) m_camDistance = 3000.0f;
            }
        }
        //g_iLastMouseX = g_iMouseX;
        //g_iLastMouseY = g_iMouseY;

        {
            float rp = m_camPitch * (float)M_PI / 180.0f;
            float ry = m_camYaw * (float)M_PI / 180.0f;
            Vector offset(std::cos(rp) * std::cos(ry), std::cos(rp) * std::sin(ry), std::sin(rp));
            offset *= -m_camDistance;
            g_vecEye = m_camTarget + offset;
            g_vecAt = m_camTarget;
        }

        for (size_t i = 0; i < g_entities.size(); i++)
            UpdateEntityBBox(g_entities[i]);

        g_pMaterialSystem->BeginFrame(frameTime);
        ImGuiLayer::BeginFrame();

        pRenderContext = g_pMaterialSystem->GetRenderContext();
        if (pRenderContext)
        {
            pRenderContext->ClearColor3ub(51, 51, 51);
            pRenderContext->ClearBuffers(true, true, true);
            pRenderContext->Viewport(0, 0, w, h);
            pRenderContext->DepthRange(0.0f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LEQUAL);
            pRenderContext->Flush(false);

            Vector4D ambientCube[6];
            for (int side = 0; side < 6; side++)
                ambientCube[side].Init(1.0f, 1.0f, 1.0f, 1.0f);
            pRenderContext->SetAmbientLightCube(ambientCube);

            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->LoadIdentity();
            double aspect = (double)w / (double)h;
            pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);

            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->LoadIdentity();
            Vector forward = g_vecAt - g_vecEye;
            VectorNormalize(forward);
            Vector vecWorldUp(0, 0, 1);
            Vector left;
            CrossProduct(forward, vecWorldUp, left);
            VectorNormalize(left);
            Vector up;
            CrossProduct(left, forward, up);
            VectorNormalize(up);
            VMatrix matView;
            matView.Init(left.x, left.y, left.z, -DotProduct(left, g_vecEye), up.x, up.y, up.z, -DotProduct(up, g_vecEye), -forward.x, -forward.y, -forward.z, DotProduct(forward, g_vecEye), 0.0f, 0.0f, 0.0f, 1.0f);
            pRenderContext->LoadMatrix(matView);

            g_camForward = forward;
            g_camLeft = left;
            g_camUp = up;
            g_camTanHalfFov = tanf(45.0f * 0.5f * (float)M_PI / 180.0f);
            g_camAspect = (float)w / (float)h;

            for (size_t i = 0; i < g_entities.size(); i++)
                RenderEntityModel(g_entities[i], pRenderContext);

            pRenderContext->Flush(true);

            glUseProgram(0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDisable(GL_BLEND);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glDisable(GL_SCISSOR_TEST);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);

            glViewport(0, 0, w, h);

            // 从引擎拿矩阵
            VMatrix engineProj, engineView;
            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->GetMatrix(MATERIAL_PROJECTION, &engineProj);
            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->GetMatrix(MATERIAL_VIEW, &engineView);

            float glProj[16];
            float glView[16];
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    glProj[i * 4 + j] = engineProj.m[j][i];
                    glView[i * 4 + j] = engineView.m[j][i];
                }
            }

            // 【关键修复】引擎 D3D 后端使用 Y 轴朝下的坐标系，
            // 而 OpenGL 期望 Y 轴朝上。翻转 glView 的 Y 行。
            glView[1]  = -glView[1];
            glView[5]  = -glView[5];
            glView[9]  = -glView[9];
            glView[13] = -glView[13];

            memcpy(g_savedProj, glProj, sizeof(glProj));
            memcpy(g_savedView, glView, sizeof(glView));

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(glProj);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glView);

            // ★ THE CORRECTION STEP 1: Compute axisLength BEFORE the brush render loop pass! ★
            Vector gizmoCenterFallback;
            float bboxSizeFallback = 0.0f;
            if (bHasBrushSelection)
            {
                CBrush& b = g_brushes[g_iSelectedBrush];
                gizmoCenterFallback = b.m_vecPos;
                bboxSizeFallback = std::max(std::max(b.m_vecSize.x, b.m_vecSize.y), b.m_vecSize.z) * 2.0f;
            }
            else if (bHasSelection)
            {
                CEntity& e = g_entities[g_iSelectedEntity];
                gizmoCenterFallback = e.GetCenter();
                bboxSizeFallback = std::max(std::max(e.m_vecBBoxMaxs.x - e.m_vecBBoxMins.x,
                                            e.m_vecBBoxMaxs.y - e.m_vecBBoxMins.y),
                                    e.m_vecBBoxMaxs.z - e.m_vecBBoxMins.z);
            }
            float localAxisLength = bboxSizeFallback * 1.2f;
            if (localAxisLength < 30.0f) localAxisLength = 30.0f;

            // ===== 绘制可见 Brushes (Textured Surfaces, Edge Wireframes, and ALL Rotating/Scaling Gizmos) =====
            glEnable(GL_TEXTURE_2D);
            for (size_t i = 0; i < g_brushes.size(); i++)
            {
                // Group ALL brush components inside the local matrix transformation stack safely
                glPushMatrix();
                
                glTranslatef(g_brushes[i].m_vecPos.x, g_brushes[i].m_vecPos.y, g_brushes[i].m_vecPos.z);
                glRotatef(g_brushes[i].m_angRot.y, 0.0f, 1.0f, 0.0f); // Yaw (Green Ring)
                glRotatef(g_brushes[i].m_angRot.x, 1.0f, 0.0f, 0.0f); // Pitch (Red Ring)
                glRotatef(g_brushes[i].m_angRot.z, 0.0f, 0.0f, 1.0f); // Roll (Blue Ring)
                glTranslatef(-g_brushes[i].m_vecPos.x, -g_brushes[i].m_vecPos.y, -g_brushes[i].m_vecPos.z);

                DrawBrush(g_brushes[i]);
                DrawBrushEdges(g_brushes[i]);
                
                if ((int)i == g_iSelectedBrush)
                {
                    glDepthMask(GL_FALSE);
                    DrawBrushBBox(g_brushes[i], true); 
                    glDepthMask(GL_TRUE);
                }

                // Render your 3D editing spheres over corners inside vertex mode
                if (g_iGizmoMode == GIZMO_VERTEX && (int)i == g_iSelectedBrush)
                {
                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_DEPTH_TEST); 
                    for (int v = 0; v < 8; v++)
                    {
                        Vector vPos = g_brushes[i].m_vertices[v];
                        if (v == g_iHoverAxis) glColor3ub(255, 255, 0);   // Flash bright yellow on hover!
                        else glColor3ub(255, 255, 255);                 

                        DrawGlowSphere(vPos.x, vPos.y, vPos.z, 3.0f, 8);
                    }
                    glEnable(GL_DEPTH_TEST);
                }

                // ★ THE UNIFIED GIZMO DRAW ENGINE: Draw ALL 3 Gizmos inside matching local spaces! ★
                // Draw your active Translate and Scaling handles inside local space
                if ((int)i == g_iSelectedBrush && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
                {
                    int highlightAxis = (g_iActiveAxis >= 0) ? g_iActiveAxis : g_iHoverAxis;
                    
                    // ★ THE RESTORATION: Use the local brush position variable directly! ★
                    if (g_iGizmoMode == GIZMO_TRANSLATE)
                        DrawTranslateGizmo(g_brushes[i].m_vecPos, localAxisLength, highlightAxis);
                    else if (g_iGizmoMode == GIZMO_SCALE)
                        DrawScaleGizmo(g_brushes[i].m_vecPos, localAxisLength, highlightAxis);
                    else if (g_iGizmoMode == GIZMO_ROTATE)
                    {
                        // Draw your rotation rings with pixel-perfect yellow feedback!
                        unsigned char rX = 255, gX = 0,   bX = 0;
                        unsigned char rY = 0,   gY = 255, bY = 0;
                        unsigned char rZ = 0,   gZ = 128, bZ = 255;
                        
                        if (highlightAxis == 0) { rX = 255; gX = 255; bX = 0; } // Active ring turns yellow!
                        if (highlightAxis == 1) { rY = 255; gY = 255; bY = 0; }
                        if (highlightAxis == 2) { rZ = 255; gZ = 255; bZ = 0; }
                        
                        DrawRotationRing(g_brushes[i].m_vecPos, 0, localAxisLength, 64, rX, gX, bX);
                        DrawRotationRing(g_brushes[i].m_vecPos, 1, localAxisLength, 64, rY, gY, bY);
                        DrawRotationRing(g_brushes[i].m_vecPos, 2, localAxisLength, 64, rZ, gZ, bZ);
                    }
                }
                
                glPopMatrix(); // Safely clear matrix parameters
            }
            glDisable(GL_TEXTURE_2D);

            // =========================================================================
            // ★ MAKE SURE LINES 482-508 IN YOUR CURRENT FILE ARE REMOVED OR DELETED! ★
            // =========================================================================
/*
            // Locate your brush loops inside your immediate OpenGL viewport pass:
            for (size_t i = 0; i < g_brushes.size(); i++)
            {
                // --- Part 1: Keep ALL local brush geometry inside the matrix stack ---
                glPushMatrix();
                
                glTranslatef(g_brushes[i].m_vecPos.x, g_brushes[i].m_vecPos.y, g_brushes[i].m_vecPos.z);
                glRotatef(g_brushes[i].m_angRot.y, 0.0f, 1.0f, 0.0f);
                glRotatef(g_brushes[i].m_angRot.x, 1.0f, 0.0f, 0.0f);
                glRotatef(g_brushes[i].m_angRot.z, 0.0f, 0.0f, 1.0f);
                glTranslatef(-g_brushes[i].m_vecPos.x, -g_brushes[i].m_vecPos.y, -g_brushes[i].m_vecPos.z);
                
                // Draw the textured faces and wireframe outlines together
                DrawBrush(g_brushes[i]);
                DrawBrushEdges(g_brushes[i]);
                
                // Draw selection handles over the dynamic corners
                if (g_iGizmoMode == GIZMO_VERTEX && (int)i == g_iSelectedBrush)
                {
                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_DEPTH_TEST); 
                    for (int v = 0; v < 8; v++)
                    {
                        Vector vPos = g_brushes[i].m_vertices[v];
                        if (v == g_iHoverAxis) glColor3ub(255, 255, 0); 
                        else glColor3ub(255, 255, 255);

                        float hSz = 2.0f;
                        glBegin(GL_LINE_LOOP);
                        glVertex3f(vPos.x-hSz, vPos.y-hSz, vPos.z+hSz); 
                        glVertex3f(vPos.x+hSz, vPos.y-hSz, vPos.z+hSz);
                        glVertex3f(vPos.x+hSz, vPos.y+hSz, vPos.z+hSz); 
                        glVertex3f(vPos.x-hSz, vPos.y+hSz, vPos.z+hSz);
                        glEnd();
                    }
                    glEnable(GL_DEPTH_TEST);
                }
                
                glPopMatrix(); // ★ Pop matrix here to reset safely back to world space!

                // --- Part 2: Draw only the yellow selection box in absolute world space ---
                if ((int)i == g_iSelectedBrush)
                {
                    glDepthMask(GL_FALSE);
                    DrawBrushBBox(g_brushes[i], true); // Resolves the floating box issue perfectly
                    glDepthMask(GL_TRUE);
                }
            }
*/
            glDepthMask(GL_FALSE);
            for (size_t i = 0; i < g_entities.size(); i++)
                DrawEntityBBox(g_entities[i], (int)i == g_iSelectedEntity);
            //for (size_t i = 0; i < g_brushes.size(); i++)
            //    DrawBrushBBox(g_brushes[i], (int)i == g_iSelectedBrush);
            glDepthMask(GL_TRUE);

            if (g_bXRayGizmo)
                glDisable(GL_DEPTH_TEST);

            Vector gizmoCenter;
            float bboxSize = 0.0f;
            if (bGizmoOnBrush)
            {
                CBrush& b = g_brushes[g_iSelectedBrush];
                gizmoCenter = b.m_vecPos;
                bboxSize = std::max(std::max(b.m_vecSize.x, b.m_vecSize.y), b.m_vecSize.z) * 2.0f;
            }
            else if (bHasSelection)
            {
                CEntity& e = g_entities[g_iSelectedEntity];
                gizmoCenter = e.GetCenter();
                bboxSize = std::max(std::max(e.m_vecBBoxMaxs.x - e.m_vecBBoxMins.x,
                                            e.m_vecBBoxMaxs.y - e.m_vecBBoxMins.y),
                                    e.m_vecBBoxMaxs.z - e.m_vecBBoxMins.z);
            }
            float axisLength = bboxSize * 1.2f;
            if (axisLength < 30.0f) axisLength = 30.0f;

            Vector axisDirs[3] = { Vector(1,0,0), Vector(0,1,0), Vector(0,0,1) };

            //float ndcX = (g_iMouseX / (float)w) * 2.0f - 1.0f;
            //float ndcY = 1.0f - (g_iMouseY / (float)h) * 2.0f;
            // 1. Query the explicit position and size of the current central view space from ImGui
            ImVec2 canvasMin = ImGui::GetMainViewport()->WorkPos; // Basal offset of your app workspace
            float localMouseX = (float)g_iMouseX - canvasMin.x;
            float localMouseY = (float)g_iMouseY - canvasMin.y;

            // 2. Derive true viewport-centric NDC coordinates
            float ndcX = (localMouseX / (float)w) * 2.0f - 1.0f;
            float ndcY = 1.0f - (localMouseY / (float)h) * 2.0f;

            Vector rayDir = g_camForward - g_camLeft * (ndcX * g_camTanHalfFov * g_camAspect) + g_camUp * (ndcY * g_camTanHalfFov);
            VectorNormalize(rayDir);
            Vector rayOrigin = g_vecEye;

            // ===== 渲染 pick target（离屏 ID 缓冲）=====
            if (bGizmoActive && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE
                && !(g_iGizmoMode == GIZMO_SCALE && !bGizmoOnBrush))   // ★ scale 只对 brush 生效
            {
                GizmoPick::BeginRender(g_savedProj, g_savedView);
                
                // ★ THE ABSOLUTE RESOLUTION: Enforce full Depth testing for the offscreen texture buffer! ★
                // This blocks backside pixels from blindly overwriting foreground surfaces from your camera view.
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_TRUE);
                glDepthFunc(GL_LEQUAL);
                glClear(GL_DEPTH_BUFFER_BIT); // Clear old depth remnants from previous frames

                if (g_iGizmoMode == GIZMO_TRANSLATE)
                {
                    for (int a = 0; a < 3; a++)
                        GizmoPick::DrawPickableTranslateArrow(a, gizmoCenter, axisLength);
                }
                else if (g_iGizmoMode == GIZMO_ROTATE)
                {
                    for (int a = 0; a < 3; a++)
                        GizmoPick::DrawPickableRotateRing(a, gizmoCenter, axisLength, 64);
                }
                else if (g_iGizmoMode == GIZMO_SCALE)
                {
                    for (int a = 0; a < 3; a++)
                        GizmoPick::DrawPickableScaleHandle(a, gizmoCenter, axisLength);
                }
                else if (g_iGizmoMode == GIZMO_FACE && bGizmoOnBrush)
                {
                    const CBrush& b = g_brushes[g_iSelectedBrush];
                    
                    // ★ APPLY MATRIX TRANSFORMS TO FACE PULLING FBO BUFFERS TOO! ★
                    glPushMatrix();
                    glTranslatef(b.m_vecPos.x, b.m_vecPos.y, b.m_vecPos.z);
                    glRotatef(b.m_angRot.y, 0.0f, 1.0f, 0.0f);
                    glRotatef(b.m_angRot.x, 1.0f, 0.0f, 0.0f);
                    glRotatef(b.m_angRot.z, 0.0f, 0.0f, 1.0f);
                    glTranslatef(-b.m_vecPos.x, -b.m_vecPos.y, -b.m_vecPos.z);

                    GizmoPick::DrawPickableBrushFaces(b);
                    
                    glPopMatrix(); // Safely clear matrix parameters
                }    
                // ★ FIX 1: Wrap the offscreen vertex picker back inside the rotation matrix stack! ★
                else if (g_iGizmoMode == GIZMO_VERTEX && bGizmoOnBrush)
                {
                    const CBrush& b = g_brushes[g_iSelectedBrush];
                    
                    glPushMatrix();
                    glTranslatef(b.m_vecPos.x, b.m_vecPos.y, b.m_vecPos.z);
                    glRotatef(b.m_angRot.y, 0.0f, 1.0f, 0.0f);
                    glRotatef(b.m_angRot.x, 1.0f, 0.0f, 0.0f);
                    glRotatef(b.m_angRot.z, 0.0f, 0.0f, 1.0f);
                    glTranslatef(-b.m_vecPos.x, -b.m_vecPos.y, -b.m_vecPos.z);

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_DEPTH_TEST); 

                    for (int i = 0; i < 8; i++)
                    {
                        Vector vPos = b.m_vertices[i];
                        glColor4ub(i + 1, 0, 0, 255); 
                        DrawGlowSphere(vPos.x, vPos.y, vPos.z, 3.5f, 8); 
                    }
                    glEnable(GL_DEPTH_TEST);
                    glPopMatrix(); 
                }

                   
                GizmoPick::EndRender();

                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(g_savedProj);
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(g_savedView);
            }

            // ===== Hover 检测 =====
            if (bGizmoActive && g_iGizmoMode != GIZMO_NONE && !(g_iGizmoMode == GIZMO_SCALE && !bGizmoOnBrush))
            {
                ImVec2 centralWorkPos = ImGui::GetMainViewport()->WorkPos; 
                float localMouseX = (float)g_iMouseX - centralWorkPos.x;
                float localMouseY = (float)g_iMouseY - centralWorkPos.y;

                if (!g_bDraggingGizmo && g_bShowGizmo)
                {
                    g_iHoverAxis = -1;
                    g_iHoverFace = FACE_NONE; // Reset face tracker token
                    
                    float gizmoScreenX, gizmoScreenY;
                    bool gizmoVisible = WorldToScreen(gizmoCenter, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                                    g_camTanHalfFov, g_camAspect, w, h, gizmoScreenX, gizmoScreenY);
                    if (gizmoVisible)
                    {
                        float mdx = localMouseX - gizmoScreenX;
                        float mdy = localMouseY - gizmoScreenY;
                        float mouseToGizmoSq = mdx * mdx + mdy * mdy;
                        const float MAX_SCREEN_DIST = 3000.0f;
                        if (mouseToGizmoSq < MAX_SCREEN_DIST * MAX_SCREEN_DIST)
                        {
                            if (g_iGizmoMode == GIZMO_TRANSLATE)
                            {
                                GizmoPick::HandleType htype;
                                int hitAxis = GizmoPick::Pick((int)localMouseX, (int)localMouseY, htype);
                                if (hitAxis >= 0 && htype == GizmoPick::HANDLE_TRANSLATE)
                                    g_iHoverAxis = hitAxis;
                            }
                            else if (g_iGizmoMode == GIZMO_ROTATE)
                            {
                                GizmoPick::HandleType htype;
                                int hitAxis = GizmoPick::Pick((int)localMouseX, (int)localMouseY, htype);
                                if (hitAxis >= 0 && htype == GizmoPick::HANDLE_ROTATE)
                                    g_iHoverAxis = hitAxis;
                            }
                            else if (g_iGizmoMode == GIZMO_SCALE && bGizmoOnBrush)
                            {
                                GizmoPick::HandleType htype;
                                int hitAxis = GizmoPick::Pick((int)localMouseX, (int)localMouseY, htype);
                                if (hitAxis >= 0 && htype == GizmoPick::HANDLE_SCALE)
                                    g_iHoverAxis = hitAxis;
                            }
                            // ★ CLEAN FIX 1: Store face hovers in their own dedicated variable! ★
                            else if (g_iGizmoMode == GIZMO_FACE && bGizmoOnBrush)
                            {
                                GizmoPick::HandleType htype;
                                int facePixelID = GizmoPick::Pick((int)localMouseX, (int)localMouseY, htype);
                                if (facePixelID >= 0)
                                {
                                    g_iHoverFace = facePixelID; 
                                }
                            }
                            else if (g_iGizmoMode == GIZMO_VERTEX && bGizmoOnBrush)
                            {
                                GizmoPick::HandleType htype;
                                int vertexPixelID = GizmoPick::Pick((int)localMouseX, (int)localMouseY, htype);
                                if (vertexPixelID >= 0 && vertexPixelID < 8)
                                {
                                    g_iHoverAxis = vertexPixelID; // Set HoverAxis >= 0 to open Click Guard checks
                                }
                            }                            
                        }
                    }
                }
                else if (!g_bDraggingGizmo)
                {
                    g_iHoverAxis = -1;
                    g_iHoverFace = FACE_NONE;
                }
            }
            else
            {
                g_iHoverAxis = -1;
                g_iHoverFace = FACE_NONE;
            }

            // ===== 点击起始拖动 =====
            if (g_bLeftMouseDown && !g_bDraggingGizmo && g_iActiveAxis < 0 && !io.WantCaptureMouse)
            {
                if (g_iGizmoMode == GIZMO_VERTEX && bGizmoOnBrush)
                {
                    GizmoPick::HandleType htype;
                    int vertexPixelID = GizmoPick::Pick((int)localMouseX, (int)localMouseY, htype);
                    if (vertexPixelID >= 0 && vertexPixelID < 8)
                    {
                        UndoStack::Instance().PushSnapshot("Vertex Edit");   // ★
                        g_iActiveVertex = vertexPixelID;
                        g_bDraggingGizmo = true;
                        g_iActiveAxis = 0; // Lock safety constraint placeholder safely
                        
                        //g_vecDragStartVertexPos = g_brushes[g_iSelectedBrush].m_vertices[vertexPixelID];
                        g_vecFrozenGizmoCenterAtClick = gizmoCenter;
                        g_flDragStartWorldT = -1.0f;
                    }
                }
                else if (g_iGizmoMode == GIZMO_FACE && bGizmoOnBrush)
                {
                    if (g_iHoverFace != FACE_NONE)
                    {
                        UndoStack::Instance().PushSnapshot("Face Edit");   // ★
                        g_iActiveFace = g_iHoverFace;
                        g_bDraggingGizmo = true;
                        g_vecFrozenGizmoCenterAtClick = gizmoCenter; 
                        g_vecDragStartPos = g_brushes[g_iSelectedBrush].m_vecPos;

                        if (g_iActiveFace == 2 || g_iActiveFace == 3)      g_iActiveAxis = 0;
                        else if (g_iActiveFace == 4 || g_iActiveFace == 5) g_iActiveAxis = 1;
                        else if (g_iActiveFace == 0 || g_iActiveFace == 1) g_iActiveAxis = 2;

                        if (g_iActiveAxis == 0)      g_flDragStartSize = g_brushes[g_iSelectedBrush].m_vecSize.x;
                        else if (g_iActiveAxis == 1) g_flDragStartSize = g_brushes[g_iSelectedBrush].m_vecSize.y;
                        else                          g_flDragStartSize = g_brushes[g_iSelectedBrush].m_vecSize.z;

                        g_flDragStartWorldT = -1.0f;
                    }
                }
                // ★ CLEAN ISOLATION: Re-route general gizmo inputs inside this explicit ELSE branch! ★
                else if (bGizmoActive && g_iHoverAxis >= 0 && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
                {
                    g_vecFrozenGizmoCenterAtClick = gizmoCenter; 

                    if (g_iGizmoMode == GIZMO_TRANSLATE)
                    {
                        UndoStack::Instance().PushSnapshot("Translate");   // ★
                        g_iActiveAxis = g_iHoverAxis;
                        g_bDraggingGizmo = true;
                        if (bGizmoOnBrush) g_vecDragStartPos = g_brushes[g_iSelectedBrush].m_vecPos;
                        else g_vecDragStartPos = g_entities[g_iSelectedEntity].m_vecPos;
                        g_flDragStartWorldT = -1.0f; 
                    }
                    else if (g_iGizmoMode == GIZMO_ROTATE)
                    {
                        UndoStack::Instance().PushSnapshot("Rotate");   // ★
                        g_iActiveAxis = g_iHoverAxis;
                        g_bDraggingGizmo = true;
                        //g_flDragStartAngle = -1000.0f; 
                        //if (bGizmoOnBrush) g_angDragStartRot = g_brushes[g_iSelectedBrush].m_angRot;
                        //else g_angDragStartRot = g_entities[g_iSelectedEntity].m_angRot;
                    }
                    else if (g_iGizmoMode == GIZMO_SCALE && bGizmoOnBrush)
                    {
                        UndoStack::Instance().PushSnapshot("Scale");   // ★
                        g_iActiveAxis = g_iHoverAxis;
                        g_bDraggingGizmo = true;
                        g_vecDragStartPos = g_brushes[g_iSelectedBrush].m_vecPos;

                        if (g_iActiveAxis == 0)      g_flDragStartSize = g_brushes[g_iSelectedBrush].m_vecSize.x;
                        else if (g_iActiveAxis == 1) g_flDragStartSize = g_brushes[g_iSelectedBrush].m_vecSize.y;
                        else                          g_flDragStartSize = g_brushes[g_iSelectedBrush].m_vecSize.z;

                        g_flDragStartWorldT = -1.0f;
                    }                    
                }
                else
                {
                    ImVec2 canvasMin = ImGui::GetMainViewport()->WorkPos; 
                    float localMouseX = (float)g_iMouseX - canvasMin.x;
                    float localMouseY = (float)g_iMouseY - canvasMin.y;

                    int hitEntity = -1;
                    for (int i = (int)g_entities.size() - 1; i >= 0; i--)
                    {
                        if (IsMouseOverEntityBBox(g_entities[i], g_vecEye, g_camLeft, g_camUp, g_camForward,
                                                g_camTanHalfFov, g_camAspect, w, h, (int)localMouseX, (int)localMouseY))
                        { hitEntity = i; break; }
                    }

                    if (hitEntity >= 0)
                    {
                        g_iSelectedEntity = hitEntity;
                        g_iSelectedBrush  = -1;
                    }
                    else
                    {
                        int hitBrush = -1;
                        for (int i = (int)g_brushes.size() - 1; i >= 0; i--)
                        {
                            if (IsMouseOverBrushBBox(g_brushes[i], g_vecEye, g_camLeft, g_camUp, g_camForward,
                                                    g_camTanHalfFov, g_camAspect, w, h, (int)localMouseX, (int)localMouseY))
                            { hitBrush = i; break; }
                        }
                        g_iSelectedBrush  = hitBrush;
                        g_iSelectedEntity = -1;
                    }
                }
            }

            // ===== 拖动中 =====
            if (bGizmoActive && g_bDraggingGizmo && g_bLeftMouseDown && g_iActiveAxis >= 0)
            {
                if (g_iGizmoMode == GIZMO_TRANSLATE)
                {
                    Vector p0 = g_vecFrozenGizmoCenterAtClick; 
                    Vector p1 = g_vecFrozenGizmoCenterAtClick + axisDirs[g_iActiveAxis] * axisLength;
                    
                    float sx0, sy0, sx1, sy1;
                    if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                        WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                    {
                        float dx = sx1 - sx0, dy = sy1 - sy0;
                        float lenSq = dx*dx + dy*dy;
                        if (lenSq > 1.0f)
                        {
                            ImVec2 centralWorkPos = ImGui::GetMainViewport()->WorkPos; 
                            float localMouseX = (float)g_iMouseX - centralWorkPos.x;
                            float localMouseY = (float)g_iMouseY - centralWorkPos.y;

                            float t = ((localMouseX - sx0) * dx + (localMouseY - sy0) * dy) / lenSq;
                            float worldDelta = t * axisLength;

                            if (g_flDragStartWorldT < 0.0f)
                                g_flDragStartWorldT = worldDelta;
                            else
                            {
                                float deltaT = worldDelta - g_flDragStartWorldT;

                                // ★ 绝对位置模式
                                if (bGizmoOnBrush)
                                {
                                    CBrush& activeBrush = g_brushes[g_iSelectedBrush];

                                    // 1. 未吸附的目标位置（从拖动起始位置算）
                                    Vector rawPos = g_vecDragStartPos + axisDirs[g_iActiveAxis] * deltaT;

                                    // 2. 吸附
                                    Vector snappedPos = SnapToGrid(rawPos);

                                    // 3. 计算"从当前到目标"的增量
                                    Vector delta = snappedPos - activeBrush.m_vecPos;

                                    // 4. 应用到顶点和中心
                                    for (int v = 0; v < 8; v++)
                                        activeBrush.m_vertices[v] += delta;
                                    activeBrush.m_vecPos = snappedPos;
                                }
                                else
                                {
                                    // entity：直接设位置
                                    Vector rawPos = g_vecDragStartPos + axisDirs[g_iActiveAxis] * deltaT;
                                    g_entities[g_iSelectedEntity].m_vecPos = SnapToGrid(rawPos);
                                }

                                // ★ 不再更新 g_flDragStartWorldT（因为用的是"绝对位置"模式）
                            }
                        }
                    }
                }
                else if (g_iGizmoMode == GIZMO_ROTATE)
                {
                    // 用像素位移算角度（和之前一样）
                    int screenDeltaX = g_iMouseX - g_iLastMouseX;
                    float deltaDeg = (float)screenDeltaX * 0.5f;

                    if (fabsf(deltaDeg) > 0.001f)
                    {
                        QAngle angDelta(0, 0, 0);
                        if (g_iActiveAxis == 0)      angDelta.x = deltaDeg;
                        else if (g_iActiveAxis == 1) angDelta.y = deltaDeg;
                        else                          angDelta.z = deltaDeg;

                        if (bGizmoOnBrush)
                        {
                            CBrush& activeBrush = g_brushes[g_iSelectedBrush];

                            // 1. 把 delta 旋转直接应用到顶点
                            activeBrush.RotateVertices(angDelta);

                            // 2. 重算 bbox + 法线
                            activeBrush.RecalcBoundsFromVertices();
                            activeBrush.RecalcFaceNormals();

                            // 3. m_angRot 保持 (0,0,0)（旋转已烘进顶点）
                        }
                        else
                        {
                            // entity 仍然用 m_angRot（entity 是点，没有顶点）
                            matrix3x4_t matStart, matDelta, matFinal;
                            AngleMatrix(g_entities[g_iSelectedEntity].m_angRot, matStart);
                            AngleMatrix(angDelta, matDelta);
                            ConcatTransforms(matDelta, matStart, matFinal);
                            QAngle finalAngles;
                            MatrixAngles(matFinal, finalAngles);
                            g_entities[g_iSelectedEntity].m_angRot = finalAngles;
                        }
                    }
                }
                else if (g_iGizmoMode == GIZMO_SCALE && bGizmoOnBrush)
                {
                    Vector p0 = g_vecFrozenGizmoCenterAtClick;
                    Vector p1 = g_vecFrozenGizmoCenterAtClick + axisDirs[g_iActiveAxis] * axisLength;

                    float sx0, sy0, sx1, sy1;
                    if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                        WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                    {
                        float dx = sx1 - sx0, dy = sy1 - sy0;
                        float lenSq = dx*dx + dy*dy;
                        if (lenSq > 1.0f)
                        {
                            ImVec2 centralWorkPos = ImGui::GetMainViewport()->WorkPos;
                            float localMouseX = (float)g_iMouseX - centralWorkPos.x;
                            float localMouseY = (float)g_iMouseY - centralWorkPos.y;

                            float t = ((localMouseX - sx0) * dx + (localMouseY - sy0) * dy) / lenSq;
                            if (g_flDragStartWorldT < 0.0f) g_flDragStartWorldT = t;

                            float deltaT = t - g_flDragStartWorldT;

                            float scaleFactor = 1.0f + deltaT * 0.5f;
                            if (scaleFactor < 0.01f) scaleFactor = 0.01f;

                            CBrush& activeBrush = g_brushes[g_iSelectedBrush];

                            // 1. 缩放顶点（一次）
                            for (int v = 0; v < 8; v++)
                            {
                                Vector localV = activeBrush.m_vertices[v] - activeBrush.m_vecPos;
                                localV[g_iActiveAxis] *= scaleFactor;
                                activeBrush.m_vertices[v] = activeBrush.m_vecPos + localV;
                            }

                            // 2. 从顶点重算 bbox + 法线
                            activeBrush.RecalcBoundsFromVertices();
                            activeBrush.RecalcFaceNormals();

                            g_flDragStartWorldT = t;
                        }
                    }
                }
                else if (g_iGizmoMode == GIZMO_FACE && bGizmoOnBrush && g_iActiveFace != -1)
                {
                    Vector p0 = g_vecFrozenGizmoCenterAtClick;
                    Vector p1 = g_vecFrozenGizmoCenterAtClick + axisDirs[g_iActiveAxis] * axisLength;

                    float sx0, sy0, sx1, sy1;
                    if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                        WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                    {
                        float dx = sx1 - sx0, dy = sy1 - sy0;
                        float lenSq = dx*dx + dy*dy;
                        if (lenSq > 1.0f)
                        {
                            ImVec2 centralWorkPos = ImGui::GetMainViewport()->WorkPos;
                            float localMouseX = (float)g_iMouseX - centralWorkPos.x;
                            float localMouseY = (float)g_iMouseY - centralWorkPos.y;

                            float t = ((localMouseX - sx0) * dx + (localMouseY - sy0) * dy) / lenSq;
                            if (g_flDragStartWorldT < 0.0f) g_flDragStartWorldT = t;   

                            float deltaT = t - g_flDragStartWorldT;
                            
                            // Calculate your displacement distance relative to your click anchor frame
                            float totalExtrusionDelta = deltaT * axisLength;
                            
                            // ★ THE INTERACTION FIX 2: Extrude vertices directly, preserving warped shapes! ★
                            CBrush& activeBrush = g_brushes[g_iSelectedBrush];
                            
                            // Determine displacement direction based on positive vs negative faces
                            float directionalSign = 1.0f;
                            if (g_iActiveFace == 1 || g_iActiveFace == 3 || g_iActiveFace == 5)
                                directionalSign = -1.0f;
                                
                            // Calculate movement vector along the active manipulation axis
                            Vector displacementVector(0, 0, 0);
                            displacementVector[g_iActiveAxis] = totalExtrusionDelta * directionalSign;
                            
                            // Get the 4 vertex points explicitly mapped to our active face index ring
                            // Mapping directly from the standard structural definitions we built in Brush.cpp
                            // ★ THE ALIGNMENT REPAIR: Clean vertex lookup array rings ★
                            int targetVertexIndices[4];
                            if (g_iActiveFace == 0)      { targetVertexIndices[0]=4; targetVertexIndices[1]=5; targetVertexIndices[2]=6; targetVertexIndices[3]=7; } // +Z
                            else if (g_iActiveFace == 1) { targetVertexIndices[0]=0; targetVertexIndices[1]=3; targetVertexIndices[2]=2; targetVertexIndices[3]=1; } // -Z
                            else if (g_iActiveFace == 2) { targetVertexIndices[0]=1; targetVertexIndices[1]=2; targetVertexIndices[2]=6; targetVertexIndices[3]=5; } // +X
                            else if (g_iActiveFace == 3) { targetVertexIndices[0]=0; targetVertexIndices[1]=4; targetVertexIndices[2]=7; targetVertexIndices[3]=3; } // -X
                            else if (g_iActiveFace == 4) { targetVertexIndices[0]=3; targetVertexIndices[1]=7; targetVertexIndices[2]=6; targetVertexIndices[3]=2; } // +Y
                            else                          { targetVertexIndices[0]=0; targetVertexIndices[1]=1; targetVertexIndices[2]=5; targetVertexIndices[3]=4; } // -Y


                            // Extrude ONLY the active face vertices, keeping any custom vertex adjustments intact!
                            for (int v = 0; v < 4; v++)
                            {
                                int idx = targetVertexIndices[v];
                                activeBrush.m_vertices[idx] += displacementVector;
                            }

                            // Recalculate planar orientation geometry normals dynamically to warp textures instantly
                            activeBrush.RecalcBoundsFromVertices();
                            activeBrush.RecalcFaceNormals();
                            
                            // Clean drag tracker states for subsequent frame calculations
                            g_flDragStartWorldT = t; 
                        }
                    }
                }
                // ★ UPGRADE: Clean World-Space Proj-T Drag Engine for Vertex Editing! ★
                else if (g_iGizmoMode == GIZMO_VERTEX && bGizmoOnBrush && g_iActiveVertex != -1)
                {
                    float deltaScreenX = (float)g_iMouseX - (float)g_iLastMouseX;
                    float deltaScreenY = (float)g_iMouseY - (float)g_iLastMouseY;

                    CBrush& activeBrush = g_brushes[g_iSelectedBrush];
                    
                    // Translate flat 2D viewport delta pixels into true 3D spatial transformations
                    float dragSensitivity = m_camDistance * 0.0015f; 
                    Vector worldDisplacement = (g_camLeft * deltaScreenX - g_camUp * deltaScreenY) * dragSensitivity;

                    // Update ONLY the selected handle coordinates point in memory
                    activeBrush.m_vertices[g_iActiveVertex] += worldDisplacement;
                    activeBrush.RecalcBoundsFromVertices();
                    activeBrush.RecalcFaceNormals();

                    // Recalculate planar orientation geometry normals dynamically to warp textures instantly
                    for (int f = 0; f < 6; f++)
                    {
                        int i0 = activeBrush.m_faces[f].m_nVertexIndices[0];
                        int i1 = activeBrush.m_faces[f].m_nVertexIndices[1];
                        int i2 = activeBrush.m_faces[f].m_nVertexIndices[2];
                        
                        Vector v0 = activeBrush.m_vertices[i0];
                        Vector v1 = activeBrush.m_vertices[i1];
                        Vector v2 = activeBrush.m_vertices[i2];
                        
                        Vector edge1 = v1 - v0;
                        Vector edge2 = v2 - v0;
                        Vector faceNormal;
                        CrossProduct(edge1, edge2, faceNormal);
                        VectorNormalize(faceNormal);
                        
                        activeBrush.m_faces[f].m_vecNormal = faceNormal;
                    }
                }
            }    
            // ===== 绘制 entity 的可见 gizmo =====
            // brush 的 gizmo 已经在 brush 循环里画了，这里只画 entity 的
            if (!bGizmoOnBrush && bHasSelection && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
            {
                int highlightAxis = (g_iActiveAxis >= 0) ? g_iActiveAxis : g_iHoverAxis;

                if (g_iGizmoMode == GIZMO_TRANSLATE)
                    DrawTranslateGizmo(gizmoCenter, axisLength, highlightAxis);
                else if (g_iGizmoMode == GIZMO_ROTATE)
                {
                    unsigned char rX = 255, gX = 0,   bX = 0;
                    unsigned char rY = 0,   gY = 255, bY = 0;
                    unsigned char rZ = 0,   gZ = 128, bZ = 255;
                    if (highlightAxis == 0) { rX = 255; gX = 255; bX = 0; }
                    if (highlightAxis == 1) { rY = 255; gY = 255; bY = 0; }
                    if (highlightAxis == 2) { rZ = 255; gZ = 255; bZ = 0; }
                    DrawRotationRing(gizmoCenter, 0, axisLength, 64, rX, gX, bX);
                    DrawRotationRing(gizmoCenter, 1, axisLength, 64, rY, gY, bY);
                    DrawRotationRing(gizmoCenter, 2, axisLength, 64, rZ, gZ, bZ);
                }
                // scale 不支持 entity，不画
            }
            // ===== UI 面板 =====
            DrawMenuBar();

            EditorLayout::Begin();

            DrawCompileOptionsWindow();

            BrushPanelResult brushResult;

            DrawEntityPanel(g_entities.data(), (int)g_entities.size(), &g_iSelectedEntity,
                            &g_iSelectedBrush, brushResult);

            DrawBrushPanel(g_brushes.data(), (int)g_brushes.size(), &g_iSelectedBrush,
                        &g_iSelectedEntity, brushResult);

            // ★ 处理新建 brush
            if (brushResult.bRequestNew)
            {
                UndoStack::Instance().PushSnapshot("New Brush");   // ★
                CBrush b;
                b.m_vecPos = Vector(0, 0, 32);
                b.m_vecSize = Vector(32, 32, 32);
                b.m_iTexId = 0;
                b.RebuildPrimitiveMesh();
                g_brushes.push_back(b);
                g_iSelectedBrush = (int)g_brushes.size() - 1;
            }

            if (brushResult.bRequestDeleteEntity &&
                brushResult.iTargetEntityIndex >= 0 &&
                brushResult.iTargetEntityIndex < (int)g_entities.size())
            {
                UndoStack::Instance().PushSnapshot("Delete Entity");   // ★
                g_entities.erase(g_entities.begin() + brushResult.iTargetEntityIndex);

                // 修正选中索引
                if (g_iSelectedEntity == brushResult.iTargetEntityIndex)
                    g_iSelectedEntity = -1;
                else if (g_iSelectedEntity > brushResult.iTargetEntityIndex)
                    g_iSelectedEntity--;
            }

            // ★ 处理从 Entity Palette 创建新 entity
            if (brushResult.bRequestNewEntity && !brushResult.szNewEntityClass.empty())
            {
                UndoStack::Instance().PushSnapshot("New Entity");   // ★
                std::string modelPath = FgdManager::Instance().GetModelPathForClass(brushResult.szNewEntityClass);
                printf("[Palette] selected class='%s' model='%s'\n",
                    brushResult.szNewEntityClass.c_str(), modelPath.c_str());

                CEntity e;
                e.m_iType = ENTITY_MODEL;
                e.m_szName = brushResult.szNewEntityClass;
                e.m_vecPos = Vector(0, 0, 0);

                if (!modelPath.empty())
                {
                    e.m_hMdl = g_pMDLCache->FindMDL(modelPath.c_str());
                    if (e.m_hMdl == MDLHANDLE_INVALID)
                        printf("[Palette] FindMDL failed for '%s'\n", modelPath.c_str());
                }
                else
                {
                    e.m_hMdl = MDLHANDLE_INVALID;
                }

                g_entities.push_back(e);
                g_iSelectedEntity = (int)g_entities.size() - 1;
            }

            if (brushResult.bRequestDelete &&
                brushResult.iTargetIndex >= 0 &&
                brushResult.iTargetIndex < (int)g_brushes.size())
            {
                UndoStack::Instance().PushSnapshot("Delete Brush");   // ★ 加这个
                g_brushes.erase(g_brushes.begin() + brushResult.iTargetIndex);
                g_iSelectedBrush = -1;
            }

            if (brushResult.bRequestDuplicate &&
                brushResult.iTargetIndex >= 0 &&
                brushResult.iTargetIndex < (int)g_brushes.size())
            {
                UndoStack::Instance().PushSnapshot("Duplicate Brush");   // ★ 加这个
                CBrush b = g_brushes[brushResult.iTargetIndex];

                Vector offset(32.0f, 0, 0);

                // ★ 关键：顶点也平移
                for (auto& v : b.m_vertices)
                    v += offset;

                // m_vecPos 也平移（和顶点保持一致）
                b.m_vecPos += offset;

                g_brushes.push_back(b);
                g_iSelectedBrush = (int)g_brushes.size() - 1;
            }

            DrawConsolePanel(m_camTarget, m_camDistance, g_bShowGizmo, g_bXRayGizmo);

            EditorLayout::End();

            ImGuiLayer::EndFrameAndRender(w, h);
        }
        g_pMaterialSystem->EndFrame();
        g_pMaterialSystem->SwapBuffers();

        // ★ THE FIX: Record the historical mouse position at the VERY END of the frame! ★
        g_iLastMouseX = g_iMouseX;
        g_iLastMouseY = g_iMouseY;        
    }

    ImGuiLayer::Shutdown();
    _exit(0);
    return 0;
}