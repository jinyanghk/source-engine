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

IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

extern void *CreateSDLMgr();

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

enum GizmoMode { GIZMO_NONE = 0, GIZMO_TRANSLATE, GIZMO_ROTATE };

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

    GizmoPick::Init(w, h);

    // 初始场景：地板 + 两面墙
    {
        CBrush floor;
        floor.m_vecPos = Vector(0, 0, -8);
        floor.m_vecSize = Vector(256, 256, 8);
        floor.m_iTexId = 2;
        g_brushes.push_back(floor);

        CBrush wallN;
        wallN.m_vecPos = Vector(0, 256, 128);
        wallN.m_vecSize = Vector(256, 8, 128);
        wallN.m_iTexId = 1;
        g_brushes.push_back(wallN);

        CBrush wallW;
        wallW.m_vecPos = Vector(-256, 0, 128);
        wallW.m_vecSize = Vector(8, 256, 128);
        wallW.m_iTexId = 1;
        g_brushes.push_back(wallW);
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
    float m_camDistance = 450.0f;

    Vector g_vecEye(0,0,0), g_vecAt(0,0,0);
    Vector g_camForward(0,0,0), g_camLeft(0,0,0), g_camUp(0,0,0);
    float g_camTanHalfFov = 0.0f, g_camAspect = 1.0f;

    int g_iGizmoMode = GIZMO_TRANSLATE;
    bool g_bShowGizmo = true;
    bool g_bXRayGizmo = false;
    int g_iActiveAxis = -1;
    int g_iHoverAxis = -1;
    float g_flDragStartWorldT = 0.0f;
    float g_flDragStartAngle = 0.0f;
    Vector g_vecDragStartPos;
    QAngle g_angDragStartRot;

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
                { g_bLeftMouseDown = false; g_bDraggingGizmo = false; g_iActiveAxis = -1; }
                else if (event.button.button == SDL_BUTTON_RIGHT) g_bRightMouseDown = false;
                else if (event.button.button == SDL_BUTTON_MIDDLE) g_bMiddleMouseDown = false;
                break;
            case SDL_KEYDOWN:
                if (!io.WantCaptureKeyboard)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE) bRunning = false;
                    else if (event.key.keysym.sym == SDLK_1) g_iGizmoMode = GIZMO_TRANSLATE;
                    else if (event.key.keysym.sym == SDLK_2) g_iGizmoMode = GIZMO_ROTATE;
                    else if (event.key.keysym.sym == SDLK_3) g_iGizmoMode = GIZMO_NONE;
                    else if (event.key.keysym.sym == SDLK_r)
                    {
                        m_camTarget = Vector(0, 0, 40);
                        m_camYaw = 90.0f;
                        m_camPitch = -15.0f;
                        m_camDistance = 450.0f;
                        Msg("[HAMMER] Camera reset\n");
                    }
                    else if (event.key.keysym.sym == SDLK_z && !(event.key.keysym.mod & KMOD_SHIFT))
                    { m_camDistance *= 0.85f; if (m_camDistance < 20.0f) m_camDistance = 20.0f; }
                    else if (event.key.keysym.sym == SDLK_z && (event.key.keysym.mod & KMOD_SHIFT))
                    { m_camDistance *= 1.15f; if (m_camDistance > 3000.0f) m_camDistance = 3000.0f; }
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
        g_iLastMouseX = g_iMouseX;
        g_iLastMouseY = g_iMouseY;

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

            glEnable(GL_TEXTURE_2D);
            for (size_t i = 0; i < g_brushes.size(); i++)
                DrawBrush(g_brushes[i]);
            glDisable(GL_TEXTURE_2D);

            for (size_t i = 0; i < g_brushes.size(); i++)
                DrawBrushEdges(g_brushes[i]);

            for (size_t i = 0; i < g_entities.size(); i++)
            {
                if (g_entities[i].m_iType == ENTITY_PLAYER_START)
                    DrawPlayerStartModel(g_entities[i].m_vecPos, g_entities[i].m_angRot);
            }

            glDepthMask(GL_FALSE);
            for (size_t i = 0; i < g_entities.size(); i++)
                DrawEntityBBox(g_entities[i], (int)i == g_iSelectedEntity);
            for (size_t i = 0; i < g_brushes.size(); i++)
                DrawBrushBBox(g_brushes[i], (int)i == g_iSelectedBrush);
            glDepthMask(GL_TRUE);

            if (g_bXRayGizmo)
                glDisable(GL_DEPTH_TEST);

            // 用一个静态 dummy 让后面的 sel.m_vecBBox* 代码不必改，
            // 但所有 gizmo / 拖动 / UI 逻辑都要用 bHasSelection 保护。
            static CEntity s_dummyEntity;
            CEntity& sel = bHasSelection ? g_entities[g_iSelectedEntity] : s_dummyEntity;

            Vector gizmoCenter = sel.GetCenter();
            float bboxSize = std::max(std::max(sel.m_vecBBoxMaxs.x - sel.m_vecBBoxMins.x,
                                               sel.m_vecBBoxMaxs.y - sel.m_vecBBoxMins.y),
                                      sel.m_vecBBoxMaxs.z - sel.m_vecBBoxMins.z);
            float axisLength = bboxSize * 1.2f;
            if (axisLength < 30.0f) axisLength = 30.0f;

            Vector axisDirs[3] = { Vector(1,0,0), Vector(0,1,0), Vector(0,0,1) };
            float ndcX = (g_iMouseX / (float)w) * 2.0f - 1.0f;
            float ndcY = 1.0f - (g_iMouseY / (float)h) * 2.0f;
            Vector rayDir = g_camForward - g_camLeft * (ndcX * g_camTanHalfFov * g_camAspect) + g_camUp * (ndcY * g_camTanHalfFov);
            VectorNormalize(rayDir);
            Vector rayOrigin = g_vecEye;

            // ===== 渲染 pick target（离屏 ID 缓冲）=====
            if (bHasSelection && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
            {
                GizmoPick::BeginRender(g_savedProj, g_savedView);
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
                GizmoPick::EndRender();

                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(g_savedProj);
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(g_savedView);
            }

            // ===== Hover 检测 =====
            if (bHasSelection)
            {
                if (!g_bDraggingGizmo && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
                {
                    g_iHoverAxis = -1;
                    float gizmoScreenX, gizmoScreenY;
                    bool gizmoVisible = WorldToScreen(gizmoCenter, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                                    g_camTanHalfFov, g_camAspect, w, h, gizmoScreenX, gizmoScreenY);
                    if (gizmoVisible)
                    {
                        float mdx = g_iMouseX - gizmoScreenX;
                        float mdy = g_iMouseY - gizmoScreenY;
                        float mouseToGizmoSq = mdx * mdx + mdy * mdy;
                        const float MAX_SCREEN_DIST = 3000.0f;
                        if (mouseToGizmoSq < MAX_SCREEN_DIST * MAX_SCREEN_DIST)
                        {
                            if (g_iGizmoMode == GIZMO_TRANSLATE)
                            {
                                GizmoPick::HandleType htype;
                                int hitAxis = GizmoPick::Pick(g_iMouseX, g_iMouseY, htype);
                                if (hitAxis >= 0 && htype == GizmoPick::HANDLE_TRANSLATE)
                                    g_iHoverAxis = hitAxis;
                                else
                                    g_iHoverAxis = -1;
                            }
                            else if (g_iGizmoMode == GIZMO_ROTATE)
                            {
                                float tolerance = axisLength * 0.08f;
                                if (tolerance < 4.0f) tolerance = 4.0f;
                                float bestDelta = tolerance;
                                for (int a = 0; a < 3; a++)
                                {
                                    Vector normal = axisDirs[a];
                                    float denom = DotProduct(rayDir, normal);
                                    if (fabsf(denom) < 0.2f) continue;
                                    float t = DotProduct(gizmoCenter - rayOrigin, normal) / denom;
                                    if (t < 0.0f) continue;
                                    Vector hitPoint = rayOrigin + rayDir * t;
                                    Vector offset = hitPoint - gizmoCenter;
                                    Vector u, v;
                                    switch (a)
                                    {
                                    case 0: u = Vector(0,1,0); v = Vector(0,0,1); break;
                                    case 1: u = Vector(1,0,0); v = Vector(0,0,1); break;
                                    case 2: u = Vector(1,0,0); v = Vector(0,1,0); break;
                                    }
                                    float pu = DotProduct(offset, u);
                                    float pv = DotProduct(offset, v);
                                    float d = sqrtf(pu*pu + pv*pv);
                                    float delta = fabsf(d - axisLength);
                                    if (delta < bestDelta) { bestDelta = delta; g_iHoverAxis = a; }
                                }
                            }
                        }
                    }
                }
                else if (!g_bDraggingGizmo)
                {
                    g_iHoverAxis = -1;
                }
            }
            else
            {
                g_iHoverAxis = -1;
            }

            // ===== 点击起始拖动 =====
            if (g_bLeftMouseDown && !g_bDraggingGizmo && g_iActiveAxis < 0 && !io.WantCaptureMouse)
            {
                if (bHasSelection && g_iHoverAxis >= 0 && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
                {
                    g_iActiveAxis = g_iHoverAxis;
                    g_bDraggingGizmo = true;
                    if (g_iGizmoMode == GIZMO_TRANSLATE)
                    {
                        g_vecDragStartPos = sel.m_vecPos;
                        Vector p0 = gizmoCenter;
                        Vector p1 = gizmoCenter + axisDirs[g_iActiveAxis] * axisLength;
                        float sx0, sy0, sx1, sy1;
                        g_flDragStartWorldT = -1.0f;
                        if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                            WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                        {
                            float dx = sx1 - sx0, dy = sy1 - sy0;
                            float lenSq = dx*dx + dy*dy;
                            if (lenSq > 1.0f)
                            {
                                float t = ((g_iMouseX - sx0) * dx + (g_iMouseY - sy0) * dy) / lenSq;
                                t = std::max(0.0f, std::min(1.0f, t));
                                g_flDragStartWorldT = t * axisLength;
                            }
                        }
                    }
                    else if (g_iGizmoMode == GIZMO_ROTATE)
                    {
                        g_flDragStartAngle = 0.0f;
                        RayRingPlaneAngle(rayOrigin, rayDir, gizmoCenter, g_iActiveAxis, g_flDragStartAngle);
                        g_angDragStartRot = sel.m_angRot;
                    }
                }
                else
                {
                    // 点空白 -> 拾取 entity
                    int hit = -1;
                    for (int i = (int)g_entities.size() - 1; i >= 0; i--)
                    {
                        if (IsMouseOverEntityBBox(g_entities[i], g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, g_iMouseX, g_iMouseY))
                        {
                            hit = i;
                            break;
                        }
                    }
                    g_iSelectedEntity = hit;   // 没拾到 -> -1（取消选中）
                }
            }

            // ===== 拖动中 =====
            if (bHasSelection && g_bDraggingGizmo && g_bLeftMouseDown && g_iActiveAxis >= 0)
            {
                if (g_iGizmoMode == GIZMO_TRANSLATE)
                {
                    Vector p0 = gizmoCenter;
                    Vector p1 = gizmoCenter + axisDirs[g_iActiveAxis] * axisLength;
                    float sx0, sy0, sx1, sy1;
                    if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                        WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                    {
                        float dx = sx1 - sx0, dy = sy1 - sy0;
                        float lenSq = dx*dx + dy*dy;
                        if (lenSq > 1.0f)
                        {
                            float t = ((g_iMouseX - sx0) * dx + (g_iMouseY - sy0) * dy) / lenSq;
                            t = std::max(0.0f, std::min(1.0f, t));
                            float worldDelta = t * axisLength;
                            if (g_flDragStartWorldT < 0.0f) g_flDragStartWorldT = worldDelta;
                            float deltaT = worldDelta - g_flDragStartWorldT;
                            sel.m_vecPos = g_vecDragStartPos + axisDirs[g_iActiveAxis] * deltaT;
                        }
                    }
                }
                else if (g_iGizmoMode == GIZMO_ROTATE)
                {
                    float curAngle = 0.0f;
                    if (RayRingPlaneAngle(rayOrigin, rayDir, gizmoCenter, g_iActiveAxis, curAngle))
                    {
                        float deltaAngle = curAngle - g_flDragStartAngle;
                        if (deltaAngle > M_PI) deltaAngle -= 2.0f * M_PI;
                        if (deltaAngle < -M_PI) deltaAngle += 2.0f * M_PI;
                        float deltaDeg = deltaAngle * (180.0f / M_PI);
                        if (g_iActiveAxis == 0)      sel.m_angRot.x = g_angDragStartRot.x + deltaDeg;
                        else if (g_iActiveAxis == 1) sel.m_angRot.y = g_angDragStartRot.y + deltaDeg;
                        else                          sel.m_angRot.z = g_angDragStartRot.z + deltaDeg;
                    }
                }
            }

            // ===== 绘制可见 gizmo =====
            if (bHasSelection && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
            {
                int highlightAxis = (g_iActiveAxis >= 0) ? g_iActiveAxis : g_iHoverAxis;
                if (g_iGizmoMode == GIZMO_TRANSLATE)
                    DrawTranslateGizmo(gizmoCenter, axisLength, highlightAxis);
                else if (g_iGizmoMode == GIZMO_ROTATE)
                {
                    unsigned char rX=255,gX=0,bX=0;
                    unsigned char rY=0,gY=255,bY=0;
                    unsigned char rZ=0,gZ=128,bZ=255;
                    if (highlightAxis == 0) { rX=255; gX=255; bX=0; }
                    if (highlightAxis == 1) { rY=255; gY=255; bY=0; }
                    if (highlightAxis == 2) { rZ=255; gZ=255; bZ=0; }
                    DrawRotationRing(gizmoCenter, 0, axisLength, 64, rX, gX, bX);
                    DrawRotationRing(gizmoCenter, 1, axisLength, 64, rY, gY, bY);
                    DrawRotationRing(gizmoCenter, 2, axisLength, 64, rZ, gZ, bZ);
                }
            }

            // ===== UI 面板 =====
            DrawMenuBar();

            EditorLayout::Begin();

            std::string newClassFromPalette;
            DrawEntityPalette(newClassFromPalette);

            if (!newClassFromPalette.empty())
            {
                std::string modelPath = FgdManager::Instance().GetModelPathForClass(newClassFromPalette);
                printf("[Palette] selected class='%s' model='%s'\n",
                    newClassFromPalette.c_str(), modelPath.c_str());

                CEntity e;
                e.m_iType = ENTITY_MODEL;
                e.m_szName = newClassFromPalette;
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

            BrushPanelResult brushResult;
            DrawSelectionPanel(g_entities.data(), (int)g_entities.size(), &g_iSelectedEntity,
                            g_brushes.data(), (int)g_brushes.size(), &g_iSelectedBrush,
                            brushResult);

            if (brushResult.bRequestDeleteEntity &&
                brushResult.iTargetEntityIndex >= 0 &&
                brushResult.iTargetEntityIndex < (int)g_entities.size())
            {
                g_entities.erase(g_entities.begin() + brushResult.iTargetEntityIndex);

                // 修正选中索引
                if (g_iSelectedEntity == brushResult.iTargetEntityIndex)
                    g_iSelectedEntity = -1;
                else if (g_iSelectedEntity > brushResult.iTargetEntityIndex)
                    g_iSelectedEntity--;
            }

            if (brushResult.bRequestNew)
            {
                CBrush b;
                b.m_vecPos = Vector(0, 0, 32);
                b.m_vecSize = Vector(32, 32, 32);
                b.m_iTexId = 0;
                g_brushes.push_back(b);
                g_iSelectedBrush = (int)g_brushes.size() - 1;
            }
            if (brushResult.bRequestDelete &&
                brushResult.iTargetIndex >= 0 &&
                brushResult.iTargetIndex < (int)g_brushes.size())
            {
                g_brushes.erase(g_brushes.begin() + brushResult.iTargetIndex);
                g_iSelectedBrush = -1;
            }
            if (brushResult.bRequestDuplicate &&
                brushResult.iTargetIndex >= 0 &&
                brushResult.iTargetIndex < (int)g_brushes.size())
            {
                CBrush b = g_brushes[brushResult.iTargetIndex];
                b.m_vecPos.x += 32.0f;
                g_brushes.push_back(b);
                g_iSelectedBrush = (int)g_brushes.size() - 1;
            }
            DrawConsolePanel(m_camTarget, m_camDistance, g_bShowGizmo, g_bXRayGizmo);

            EditorLayout::End();

            ImGuiLayer::EndFrameAndRender(w, h);
        }
        g_pMaterialSystem->EndFrame();
        g_pMaterialSystem->SwapBuffers();
    }

    ImGuiLayer::Shutdown();
    _exit(0);
    return 0;
}