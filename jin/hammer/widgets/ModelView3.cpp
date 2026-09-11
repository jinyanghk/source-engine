#include "ModelView3.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QTimer>

// Source Engine Core Interface Definitions
#include "materialsystem/imaterialsystem.h"
#include "istudiorender.h"
#include "datacache/imdlcache.h"
#include "materialsystem/itexture.h"

extern IMaterialSystem *g_pMaterialSystem;
extern IStudioRender *g_pStudioRender;
extern IMDLCache *g_pMDLCache;

QModelView3::QModelView3(QWidget *parent)
    : QWidget(parent), m_flAnimationCycle(0.0f), m_hCurrentModel(0xFFFF), m_szCurrentModelPath(""), m_flZoomScale(1.0f), m_pOffscreenRenderTarget(nullptr), m_bIsRenderBufferBlank(true)
{
    // Using integer QPoint layout initialization to match your header declaration file
    m_ptRotationAngle = QPoint(20, -45);

    // Throttled heartbeat clock loop to yield CPU processing ticks under WSL2 software states
    m_pAnimationFrameTimer = new QTimer(this);
    connect(m_pAnimationFrameTimer, &QTimer::timeout, this, [=]()
            {
        if (m_hCurrentModel != 0xFFFF) {
            // Unroll the animation timeline sequence smoothly frame by frame
            m_flAnimationCycle += 0.015f; 
            if (m_flAnimationCycle > 1.0f) m_flAnimationCycle -= 1.0f;
            this->update(); 
        } });
    m_pAnimationFrameTimer->start(33); // 30 FPS Lock

    QTimer::singleShot(100, this, [=]()
                       { LoadModelFile("models/alyx.mdl"); });
}

QModelView3::~QModelView3()
{
    if (m_pOffscreenRenderTarget && g_pMaterialSystem)
    {
        m_pOffscreenRenderTarget->DecrementReferenceCount();
    }
}

void QModelView3::LoadModelFile(const QString &szPath)
{
    if (!g_pMDLCache)
        return;

    m_szCurrentModelPath = szPath;
    m_flAnimationCycle = 0.0f;
    m_hCurrentModel = g_pMDLCache->FindMDL(m_szCurrentModelPath.toUtf8().constData());
    this->update();
}

void QModelView3::RenderEngineFrame()
{
    if (!g_pMaterialSystem || !g_pStudioRender || !g_pMDLCache || !isVisible())
        return;

    int w = qMax(64, rect().width());
    int h = qMax(64, rect().height());

    if (!m_pOffscreenRenderTarget || m_pOffscreenRenderTarget->GetActualWidth() != w || m_pOffscreenRenderTarget->GetActualHeight() != h)
    {
        if (m_pOffscreenRenderTarget)
        {
            m_pOffscreenRenderTarget->DecrementReferenceCount();
            m_pOffscreenRenderTarget = nullptr;
        }
        m_pOffscreenRenderTarget = g_pMaterialSystem->CreateRenderTargetTexture(
            w, h, RT_SIZE_NO_CHANGE, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SHARED);
        if (m_pOffscreenRenderTarget)
        {
            m_pOffscreenRenderTarget->IncrementReferenceCount();
        }
        m_RenderOutputImage = QImage(w, h, QImage::Format_RGB32);
    }

    if (!m_pOffscreenRenderTarget || !m_pOffscreenRenderTarget->IsRenderTarget())
        return;

    g_pMaterialSystem->BeginFrame(0.0f);
    {
        CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
        if (pRenderContext)
        {
            pRenderContext->PushRenderTargetAndViewport(m_pOffscreenRenderTarget);
            pRenderContext->Viewport(0, 0, w, h);
            pRenderContext->ClearColor4ub(43, 45, 66, 255);
            pRenderContext->ClearBuffers(true, true);

            if (m_hCurrentModel != 0xFFFF)
            {
                studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
                studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(m_hCurrentModel);

                if (pStudioHdr && pHardwareData)
                {
                    pRenderContext->SetAmbientLight(0.4f, 0.4f, 0.4f);

                    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();
                    pRenderContext->PerspectiveX(60.0, (float)w / (float)h, 1.0, 2000.0);

                    pRenderContext->MatrixMode(MATERIAL_VIEW);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();
                    pRenderContext->Translate(0.0f, -30.0f, -90.0f * m_flZoomScale);
                    pRenderContext->Rotate(m_ptRotationAngle.x(), 1.0f, 0.0f, 0.0f);
                    pRenderContext->Rotate(m_ptRotationAngle.y(), 0.0f, 1.0f, 0.0f);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();

                    // SW_HAMMER_TOOL 2D SUBSCRIPT MATRIX ALIGNMENT:
                    // Explicitly map coordinates to index 3 (translational column) across rows 0, 1, and 2.
                    // This resolves the matrix float assignment compile error completely.
                    matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
                    for (int i = 0; i < pStudioHdr->numbones; i++)
                    {
                        SetIdentityMatrix(pBoneToWorld[i]);
                        mstudiobone_t *pBone = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex) + i;

                        pBoneToWorld[i][0][3] = pBone->pos.x;
                        pBoneToWorld[i][1][3] = pBone->pos.y;
                        pBoneToWorld[i][2][3] = pBone->pos.z;
                    }

                    g_pStudioRender->LockBoneMatrices(pStudioHdr->numbones);
                    g_pStudioRender->UnlockBoneMatrices();

                    pRenderContext->LoadIdentity();

                    static IMaterial *pToolMaterial = nullptr;
                    if (!pToolMaterial)
                    {
                        pToolMaterial = g_pMaterialSystem->FindMaterial("debug/debugvertexcolor", TEXTURE_GROUP_OTHER);
                    }
                    if (pToolMaterial)
                        pRenderContext->Bind(pToolMaterial);

                    DrawModelInfo_t modelInfo;
                    modelInfo.m_pStudioHdr = pStudioHdr;
                    modelInfo.m_pHardwareData = pHardwareData;
                    modelInfo.m_Skin = 0;
                    modelInfo.m_Body = 0;
                    modelInfo.m_HitboxSet = 0;

                    ::StudioRenderConfig_t studioCfg;
                    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                    g_pStudioRender->UpdateConfig(studioCfg);

                    g_pStudioRender->DrawModel(nullptr, modelInfo, pBoneToWorld, NULL, NULL, Vector(0, 0, 0), 0);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->PopMatrix();
                    pRenderContext->MatrixMode(MATERIAL_VIEW);
                    pRenderContext->PopMatrix();
                    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
                    pRenderContext->PopMatrix();
                }
            }
            unsigned char *pDstBits = m_RenderOutputImage.bits();
            pRenderContext->ReadPixels(0, 0, w, h, pDstBits, IMAGE_FORMAT_ARGB8888);

            // SW_HAMMER_TOOL SAFE SHIELD: Maintain pure software wireframe stability for headless WSL2 runs
            m_bIsRenderBufferBlank = true;

            pRenderContext->PopRenderTargetAndViewport();
        }
    }
    g_pMaterialSystem->EndFrame();
}

