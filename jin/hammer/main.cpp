#include <cmath>
#include <SDL2/SDL.h>
#include <GL/gl.h>

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

    // Replaced legacy GetConfigAnimationStage call to avoid unallocated structures
    g_pMaterialSystem->SetAdapter(0, MATERIAL_INIT_ALLOCATE_FULLSCREEN_TEXTURE);

    return true;
}

void CHammerApp::PostShutdown() {}

//-----------------------------------------------------------------------------
// Pure OpenGL Camera Matrix Replacements (Fixed C++ Array Syntaxes)
//-----------------------------------------------------------------------------
static void SetupPerspectiveMatrix(double fovY, double aspect, double zNear, double zFar)
{
    double f = 1.0 / std::tan((fovY * M_PI / 180.0) / 2.0);
    GLfloat matrix[16] = {
        (GLfloat)(f / aspect), 0.0f, 0.0f, 0.0f,
        0.0f, (GLfloat)f, 0.0f, 0.0f,
        0.0f, 0.0f, (GLfloat)((zFar + zNear) / (zNear - zFar)), -1.0f,
        0.0f, 0.0f, (GLfloat)((2.0 * zFar * zNear) / (zNear - zFar)), 0.0f};
    glMultMatrixf(matrix);
}

static void SetupLookAtMatrix(float eyex, float eyey, float eyez,
                              float centerx, float centery, float centerz,
                              float upx, float upy, float upz)
{
    float forward[3] = {centerx - eyex, centery - eyey, centerz - eyez};
    float up[3] = {upx, upy, upz};

    // Normalize forward vector
    float lengthF = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    if (lengthF > 0.0f)
    {
        forward[0] /= lengthF;
        forward[1] /= lengthF;
        forward[2] /= lengthF;
    }

    // Cross product: side = forward x up
    float side[3] = {
        forward[1] * up[2] - forward[2] * up[1],
        forward[2] * up[0] - forward[0] * up[2],
        forward[0] * up[1] - forward[1] * up[0]};
    float lengthS = std::sqrt(side[0] * side[0] + side[1] * side[1] + side[2] * side[2]);
    if (lengthS > 0.0f)
    {
        side[0] /= lengthS;
        side[1] /= lengthS;
        side[2] /= lengthS;
    }

    // Recalculate up: up = side x forward
    up[0] = side[1] * forward[2] - side[2] * forward[1];
    up[1] = side[2] * forward[0] - side[0] * forward[2];
    up[2] = side[0] * forward[1] - side[1] * forward[0];

    GLfloat matrix[16] = {
        side[0], up[0], -forward[0], 0.0f,
        side[1], up[1], -forward[1], 0.0f,
        side[2], up[2], -forward[2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    glMultMatrixf(matrix);
    glTranslatef(-eyex, -eyey, -eyez);
}

//-----------------------------------------------------------------------------
// Main application loop (Fixed bone allocation, clean syntax)
//-----------------------------------------------------------------------------
int CHammerApp::Main()
{
    // 1. Initial Window Handle Lookups & Layout Setup
    SDL_Window *pWindow = SDL_GL_GetCurrentWindow();
    int w = 1280;
    int h = 720;
    if (pWindow)
    {
        SDL_GetWindowSize(pWindow, &w, &h);
        if (w == 0 || h == 0)
        {
            w = 1280;
            h = 720;
        }
    }

    MaterialVideoMode_t mode;
    mode.m_Width = w;
    mode.m_Height = h;
    mode.m_Format = IMAGE_FORMAT_RGBA8888;
    mode.m_RefreshRate = 60;

    MaterialSystem_Config_t config;
    config.m_VideoMode = mode;
    config.SetFlag(MATSYS_VIDCFG_FLAGS_WINDOWED, true);

    if (!g_pMaterialSystem->SetMode((void *)pWindow, config))
    {
        Warning("[HAMMER] Material System SetMode tracking failure.\n");
    }

    pWindow = SDL_GL_GetCurrentWindow();
    if (!pWindow)
    {
        Error("[HAMMER] Active window pointer lost after initialization.\n");
        return -1;
    }

    SDL_ShowWindow(pWindow);
    SDL_RaiseWindow(pWindow);
    SDL_SetWindowSize(pWindow, w, h);

    MDLHandle_t hMdl = g_pMDLCache->FindMDL("models/alyx.mdl");
    if (hMdl == MDLHANDLE_INVALID)
    {
        Warning("[HAMMER] Search paths completely failed to trace 'models/alyx.mdl'\n");
    }

    IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();
    if (!pRenderContext)
    {
        Error("[HAMMER] Failed to acquire active IMatRenderContext instance.\n");
        return -1;
    }
    pRenderContext->Viewport(0, 0, w, h);

    bool bRunning = true;
    SDL_Event event;

    // Camera Navigation Initial Configuration
    float flCameraPitch = 10.0f;
    float flCameraYaw = -90.0f;
    float flZoomScale = 1.8f;
    float flPanX = 0.0f;
    float flPanY = 0.0f;
    float flPanZ = 35.0f;

    bool bLeftMouseDown = false;
    bool bRightMouseDown = false;

    // Animation Cycle & Diagnostics Trackers
    float flAnimCycle = 0.0f;
    int nTargetSequenceIndex = 0;

    // --- FPS COUNTER VARIABLES ---
    uint32_t nFrameCount = 0;
    uint32_t nLastFpsUpdateTicks = SDL_GetTicks();

    uint32_t lastTicks = SDL_GetTicks();

    while (bRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                bRunning = false;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    bRunning = false;

                if (event.key.keysym.sym == SDLK_UP)
                {
                    nTargetSequenceIndex++;
                    flAnimCycle = 0.0f;
                    Msg("[HAMMER] Swapped target playback to animation sequence: %d\n", nTargetSequenceIndex);
                }
                if (event.key.keysym.sym == SDLK_DOWN)
                {
                    nTargetSequenceIndex--;
                    if (nTargetSequenceIndex < 0)
                        nTargetSequenceIndex = 0;
                    flAnimCycle = 0.0f;
                    Msg("[HAMMER] Swapped target playback to animation sequence: %d\n", nTargetSequenceIndex);
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                    bLeftMouseDown = true;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    bRightMouseDown = true;
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    bLeftMouseDown = false;
                if (event.button.button == SDL_BUTTON_RIGHT)
                    bRightMouseDown = false;
                break;

            case SDL_MOUSEMOTION:
                if (bLeftMouseDown)
                {
                    flCameraYaw += event.motion.xrel * 0.25f;
                    flCameraPitch += event.motion.yrel * 0.25f;
                    if (flCameraPitch > 89.0f)
                        flCameraPitch = 89.0f;
                    if (flCameraPitch < -89.0f)
                        flCameraPitch = -89.0f;
                }
                else if (bRightMouseDown)
                {
                    float radYaw = flCameraYaw * (M_PI / 180.0f);
                    flPanX -= (std::sin(radYaw) * event.motion.xrel) * 0.05f * flZoomScale;
                    flPanY += (std::cos(radYaw) * event.motion.xrel) * 0.05f * flZoomScale;
                    flPanZ += event.motion.yrel * 0.05f * flZoomScale;
                }
                break;

            case SDL_MOUSEWHEEL:
                flZoomScale -= event.wheel.y * 0.15f * (flZoomScale * 0.4f);
                if (flZoomScale < 0.1f)
                    flZoomScale = 0.1f;
                if (flZoomScale > 15.0f)
                    flZoomScale = 15.0f;
                break;
            }
        }

        uint32_t currentTicks = SDL_GetTicks();
        float frameTime = (currentTicks - lastTicks) / 1000.0f;
        if (frameTime == 0.0f)
            frameTime = 0.01f;
        lastTicks = currentTicks;

        flAnimCycle += frameTime * 0.4f;
        if (flAnimCycle > 1.0f)
            flAnimCycle -= 1.0f;

        // --- PROCESSING LIVE PERFORMANCE FPS COUNTER CALCULATION ---
        nFrameCount++;
        if (currentTicks - nLastFpsUpdateTicks >= 1000)
        {
            char szTitleBuffer[128];
            sprintf(szTitleBuffer, "Source SDL2 Render Workspace Tool | NATIVE PERFORMANCE: %d FPS", nFrameCount);
            SDL_SetWindowTitle(pWindow, szTitleBuffer);

            nFrameCount = 0;
            nLastFpsUpdateTicks = currentTicks;
        }

        g_pMaterialSystem->BeginFrame(frameTime);

        pRenderContext = g_pMaterialSystem->GetRenderContext();
        if (pRenderContext)
        {
            pRenderContext->ClearColor3ub(51, 51, 51);
            pRenderContext->ClearBuffers(true, true, true);

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

            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->LoadIdentity();

            SDL_GetWindowSize(pWindow, &w, &h);
            double aspect = (h == 0) ? 1.0 : (double)w / (double)h;
            pRenderContext->PerspectiveX(45.0, aspect, 1.0, 2000.0);

            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->LoadIdentity();

            float radPitch = flCameraPitch * (M_PI / 180.0f);
            float radYaw = flCameraYaw * (M_PI / 180.0f);
            float distance = 50.0f * flZoomScale;

            Vector vecEye(
                distance * std::cos(radPitch) * std::cos(radYaw),
                distance * std::cos(radPitch) * std::sin(radYaw),
                distance * std::sin(radPitch));

            Vector vecAt(flPanX, flPanY, flPanZ);
            vecEye += vecAt;
            Vector vecUp(0, 0, 1);

            Vector forward = vecAt - vecEye;
            VectorNormalize(forward);
            Vector left;
            CrossProduct(vecUp, forward, left);
            VectorNormalize(left);
            Vector up;
            CrossProduct(forward, left, up);

            VMatrix matView;
            matView.Init(
                left.x, left.y, left.z, -DotProduct(left, vecEye),
                up.x, up.y, up.z, -DotProduct(up, vecEye),
                -forward.x, -forward.y, -forward.z, DotProduct(forward, vecEye),
                0.0f, 0.0f, 0.0f, 1.0f);
            pRenderContext->LoadMatrix(matView);

            if (hMdl != MDLHANDLE_INVALID)
            {
                studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(hMdl);
                studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(hMdl);

                if (pStudioHdr && pHardwareData)
                {
                    pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->LoadIdentity();

                    g_pStudioRender->BeginFrame();

                    ::StudioRenderConfig_t studioCfg;
                    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                    studioCfg.drawEntities = 1;
                    studioCfg.bSoftwareSkin = false;
                    studioCfg.bSoftwareLighting = false;
                    studioCfg.bNoSoftware = true;

                    g_pStudioRender->UpdateConfig(studioCfg);
                    g_pStudioRender->ForcedMaterialOverride(nullptr);

                    g_pStudioRender->SetAlphaModulation(1.0f);
                    g_pStudioRender->SetColorModulation(Vector(1.0f, 1.0f, 1.0f).Base());

                    DrawModelInfo_t drawInfo;
                    drawInfo.m_pStudioHdr = pStudioHdr;
                    drawInfo.m_pHardwareData = pHardwareData;
                    drawInfo.m_Decals = STUDIORENDER_DECAL_INVALID;
                    drawInfo.m_Skin = 0;
                    drawInfo.m_Body = 0;
                    drawInfo.m_HitboxSet = 0;
                    drawInfo.m_pClientEntity = nullptr;
                    drawInfo.m_Lod = 0;
                    drawInfo.m_pColorMeshes = nullptr;

                    matrix3x4_t poseBones[MAXSTUDIOBONES] = {};
                    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);

                    if (pBoneArray != nullptr)
                    {
                        int numBonesToProcess = (pStudioHdr->numbones < MAXSTUDIOBONES) ? pStudioHdr->numbones : MAXSTUDIOBONES;
                        int nSeqIndex = clamp(nTargetSequenceIndex, 0, pStudioHdr->numlocalseq - 1);

                        for (int i = 0; i < numBonesToProcess; i++)
                        {
                            Vector bonePos = pBoneArray[i].pos;
                            Quaternion boneQuat = pBoneArray[i].quat;

                            const char *pszBoneName = pBoneArray[i].pszName();
                            if (pszBoneName != nullptr && pBoneArray[i].parent != -1)
                            {
                                float timelineFactor = (flAnimCycle * M_PI * 2.0f);

                                if (nSeqIndex > 0)
                                {
                                    if (strstr(pszBoneName, "Forearm") || strstr(pszBoneName, "Hand"))
                                    {
                                        float sequencePoseAngle = (float)nSeqIndex * 0.15f;
                                        boneQuat.x += std::sin(timelineFactor) * 0.1f + (sequencePoseAngle * 0.05f);
                                    }
                                }
                                else
                                {
                                    if (strstr(pszBoneName, "Spine2") || strstr(pszBoneName, "Spine4") || strstr(pszBoneName, "Neck"))
                                    {
                                        float breatheSway = std::sin(timelineFactor) * 0.015f;
                                        boneQuat.x += breatheSway;
                                        boneQuat.y += std::cos(timelineFactor * 0.5f) * 0.008f;
                                    }
                                }
                            }

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

					// --- FIXED: INJECTING DYNAMIC FACIAL EXPRESSION FLEXES ---
					// Swapped out array bounds and methods to match your specific engine fork naming signatures
					float pFlexWeights[MAXSTUDIOFLEXDESC] = { 0.0f };
					float pFlexDelayedWeights[MAXSTUDIOFLEXDESC] = { 0.0f };

					if (pStudioHdr->numflexdesc > 0)
					{
						float timelineFactor = (flAnimCycle * M_PI * 2.0f);
						
						// Iterate through the model's baked shape morph controllers to find standard face tracks
						for (int f = 0; f < pStudioHdr->numflexdesc; f++)
						{
							mstudioflexdesc_t *pFlexDesc = pStudioHdr->pFlexdesc(f);
							if (pFlexDesc && pFlexDesc->pszFACS())
							{
								const char* pszFlexName = pFlexDesc->pszFACS();
								
								// 1. Natural Blinking Loop: Maps an intermittent quick snap closure onto her eyelids
								if (strstr(pszFlexName, "blink") || strstr(pszFlexName, "lid_closer"))
								{
									// Generates a rapid pulse wave to simulate intermittent blinking periods
									float flBlinkPulse = std::sin(timelineFactor * 3.0f);
									pFlexWeights[f] = (flBlinkPulse > 0.7f) ? 1.0f : 0.0f;
								}
								// 2. Subtle Jaw/Mouth expressions shifting with our breath timelines
								else if (strstr(pszFlexName, "jaw_drop") || strstr(pszFlexName, "mouth_open"))
								{
									pFlexWeights[f] = std::abs(std::sin(timelineFactor * 0.5f)) * 0.25f;
								}
								else if (strstr(pszFlexName, "smile") || strstr(pszFlexName, "corner_puller"))
								{
									pFlexWeights[f] = 0.35f; // Constant subtle resting tools smirk expression
								}
							}
						}
					}

					DrawModelResults_t modelResults;
					memset(&modelResults, 0, sizeof(DrawModelResults_t));

					g_pStudioRender->DrawModel(
						&modelResults, 
						drawInfo, 
						poseBones, 
						pFlexWeights,        // Passing our dynamic expression array to the vertex shader
						pFlexDelayedWeights, // Passing required trailing shape modifiers
						Vector(0, 0, 0), 
						STUDIORENDER_DRAW_ENTIRE_MODEL 
					);

                    g_pStudioRender->EndFrame();
                }
            }
            pRenderContext->Flush(true);
        }
        g_pMaterialSystem->EndFrame();
        g_pMaterialSystem->SwapBuffers();
    }
    Msg("[HAMMER] Exiting rendering presentation loop smoothly.\n");
    _exit(0);
    return 0;
}