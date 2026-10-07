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

IMaterialSystem *g_pMaterialSystem;
IFileSystem *g_pFileSystem;
IDataCache *g_pDataCache;
IInputSystem *g_pInputSystem;
IStudioRender *g_pStudioRender;
IMDLCache *g_pMDLCache;

extern void *CreateSDLMgr();

//-----------------------------------------------------------------------------
struct CBrush
{
    Vector m_vecPos;
    Vector m_vecSize;
    QAngle m_angRot;
    int    m_iTexId;

    CBrush() : m_vecPos(0,0,0), m_vecSize(32,32,32), m_angRot(0,0,0), m_iTexId(0) {}

    Vector GetBBoxMins() const { return m_vecPos - m_vecSize; }
    Vector GetBBoxMaxs() const { return m_vecPos + m_vecSize; }
};

enum EntityType { ENTITY_MODEL = 0, ENTITY_PLAYER_START = 1 };

struct CEntity
{
    EntityType   m_iType;
    MDLHandle_t  m_hMdl;
    const char*  m_szName;
    Vector       m_vecPos;
    QAngle       m_angRot;
    Vector       m_vecBBoxMins;
    Vector       m_vecBBoxMaxs;

    CEntity()
        : m_iType(ENTITY_MODEL)
        , m_hMdl(MDLHANDLE_INVALID)
        , m_szName("")
        , m_vecPos(0, 0, 0)
        , m_angRot(0, 0, 0)
        , m_vecBBoxMins(-16, -16, 0)
        , m_vecBBoxMaxs(16, 16, 72)
    {}

    Vector GetCenter() const
    {
        return Vector(
            (m_vecBBoxMins.x + m_vecBBoxMaxs.x) * 0.5f,
            (m_vecBBoxMins.y + m_vecBBoxMaxs.y) * 0.5f,
            (m_vecBBoxMins.z + m_vecBBoxMaxs.z) * 0.5f
        );
    }
};

std::vector<CEntity> g_entities;
std::vector<CBrush>  g_brushes;
int g_iSelectedEntity = 0;

GLuint g_texChecker = 0;
GLuint g_texBrick = 0;
GLuint g_texFloor = 0;

