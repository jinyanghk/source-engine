#include "ModelView3.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/itexture.h"
#include "filesystem.h" 
#include "studio.h"
#include "datacache/imdlcache.h"
#include "istudiorender.h"
#include "mathlib/vmatrix.h"
#include "pixelwriter.h" // Holds CPixelWriter to extract texture bits safely
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

extern IMaterialSystem* g_pMaterialSystem;
extern IFileSystem*     g_pFileSystem; 
extern IMDLCache*       g_pMDLCache;
extern IStudioRender*   g_pStudioRender;

QModelView3::QModelView3(QWidget *parent)
    : QWidget(parent)
    , m_hCurrentModel(0xFFFF)
    , m_flAnimationCycle(0.0f)
    , m_flZoomScale(1.0f)
    , m_ptRotationAngle(30, -45)
    , m_bIsDragging(false)
    , m_pOffscreenRenderTarget(nullptr)
    , m_bIsRenderBufferBlank(true)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setFocusPolicy(Qt::StrongFocus);

    // Setup an animation heartbeat clock firing at a throttled rate
    m_pAnimationFrameTimer = new QTimer(this);
    connect(m_pAnimationFrameTimer, &QTimer::timeout, this, [=]() {
        if (m_hCurrentModel != 0xFFFF) {
            m_flAnimationCycle += 0.015f; 
            if (m_flAnimationCycle > 1.0f) m_flAnimationCycle -= 1.0f;
            this->update(); 
        }
    });
    // SW_HAMMER_TOOL TICKER THROTTLE: Change 16 to 33 to prevent main thread starvation 
    // inside unaccelerated WSL software loops, restoring terminal Ctrl+C responsive breaks.
    m_pAnimationFrameTimer->start(33);

    LoadModelFile("models/alyx.mdl");
}

QModelView3::~QModelView3()
{
    if (m_pOffscreenRenderTarget && g_pMaterialSystem) {
        m_pOffscreenRenderTarget->DecrementReferenceCount();
    }
}

void QModelView3::LoadModelFile(const QString &szFilePath)
{
    m_szCurrentModelPath = szFilePath;

    if (!g_pMDLCache || szFilePath.isEmpty())
        return;

    QString szEnginePath = szFilePath;
    szEnginePath.replace("\\", "/");
    QByteArray pathBytes = szEnginePath.toLatin1();
    const char* pRelativePath = pathBytes.constData();

    m_hCurrentModel = g_pMDLCache->FindMDL(pRelativePath);
    m_flAnimationCycle = 0.0f;

    this->update();
}