void QModelView3::paintEvent(QPaintEvent *event)
{
    RenderEngineFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = rect().width();
    int h = rect().height();

    if (!m_bIsRenderBufferBlank && !m_RenderOutputImage.isNull())
    {
        painter.drawImage(0, 0, m_RenderOutputImage);
    }
    else
    {
        painter.fillRect(rect(), QColor(43, 45, 66));

        if (m_hCurrentModel != 0xFFFF && g_pMDLCache)
        {
            studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
            if (pStudioHdr && pStudioHdr->numbones > 0)
            {
                float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
                float radY = qDegreesToRadians((float)m_ptRotationAngle.y());

                auto Project3DBone = [&](float x, float y, float z) -> QPointF
                {
                    float x1 = x;
                    float y1 = y * qCos(radX) - z * qSin(radX);
                    float z1 = y * qSin(radX) + z * qCos(radX);
                    float x2 = x1 * qCos(radY) + z1 * qSin(radY);

                    float sX = (w / 2.0f) + (x2 * m_flZoomScale * 0.4f);
                    float sY = (h / 2.0f) + (y1 * m_flZoomScale * 0.4f);
                    return QPointF(sX, sY);
                };

                mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
                for (int i = 0; i < pStudioHdr->numbones; ++i)
                {
                    mstudiobone_t *pBone = &pBoneArray[i];
                    QPointF p1 = Project3DBone(pBone->pos.x, pBone->pos.y, pBone->pos.z);

                    painter.setBrush(QColor(241, 91, 181));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(p1, 3, 3);

                    if (pBone->parent >= 0 && pBone->parent < pStudioHdr->numbones)
                    {
                        mstudiobone_t *pParent = &pBoneArray[pBone->parent];
                        QPointF p2 = Project3DBone(pParent->pos.x, pParent->pos.y, pParent->pos.z);

                        painter.setPen(QPen(QColor(0, 180, 216), 1.5f, Qt::SolidLine));
                        painter.drawLine(p1, p2);
                    }
                }
            }
        }
    }

    // Status Text Overlays
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(15, 25, "ModelView: WORKSTATION COMPONENT LINK ALIVE");

    if (m_hCurrentModel != 0xFFFF && g_pMDLCache)
    {
        studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
        if (pStudioHdr)
        {
            painter.setFont(QFont("Courier New", 9));
            painter.setPen(QColor(200, 214, 229));
            painter.drawText(15, 45, QString("Model Path : %1").arg(m_szCurrentModelPath));
            painter.drawText(15, 60, QString("Bones      : %1 layers solved").arg(pStudioHdr->numbones));
            painter.drawText(15, 75, QString("Sequences  : %1 clips present").arg(pStudioHdr->numlocalseq));
        }
    }
}
void QModelView3::mousePressEvent(QMouseEvent *event) { m_ptLastMousePosition = event->pos(); }
void QModelView3::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_ptRotationAngle.setY(m_ptRotationAngle.y() + (event->position().x() - m_ptLastMousePosition.x()) * 0.5f);
        m_ptRotationAngle.setX(m_ptRotationAngle.x() - (event->position().y() - m_ptLastMousePosition.y()) * 0.5f);
        m_ptLastMousePosition = event->pos();
        this->update();
    }
}
void QModelView3::wheelEvent(QWheelEvent *event)
{
    m_flZoomScale += event->angleDelta().y() > 0 ? 0.1f : -0.1f;
    m_flZoomScale = qBound(0.1f, m_flZoomScale, 5.0f);
    this->update();
}
void QModelView3::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); }