static void CreatePlaceholderTextures()
{
    const int SIZE = 128;
    unsigned char data[SIZE * SIZE * 4];

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
        {
            int c = ((x / 16) + (y / 16)) % 2;
            unsigned char v = c ? 220 : 120;
            int idx = (y * SIZE + x) * 4;
            data[idx+0] = v; data[idx+1] = v; data[idx+2] = v; data[idx+3] = 255;
        }
    glGenTextures(1, &g_texChecker);
    glBindTexture(GL_TEXTURE_2D, g_texChecker);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
        {
            int row = y / 16;
            int mortarX = (x + (row % 2) * 16) % 32;
            int mortarY = y % 16;
            int idx = (y * SIZE + x) * 4;
            bool isMortar = (mortarX < 2) || (mortarY < 2);
            if (isMortar) { data[idx+0]=100; data[idx+1]=100; data[idx+2]=100; }
            else
            {
                int rand = ((x * 7 + y * 13) % 30) - 15;
                data[idx+0] = (unsigned char)(160 + rand);
                data[idx+1] = (unsigned char)(70 + rand / 2);
                data[idx+2] = (unsigned char)(50 + rand / 2);
            }
            data[idx+3] = 255;
        }
    glGenTextures(1, &g_texBrick);
    glBindTexture(GL_TEXTURE_2D, g_texBrick);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
        {
            int c = ((x / 8) + (y / 8)) % 2;
            unsigned char v = c ? 180 : 140;
            int idx = (y * SIZE + x) * 4;
            data[idx+0] = v;
            data[idx+1] = (unsigned char)(v * 0.9f);
            data[idx+2] = (unsigned char)(v * 0.7f);
            data[idx+3] = 255;
        }
    glGenTextures(1, &g_texFloor);
    glBindTexture(GL_TEXTURE_2D, g_texFloor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE, SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

static GLuint GetBrushTexture(int id)
{
    if (id == 0) return g_texChecker;
    if (id == 1) return g_texBrick;
    return g_texFloor;
}

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
static void BuildGLProjectionMatrix(float* m, float fovY, float aspect, float zNear, float zFar)
{
    float f = 1.0f / tanf(fovY * 0.5f * (float)M_PI / 180.0f);
    float rangeRecip = 1.0f / (zNear - zFar);
    memset(m, 0, sizeof(float) * 16);
    m[0] = f / aspect; m[5] = f;
    m[10] = (zNear + zFar) * rangeRecip; m[11] = -1.0f;
    m[14] = 2.0f * zNear * zFar * rangeRecip;
}

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
    m[0] = side.x; m[4] = side.y; m[8] = side.z; m[12] = -DotProduct(side, eye);
    m[1] = up.x; m[5] = up.y; m[9] = up.z; m[13] = -DotProduct(up, eye);
    m[2] = -forward.x; m[6] = -forward.y; m[10] = -forward.z; m[14] = DotProduct(forward, eye);
    m[15] = 1.0f;
}

enum GizmoMode { GIZMO_NONE = 0, GIZMO_TRANSLATE, GIZMO_ROTATE };

//-----------------------------------------------------------------------------
static void DrawBrush(const CBrush& b)
{
    Vector mins = b.GetBBoxMins();
    Vector maxs = b.GetBBoxMaxs();
    float x0 = mins.x, x1 = maxs.x;
    float y0 = mins.y, y1 = maxs.y;
    float z0 = mins.z, z1 = maxs.z;
    const float TEX_SCALE = 1.0f / 64.0f;

    glBindTexture(GL_TEXTURE_2D, GetBrushTexture(b.m_iTexId));
    glColor3ub(255, 255, 255);
    glBegin(GL_QUADS);
    // +Z 顶面
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (y1 - y0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z1);
        glTexCoord2f(su,0); glVertex3f(x1, y0, z1);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z1);
        glTexCoord2f(0,sv); glVertex3f(x0, y1, z1);
    }
    // -Z 底面
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (y1 - y0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z0);
        glTexCoord2f(0,sv); glVertex3f(x0, y1, z0);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z0);
        glTexCoord2f(su,0); glVertex3f(x1, y0, z0);
    }
    // +X
    {
        float su = (y1 - y0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x1, y0, z0);
        glTexCoord2f(su,0); glVertex3f(x1, y1, z0);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z1);
        glTexCoord2f(0,sv); glVertex3f(x1, y0, z1);
    }
    // -X
    {
        float su = (y1 - y0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z0);
        glTexCoord2f(0,sv); glVertex3f(x0, y0, z1);
        glTexCoord2f(su,sv); glVertex3f(x0, y1, z1);
        glTexCoord2f(su,0); glVertex3f(x0, y1, z0);
    }
    // +Y
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y1, z0);
        glTexCoord2f(su,0); glVertex3f(x1, y1, z0);
        glTexCoord2f(su,sv); glVertex3f(x1, y1, z1);
        glTexCoord2f(0,sv); glVertex3f(x0, y1, z1);
    }
    // -Y
    {
        float su = (x1 - x0) * TEX_SCALE, sv = (z1 - z0) * TEX_SCALE;
        glTexCoord2f(0,0); glVertex3f(x0, y0, z0);
        glTexCoord2f(0,sv); glVertex3f(x0, y0, z1);
        glTexCoord2f(su,sv); glVertex3f(x1, y0, z1);
        glTexCoord2f(su,0); glVertex3f(x1, y0, z0);
    }
    glEnd();
}

