#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "appframework/AppFramework.h"
#include "tier0/dbg.h"
#include "vstdlib/cvar.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "istudiorender.h"
#include "filesystem_init.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "vphysics_interface.h"
#include "tier0/icommandline.h"
#include "appframework/ilaunchermgr.h"
#include "mathlib/vmatrix.h"
#include "tier1/KeyValues.h"
#include "studio.h"

// Explicit include for input system interface mapping
#include "inputsystem/iinputsystem.h"

//-----------------------------------------------------------------------------
// Global systems
//-----------------------------------------------------------------------------
IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

extern void *CreateSDLMgr();

// Globals for dynamic model tracking
std::vector<std::string> g_ModelList;
std::string g_CurrentModelPath = "";
std::string g_GameFolder = "hl2";

// 命令行参数
static int g_nWindowWidth = 1280;
static int g_nWindowHeight = 720;
static bool g_bFullscreen = false;
static bool g_bDirectModelMode = false; // Tracks if launched explicitly to render 1 standalone model

// 动画状态
int g_CurrentSequenceIndex = 0;
float g_flAnimTime = 0.0f;
static bool g_bAnimPlaybackPlaying = true;

//-----------------------------------------------------------------------------
// PURE VIRTUAL VPK ARCHIVE RECURSIVE SCANNER
//-----------------------------------------------------------------------------
void ScanModelsDirectoryRecursive(const std::string &path)
{
    FileFindHandle_t findHandle;
    std::string searchFilter = path + "/*";
    const char *pFileName = g_pFileSystem->FindFirstEx(searchFilter.c_str(), "GAME", &findHandle);

    while (pFileName)
    {
        if (pFileName != '\0' && pFileName != '.')
        {
            std::string fullPath = path + "/" + pFileName;

            if (g_pFileSystem->FindIsDirectory(findHandle))
            {
                ScanModelsDirectoryRecursive(fullPath);
            }
            else if (fullPath.size() > 4 && fullPath.substr(fullPath.size() - 4) == ".mdl")
            {
                size_t modelsPos = fullPath.find("models/");
                if (modelsPos != std::string::npos)
                {
                    std::string fixedPath = fullPath.substr(modelsPos);
                    std::replace(fixedPath.begin(), fixedPath.end(), '\\', '/');

                    if (std::find(g_ModelList.begin(), g_ModelList.end(), fixedPath) == g_ModelList.end())
                    {
                        g_ModelList.push_back(fixedPath);
                    }
                }
            }
        }
        pFileName = g_pFileSystem->FindNext(findHandle);
    }
    g_pFileSystem->FindClose(findHandle);
}

//-----------------------------------------------------------------------------
// The application object
//-----------------------------------------------------------------------------
class CHammerApp : public CAppSystemGroup
{
public:
    virtual bool Create();
    virtual bool PreInit();
    virtual int Main();
    virtual void PostShutdown() {}
    virtual void Destroy();
};

CHammerApp g_ApplicationObject;

int main(int argc, char *argv[])
{
    printf("[DEBUG] Entered global main() execution block.\n");
    CommandLine()->CreateCmdLine(argc, argv);

    printf("[DEBUG] Invoking g_ApplicationObject.Run()...\n");
    int nRet = g_ApplicationObject.Run();

    printf("[DEBUG] Application shutting down safely with return code: %d\n", nRet);
    return nRet;
}

