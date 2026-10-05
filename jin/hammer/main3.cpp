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
        return SPEW_DEBUGGER;
    else if (type == SPEW_ERROR)
    {
        Msg("Hammer Error %s\n", pMsg);
        return SPEW_ABORT;
    }
    else
        return SPEW_CONTINUE;
}

//-----------------------------------------------------------------------------
bool CHammerApp::PreInit()
{
    SpewOutputFunc(HammerSpewFunc);

    CFSSearchPathsInit initInfo;
    initInfo.m_pFileSystem = g_pFileSystem;
    initInfo.m_pDirectoryName = "hl2";

    if (FileSystem_LoadSearchPaths(initInfo) != FS_OK)
        Error("Unable to load search paths!\n");

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
static void BuildGLProjectionMatrix(float* m, float fovY, float aspect, float zNear, float zFar)
{
    float f = 1.0f / tanf(fovY * 0.5f * (float)M_PI / 180.0f);
    float rangeRecip = 1.0f / (zNear - zFar);
    memset(m, 0, sizeof(float) * 16);
    m[0]  = f / aspect;
    m[5]  = f;
    m[10] = (zNear + zFar) * rangeRecip;
    m[11] = -1.0f;
    m[14] = 2.0f * zNear * zFar * rangeRecip;
}

//-----------------------------------------------------------------------------
static void BuildGLViewMatrix(float* m, const Vector& eye, const Vector& target, const Vector& worldUp)
{
    Vector forward = target - eye;
    VectorNormalize(forward);
    Vector side;
    CrossProduct(forward, worldUp, side);
    VectorNormalize(side);
    Vector up;
    CrossProduct(side, forward, up);
    memset(m, 0, sizeof(float) * 16);
    m[0]  = side.x;     m[4]  = side.y;     m[8]  = side.z;     m[12] = -DotProduct(side, eye);
    m[1]  = up.x;       m[5]  = up.y;       m[9]  = up.z;       m[13] = -DotProduct(up, eye);
    m[2]  = -forward.x; m[6]  = -forward.y; m[10] = -forward.z; m[14] = DotProduct(forward, eye);
    m[15] = 1.0f;
}

//-----------------------------------------------------------------------------
enum GizmoMode
{
    GIZMO_NONE = 0,
    GIZMO_TRANSLATE,
    GIZMO_ROTATE
};

//-----------------------------------------------------------------------------
static void DrawRotationRing(const Vector& center, int axis, float radius, int segments,
                             unsigned char r, unsigned char g, unsigned char b)
{
    glColor3ub(r, g, b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; i++)
    {
        float angle = (float)i / segments * 2.0f * (float)M_PI;
        float c = cosf(angle) * radius;
        float s = sinf(angle) * radius;
        switch (axis)
        {
        case 0: glVertex3f(center.x, center.y + c, center.z + s); break;
        case 1: glVertex3f(center.x + c, center.y, center.z + s); break;
        case 2: glVertex3f(center.x + c, center.y + s, center.z); break;
        }
    }
    glEnd();
}

//-----------------------------------------------------------------------------
static void DrawTranslateGizmo(const Vector& center, float length, int highlightAxis)
{
    float arrowSize = length * 0.12f;
    float arrowWidth = length * 0.06f;
    
    unsigned char cX[3] = {255, 0, 0};
    unsigned char cY[3] = {0, 255, 0};
    unsigned char cZ[3] = {0, 128, 255};
    if (highlightAxis == 0) { cX[0]=255; cX[1]=255; cX[2]=0; }
    if (highlightAxis == 1) { cY[0]=255; cY[1]=255; cY[2]=0; }
    if (highlightAxis == 2) { cZ[0]=255; cZ[1]=255; cZ[2]=0; }
    
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3ub(cX[0], cX[1], cX[2]);
    glVertex3f(center.x, center.y, center.z);
    glVertex3f(center.x + length, center.y, center.z);
    glColor3ub(cY[0], cY[1], cY[2]);
    glVertex3f(center.x, center.y, center.z);
    glVertex3f(center.x, center.y + length, center.z);
    glColor3ub(cZ[0], cZ[1], cZ[2]);
    glVertex3f(center.x, center.y, center.z);
    glVertex3f(center.x, center.y, center.z + length);
    glEnd();
    
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glColor3ub(cX[0], cX[1], cX[2]);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y + arrowWidth, center.z);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y - arrowWidth, center.z);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y, center.z + arrowWidth);
    glVertex3f(center.x + length, center.y, center.z);
    glVertex3f(center.x + length - arrowSize, center.y, center.z - arrowWidth);
    glColor3ub(cY[0], cY[1], cY[2]);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x + arrowWidth, center.y + length - arrowSize, center.z);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x - arrowWidth, center.y + length - arrowSize, center.z);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x, center.y + length - arrowSize, center.z + arrowWidth);
    glVertex3f(center.x, center.y + length, center.z);
    glVertex3f(center.x, center.y + length - arrowSize, center.z - arrowWidth);
    glColor3ub(cZ[0], cZ[1], cZ[2]);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x + arrowWidth, center.y, center.z + length - arrowSize);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x - arrowWidth, center.y, center.z + length - arrowSize);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x, center.y + arrowWidth, center.z + length - arrowSize);
    glVertex3f(center.x, center.y, center.z + length);
    glVertex3f(center.x, center.y - arrowWidth, center.z + length - arrowSize);
    glEnd();
    glLineWidth(1.0f);
}