static void DrawBrushEdges(const CBrush& b)
{
    Vector mins = b.GetBBoxMins();
    Vector maxs = b.GetBBoxMaxs();
    float x0 = mins.x, x1 = maxs.x;
    float y0 = mins.y, y1 = maxs.y;
    float z0 = mins.z, z1 = maxs.z;
    glLineWidth(1.5f);
    glColor3ub(60, 60, 60);
    glBegin(GL_LINES);
    glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0);
    glVertex3f(x1,y0,z0); glVertex3f(x1,y1,z0);
    glVertex3f(x1,y1,z0); glVertex3f(x0,y1,z0);
    glVertex3f(x0,y1,z0); glVertex3f(x0,y0,z0);
    glVertex3f(x0,y0,z1); glVertex3f(x1,y0,z1);
    glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1);
    glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
    glVertex3f(x0,y1,z1); glVertex3f(x0,y0,z1);
    glVertex3f(x0,y0,z0); glVertex3f(x0,y0,z1);
    glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1);
    glVertex3f(x1,y1,z0); glVertex3f(x1,y1,z1);
    glVertex3f(x0,y1,z0); glVertex3f(x0,y1,z1);
    glEnd();
    glLineWidth(1.0f);
}

static void DrawPlayerStartModel(const Vector& pos, const QAngle& ang)
{
    const float radius = 16.0f;
    const float height = 72.0f;
    const int SEG = 16;
    glLineWidth(2.0f);
    glColor3ub(0, 200, 0);
    for (int ring = 0; ring < 2; ring++)
    {
        float z = pos.z + (ring == 0 ? 0.0f : height);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < SEG; i++)
        {
            float a = (float)i / SEG * 2.0f * (float)M_PI;
            glVertex3f(pos.x + cosf(a) * radius, pos.y + sinf(a) * radius, z);
        }
        glEnd();
    }
    glBegin(GL_LINES);
    for (int i = 0; i < 4; i++)
    {
        float a = (float)i / 4 * 2.0f * (float)M_PI;
        float cx = pos.x + cosf(a) * radius;
        float cy = pos.y + sinf(a) * radius;
        glVertex3f(cx, cy, pos.z);
        glVertex3f(cx, cy, pos.z + height);
    }
    glEnd();
    float yawRad = ang.y * (float)M_PI / 180.0f;
    float dx = cosf(yawRad), dy = sinf(yawRad);
    Vector tip = Vector(pos.x, pos.y, pos.z + height);
    Vector head = Vector(pos.x + dx * 30.0f, pos.y + dy * 30.0f, pos.z + height);
    glLineWidth(3.0f);
    glColor3ub(0, 255, 0);
    glBegin(GL_LINES);
    glVertex3f(tip.x, tip.y, tip.z);
    glVertex3f(head.x, head.y, head.z);
    float wingL = 8.0f;
    Vector left(-dy, dx, 0);
    Vector tipL = head - Vector(dx, dy, 0) * 8.0f + left * wingL;
    Vector tipR = head - Vector(dx, dy, 0) * 8.0f - left * wingL;
    glVertex3f(head.x, head.y, head.z); glVertex3f(tipL.x, tipL.y, tipL.z);
    glVertex3f(head.x, head.y, head.z); glVertex3f(tipR.x, tipR.y, tipR.z);
    glEnd();
    glLineWidth(1.0f);
}

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
    glVertex3f(center.x, center.y, center.z); glVertex3f(center.x + length, center.y, center.z);
    glColor3ub(cY[0], cY[1], cY[2]);
    glVertex3f(center.x, center.y, center.z); glVertex3f(center.x, center.y + length, center.z);
    glColor3ub(cZ[0], cZ[1], cZ[2]);
    glVertex3f(center.x, center.y, center.z); glVertex3f(center.x, center.y, center.z + length);
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

static bool RayRingPlaneAngle(const Vector& rayOrigin, const Vector& rayDir,
                              const Vector& center, int axis, float& outAngle)
{
    Vector normal = (axis == 0) ? Vector(1,0,0) : (axis == 1) ? Vector(0,1,0) : Vector(0,0,1);
    float denom = DotProduct(rayDir, normal);
    if (fabsf(denom) < 1e-6f) return false;
    float t = DotProduct(center - rayOrigin, normal) / denom;
    if (t < 0.0f) return false;
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
    outAngle = atan2f(pv, pu);
    return true;
}

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