bool CHammerApp::Create()
{
    printf("[DEBUG] Inside CHammerApp::Create() stage.\n");
    CommandLine()->AppendParm("-hlmv", NULL);

    g_bFullscreen = CommandLine()->FindParm("-f") != 0;
    g_nWindowWidth = CommandLine()->ParmValue("-w", 1280);
    g_nWindowHeight = CommandLine()->ParmValue("-h", 720);

    printf("[DEBUG] Parsed args: -w=%d, -h=%d, -f=%s\n",
           g_nWindowWidth, g_nWindowHeight, g_bFullscreen ? "yes" : "no");

    if (g_nWindowWidth < 320)
        g_nWindowWidth = 320;
    if (g_nWindowHeight < 240)
        g_nWindowHeight = 240;

    IAppSystem *pSystem;
    AppModule_t cvarModule = LoadModule(VStdLib_GetICVarFactory());
    pSystem = AddSystem(cvarModule, CVAR_INTERFACE_VERSION);
    if (!pSystem)
    {
        printf("[DEBUG ERROR] Failed to bind CVAR module interface.\n");
        return false;
    }

    char pFileSystemDLL[MAX_PATH];
    bool bSteam;
    if (FileSystem_GetFileSystemDLLName(pFileSystemDLL, MAX_PATH, bSteam) != FS_OK)
    {
        printf("[DEBUG ERROR] Failed to query FileSystem DLL filename pointer mappings.\n");
        return false;
    }

    AppModule_t fileSystemModule = LoadModule(pFileSystemDLL);
    g_pFileSystem = (IFileSystem *)AddSystem(fileSystemModule, FILESYSTEM_INTERFACE_VERSION);
    if (!g_pFileSystem)
    {
        printf("[DEBUG ERROR] Failed to load structural FileSystem module factory.\n");
        return false;
    }

    printf("[DEBUG] Setting FileSystem baseline directory path trackers...\n");
    FileSystem_SetBasePaths(g_pFileSystem);

    g_pFileSystem->AddSearchPath("hl2/bin", "BIN");

    //=============================================================================
    // DETERMINING EXCLUSIVE MODE OPERATION AT CREATE TIME
    //=============================================================================
    const char *pModelParamCheck = CommandLine()->ParmValue("-model", nullptr);
    if (!pModelParamCheck)
    {
        int nParmCount = CommandLine()->ParmCount();
        if (nParmCount > 1)
        {
            const char *pLastParam = CommandLine()->GetParm(nParmCount - 1);
            if (pLastParam && pLastParam[0] != '-' && strstr(pLastParam, ".mdl"))
            {
                pModelParamCheck = pLastParam;
            }
        }
    }

    if (pModelParamCheck && strlen(pModelParamCheck) > 4)
    {
        g_bDirectModelMode = true;
        printf("[HLMV] Direct standalone file execution target registered. Skipping VPK scan paths initialization.\n");
    }

    const char *pGameDirParam = CommandLine()->ParmValue("-game", "hl2");
    g_GameFolder = pGameDirParam;

    if (g_GameFolder != "hl2")
    {
        std::string modBinPath = g_GameFolder + "/bin";
        printf("[DEBUG] Injecting active mod binary search path destination: %s\n", modBinPath.c_str());
        g_pFileSystem->AddSearchPath(modBinPath.c_str(), "BIN");

        std::string siblingBinPath = "../" + g_GameFolder + "/bin";
        g_pFileSystem->AddSearchPath(siblingBinPath.c_str(), "BIN");
    }

    AppSystemInfo_t appSystems[] =
        {
            {"materialsystem.dll", MATERIAL_SYSTEM_INTERFACE_VERSION},
            {"inputsystem.dll", INPUTSYSTEM_INTERFACE_VERSION},
            {"studiorender.dll", STUDIO_RENDER_INTERFACE_VERSION},
            {"vphysics.dll", VPHYSICS_INTERFACE_VERSION},
            {"datacache.dll", DATACACHE_INTERFACE_VERSION},
            {"datacache.dll", MDLCACHE_INTERFACE_VERSION},
            {"datacache.dll", STUDIO_DATA_CACHE_INTERFACE_VERSION},
            {"", ""}};

    AddSystem((IAppSystem *)CreateSDLMgr(), SDLMGR_INTERFACE_VERSION);

    if (!AddSystems(appSystems))
    {
        return false;
    }

    g_pMaterialSystem = (IMaterialSystem *)FindSystem(MATERIAL_SYSTEM_INTERFACE_VERSION);
    g_pDataCache = (IDataCache *)FindSystem(DATACACHE_INTERFACE_VERSION);
    g_pInputSystem = (IInputSystem *)FindSystem(INPUTSYSTEM_INTERFACE_VERSION);
    g_pStudioRender = (IStudioRender *)FindSystem(STUDIO_RENDER_INTERFACE_VERSION);
    g_pMDLCache = (IMDLCache *)FindSystem(MDLCACHE_INTERFACE_VERSION);

    g_pMaterialSystem->SetShaderAPI("shaderapidx9.dll");
    return true;
}

