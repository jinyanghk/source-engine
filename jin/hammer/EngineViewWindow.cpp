#include "EngineViewWindow.h"

// 1. INCLUDE THE MISSING CONCRETE STRUCT DATA DEFINITIONS
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h" 
#include "istudiorender.h"
#include "datacache/imdlcache.h"
#include <QDebug>

extern IMaterialSystem *g_pMaterialSystem;
extern IStudioRender *g_pStudioRender;
extern IMDLCache *g_pMDLCache;
extern MDLHandle_t g_hActiveHammerModel; 

EngineViewWindow::EngineViewWindow(QWindow *parent)
    : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);
    create();

    if (g_pMaterialSystem)
    {
        void* nativeWinHandle = reinterpret_cast<void*>(this->winId());
        g_pMaterialSystem->AddView(nativeWinHandle);

        // 2. CORRECT METHOD SIGNATURE FOR DISPLAY MODE EVALUATION
        MaterialVideoMode_t currentVideoMode;
        g_pMaterialSystem->GetDisplayMode(currentVideoMode);

        // Create a temporary local config instance populated with safe defaults
        MaterialSystem_Config_t matSysCfg;
        memset(&matSysCfg, 0, sizeof(MaterialSystem_Config_t));
        matSysCfg.m_VideoMode = currentVideoMode;
        matSysCfg.m_Flags = 0;

        // Force initialize layout device buffers on this exact window pointer handle
        g_pMaterialSystem->SetMode(nativeWinHandle, matSysCfg);
        qDebug() << "[StrataEngine] AddView and SetMode assigned to Window ID:" << this->winId();
    }
}

EngineViewWindow::~EngineViewWindow()
{
    if (g_pMaterialSystem)
    {
        g_pMaterialSystem->RemoveView(reinterpret_cast<void*>(this->winId()));
    }
}

void EngineViewWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);
    if (isExposed()) {
        RenderFrame();
    }
}

void EngineViewWindow::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

// 3. CORRECTED NAMESPACE BINDING ERROR (EngineViewWindow instead of MainWindow)
void EngineViewWindow::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

void EngineViewWindow::RenderFrame()
{
    if (!g_pMaterialSystem || !g_pStudioRender || !g_pMDLCache) return;

    void* nativeWinHandle = reinterpret_cast<void*>(this->winId());
    g_pMaterialSystem->SetView(nativeWinHandle);

    g_pMaterialSystem->BeginFrame(0.016f);
    {
        CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
        if (pRenderContext)
        {
            int w = this->width();
            int h = this->height();

            pRenderContext->Viewport(0, 0, w, h);
            pRenderContext->ClearColor4ub(160, 165, 180, 255); // Slate gray backdrop canvas
            pRenderContext->ClearBuffers(true, true, true);
            pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);

            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->PushMatrix();
            pRenderContext->LoadIdentity();
            pRenderContext->PerspectiveX(65.0, (float)w / (float)h, 1.0, 1000.0);

            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->PushMatrix();
            pRenderContext->LoadIdentity();
            pRenderContext->Translate(0.0f, -25.0f, -65.0f); 

            pRenderContext->MatrixMode(MATERIAL_MODEL);
            pRenderContext->PushMatrix();
            pRenderContext->LoadIdentity();

            if (g_hActiveHammerModel != 0xFFFF)
            {
                // 1. 🔥 THE SHIELD: INTERCEPT MATERIAL LOADING IMMEDIATELY! 🔥
                // We fetch a primitive debug shader that does not use light states.
                IMaterial *pDebugMat = g_pMaterialSystem->FindMaterial("debug/debugvertexcolor", TEXTURE_GROUP_OTHER);
                if (pDebugMat) 
                {
                    // Forcing this override HERE tells LoadMaterials() to discard complex 
                    // shader compilation, bypassing skin_dx9_helper entirely during asset setup!
                    g_pStudioRender->ForcedMaterialOverride(pDebugMat);
                }

                // 2. NOW SAFELY FETCH HEADERS WITHOUT CAUSING INITIALIZATION CRASHES
                studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(g_hActiveHammerModel);
                studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(g_hActiveHammerModel);

                if (pStudioHdr && pHardwareData)
                {
                    DrawModelInfo_t modelInfo;
                    modelInfo.m_pStudioHdr = pStudioHdr;
                    modelInfo.m_pHardwareData = pHardwareData;
                    modelInfo.m_Skin = 0;
                    modelInfo.m_Body = 0;
                    modelInfo.m_HitboxSet = 0;

                    matrix3x4_t identityBones[MAXSTUDIOBONES];
                    for (int i = 0; i != MAXSTUDIOBONES; ++i) {
                        SetIdentityMatrix(identityBones[i]);
                    }

                    g_pStudioRender->SetLocalLights(0, nullptr);

                    ::StudioRenderConfig_t studioCfg;
                    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                    studioCfg.drawEntities = 1;
                    studioCfg.bSoftwareLighting = false;
                    studioCfg.bWireframe = true; // Maintain wireframe for structural stability
                    g_pStudioRender->UpdateConfig(studioCfg);

                    // 3. EXECUTE SAFE HARMONIC DRAW CALL
                    g_pStudioRender->DrawModel(nullptr, modelInfo, identityBones, NULL, NULL, Vector(0, 0, 0), 0);
                }

                // 4. Safely release override state for subsequent frame evaluations
                g_pStudioRender->ForcedMaterialOverride(nullptr);
            }

            pRenderContext->MatrixMode(MATERIAL_MODEL);
            pRenderContext->PopMatrix();
            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->PopMatrix();
            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->PopMatrix();
        }
    }
    g_pMaterialSystem->EndFrame();
    g_pMaterialSystem->Flush(true);
}