static float RayToAxisSegmentDistSq(const Vector& rayOrigin, const Vector& rayDir,
                                    const Vector& axisOrigin, const Vector& axisDir,
                                    float axisLength, float& outT)
{
    Vector w0 = rayOrigin - axisOrigin;
    float a = DotProduct(rayDir, rayDir);
    float b = DotProduct(rayDir, axisDir);
    float c = DotProduct(axisDir, axisDir);
    float d = DotProduct(rayDir, w0);
    float e = DotProduct(axisDir, w0);
    float denom = a * c - b * b;
    if (fabsf(denom) < 1e-6f) { outT = 0; return 1e9f; }
    float tRay  = (b * e - c * d) / denom;
    float tAxis = (a * e - b * d) / denom;
    if (tRay < 0.0f) tRay = 0.0f;
    if (tAxis < 0.0f) tAxis = 0.0f;
    if (tAxis > axisLength) tAxis = axisLength;
    Vector pRay  = rayOrigin + rayDir * tRay;
    Vector pAxis = axisOrigin + axisDir * tAxis;
    outT = tAxis;
    return (pRay - pAxis).LengthSqr();
}

static bool IsMouseOverEntityBBox(const CEntity& ent,
                                  const Vector& camEye, const Vector& camLeft, const Vector& camUp, const Vector& camForward,
                                  float tanHalfFov, float aspect,
                                  int screenW, int screenH,
                                  int mouseX, int mouseY)
{
    Vector corners[8] = {
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMins.z),
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMaxs.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMins.y, ent.m_vecBBoxMaxs.z),
        Vector(ent.m_vecBBoxMins.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMaxs.z),
        Vector(ent.m_vecBBoxMaxs.x, ent.m_vecBBoxMaxs.y, ent.m_vecBBoxMaxs.z),
    };
    float sx[8], sy[8]; bool ok[8];
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    int okCount = 0;
    for (int i = 0; i < 8; i++)
    {
        ok[i] = WorldToScreen(corners[i], camEye, camLeft, camUp, camForward, tanHalfFov, aspect, screenW, screenH, sx[i], sy[i]);
        if (ok[i]) { minX = std::min(minX, sx[i]); maxX = std::max(maxX, sx[i]); minY = std::min(minY, sy[i]); maxY = std::max(maxY, sy[i]); okCount++; }
    }
    if (okCount < 2) return false;
    return (mouseX >= minX && mouseX <= maxX && mouseY >= minY && mouseY <= maxY);
}

static void BuildRotationMatrixFromQAngle(const QAngle& angles, VMatrix& outMatrix)
{
    matrix3x4_t mat;
    AngleMatrix(angles, mat);
    outMatrix.Identity();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            outMatrix.m[i][j] = mat.m_flMatVal[i][j];
}