//-----------------------------------------------------------------------------
static float RayRingHitAngle(const Vector& rayOrigin, const Vector& rayDir,
                             const Vector& center, int axis, float radius)
{
    Vector normal;
    switch (axis)
    {
    case 0: normal = Vector(1, 0, 0); break;
    case 1: normal = Vector(0, 1, 0); break;
    case 2: normal = Vector(0, 0, 1); break;
    }
    
    float denom = DotProduct(rayDir, normal);
    if (fabsf(denom) < 1e-6f) return -1.0f;
    
    float t = DotProduct(center - rayOrigin, normal) / denom;
    if (t < 0.0f) return -1.0f;
    
    Vector hitPoint = rayOrigin + rayDir * t;
    Vector offset = hitPoint - center;
    
    Vector u, v;
    switch (axis)
    {
    case 0: u = Vector(0, 1, 0); v = Vector(0, 0, 1); break;
    case 1: u = Vector(1, 0, 0); v = Vector(0, 0, 1); break;
    case 2: u = Vector(1, 0, 0); v = Vector(0, 1, 0); break;
    }
    
    float pu = DotProduct(offset, u);
    float pv = DotProduct(offset, v);
    float distFromCenter = sqrtf(pu*pu + pv*pv);
    
    float tolerance = radius * 0.25f;
    if (fabsf(distFromCenter - radius) > tolerance) return -1.0f;
    
    return atan2f(pv, pu);
}

//-----------------------------------------------------------------------------
// 把世界坐标点投影到屏幕坐标（像素）
// 返回 true 表示在相机前方
//-----------------------------------------------------------------------------
static bool WorldToScreen(const Vector& worldPos,
                          const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                          float tanHalfFov, float aspect,
                          int screenW, int screenH,
                          float& outX, float& outY)
{
    Vector rel = worldPos - camEye;
    float depth = DotProduct(rel, camForward);
    if (depth < 0.01f) return false;
    
    float xCam = DotProduct(rel, camLeft);
    float yCam = DotProduct(rel, camUp);
    
    float ndcX = (xCam / depth) / (tanHalfFov * aspect);
    float ndcY = (yCam / depth) / tanHalfFov;
    
    outX = (ndcX * 0.5f + 0.5f) * screenW;
    outY = (1.0f - (ndcY * 0.5f + 0.5f)) * screenH;
    return true;
}

//-----------------------------------------------------------------------------
// 计算鼠标点到 2D 屏幕线段的最短距离
//-----------------------------------------------------------------------------
static float DistToScreenSegment(float px, float py,
                                 float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0;
    float dy = y1 - y0;
    float lenSq = dx*dx + dy*dy;
    if (lenSq < 1e-6f)
        return sqrtf((px - x0)*(px - x0) + (py - y0)*(py - y0));
    
    float t = ((px - x0) * dx + (py - y0) * dy) / lenSq;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    float projX = x0 + t * dx;
    float projY = y0 + t * dy;
    return sqrtf((px - projX)*(px - projX) + (py - projY)*(py - projY));
}

//-----------------------------------------------------------------------------
static void BuildRotationMatrixFromQAngle(const QAngle& angles, VMatrix& outMatrix)
{
    matrix3x4_t mat;
    AngleMatrix(angles, mat);
    
    outMatrix.Identity();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            outMatrix.m[i][j] = mat.m_flMatVal[i][j];
}