void CHammerApp::Destroy()
{
    g_pMaterialSystem = NULL;
    g_pFileSystem = NULL;
    g_pDataCache = NULL;
    g_pInputSystem = NULL;
    g_pStudioRender = NULL;
    g_pMDLCache = NULL;
}

SpewRetval_t HammerSpewFunc(SpewType_t type, tchar const *pMsg)
{
    if (type == SPEW_ASSERT)
        return SPEW_DEBUGGER;
    else if (type == SPEW_ERROR)
    {
        printf("[SPEW CRITICAL ERROR] %s\n", pMsg);
        return SPEW_ABORT;
    }
    else
    {
        return SPEW_CONTINUE;
    }
}

bool CHammerApp::PreInit()
{
    printf("[DEBUG] Inside CHammerApp::PreInit() setup block pass.\n");
    SpewOutputFunc(HammerSpewFunc);

    g_GameFolder = CommandLine()->ParmValue("-game", "hl2");
    printf("[DEBUG] Extracted parsed -game command argument string data token: '%s'\n", g_GameFolder.c_str());

    CFSSearchPathsInit initInfo;
    initInfo.m_pFileSystem = g_pFileSystem;
    initInfo.m_pDirectoryName = g_GameFolder.c_str();

    std::string directGameInfo = g_GameFolder + "/gameinfo.txt";
    std::string siblingGameInfo = "../" + g_GameFolder + "/gameinfo.txt";

    FILE *pFile = fopen(directGameInfo.c_str(), "r");
    if (pFile)
    {
        fclose(pFile);
        initInfo.m_pDirectoryName = g_GameFolder.c_str();
    }
    else
    {
        FILE *pSibFile = fopen(siblingGameInfo.c_str(), "r");
        if (pSibFile)
        {
            fclose(pSibFile);
            g_GameFolder = "../" + g_GameFolder;
            initInfo.m_pDirectoryName = g_GameFolder.c_str();
        }
    }

    // ALWAYS mount the game and mod paths directly to clear any tracking barriers
    g_pFileSystem->AddSearchPath(initInfo.m_pDirectoryName, "GAME");

    printf("[DEBUG] Scanning and mounting virtual VPK split chunks inside: %s\n", initInfo.m_pDirectoryName);
    for (int archiveIdx = 0; archiveIdx < 20; archiveIdx++)
    {
        char szVpkPathBuffer[MAX_PATH];
        snprintf(szVpkPathBuffer, sizeof(szVpkPathBuffer), "%s/%s_pak_%03d.vpk",
                 initInfo.m_pDirectoryName, CommandLine()->ParmValue("-game", "hl2"), archiveIdx);

        FILE *pTestVpk = fopen(szVpkPathBuffer, "rb");
        if (pTestVpk)
        {
            fclose(pTestVpk);
            printf("[DEBUG] Natively injecting discovered packed mod archive target: %s\n", szVpkPathBuffer);
            g_pFileSystem->AddSearchPath(szVpkPathBuffer, "GAME");
        }
    }

    std::string texVpk = std::string(initInfo.m_pDirectoryName) + "/" + CommandLine()->ParmValue("-game", "hl2") + "_textures.vpk";
    std::string misVpk = std::string(initInfo.m_pDirectoryName) + "/" + CommandLine()->ParmValue("-game", "hl2") + "_misc.vpk";

    FILE *pTexF = fopen(texVpk.c_str(), "rb");
    if (pTexF)
    {
        fclose(pTexF);
        g_pFileSystem->AddSearchPath(texVpk.c_str(), "GAME");
    }
    FILE *pMisF = fopen(misVpk.c_str(), "rb");
    if (pMisF)
    {
        fclose(pMisF);
        g_pFileSystem->AddSearchPath(misVpk.c_str(), "GAME");
    }

    if (g_GameFolder != "hl2" && g_GameFolder != "../hl2")
    {
        g_pFileSystem->AddSearchPath("hl2", "GAME");
    }

    printf("[DEBUG] Executing final FileSystem_LoadSearchPaths() pass metrics configuration...\n");
    FileSystem_LoadSearchPaths(initInfo);
    g_pMaterialSystem->EnableEditorMaterials();
    g_pMaterialSystem->SetAdapter(0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE);

    if (g_pMDLCache)
    {
        printf("[DEBUG] Initializing Studio Model Cache Subsystem...\n");
        g_pMDLCache->Init();
    }

    return true;
}