static void UpdateEntityBBox(CEntity& ent)
{
    if (ent.m_iType == ENTITY_PLAYER_START)
    {
        Vector halfSize(16, 16, 36);
        ent.m_vecBBoxMins = ent.m_vecPos - halfSize;
        ent.m_vecBBoxMaxs = ent.m_vecPos + halfSize;
        return;
    }
    if (ent.m_hMdl == MDLHANDLE_INVALID) return;
    studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(ent.m_hMdl);
    if (!pStudioHdr) return;

    VMatrix matModelTranslation;
    matModelTranslation.Identity();
    matModelTranslation.m[0][3] = ent.m_vecPos.x;
    matModelTranslation.m[1][3] = ent.m_vecPos.y;
    matModelTranslation.m[2][3] = ent.m_vecPos.z;
    VMatrix matModelRotation;
    BuildRotationMatrixFromQAngle(ent.m_angRot, matModelRotation);
    VMatrix matModel;
    MatrixMultiply(matModelTranslation, matModelRotation, matModel);

    matrix3x4_t matModel3x4;
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 4; c++)
            matModel3x4.m_flMatVal[r][c] = matModel.m[r][c];

    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
    if (!pBoneArray) return;
    int numBones = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
    Vector localMins(1e9f, 1e9f, 1e9f);
    Vector localMaxs(-1e9f, -1e9f, -1e9f);
    matrix3x4_t poseBones[MAXSTUDIOBONES] = {};
    for (int i = 0; i < numBones; i++)
    {
        Vector bonePos = pBoneArray[i].pos;
        Quaternion boneQuat = pBoneArray[i].quat;
        QuaternionMatrix(boneQuat, bonePos, poseBones[i]);
        int parentIdx = pBoneArray[i].parent;
        if (parentIdx >= 0 && parentIdx < numBones)
        {
            matrix3x4_t temp; MatrixCopy(poseBones[i], temp);
            ConcatTransforms(poseBones[parentIdx], temp, poseBones[i]);
        }
    }
    for (int i = 0; i < numBones; i++)
    {
        matrix3x4_t finalBone;
        ConcatTransforms(matModel3x4, poseBones[i], finalBone);
        MatrixCopy(finalBone, poseBones[i]);
    }
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
    ent.m_vecBBoxMins = localMins;
    ent.m_vecBBoxMaxs = localMaxs;
}

static void RenderEntityModel(CEntity& ent, IMatRenderContext *pRenderContext)
{
    if (ent.m_iType == ENTITY_PLAYER_START) return;
    if (ent.m_hMdl == MDLHANDLE_INVALID) return;
    studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(ent.m_hMdl);
    studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(ent.m_hMdl);
    if (!pStudioHdr || !pHardwareData) return;

    VMatrix matModelTranslation;
    matModelTranslation.Identity();
    matModelTranslation.m[0][3] = ent.m_vecPos.x;
    matModelTranslation.m[1][3] = ent.m_vecPos.y;
    matModelTranslation.m[2][3] = ent.m_vecPos.z;
    VMatrix matModelRotation;
    BuildRotationMatrixFromQAngle(ent.m_angRot, matModelRotation);
    VMatrix matModel;
    MatrixMultiply(matModelTranslation, matModelRotation, matModel);

    pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);
    pRenderContext->MatrixMode(MATERIAL_MODEL);
    pRenderContext->LoadIdentity();

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
    if (pBoneArray)
    {
        int numBones = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
        for (int i = 0; i < numBones; i++)
        {
            Vector bonePos = pBoneArray[i].pos;
            Quaternion boneQuat = pBoneArray[i].quat;
            QuaternionMatrix(boneQuat, bonePos, poseBones[i]);
            int parentIdx = pBoneArray[i].parent;
            if (parentIdx >= 0 && parentIdx < numBones)
            {
                matrix3x4_t temp; MatrixCopy(poseBones[i], temp);
                ConcatTransforms(poseBones[parentIdx], temp, poseBones[i]);
            }
        }
        matrix3x4_t matModel3x4;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 4; c++)
                matModel3x4.m_flMatVal[r][c] = matModel.m[r][c];
        for (int i = 0; i < numBones; i++)
        {
            matrix3x4_t finalBone;
            ConcatTransforms(matModel3x4, poseBones[i], finalBone);
            MatrixCopy(finalBone, poseBones[i]);
        }
        g_pStudioRender->LockBoneMatrices(numBones);
        g_pStudioRender->UnlockBoneMatrices();
    }
    float pFlexWeights[MAXSTUDIOFLEXDESC] = {0.0f};
    float pFlexDelayedWeights[MAXSTUDIOFLEXDESC] = {0.0f};
    DrawModelResults_t modelResults;
    memset(&modelResults, 0, sizeof(DrawModelResults_t));
    g_pStudioRender->DrawModel(&modelResults, drawInfo, poseBones, pFlexWeights, pFlexDelayedWeights, Vector(0, 0, 0), STUDIORENDER_DRAW_ENTIRE_MODEL);
    g_pStudioRender->EndFrame();
    pRenderContext->OverrideDepthEnable(false, false);
}