//-----------------------------------------------------------------------------
int CHammerApp::Main()
{
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
        Warning("[HAMMER] Material System SetMode tracking failure.\n");

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
    io.IniFilename = nullptr;

    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    MDLHandle_t hMdl = g_pMDLCache->FindMDL("models/alyx.mdl");
    IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();

    bool bRunning = true;
    SDL_Event event;

    float flCameraPitch = 20.0f;
    float flCameraYaw = -90.0f;
    float flZoomScale = 3.5f;
    float flPanX = 0.0f;
    float flPanY = 0.0f;
    float flPanZ = 35.0f;

    static Vector vecTargetPos(0.0f, 0.0f, 0.0f);
    static QAngle vecTargetRot(0.0f, 0.0f, 0.0f);

    Vector g_vecEye(0, 0, 0);
    Vector g_vecAt(0, 0, 0);

    Vector g_vecBBoxMins(-16.0f, -16.0f, 0.0f);
    Vector g_vecBBoxMaxs(16.0f, 16.0f, 72.0f);

    Vector g_camForward(0, 0, 0);
    Vector g_camLeft(0, 0, 0);
    Vector g_camUp(0, 0, 0);
    float   g_camTanHalfFov = 0.0f;
    float   g_camAspect = 1.0f;

    int g_iGizmoMode = GIZMO_TRANSLATE;
    bool g_bShowGizmo = true;
    int g_iActiveAxis = -1;
    int g_iHoverAxis = -1;
    float g_flDragStartWorldT = 0.0f;
    float g_flDragStartAngle = 0.0f;
    Vector g_vecDragStartPos;
    QAngle g_angDragStartRot;

    bool g_bMouseDown = false;
    bool g_bDraggingGizmo = false;
    int  g_iMouseX = 0, g_iMouseY = 0;
    int  g_iLastMouseX = 0, g_iLastMouseY = 0;

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

            case SDL_MOUSEMOTION:
                g_iMouseX = event.motion.x;
                g_iMouseY = event.motion.y;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    g_bMouseDown = true;
                    g_iMouseX = event.button.x;
                    g_iMouseY = event.button.y;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    g_bMouseDown = false;
                    g_bDraggingGizmo = false;
                    g_iActiveAxis = -1;
                }
                break;

            case SDL_KEYDOWN:
                if (!io.WantCaptureKeyboard)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        bRunning = false;
                    else if (event.key.keysym.sym == SDLK_q)
                        g_iGizmoMode = GIZMO_NONE;
                    else if (event.key.keysym.sym == SDLK_w)
                        g_iGizmoMode = GIZMO_TRANSLATE;
                    else if (event.key.keysym.sym == SDLK_r)
                        g_iGizmoMode = GIZMO_ROTATE;
                }
                break;
            }
        }

        uint32_t currentTicks = SDL_GetTicks();
        float frameTime = (currentTicks - lastTicks) / 1000.0f;
        if (frameTime == 0.0f) frameTime = 0.01f;
        lastTicks = currentTicks;

        int mouseDX = g_iMouseX - g_iLastMouseX;
        int mouseDY = g_iMouseY - g_iLastMouseY;

        // 相机控制（拖动 Gizmo 时禁用）
        if (!io.WantCaptureMouse && !g_bDraggingGizmo)
        {
            if (g_bMouseDown && !io.KeyAlt)
            {
                flCameraYaw += mouseDX * 0.25f;
                flCameraPitch += mouseDY * 0.25f;
                if (flCameraPitch > 89.0f) flCameraPitch = 89.0f;
                if (flCameraPitch < -89.0f) flCameraPitch = -89.0f;
            }
            else if (io.MouseDown[1])
            {
                float radYaw = flCameraYaw * (M_PI / 180.0f);
                flPanX += (std::sin(radYaw) * mouseDX) * 0.05f * flZoomScale;
                flPanY -= (std::cos(radYaw) * mouseDX) * 0.05f * flZoomScale;
                flPanZ += mouseDY * 0.05f * flZoomScale;
            }

            if (io.MouseWheel != 0.0f)
            {
                flZoomScale -= io.MouseWheel * 0.15f * (flZoomScale * 0.4f);
                if (flZoomScale < 0.1f) flZoomScale = 0.1f;
                if (flZoomScale > 15.0f) flZoomScale = 15.0f;
            }
        }
        g_iLastMouseX = g_iMouseX;
        g_iLastMouseY = g_iMouseY;

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
                ambientCube[side].Init(1.0f, 1.0f, 1.0f, 1.0f);
            pRenderContext->SetAmbientLightCube(ambientCube);
            
            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->LoadIdentity();
            double aspect = (double)w / (double)h;
            pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);
            
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
            
            g_vecEye = vecEye;
            g_vecAt = vecAt;
            g_camForward = forward;
            g_camLeft = left;
            g_camUp = up;
            g_camTanHalfFov = tanf(45.0f * 0.5f * (float)M_PI / 180.0f);
            g_camAspect = (float)w / (float)h;

            // 构建模型矩阵 = 平移 × 旋转
            VMatrix matModelTranslation;
            matModelTranslation.Identity();
            matModelTranslation.m[0][3] = vecTargetPos.x;
            matModelTranslation.m[1][3] = vecTargetPos.y;
            matModelTranslation.m[2][3] = vecTargetPos.z;
            
            VMatrix matModelRotation;
            BuildRotationMatrixFromQAngle(vecTargetRot, matModelRotation);
            
            VMatrix matModel;
            MatrixMultiply(matModelTranslation, matModelRotation, matModel);

            if (hMdl != MDLHANDLE_INVALID)
            {
                studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(hMdl);
                studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(hMdl);
                if (pStudioHdr && pHardwareData)
                {
                    pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);
                    
                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->LoadMatrix(matModel);
                    
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
                        
                        // 【关键】把模型矩阵（平移+旋转）应用到每根骨骼上
                        // matModel 是 VMatrix，转成 matrix3x4_t 后与每根骨骼的矩阵做 Concat
                        matrix3x4_t matModel3x4;
                        for (int r = 0; r < 3; r++)
                            for (int c = 0; c < 4; c++)
                                matModel3x4.m_flMatVal[r][c] = matModel.m[r][c];
                        
                        for (int i = 0; i < numBonesToProcess; i++)
                        {
                            matrix3x4_t finalBone;
                            ConcatTransforms(matModel3x4, poseBones[i], finalBone);
                            MatrixCopy(finalBone, poseBones[i]);
                        }
                        
                        g_pStudioRender->LockBoneMatrices(numBonesToProcess);
                        g_pStudioRender->UnlockBoneMatrices();
                    }
                    
                    float pFlexWeights[MAXSTUDIOFLEXDESC] = {0.0f};
                    float pFlexDelayedWeights[MAXSTUDIOFLEXDESC] = {0.0f};
                    DrawModelResults_t modelResults;
                    memset(&modelResults, 0, sizeof(DrawModelResults_t));
                    g_pStudioRender->DrawModel(&modelResults, drawInfo, poseBones, pFlexWeights, pFlexDelayedWeights, Vector(0, 0, 0), STUDIORENDER_DRAW_ENTIRE_MODEL);
                    g_pStudioRender->EndFrame();
                    pRenderContext->OverrideDepthEnable(false, false);
                    
                    // 遍历骨骼计算本地包围盒
                    Vector localMins(1e9f, 1e9f, 1e9f);
                    Vector localMaxs(-1e9f, -1e9f, -1e9f);
                    
                    if (pBoneArray != nullptr)
                    {
                        int numBones = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
                        for (int i = 0; i < numBones; i++)
                        {
                            Vector bonePosModel(poseBones[i].m_flMatVal[0][3], poseBones[i].m_flMatVal[1][3], poseBones[i].m_flMatVal[2][3]);
                            localMins.x = std::min(localMins.x, bonePosModel.x);
                            localMins.y = std::min(localMins.y, bonePosModel.y);
                            localMins.z = std::min(localMins.z, bonePosModel.z);
                            localMaxs.x = std::max(localMaxs.x, bonePosModel.x);
                            localMaxs.y = std::max(localMaxs.y, bonePosModel.y);
                            localMaxs.z = std::max(localMaxs.z, bonePosModel.z);
                        }
                        localMins -= Vector(15.0f, 15.0f, 6.0f);
                        localMaxs += Vector(15.0f, 15.0f, 14.0f);
                        
                        Vector corners[8] = {
                            Vector(localMins.x, localMins.y, localMins.z),
                            Vector(localMaxs.x, localMins.y, localMins.z),
                            Vector(localMins.x, localMaxs.y, localMins.z),
                            Vector(localMaxs.x, localMaxs.y, localMins.z),
                            Vector(localMins.x, localMins.y, localMaxs.z),
                            Vector(localMaxs.x, localMins.y, localMaxs.z),
                            Vector(localMins.x, localMaxs.y, localMaxs.z),
                            Vector(localMaxs.x, localMaxs.y, localMaxs.z),
                        };
                        
                        Vector worldMins(1e9f, 1e9f, 1e9f);
                        Vector worldMaxs(-1e9f, -1e9f, -1e9f);
                        for (int c = 0; c < 8; c++)
                        {
                            Vector wc = matModel.VMul4x3(corners[c]);
                            worldMins.x = std::min(worldMins.x, wc.x);
                            worldMins.y = std::min(worldMins.y, wc.y);
                            worldMins.z = std::min(worldMins.z, wc.z);
                            worldMaxs.x = std::max(worldMaxs.x, wc.x);
                            worldMaxs.y = std::max(worldMaxs.y, wc.y);
                            worldMaxs.z = std::max(worldMaxs.z, wc.z);
                        }
                        
                        g_vecBBoxMins = worldMins;
                        g_vecBBoxMaxs = worldMaxs;
                    }
                }
            }

            pRenderContext->Flush(true);

            // 纯 OpenGL 绘制
            glUseProgram(0);
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glDisable(GL_SCISSOR_TEST);
            glDisable(GL_DEPTH_TEST);
            
            glViewport(0, 0, w, h);
            
            float glProj[16];
            BuildGLProjectionMatrix(glProj, 45.0f, (float)aspect, 1.0f, 2000.0f);
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(glProj);
            
            float glView[16];
            BuildGLViewMatrix(glView, g_vecEye, g_vecAt, Vector(0, 0, 1));
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glView);
            
            // 包围盒（世界坐标 AABB）
            float xmin = g_vecBBoxMins.x, xmax = g_vecBBoxMaxs.x;
            float ymin = g_vecBBoxMins.y, ymax = g_vecBBoxMaxs.y;
            float zmin = g_vecBBoxMins.z, zmax = g_vecBBoxMaxs.z;
            
            glLineWidth(3.0f);
            glColor3ub(255, 255, 0);
            glBegin(GL_LINES);
            glVertex3f(xmin, ymin, zmin); glVertex3f(xmax, ymin, zmin);
            glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymax, zmin);
            glVertex3f(xmax, ymax, zmin); glVertex3f(xmin, ymax, zmin);
            glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymin, zmin);
            glVertex3f(xmin, ymin, zmax); glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymin, zmax); glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmax, ymax, zmax); glVertex3f(xmin, ymax, zmax);
            glVertex3f(xmin, ymax, zmax); glVertex3f(xmin, ymin, zmax);
            glVertex3f(xmin, ymin, zmin); glVertex3f(xmin, ymin, zmax);
            glVertex3f(xmax, ymin, zmin); glVertex3f(xmax, ymin, zmax);
            glVertex3f(xmax, ymax, zmin); glVertex3f(xmax, ymax, zmax);
            glVertex3f(xmin, ymax, zmin); glVertex3f(xmin, ymax, zmax);
            glEnd();
            glLineWidth(1.0f);

            // Gizmo
            if (g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
            {
                Vector gizmoCenter(
                    (g_vecBBoxMins.x + g_vecBBoxMaxs.x) * 0.5f,
                    (g_vecBBoxMins.y + g_vecBBoxMaxs.y) * 0.5f,
                    (g_vecBBoxMins.z + g_vecBBoxMaxs.z) * 0.5f
                );
                
                float bboxSize = std::max(std::max(g_vecBBoxMaxs.x - g_vecBBoxMins.x,
                                                   g_vecBBoxMaxs.y - g_vecBBoxMins.y),
                                          g_vecBBoxMaxs.z - g_vecBBoxMins.z);
                float axisLength = bboxSize * 0.8f;
                if (axisLength < 20.0f) axisLength = 20.0f;
                
                Vector axisDirs[3] = { Vector(1,0,0), Vector(0,1,0), Vector(0,0,1) };
                
                // 鼠标射线（旋转圆环检测用）
                float ndcX = (g_iMouseX / (float)w) * 2.0f - 1.0f;
                float ndcY = 1.0f - (g_iMouseY / (float)h) * 2.0f;
                Vector rayDir = g_camForward 
                              - g_camLeft * (ndcX * g_camTanHalfFov * g_camAspect)
                              + g_camUp * (ndcY * g_camTanHalfFov);
                VectorNormalize(rayDir);
                Vector rayOrigin = g_vecEye;
                
                // ============================================================
                // 悬停检测：屏幕空间投影（最稳的方案）
                // ============================================================
                g_iHoverAxis = -1;
                const float HIT_TOLERANCE_PX = 15.0f;
                
                if (g_iGizmoMode == GIZMO_TRANSLATE)
                {
                    float bestDist = HIT_TOLERANCE_PX;
                    for (int a = 0; a < 3; a++)
                    {
                        Vector p0 = gizmoCenter;
                        Vector p1 = gizmoCenter + axisDirs[a] * axisLength;
                        
                        float sx0, sy0, sx1, sy1;
                        bool ok0 = WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                                 g_camTanHalfFov, g_camAspect, w, h, sx0, sy0);
                        bool ok1 = WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                                 g_camTanHalfFov, g_camAspect, w, h, sx1, sy1);
                        if (!ok0 || !ok1) continue;
                        
                        float dist = DistToScreenSegment((float)g_iMouseX, (float)g_iMouseY, sx0, sy0, sx1, sy1);
                        if (dist < bestDist)
                        {
                            bestDist = dist;
                            g_iHoverAxis = a;
                        }
                    }
                }
                else if (g_iGizmoMode == GIZMO_ROTATE)
                {
                    const int SAMPLES = 32;
                    for (int a = 0; a < 3; a++)
                    {
                        float minDist = 1e9f;
                        Vector u, v;
                        switch (a)
                        {
                        case 0: u = Vector(0,1,0); v = Vector(0,0,1); break;
                        case 1: u = Vector(1,0,0); v = Vector(0,0,1); break;
                        case 2: u = Vector(1,0,0); v = Vector(0,1,0); break;
                        }
                        
                        for (int i = 0; i < SAMPLES; i++)
                        {
                            float ang = (float)i / SAMPLES * 2.0f * (float)M_PI;
                            Vector ringPoint = gizmoCenter + u * (cosf(ang) * axisLength) + v * (sinf(ang) * axisLength);
                            
                            float sx, sy;
                            if (WorldToScreen(ringPoint, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                              g_camTanHalfFov, g_camAspect, w, h, sx, sy))
                            {
                                float d = sqrtf((sx - g_iMouseX)*(sx - g_iMouseX) + (sy - g_iMouseY)*(sy - g_iMouseY));
                                if (d < minDist) minDist = d;
                            }
                        }
                        
                        if (minDist < HIT_TOLERANCE_PX)
                        {
                            g_iHoverAxis = a;
                            break;
                        }
                    }
                }
                
                // ============================================================
                // 鼠标按下 → 开始拖动
                // ============================================================
                if (g_bMouseDown && !g_bDraggingGizmo && g_iActiveAxis < 0 && !io.WantCaptureMouse)
                {
                    if (g_iHoverAxis >= 0)
                    {
                        g_iActiveAxis = g_iHoverAxis;
                        g_bDraggingGizmo = true;
                        
                        if (g_iGizmoMode == GIZMO_TRANSLATE)
                        {
                            g_vecDragStartPos = vecTargetPos;
                            
                            // 用屏幕空间的 t 作为起始值
                            Vector p0 = gizmoCenter;
                            Vector p1 = gizmoCenter + axisDirs[g_iActiveAxis] * axisLength;
                            float sx0, sy0, sx1, sy1;
                            g_flDragStartWorldT = -1.0f;
                            if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                              g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                                WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                              g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                            {
                                float dx = sx1 - sx0;
                                float dy = sy1 - sy0;
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
                            g_flDragStartAngle = RayRingHitAngle(rayOrigin, rayDir, gizmoCenter, g_iActiveAxis, axisLength);
                            g_angDragStartRot = vecTargetRot;
                        }
                    }
                }
                
                // ============================================================
                // 拖动中
                // ============================================================
                if (g_iActiveAxis >= 0 && g_bMouseDown)
                {
                    if (g_iGizmoMode == GIZMO_TRANSLATE)
                    {
                        Vector p0 = gizmoCenter;
                        Vector p1 = gizmoCenter + axisDirs[g_iActiveAxis] * axisLength;
                        float sx0, sy0, sx1, sy1;
                        if (WorldToScreen(p0, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                          g_camTanHalfFov, g_camAspect, w, h, sx0, sy0) &&
                            WorldToScreen(p1, g_vecEye, g_camLeft, g_camUp, g_camForward,
                                          g_camTanHalfFov, g_camAspect, w, h, sx1, sy1))
                        {
                            float dx = sx1 - sx0;
                            float dy = sy1 - sy0;
                            float lenSq = dx*dx + dy*dy;
                            if (lenSq > 1.0f)
                            {
                                float t = ((g_iMouseX - sx0) * dx + (g_iMouseY - sy0) * dy) / lenSq;
                                t = std::max(0.0f, std::min(1.0f, t));
                                
                                float worldDelta = t * axisLength;
                                
                                if (g_flDragStartWorldT < 0.0f)
                                {
                                    // 第一次进入拖动，记录起始 t
                                    g_flDragStartWorldT = worldDelta;
                                }
                                else
                                {
                                    float deltaT = worldDelta - g_flDragStartWorldT;
                                    vecTargetPos = g_vecDragStartPos + axisDirs[g_iActiveAxis] * deltaT;
                                }
                            }
                        }
                    }
                    else if (g_iGizmoMode == GIZMO_ROTATE)
                    {
                        float curAngle = RayRingHitAngle(rayOrigin, rayDir, gizmoCenter, g_iActiveAxis, axisLength);
                        if (curAngle >= 0.0f)
                        {
                            float deltaAngle = curAngle - g_flDragStartAngle;
                            if (deltaAngle > M_PI) deltaAngle -= 2.0f * M_PI;
                            if (deltaAngle < -M_PI) deltaAngle += 2.0f * M_PI;
                            
                            float deltaDeg = deltaAngle * (180.0f / M_PI);
                            
                            if (g_iActiveAxis == 0)      vecTargetRot.x = g_angDragStartRot.x + deltaDeg;
                            else if (g_iActiveAxis == 1) vecTargetRot.y = g_angDragStartRot.y + deltaDeg;
                            else                          vecTargetRot.z = g_angDragStartRot.z + deltaDeg;
                        }
                    }
                }
                
                // ============================================================
                // 绘制 Gizmo
                // ============================================================
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
            else
            {
                g_iHoverAxis = -1;
            }

            // ImGui UI
            ImGui::Begin("Hammer Operator Console");
            ImGui::Text("Camera: Left drag (rotate) | Right drag (pan) | Wheel (zoom)");
            ImGui::Text("Gizmo:  [Q] None  [W] Translate  [R] Rotate");
            ImGui::Text("Current Mode: %s", 
                g_iGizmoMode == GIZMO_NONE ? "None" :
                g_iGizmoMode == GIZMO_TRANSLATE ? "Translate" : "Rotate");
            ImGui::Separator();
            ImGui::Text("Position:");
            ImGui::SliderFloat("Pos X", &vecTargetPos.x, -100.0f, 100.0f, "%.2f");
            ImGui::SliderFloat("Pos Y", &vecTargetPos.y, -100.0f, 100.0f, "%.2f");
            ImGui::SliderFloat("Pos Z", &vecTargetPos.z, -100.0f, 100.0f, "%.2f");
            ImGui::Separator();
            ImGui::Text("Rotation (Pitch/Yaw/Roll):");
            ImGui::SliderFloat("Pitch", &vecTargetRot.x, -180.0f, 180.0f, "%.1f");
            ImGui::SliderFloat("Yaw",   &vecTargetRot.y, -180.0f, 180.0f, "%.1f");
            ImGui::SliderFloat("Roll",  &vecTargetRot.z, -180.0f, 180.0f, "%.1f");
            ImGui::Separator();
            ImGui::Checkbox("Show Gizmo", &g_bShowGizmo);
            
            if (g_iActiveAxis >= 0)
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "DRAGGING: %s", 
                    g_iActiveAxis == 0 ? "X" : g_iActiveAxis == 1 ? "Y" : "Z");
            else if (g_iHoverAxis >= 0)
                ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Hover: %s", 
                    g_iHoverAxis == 0 ? "X" : g_iHoverAxis == 1 ? "Y" : "Z");
            
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
            }
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
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