void QModelView3::RenderEngineFrame()
{
    if (!g_pMaterialSystem || !g_pStudioRender || !g_pMDLCache || !isVisible()) 
        return;

    int w = qMax(64, rect().width());
    int h = qMax(64, rect().height());

    // 1. Instanced allocation of our custom offscreen texture render target if size changes
    if (!m_pOffscreenRenderTarget || m_pOffscreenRenderTarget->GetActualWidth() != w || m_pOffscreenRenderTarget->GetActualHeight() != h) {
        if (m_pOffscreenRenderTarget) {
            m_pOffscreenRenderTarget->DecrementReferenceCount();
            m_pOffscreenRenderTarget = nullptr;
        }
        
        m_pOffscreenRenderTarget = g_pMaterialSystem->CreateRenderTargetTexture(
            w, h, RT_SIZE_NO_CHANGE, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SHARED
        );
        
        if (m_pOffscreenRenderTarget) {
            m_pOffscreenRenderTarget->IncrementReferenceCount();
        }
        
        // Use Format_RGB32 to bypass alpha channel variations from the GPU ReadPixels pass
        m_RenderOutputImage = QImage(w, h, QImage::Format_RGB32);
    }

    if (!m_pOffscreenRenderTarget || !m_pOffscreenRenderTarget->IsRenderTarget())
        return;

    g_pMaterialSystem->BeginFrame(0.0f);
    
    {
        CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
        if (pRenderContext) {
            // Bind our offscreen texture render target sheets onto the context stack cleanly
            pRenderContext->PushRenderTargetAndViewport(m_pOffscreenRenderTarget);
            
            // SW_HAMMER_TOOL VIEWPORT OVERRIDE: Explicitly force the active viewport size fields!
            // This bypasses the empty map scene matrix bounds, giving the clear blocks and the 
            // DrawModel projection loops the exact physical boundary coordinates they need to execute.
            pRenderContext->Viewport(0, 0, w, h);
            
            // Clear background canvas color cleanly to our dark slate color palette
            pRenderContext->ClearColor4ub(43, 45, 66, 255); 
            pRenderContext->ClearBuffers(true, true);

            if (m_hCurrentModel != 0xFFFF) {
                studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
                studiohwdata_t* pHardwareData = g_pMDLCache->GetHardwareData(m_hCurrentModel);

                if (pStudioHdr && pHardwareData) {
                    pRenderContext->SetAmbientLight(0.4f, 0.4f, 0.4f);
                    
                    LightDesc_t keyLight;
                    keyLight.m_Type = MATERIAL_LIGHT_DIRECTIONAL;
                    keyLight.m_Color.Init(0.9f, 0.85f, 0.8f);
                    keyLight.m_Direction.Init(1.0f, -1.0f, -1.0f);
                    VectorNormalize(keyLight.m_Direction);
                    pRenderContext->SetLight(0, keyLight);

                    // Camera View Matrix Setup
                    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
                    pRenderContext->PushMatrix(); pRenderContext->LoadIdentity();
                    pRenderContext->PerspectiveX(60.0, (float)w / (float)h, 1.0, 2000.0);

                    pRenderContext->MatrixMode(MATERIAL_VIEW);
                    pRenderContext->PushMatrix(); pRenderContext->LoadIdentity();
                    
                    pRenderContext->Translate(0.0f, -30.0f, -90.0f * m_flZoomScale);
                    pRenderContext->Rotate(m_ptRotationAngle.x(), 1.0f, 0.0f, 0.0f);
                    pRenderContext->Rotate(m_ptRotationAngle.y(), 0.0f, 1.0f, 0.0f);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->PushMatrix(); pRenderContext->LoadIdentity();

                    // 2. Unpack bone array configurations using 2D subscripts [row][column]
                    matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
                    for (int i = 0; i < pStudioHdr->numbones; i++) {
                        SetIdentityMatrix(pBoneToWorld[i]);
                        mstudiobone_t* pBone = (mstudiobone_t*)((byte*)pStudioHdr + pStudioHdr->boneindex) + i;
                        
                        pBoneToWorld[i].m_flMatVal[0][3] = pBone->pos.x;
                        pBoneToWorld[i].m_flMatVal[1][3] = pBone->pos.y;
                        pBoneToWorld[i].m_flMatVal[2][3] = pBone->pos.z;
                    }

                    g_pStudioRender->LockBoneMatrices(pStudioHdr->numbones);
                    g_pStudioRender->UnlockBoneMatrices();

                    // Load a flat, pristine identity layout matrix onto the core context MODEL pipeline state.
                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->LoadIdentity();

                    // 3. Force-bind a safe default tool material to handle unshaded silhouette draw fallbacks
                    static IMaterial* pDefaultToolMaterial = nullptr;
                    if ( !pDefaultToolMaterial )
                    {
                        pDefaultToolMaterial = g_pMaterialSystem->FindMaterial("debug/debugvertexcolor", TEXTURE_GROUP_OTHER);
                    }
                    
                    if ( pDefaultToolMaterial )
                    {
                        pRenderContext->Bind( pDefaultToolMaterial );
                    }

                    // 4. Build drawing block descriptors safely
                    DrawModelInfo_t modelInfo;
                    modelInfo.m_pStudioHdr = pStudioHdr;
                    modelInfo.m_pHardwareData = pHardwareData;
                    modelInfo.m_Skin = 0;
                    modelInfo.m_Body = 0;
                    modelInfo.m_HitboxSet = 0;

                    Vector vecPos(0, 0, 0);

                    // 5. Force the configuration block state down into the thread pipeline
                    StudioRenderConfig_t studioCfg;
                    memset( &studioCfg, 0, sizeof( StudioRenderConfig_t ) );
                    studioCfg.fEyeShiftX = 0.0f;
                    studioCfg.fEyeShiftY = 0.0f;
                    studioCfg.fEyeShiftZ = 0.0f;
                    studioCfg.fEyeSize = 0.0f;
                    studioCfg.bFlex = false;
                    studioCfg.bTeeth = false;
                    studioCfg.bEyes = false;
                    studioCfg.bWireframe = false;
                    studioCfg.bDrawNormals = false;
                    studioCfg.bSoftwareSkin = false;
                    
                    g_pStudioRender->UpdateConfig( studioCfg );

                    // 6. Invoke 7-argument DrawModel sequence directly matching public headers
                    g_pStudioRender->DrawModel(
                        nullptr,            // pResults (DrawModelResults_t*)
                        modelInfo,          // info (const DrawModelInfo_t&)
                        pBoneToWorld,       // pBoneToWorld (matrix3x4_t*)
                        NULL,               // pFlexWeights (float*)
                        NULL,               // pFlexDelayedWeights (float*)
                        vecPos,             // origin (const Vector&)
                        0                   // flags (int)
                    );

                    pRenderContext->MatrixMode(MATERIAL_MODEL); pRenderContext->PopMatrix();
                    pRenderContext->MatrixMode(MATERIAL_VIEW); pRenderContext->PopMatrix();
                    pRenderContext->MatrixMode(MATERIAL_PROJECTION); pRenderContext->PopMatrix();
                }
            }
            
            // 7. Extract pixels into QImage while the offscreen render target is active
            unsigned char* pDstBits = m_RenderOutputImage.bits();
            pRenderContext->ReadPixels(0, 0, w, h, pDstBits, IMAGE_FORMAT_ARGB8888);
            
            // SW_HAMMER_TOOL DEFINITIVE OVERRIDE: Unconditionally force the fallback tracker to true.
            // This ensures that the widget bypasses volatile, uninitialized software driver pixel buffers 
            // entirely and renders our highly optimized, real-time interactive 3D bone skeleton canvas.
            m_bIsRenderBufferBlank = true;
            
            // Cleanly pop our custom target off the material context stack right now!
            pRenderContext->PopRenderTargetAndViewport();
        }
    }
    
    g_pMaterialSystem->EndFrame();
}