static void DrawEntityBBox(const CEntity& ent, bool bSelected)
{
    float xmin = ent.m_vecBBoxMins.x, xmax = ent.m_vecBBoxMaxs.x;
    float ymin = ent.m_vecBBoxMins.y, ymax = ent.m_vecBBoxMaxs.y;
    float zmin = ent.m_vecBBoxMins.z, zmax = ent.m_vecBBoxMaxs.z;
    if (bSelected) { glLineWidth(3.0f); glColor3ub(255, 255, 0); }
    else { glLineWidth(1.5f); glColor3ub(120, 120, 120); }
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
}

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
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io; io.IniFilename = nullptr;
    ImGui_ImplSDL2_InitForOpenGL(pWindow, glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    CreatePlaceholderTextures();

    {
        CEntity alyx;
        alyx.m_iType = ENTITY_MODEL;
        alyx.m_hMdl = g_pMDLCache->FindMDL("models/alyx.mdl");
        alyx.m_szName = "Alyx";
        alyx.m_vecPos = Vector(-80, 0, 0);
        g_entities.push_back(alyx);

        CEntity start;
        start.m_iType = ENTITY_PLAYER_START;
        start.m_hMdl = MDLHANDLE_INVALID;
        start.m_szName = "info_player_start";
        start.m_vecPos = Vector(80, 0, 0);
        start.m_angRot = QAngle(0, 0, 0);
        g_entities.push_back(start);
    }
    g_iSelectedEntity = 0;

    // 【终极组合】Brush 用反向 Z（引擎视图矩阵 Z 轴朝下）
    {
        // 地板：世界 z=+8（屏幕上在 Alyx 脚下）
        CBrush floor;
        floor.m_vecPos = Vector(0, 0, -8);
        floor.m_vecSize = Vector(256, 256, 8);
        floor.m_iTexId = 2;
        g_brushes.push_back(floor);

        // 北墙：世界 z=-128（屏幕上向上延伸）
        CBrush wallN;
        wallN.m_vecPos = Vector(0, 256, 128);
        wallN.m_vecSize = Vector(256, 8, 128);
        wallN.m_iTexId = 1;
        g_brushes.push_back(wallN);

        // 西墙：同上
        CBrush wallW;
        wallW.m_vecPos = Vector(-256, 0, 128);
        wallW.m_vecSize = Vector(8, 256, 128);
        wallW.m_iTexId = 1;
        g_brushes.push_back(wallW);
    }

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

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    uint32_t lastTicks = SDL_GetTicks();

    while (bRunning)
    {
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
            case SDL_QUIT: bRunning = false; break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                { w = event.window.data1; h = event.window.data2; }
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
            if (g_bLeftMouseDown && g_iHoverAxis < 0)
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
            // 而 OpenGL 期望 Y 轴朝上。翻转 glView 的 Y 行（OpenGL 列主序的第 1 列）
            // 来纠正这个差异，让手写的 OpenGL 绘制（Brush/Gizmo/包围盒）和
            // 引擎渲染的模型（Alyx/player_start）视觉对齐。
            glView[1]  = -glView[1];
            glView[5]  = -glView[5];
            glView[9]  = -glView[9];
            glView[13] = -glView[13];

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

            if (g_bXRayGizmo)
                glDisable(GL_DEPTH_TEST);

            CEntity& sel = g_entities[g_iSelectedEntity];
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
                    const float MAX_SCREEN_DIST = 300.0f;
                    if (mouseToGizmoSq < MAX_SCREEN_DIST * MAX_SCREEN_DIST)
                    {
                        if (g_iGizmoMode == GIZMO_TRANSLATE)
                        {
                            float threshold = axisLength * 0.06f;
                            if (threshold < 2.0f) threshold = 2.0f;
                            float bestDistSq = threshold * threshold;
                            for (int a = 0; a < 3; a++)
                            {
                                float tDummy;
                                float distSq = RayToAxisSegmentDistSq(rayOrigin, rayDir, gizmoCenter, axisDirs[a], axisLength, tDummy);
                                if (distSq < bestDistSq) { bestDistSq = distSq; g_iHoverAxis = a; }
                            }
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

            if (g_bLeftMouseDown && !g_bDraggingGizmo && g_iActiveAxis < 0 && !io.WantCaptureMouse)
            {
                if (g_iHoverAxis >= 0 && g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
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
                    for (int i = (int)g_entities.size() - 1; i >= 0; i--)
                    {
                        if (IsMouseOverEntityBBox(g_entities[i], g_vecEye, g_camLeft, g_camUp, g_camForward, g_camTanHalfFov, g_camAspect, w, h, g_iMouseX, g_iMouseY))
                        {
                            g_iSelectedEntity = i;
                            break;
                        }
                    }
                }
            }

            if (g_bDraggingGizmo && g_bLeftMouseDown && g_iActiveAxis >= 0)
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

            if (g_bShowGizmo && g_iGizmoMode != GIZMO_NONE)
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

            ImGui::Begin("Hammer Operator Console");
            ImGui::Text("Camera: LMB empty = orbit | RMB/MMB = pan | Wheel = zoom");
            ImGui::Text("        W/A/S/D = fly | Q/E = down/up | Z / Shift+Z = zoom");
            ImGui::Text("        [R] = Reset Camera");
            ImGui::Text("Gizmo:  1 = Translate | 2 = Rotate | 3 = None");
            ImGui::Text("Current: %s",
                g_iGizmoMode == GIZMO_NONE ? "None" :
                g_iGizmoMode == GIZMO_TRANSLATE ? "Translate" : "Rotate");
            ImGui::Separator();
            ImGui::Text("Entities:");
            for (size_t i = 0; i < g_entities.size(); i++)
            {
                bool bSel = (int)i == g_iSelectedEntity;
                if (ImGui::Selectable(g_entities[i].m_szName, bSel))
                    g_iSelectedEntity = (int)i;
            }
            ImGui::Separator();
            if (g_iSelectedEntity >= 0 && g_iSelectedEntity < (int)g_entities.size())
            {
                CEntity& sel2 = g_entities[g_iSelectedEntity];
                ImGui::Text("Selected: %s", sel2.m_szName);
                ImGui::Separator();
                ImGui::SliderFloat("Pos X", &sel2.m_vecPos.x, -256.0f, 256.0f, "%.2f");
                ImGui::SliderFloat("Pos Y", &sel2.m_vecPos.y, -256.0f, 256.0f, "%.2f");
                ImGui::SliderFloat("Pos Z", &sel2.m_vecPos.z, -50.0f, 200.0f, "%.2f");
                ImGui::Separator();
                if (sel2.m_iType == ENTITY_PLAYER_START)
                    ImGui::SliderFloat("Yaw", &sel2.m_angRot.y, -180.0f, 180.0f, "%.1f");
                else
                {
                    ImGui::SliderFloat("Pitch", &sel2.m_angRot.x, -180.0f, 180.0f, "%.1f");
                    ImGui::SliderFloat("Yaw",   &sel2.m_angRot.y, -180.0f, 180.0f, "%.1f");
                    ImGui::SliderFloat("Roll",  &sel2.m_angRot.z, -180.0f, 180.0f, "%.1f");
                }
                ImGui::Separator();
            }
            ImGui::Checkbox("Show Gizmo", &g_bShowGizmo);
            ImGui::Checkbox("X-Ray Gizmo", &g_bXRayGizmo);
            ImGui::Text("Cam Target: (%.1f, %.1f, %.1f)", m_camTarget.x, m_camTarget.y, m_camTarget.z);
            ImGui::Text("Cam Dist: %.1f", m_camDistance);
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Press R to reset camera!");
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