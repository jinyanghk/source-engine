#include <cmath>
#include <cstdio>
#include <vector>
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
#include "materialsystem/materialsystem_config.h"
#include "istudiorender.h"
#include "filesystem_init.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "vphysics_interface.h"
#include "tier0/icommandline.h"
#include "appframework/ilaunchermgr.h"
#include "mathlib/vmatrix.h"

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

//-----------------------------------------------------------------------------
// The application object
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

//-----------------------------------------------------------------------------
// Create all singleton systems
//-----------------------------------------------------------------------------
bool CHammerApp::Create()
{
    CommandLine()->AppendParm("-hammer", NULL);

    IAppSystem *pSystem;

    AppModule_t cvarModule = LoadModule(VStdLib_GetICVarFactory());
    pSystem = AddSystem(cvarModule, CVAR_INTERFACE_VERSION);
    if (!pSystem)
        return false;

    bool bSteam;
    char pFileSystemDLL[MAX_PATH];
    if (FileSystem_GetFileSystemDLLName(pFileSystemDLL, MAX_PATH, bSteam) != FS_OK)
        return false;

    AppModule_t fileSystemModule = LoadModule(pFileSystemDLL);
    g_pFileSystem = (IFileSystem *)AddSystem(fileSystemModule, FILESYSTEM_INTERFACE_VERSION);

    FileSystem_SetBasePaths(g_pFileSystem);

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
        return false;

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

//-----------------------------------------------------------------------------
SpewRetval_t HammerSpewFunc(SpewType_t type, tchar const *pMsg)
{
    if (type == SPEW_ASSERT)
    {
        return SPEW_DEBUGGER;
    }
    else if (type == SPEW_ERROR)
    {
        Msg("Hammer Error %s\n", pMsg);
        return SPEW_ABORT;
    }
    else
    {
        return SPEW_CONTINUE;
    }
}

//-----------------------------------------------------------------------------
// Init, shutdown
//-----------------------------------------------------------------------------
bool CHammerApp::PreInit()
{
    SpewOutputFunc(HammerSpewFunc);

    CFSSearchPathsInit initInfo;
    initInfo.m_pFileSystem = g_pFileSystem;
    initInfo.m_pDirectoryName = "hl2";

    if (FileSystem_LoadSearchPaths(initInfo) != FS_OK)
    {
        Error("Unable to load search paths!\n");
    }

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
// Main application loop
//-----------------------------------------------------------------------------
int CHammerApp::Main()
{
    // 1. Parse Command Line Parameters for Window/Screen Sizing
    bool bFullscreen = (CommandLine()->CheckParm("-f") != nullptr);
    int w = 1280;
    int h = 720;

    if (bFullscreen)
    {
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0)
        {
            w = dm.w;
            h = dm.h;
        }
    }
    else
    {
        w = CommandLine()->ParmValue("-w", 1280);
        h = CommandLine()->ParmValue("-h", 720);
    }

    // 2. Initial Window Handle Lookups & Layout Configurations
    SDL_Window *pWindow = SDL_GL_GetCurrentWindow();
    if (pWindow)
    {
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

    MaterialVideoMode_t mode;
    mode.m_Width = w;
    mode.m_Height = h;
    mode.m_Format = IMAGE_FORMAT_RGBA8888;
    mode.m_RefreshRate = 60;

    MaterialSystem_Config_t config;
    config.m_VideoMode = mode;
    config.SetFlag(MATSYS_VIDCFG_FLAGS_WINDOWED, !bFullscreen);

    if (!g_pMaterialSystem->SetMode((void *)pWindow, config))
    {
        Warning("[HAMMER] Material System SetMode tracking failure.\n");
    }

    pWindow = SDL_GL_GetCurrentWindow();
    if (pWindow)
    {
        SDL_ShowWindow(pWindow);
        SDL_RaiseWindow(pWindow);
    }

    SDL_GLContext glContext = SDL_GL_GetCurrentContext();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr; // disable .ini file

    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    MDLHandle_t hMdl = g_pMDLCache->FindMDL("models/alyx.mdl");
    IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();

    bool bRunning = true;
    SDL_Event event;

    // Camera Workspace Properties
    float flCameraPitch = 20.0f;
    float flCameraYaw = -90.0f;
    float flZoomScale = 3.5f;
    float flPanX = 0.0f;
    float flPanY = 0.0f;
    float flPanZ = 35.0f;

    // Object Coordinates Controlled Contextually via the Sliders Panel
    static Vector vecTargetPos(0.0f, 0.0f, 0.0f);

    uint32_t lastTicks = SDL_GetTicks();

    while (bRunning)
    {
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch (event.type)
            {
            case SDL_QUIT:
                bRunning = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    w = event.window.data1;
                    h = event.window.data2;
                }
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

        // --- 1. FULL UNINTERRUPTED MOUSE CONTROL HOOKS ---
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
                flPanX -= (std::sin(radYaw) * io.MouseDelta.x) * 0.05f * flZoomScale;
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

        g_pMaterialSystem->BeginFrame(frameTime);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
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
            {
                ambientCube[side].Init(1.0f, 1.0f, 1.0f, 1.0f);
            }
            pRenderContext->SetAmbientLightCube(ambientCube);
            // Setup Projection Matrix natively via pure engine calls
            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->LoadIdentity();
            double aspect = (double)w / (double)h;
            pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);
            VMatrix matProj;
            pRenderContext->GetMatrix(MATERIAL_PROJECTION, &matProj);
            // Setup View Matrix natively via pure engine calls
            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->LoadIdentity();
            float radPitch = flCameraPitch * (M_PI / 180.0f);
            float radYaw = flCameraYaw * (M_PI / 180.0f);
            float distance = 50.0f * flZoomScale;
            Vector vecEye(distance * std::cos(radPitch) * std::cos(radYaw), distance * std::cos(radPitch) * std::sin(radYaw), distance * std::sin(radPitch));
            Vector vecAt(flPanX, flPanY, flPanZ);
            vecEye += vecAt;
            Vector forward = vecAt - vecEye;
            VectorNormalize(forward);
            Vector vecWorldUp(0, 0, 1);
            Vector left;
            CrossProduct(forward, vecWorldUp, left);
            VectorNormalize(left);
            Vector up;
            CrossProduct(left, forward, up);
            VectorNormalize(up);
            VMatrix matView;
            matView.Init(left.x, left.y, left.z, -DotProduct(left, vecEye), up.x, up.y, up.z, -DotProduct(up, vecEye), -forward.x, -forward.y, -forward.z, DotProduct(forward, vecEye), 0.0f, 0.0f, 0.0f, 1.0f);
            pRenderContext->LoadMatrix(matView);
            // Declare fallback default bounding box limits for safety allocations
            Vector vecMins(-16.0f, -16.0f, 0.0f);
            Vector vecMaxs(16.0f, 16.0f, 72.0f);
            // 3D character model rendering pass
            if (hMdl != MDLHANDLE_INVALID)
            {
                studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(hMdl);
                studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(hMdl);
                if (pStudioHdr && pHardwareData)
                {
                    // Pull authentic bounding limits directly from Alyx's internal model data layers
                    vecMins = pStudioHdr->view_bbmin;
                    vecMaxs = pStudioHdr->view_bbmax;
                    pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);
                    // --- THE MODEL MATRIX FIX: INJECT THE SLIDER COORDINATES DIRECTLY HERE ---
                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->LoadIdentity();
                    // FIXED: Injected coordinates directly into row-major m[row][col] arrays to avoid const reference errors
                    VMatrix matModelTranslation;
                    matModelTranslation.Identity();
                    matModelTranslation.m[0][3] = vecTargetPos.x;
                    matModelTranslation.m[1][3] = vecTargetPos.y;
                    matModelTranslation.m[2][3] = vecTargetPos.z;
                    pRenderContext->LoadMatrix(matModelTranslation);
                    g_pStudioRender->BeginFrame();
                    ::StudioRenderConfig_t studioCfg;
                    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                    studioCfg.drawEntities = 1;
                    studioCfg.bNoSoftware = true;
                    g_pStudioRender->UpdateConfig(studioCfg);
                    g_pStudioRender->ForcedMaterialOverride(nullptr);
                    g_pStudioRender->SetAlphaModulation(1.0f);
                    g_pStudioRender->SetColorModulation(Vector(1.0f, 1.0f, 1.0f).Base());
                    pRenderContext->OverrideDepthEnable(true, true);
                    DrawModelInfo_t drawInfo;
                    drawInfo.m_pStudioHdr = pStudioHdr;
                    drawInfo.m_pHardwareData = pHardwareData;
                    drawInfo.m_Decals = STUDIORENDER_DECAL_INVALID;
                    drawInfo.m_Skin = drawInfo.m_Body = drawInfo.m_HitboxSet = drawInfo.m_Lod = 0;
                    drawInfo.m_pColorMeshes = nullptr;
                    matrix3x4_t poseBones[MAXSTUDIOBONES] = {};
                    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
                    if (pBoneArray != nullptr)
                    {
                        int numBonesToProcess = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
                        for (int i = 0; i < numBonesToProcess; i++)
                        {
                            Vector bonePos = pBoneArray[i].pos;
                            Quaternion boneQuat = pBoneArray[i].quat;
                            QuaternionMatrix(boneQuat, bonePos, poseBones[i]);
                            int parentIdx = pBoneArray[i].parent;
                            if (parentIdx >= 0 && parentIdx < numBonesToProcess)
                            {
                                matrix3x4_t temporaryTransform;
                                MatrixCopy(poseBones[i], temporaryTransform);
                                ConcatTransforms(poseBones[parentIdx], temporaryTransform, poseBones[i]);
                            }
                        }
                        g_pStudioRender->LockBoneMatrices(numBonesToProcess);
                        g_pStudioRender->UnlockBoneMatrices();
                    }
                    float pFlexWeights[MAXSTUDIOFLEXDESC] = {0.0f};
                    float pFlexDelayedWeights[MAXSTUDIOFLEXDESC] = {0.0f};
                    DrawModelResults_t modelResults;
                    memset(&modelResults, 0, sizeof(DrawModelResults_t));
                    // We pass Vector(0,0,0) here because translation is already applied to the Model Matrix stack above.
                    g_pStudioRender->DrawModel(&modelResults, drawInfo, poseBones, pFlexWeights, pFlexDelayedWeights, Vector(0, 0, 0), STUDIORENDER_DRAW_ENTIRE_MODEL);
                    g_pStudioRender->EndFrame();
                    pRenderContext->OverrideDepthEnable(false, false);
                }
            }
            // --- 2. RENDER THE SLIDER CONTROL WINDOW PANEL ---
            ImGui::Begin("Hammer Operator Console");
            ImGui::Text("Viewport Controls: Left Click Drag (Rotate) | Right Click Drag (Pan)");
            ImGui::Separator();
            ImGui::Text("Manual Position Adjustment Sliders:");
            ImGui::SliderFloat("Entity X Axis", &vecTargetPos.x, -100.0f, 100.0f, "%.2f");
            ImGui::SliderFloat("Entity Y Axis", &vecTargetPos.y, -100.0f, 100.0f, "%.2f");
            ImGui::SliderFloat("Entity Z Axis", &vecTargetPos.z, -100.0f, 100.0f, "%.2f");
            ImGui::End();
            ImGui::Render();
            // --- 3. THE NATIVE 3D WIREFRAME BOUNDING BOX RENDER PASS ---
            pRenderContext->MatrixMode(MATERIAL_MODEL);
            pRenderContext->LoadIdentity();
            // FIXED: Injected coordinates directly into row-major m[row][col] arrays to avoid const reference errors
            VMatrix matBoxTranslation;
            matBoxTranslation.Identity();
            matBoxTranslation.m[0][3] = vecTargetPos.x;
            matBoxTranslation.m[1][3] = vecTargetPos.y;
            matBoxTranslation.m[2][3] = vecTargetPos.z;
            pRenderContext->LoadMatrix(matBoxTranslation);
            g_pStudioRender->ForcedMaterialOverride(nullptr);
            glLineWidth(2.5f);
            glBegin(GL_LINES);
            glColor3ub(255, 255, 0);
            // Classic Selection Yellow bounds calculation
            float xmin = vecMins.x;
            float xmax = vecMaxs.x;
            float ymin = vecMins.y;
            float ymax = vecMaxs.y;
            float zmin = vecMins.z;
            float zmax = vecMaxs.z; // Bottom Face Base
            glVertex3f(xmin, ymin, zmin);
            glVertex3f(xmax, ymin, zmin);
            glVertex3f(xmax, ymin, zmin);
            glVertex3f(xmax, ymax, zmin);
            glVertex3f(xmax, ymax, zmin);
            glVertex3f(xmin, ymax, zmin);
            glVertex3f(xmin, ymax, zmin);
            glVertex3f(xmin, ymin, zmin); // Top Face Base
            glVertex3f(xmin, ymin, zmax);
            glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmin, ymax, zmax);
            glVertex3f(xmin, ymax, zmax);
            glVertex3f(xmin, ymin, zmax); // Vertical Supporting Corner Columns
            glVertex3f(xmin, ymin, zmin);
            glVertex3f(xmin, ymin, zmax);
            glVertex3f(xmax, ymin, zmin);
            glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymax, zmin);
            glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmin, ymax, zmin);
            glVertex3f(xmin, ymax, zmax);
            glEnd();
            glLineWidth(1.0f);
            // Flipper System (Brings the window right-side up for Source's OpenGL viewport)
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
            }
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
            pRenderContext->Flush(true);
        }
        g_pMaterialSystem->EndFrame();
        g_pMaterialSystem->SwapBuffers();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    _exit(0);
    return 0;
}