int CHammerApp::Main()
{
    printf("[DEBUG] Inside CHammerApp::Main()...\n");
    SDL_Window *pWindow = SDL_GL_GetCurrentWindow();
    if (!pWindow)
    {
        printf("[DEBUG ERROR] SDL_GL_GetCurrentWindow returned NULL!\n");
        return -1;
    }
    SDL_SetWindowTitle(pWindow, "HLMV");
    SDL_SetWindowPosition(pWindow, 0, 0);
    int w = g_nWindowWidth;
    int h = g_nWindowHeight;
    if (g_bFullscreen)
    {
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0)
        {
            w = dm.w;
            h = dm.h;
        }
        SDL_SetWindowFullscreen(pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        printf("[DEBUG] Fullscreen mode: using %dx%d\n", w, h);
    }
    else
    {
        SDL_SetWindowFullscreen(pWindow, 0);
        SDL_SetWindowSize(pWindow, w, h);
        printf("[DEBUG] Windowed mode: %dx%d\n", w, h);
    }
    SDL_SetWindowResizable(pWindow, SDL_FALSE);
    MaterialVideoMode_t mode;
    mode.m_Width = w;
    mode.m_Height = h;
    mode.m_Format = IMAGE_FORMAT_RGBA8888;
    mode.m_RefreshRate = 60;
    MaterialSystem_Config_t config;
    config.m_VideoMode = mode;
    config.SetFlag(MATSYS_VIDCFG_FLAGS_WINDOWED, !g_bFullscreen);
    if (!g_pMaterialSystem->SetMode((void *)pWindow, config))
    {
        Warning("[HLMV] Material System SetMode tracking failure.\n");
    }
    pWindow = SDL_GL_GetCurrentWindow();
    SDL_ShowWindow(pWindow);
    SDL_RaiseWindow(pWindow);
    SDL_GLContext glContext = SDL_GL_GetCurrentContext();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2((float)w, (float)h);
    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (!g_bDirectModelMode)
    {
        printf("[DEBUG] Beginning file asset traversal scan across VPK layers for model lists...\n");
        ScanModelsDirectoryRecursive("models");
        printf("[DEBUG SUCCESS] Traversal scan concluded. Total distinct model path assets tracked: %zu\n", g_ModelList.size());
    }
    else
    {
        printf("[HLMV] Standing file direct load active. Core browser vector scan omitted.\n");
    }

    g_CurrentModelPath = "";
    MDLHandle_t hMdl = MDLHANDLE_INVALID;

    const tchar *pTargetModelString = CommandLine()->ParmValue("-model", nullptr);
    if (!pTargetModelString)
    {
        int nParmCount = CommandLine()->ParmCount();
        if (nParmCount > 1)
        {
            const tchar *pLastParam = CommandLine()->GetParm(nParmCount - 1);
            if (pLastParam && pLastParam != '-' && strstr(pLastParam, ".mdl"))
            {
                pTargetModelString = pLastParam;
            }
        }
    }

    if (pTargetModelString && strlen(pTargetModelString) > 4)
    {
        std::string targetPath = pTargetModelString;
        std::replace(targetPath.begin(), targetPath.end(), '\\', '/');

        size_t modelsPos = targetPath.find("models/");
        if (modelsPos != std::string::npos)
        {
            targetPath = targetPath.substr(modelsPos);
        }
        else if (targetPath.rfind("models/", 0) != 0)
        {
            targetPath = "models/" + targetPath;
        }

        printf("[HLMV] Direct launch model loading initiated: %s\n", targetPath.c_str());
        g_CurrentModelPath = targetPath;

        if (std::find(g_ModelList.begin(), g_ModelList.end(), g_CurrentModelPath) == g_ModelList.end())
        {
            g_ModelList.push_back(g_CurrentModelPath);
        }

        hMdl = g_pMDLCache->FindMDL(g_CurrentModelPath.c_str());
    }

    g_CurrentSequenceIndex = 0;
    g_flAnimTime = 0.0f;
    bool bRunning = true;
    SDL_Event event;

    float flCameraPitch = -25.0f;
    float flCameraYaw = 45.0f;
    float flZoomScale = 1.0f;
    float flPanX = 0.0f;
    float flPanY = 0.0f;
    float flPanZ = 0.0f;

    bool bHasValidAnimations = false;
    studiohdr_t *pStudioHdr = nullptr;

    uint32_t lastTicks = SDL_GetTicks();
    while (bRunning)
    {
        // =============================================================================
        // STEP 1: Process ALL Input and Window Events FIRST (Before Material System!)
        // =============================================================================
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_MOUSEMOTION)
            {
                io.MousePos.x = (float)event.motion.x;
                io.MousePos.y = (float)event.motion.y;
            }
            if (event.type == SDL_MOUSEWHEEL)
            {
                io.MouseWheel += (float)event.wheel.y;
            }
            switch (event.type)
            {
            case SDL_QUIT:
                bRunning = false;
                break;
            case SDL_KEYDOWN:
                if (!io.WantCaptureKeyboard && event.key.keysym.sym == SDLK_ESCAPE)
                    bRunning = false;
                break;
            }
        }

        uint32_t currentTicks = SDL_GetTicks();
        float frameTime = (currentTicks - lastTicks) / 1000.0f;
        if (frameTime == 0.0f)
            frameTime = 0.01f;
        lastTicks = currentTicks;

        if (g_bAnimPlaybackPlaying)
        {
            g_flAnimTime += frameTime;
            if (g_flAnimTime > 1.0f)
                g_flAnimTime -= 1.0f;
        }

        if (!io.WantCaptureMouse)
        {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                flCameraYaw += io.MouseDelta.x * 0.25f;
                flCameraPitch += io.MouseDelta.y * 0.25f;
                if (flCameraPitch > 89.0f)
                    flCameraPitch = 89.0f;
                if (flCameraPitch < -89.0f)
                    flCameraPitch = -89.0f;
            }
            else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                float radYaw = flCameraYaw * (M_PI / 180.0f);
                flPanX += (std::sin(radYaw) * io.MouseDelta.x) * 0.05f * flZoomScale;
                flPanY += (std::cos(radYaw) * io.MouseDelta.x) * 0.05f * flZoomScale;
                flPanZ += io.MouseDelta.y * 0.05f * flZoomScale;
            }
            if (io.MouseWheel != 0.0f)
            {
                flZoomScale -= io.MouseWheel * 0.15f * (flZoomScale * 0.4f);
                if (flZoomScale < 0.1f)
                    flZoomScale = 0.1f;
                if (flZoomScale > 15.0f)
                    flZoomScale = 15.0f;
            }
        }

        // =============================================================================
        // STEP 2: Process 3D Geometry Pass First and Flush It Completely
        // =============================================================================
        g_pMaterialSystem->BeginFrame(frameTime);

        IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();
        if (pRenderContext)
        {
            pRenderContext->Viewport(0, 0, w, h);
            glDisable(GL_SCISSOR_TEST);
            glScissor(0, 0, w, h);

            pRenderContext->ClearColor3ub(45, 45, 48);
            pRenderContext->ClearBuffers(true, true, true);
            pRenderContext->DepthRange(0.0f, 1.0f);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_CULL_FACE);
            pRenderContext->Flush(false);

            if (hMdl != MDLHANDLE_INVALID && !g_CurrentModelPath.empty())
            {
                pStudioHdr = g_pMDLCache->GetStudioHdr(hMdl);
                if (pStudioHdr)
                {
                    if (flPanZ == 0.0f && flPanX == 0.0f && flPanY == 0.0f)
                    {
                        flPanZ = (pStudioHdr->view_bbmin.z + pStudioHdr->view_bbmax.z) * 0.5f;
                        float modelHeight = pStudioHdr->view_bbmax.z - pStudioHdr->view_bbmin.z;
                        
                        //=============================================================================
                        // FIXED CAMERA SCALE: Handle Static Un-Rigged Null Bounding Volumes Safely
                        //=============================================================================
                        if (modelHeight > 0.01f)
                        {
                            flZoomScale = modelHeight / 25.0f;
                            if (flZoomScale < 1.0f) flZoomScale = 1.0f; 
                        }
                        else
                        {
                            // Force a tight macro zoom layout if the model returns zero height
                            flZoomScale = 0.05f; 
                        }
                        //=============================================================================
                    }

                    if (pStudioHdr->numlocalseq > 0)
                    {
                        mstudioseqdesc_t *pFirstSeq = pStudioHdr->pLocalSeqdesc(0);
                        if (pFirstSeq && pFirstSeq->numblends > 0)
                            bHasValidAnimations = true;
                    }
                }
            }

            Vector4D ambientCube[6];
            for (int side = 0; side < 6; side++)
                ambientCube[side].Init(1.0f, 1.0f, 1.0f, 1.0f);
            pRenderContext->SetAmbientLightCube(ambientCube);
            pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);

            // Compute camera transformations
            float radPitch = flCameraPitch * (M_PI / 180.0f);
            float radYaw = flCameraYaw * (M_PI / 180.0f);
            float distance = 50.0f * flZoomScale;
            Vector vecEye(distance * std::cos(radPitch) * std::cos(radYaw), distance * std::cos(radPitch) * std::sin(radYaw), distance * std::sin(radPitch));
            Vector vecAt(flPanX, flPanY, flPanZ);
            vecEye += vecAt;

            Vector forward = vecAt - vecEye;
            forward.NormalizeInPlace();
            Vector vecWorldUp(0, 0, 1);
            Vector left;
            CrossProduct(forward, vecWorldUp, left);
            left.NormalizeInPlace();
            Vector up;
            CrossProduct(left, forward, up);
            up.NormalizeInPlace();

            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->LoadIdentity();
            double aspect = (h == 0) ? 1.0 : (double)w / (double)h;
            pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);

            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->LoadIdentity();
            VMatrix matView;
            matView.Identity();
            float *m = matView.Base();
            // 1D Linear base pointer indices bypasses 2D macro type-decay failures
            m[0] = left.x;
            m[1] = left.y;
            m[2] = left.z;
            m[3] = -left.Dot(vecEye);
            m[4] = up.x;
            m[5] = up.y;
            m[6] = up.z;
            m[7] = -up.Dot(vecEye);
            m[8] = -forward.x;
            m[9] = -forward.y;
            m[10] = -forward.z;
            m[11] = forward.Dot(vecEye);
            m[12] = 0.0f;
            m[13] = 0.0f;
            m[14] = 0.0f;
            m[15] = 1.0f;
            pRenderContext->LoadMatrix(matView);
            if (pStudioHdr)
            {
                studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(hMdl);
                if (pHardwareData)
                {
                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->LoadIdentity();
                    g_pStudioRender->BeginFrame();
                    ::StudioRenderConfig_t studioCfg;
                    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                    studioCfg.drawEntities = 1;
                    studioCfg.fullbright = true;
                    g_pStudioRender->UpdateConfig(studioCfg);
                    pRenderContext->BindLocalCubemap(nullptr);
                    DrawModelInfo_t drawInfo;
                    drawInfo.m_pStudioHdr = pStudioHdr;
                    drawInfo.m_pHardwareData = pHardwareData;
                    drawInfo.m_Decals = STUDIORENDER_DECAL_INVALID;
                    drawInfo.m_Skin = drawInfo.m_Body = drawInfo.m_HitboxSet = drawInfo.m_Lod = 0;
                    drawInfo.m_pColorMeshes = nullptr;
                    matrix3x4_t poseBones[MAXSTUDIOBONES];
                    for (int i = 0; i < MAXSTUDIOBONES; i++)
                    {
                        SetIdentityMatrix(poseBones[i]);
                    }
                    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
                    if (pBoneArray != nullptr)
                    {
                        int numBonesToProcess = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
                        for (int i = 0; i < numBonesToProcess; i++)
                        {
                            QuaternionMatrix(pBoneArray[i].quat, pBoneArray[i].pos, poseBones[i]);
                            int parentIdx = pBoneArray[i].parent;
                            if (parentIdx >= 0 && parentIdx < i)
                            {
                                matrix3x4_t temporaryTransform;
                                MatrixCopy(poseBones[i], temporaryTransform);
                                ConcatTransforms(poseBones[parentIdx], temporaryTransform, poseBones[i]);
                            }
                        }
                    }
                    //=============================================================================
                    // FINAL WORKING SHADER PROTOCOL: Dynamic Material Fallback Routing
                    //=============================================================================
                    bool bIsStaticProp = (pStudioHdr && pStudioHdr->numlocalseq <= 1 && pStudioHdr->numbones <= 1);
                    
                    if (bIsStaticProp)
                    {
                        // Overwrite studio flags to force unlit rendering context paths
                        studioCfg.fullbright = true;
                        g_pStudioRender->UpdateConfig(studioCfg);

                        // Bind an absolute solid material that requires no light calculations or vertex colors
                        IMaterial *pSolidShader = g_pMaterialSystem->FindMaterial("vgui/white", TEXTURE_GROUP_OTHER, true);
                        if (pSolidShader != nullptr)
                        {
                            pSolidShader->IncrementReferenceCount();
                            g_pStudioRender->ForcedMaterialOverride(pSolidShader);
                        }
                        else
                        {
                            g_pStudioRender->ForcedMaterialOverride(nullptr);
                        }
                        
                        g_pStudioRender->SetAlphaModulation(1.0f);
                        g_pStudioRender->SetColorModulation(Vector(0.0f, 0.7f, 1.0f).Base()); // Bright blue solid box face fills
                    }
                    else
                    {
                        // Complex animated game characters (Alyx) load their high-res skin materials natively
                        g_pStudioRender->ForcedMaterialOverride(nullptr);
                        g_pStudioRender->SetAlphaModulation(1.0f);
                        g_pStudioRender->SetColorModulation(Vector(1.0f, 1.0f, 1.0f).Base());
                    }
                    //=============================================================================

                    DrawModelResults_t modelResults;
                    memset(&modelResults, 0, sizeof(DrawModelResults_t));
                    g_pStudioRender->DrawModel(&modelResults, drawInfo, poseBones, nullptr, nullptr, Vector(0, 0, 0), STUDIORENDER_DRAW_ENTIRE_MODEL);
                    g_pStudioRender->EndFrame();
                    g_pStudioRender->ForcedMaterialOverride(nullptr);
                }
            }
            pRenderContext->Flush(true);
        }
        g_pMaterialSystem->EndFrame();
        // =============================================================================
        // STEP 3: Render ImGui UI safely over the unblocked frame structure
        // =============================================================================
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(w - 350, 10), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(340, (float)h - 20.0f));
        ImGui::Begin("HLMV Model Browser");
        ImGui::Text("Active Game Folder: %s", g_GameFolder.c_str());
        if (!g_bDirectModelMode)
            ImGui::Text("Total Assets Cached: %zu", g_ModelList.size());
        else
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[Standalone Direct Model Execution Mode]");
        ImGui::Separator();
        ImGui::Text("Currently Rendering:");
        if (g_CurrentModelPath.empty())
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[None Selected - Select Below]");
        else
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "%s", g_CurrentModelPath.c_str());
        if (ImGui::Button("Reset Camera", ImVec2(-1, 0)))
        {
            flCameraPitch = -25.0f;
            flCameraYaw = 45.0f;
            flZoomScale = 1.0f;
            flPanX = 0.0f;
            flPanY = 0.0f;
            flPanZ = (pStudioHdr) ? (pStudioHdr->view_bbmin.z + pStudioHdr->view_bbmax.z) * 0.5f : 0.0f;
            Msg("[HLMV] Camera reset to default\n");
        }
        ImGui::Separator();
        if (pStudioHdr != nullptr && pStudioHdr->numlocalseq > 0 && bHasValidAnimations)
        {
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Animation Sequences (%d):", pStudioHdr->numlocalseq);
            std::string comboLabel = "seq_" + std::to_string(g_CurrentSequenceIndex);
            if (g_CurrentSequenceIndex >= 0 && g_CurrentSequenceIndex < pStudioHdr->numlocalseq)
            {
                mstudioseqdesc_t *pCurrentSeqDesc = pStudioHdr->pLocalSeqdesc(g_CurrentSequenceIndex);
                if (pCurrentSeqDesc && pCurrentSeqDesc->pszLabel() && strlen(pCurrentSeqDesc->pszLabel()) > 0)
                    comboLabel = pCurrentSeqDesc->pszLabel();
            }
            if (ImGui::BeginCombo("##AnimSeqCombo", comboLabel.c_str()))
            {
                for (int seqIdx = 0; seqIdx < pStudioHdr->numlocalseq; seqIdx++)
                {
                    mstudioseqdesc_t *pSeqDesc = pStudioHdr->pLocalSeqdesc(seqIdx);
                    std::string cleanName = "seq_" + std::to_string(seqIdx);
                    if (pSeqDesc && pSeqDesc->pszLabel() && strlen(pSeqDesc->pszLabel()) > 0)
                        cleanName = pSeqDesc->pszLabel();
                    bool bIsSelected = (g_CurrentSequenceIndex == seqIdx);
                    if (ImGui::Selectable(cleanName.c_str(), bIsSelected))
                    {
                        g_CurrentSequenceIndex = seqIdx;
                        g_flAnimTime = 0.0f;
                        Msg("[HLMV] Switched to sequence: %s\n", cleanName.c_str());
                    }
                    if (bIsSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Animation Playback Controls:");
            if (ImGui::Button(g_bAnimPlaybackPlaying ? "Pause Animation (||)" : "Play Animation (>)", ImVec2(-1, 0)))
                g_bAnimPlaybackPlaying = !g_bAnimPlaybackPlaying;
            if (ImGui::SliderFloat("Scrub Timeline", &g_flAnimTime, 0.0f, 1.0f, "Cycle: %.3f"))
                g_bAnimPlaybackPlaying = false;
            ImGui::Separator();
        }
        static char szSearchFilter[256] = "";
        if (!g_bDirectModelMode)
        {
            ImGui::InputText("Filter Search", szSearchFilter, IM_ARRAYSIZE(szSearchFilter));
            ImGui::Separator();
            if (ImGui::BeginChild("ScrollingModelList"))
            {
                for (size_t i = 0; i < g_ModelList.size(); i++)
                {
                    if (strlen(szSearchFilter) > 0 && strstr(g_ModelList[i].c_str(), szSearchFilter) == nullptr)
                        continue;
                    bool bIsSelected = (g_ModelList[i] == g_CurrentModelPath);
                    if (ImGui::Selectable(g_ModelList[i].c_str(), bIsSelected))
                    {
                        g_CurrentModelPath = g_ModelList[i];
                        hMdl = g_pMDLCache->FindMDL(g_CurrentModelPath.c_str());
                        g_CurrentSequenceIndex = 0;
                        g_flAnimTime = 0.0f;
                        if (hMdl != MDLHANDLE_INVALID)
                        {
                            studiohdr_t *pHdr = g_pMDLCache->GetStudioHdr(hMdl);
                            if (pHdr)
                                flPanZ = (pHdr->view_bbmin.z + pHdr->view_bbmax.z) * 0.5f;
                        }
                        Msg("[HLMV] Swapped active model target to: %s\n", g_CurrentModelPath.c_str());
                    }
                    if (bIsSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        if (draw_data)
        {
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                ImDrawList *cmd_list = draw_data->CmdLists[n];
                for (int v_idx = 0; v_idx < cmd_list->VtxBuffer.Size; v_idx++)
                {
                    ImDrawVert &vertex = cmd_list->VtxBuffer.Data[v_idx];
                    vertex.pos.y = (float)h - vertex.pos.y;
                }
                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                {
                    ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
                    float clip_rect_h = pcmd->ClipRect.w - pcmd->ClipRect.y;
                    pcmd->ClipRect.y = (float)h - pcmd->ClipRect.w;
                    pcmd->ClipRect.w = pcmd->ClipRect.y + clip_rect_h;
                }
            }
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
        glFlush();
        g_pMaterialSystem->SwapBuffers();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
    _exit(0);
    return 0;
}