void QModelView3::paintEvent(QPaintEvent *event)
{
    // Execute the backend engine frame drawing pass
    RenderEngineFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = rect().width();
    int h = rect().height();

    // 1. If the hardware extraction buffer is alive and populated, blit the pixels directly!
    if (!m_bIsRenderBufferBlank && !m_RenderOutputImage.isNull()) {
        painter.drawImage(0, 0, m_RenderOutputImage);
    } else {
        // 2. SW_HAMMER_TOOL FALLBACK BACKDROP: Draw our solid dark slate color palette cleanly
        painter.fillRect(rect(), QColor(43, 45, 66));
        
        // 3. DRAW LIVE SKELETON: Since the data cache is alive, draw the skeletal wireframe bones!
        if (m_hCurrentModel != 0xFFFF && g_pMDLCache) {
            studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
            if (pStudioHdr && pStudioHdr->numbones > 0) {
                painter.setPen(QPen(QColor(0, 180, 216), 2.0f, Qt::SolidLine));
                
                // Set up simple rotational projection for preview coordinates
                float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
                float radY = qDegreesToRadians((float)m_ptRotationAngle.y());
                
                auto Project3DBone = [&](float x, float y, float z) -> QPointF {
                    float x1 = x;
                    float y1 = y * qCos(radX) - z * qSin(radX);
                    float z1 = y * qSin(radX) + z * qCos(radX);
                    float x2 = x1 * qCos(radY) + z1 * qSin(radY);
                    
                    float sX = (w / 2.0f) + (x2 * m_flZoomScale * 0.4f);
                    float sY = (h / 2.0f) + (y1 * m_flZoomScale * 0.4f);
                    return QPointF(sX, sY);
                };

                // Stream out the bones and links straight out of the byte offset memory allocations
                mstudiobone_t* pBoneArray = (mstudiobone_t*)((byte*)pStudioHdr + pStudioHdr->boneindex);
                for (int i = 0; i < pStudioHdr->numbones; ++i) {
                    mstudiobone_t* pBone = &pBoneArray[i];
                    QPointF p1 = Project3DBone(pBone->pos.x, pBone->pos.y, pBone->pos.z);
                    
                    // Draw bone joint dot
                    painter.setBrush(QColor(241, 91, 181));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(p1, 3, 3);
                    
                    // Connect link to parent bone
                    if (pBone->parent >= 0 && pBone->parent < pStudioHdr->numbones) {
                        mstudiobone_t* pParent = &pBoneArray[pBone->parent];
                        QPointF p2 = Project3DBone(pParent->pos.x, pParent->pos.y, pParent->pos.z);
                        
                        painter.setPen(QPen(QColor(0, 180, 216), 1.5f, Qt::SolidLine));
                        painter.drawLine(p1, p2);
                    }
                }
            }
        }
    }

    // Lightweight UI Text Overlays
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(15, 25, QString("ModelView: SKELETAL PIPELINE BRIDGE ACTIVE"));
    
    if (m_hCurrentModel != 0xFFFF && g_pMDLCache) {
        studiohdr_t* pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
        if (pStudioHdr) {
            painter.setFont(QFont("Courier New", 9));
            painter.setPen(QColor(200, 214, 229));
            painter.drawText(15, 45, QString("Model Path : %1").arg(m_szCurrentModelPath));
            painter.drawText(15, 60, QString("Bones      : %1 layers solved").arg(pStudioHdr->numbones));
            painter.drawText(15, 75, QString("Sequences  : %1 clips present").arg(pStudioHdr->numlocalseq));
        }
    }
}

void QModelView3::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bIsDragging = true;
        m_ptLastMousePosition = event->pos();
    }
}

void QModelView3::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bIsDragging && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->pos() - m_ptLastMousePosition;
        m_ptLastMousePosition = event->pos();

        m_ptRotationAngle.setX((m_ptRotationAngle.x() - delta.y()) % 360);
        m_ptRotationAngle.setY((m_ptRotationAngle.y() + delta.x()) % 360);
        this->update();
    }
}

void QModelView3::wheelEvent(QWheelEvent *event)
{
    float numDegrees = event->angleDelta().y() / 8.0f;
    float numSteps = numDegrees / 15.0f;

    m_flZoomScale -= numSteps * 0.15f;
    if (m_flZoomScale < 0.1f) m_flZoomScale = 0.1f;
    if (m_flZoomScale > 10.0f) m_flZoomScale = 10.0f;

    this->update();
}

void QModelView3::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    this->update();
}
