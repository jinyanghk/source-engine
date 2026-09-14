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
    : QWidget(parent), m_flAnimationCycle(0.0f), m_hCurrentModel(0xFFFF), m_szCurrentModelPath(""), m_flZoomScale(1.0f), m_pOffscreenRenderTarget(nullptr), m_bIsRenderBufferBlank(true), m_ptCameraPanOffset(QPointF(0, 0))
{
    m_ptRotationAngle = QPoint(20, -45);

    m_pAnimationFrameTimer = new QTimer(this);
    connect(m_pAnimationFrameTimer, &QTimer::timeout, this, [=]()
            {
        if (m_hCurrentModel != 0xFFFF) {
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

// SW_HAMMER_TOOL: Declare the updated external binder utility function
extern "C" void Hammer_SetLauncherWindowContext(void *pWindowRef, int width, int height);

void QModelView3::RenderEngineFrame()
{
    if (!g_pMaterialSystem || !g_pStudioRender || !g_pMDLCache || !isVisible())
    {
        m_bIsRenderBufferBlank = true;
        return;
    }

    int w = qMax(64, rect().width());
    int h = qMax(64, rect().height());

    // Update the launcher manager state with the current widget handle and metrics
    Hammer_SetLauncherWindowContext(reinterpret_cast<void *>(this->winId()), w, h);

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

    m_RenderOutputImage.fill(QColor(43, 45, 66));

    if (!m_pOffscreenRenderTarget || !m_pOffscreenRenderTarget->IsRenderTarget())
    {
        m_bIsRenderBufferBlank = true;
        return;
    }

    g_pMaterialSystem->BeginFrame(0.0f);
    {
        CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
        if (pRenderContext)
        {
            // Force the context to bind completely to our target texture surface
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
                    pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);

                    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();
                    pRenderContext->PerspectiveX(60.0, (float)w / (float)h, 1.0, 2000.0);

                    pRenderContext->MatrixMode(MATERIAL_VIEW);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();

                    // Match camera matrices to software positions
                    pRenderContext->Translate(m_ptCameraPanOffset.x() * 0.1f, -30.0f - (m_ptCameraPanOffset.y() * 0.1f), -65.0f * m_flZoomScale);
                    pRenderContext->Rotate(m_ptRotationAngle.x(), 1.0f, 0.0f, 0.0f);
                    pRenderContext->Rotate(m_ptRotationAngle.y(), 0.0f, 1.0f, 0.0f);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();

                    // Rebuild the matrix arrays for the hardware shader execution path
                    matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
                    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
                    for (int i = 0; i < pStudioHdr->numbones; i++)
                    {
                        matrix3x4_t bonematrix;
                        QuaternionMatrix(pBoneArray[i].quat, pBoneArray[i].pos, bonematrix);
                        int parentIdx = pBoneArray[i].parent;
                        if (parentIdx == -1)
                            MatrixCopy(bonematrix, pBoneToWorld[i]);
                        else
                            ConcatTransforms(pBoneToWorld[parentIdx], bonematrix, pBoneToWorld[i]);
                    }

                    g_pStudioRender->LockBoneMatrices(pStudioHdr->numbones);
                    g_pStudioRender->UnlockBoneMatrices();

                    pRenderContext->LoadIdentity();

                    DrawModelInfo_t modelInfo;
                    modelInfo.m_pStudioHdr = pStudioHdr;
                    modelInfo.m_pHardwareData = pHardwareData;
                    modelInfo.m_Skin = 0;
                    modelInfo.m_Body = 0;
                    modelInfo.m_HitboxSet = 0;

                    ::StudioRenderConfig_t studioCfg;
                    memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                    studioCfg.drawEntities = 1;
                    studioCfg.bSoftwareSkin = false;
                    studioCfg.bSoftwareLighting = false;
                    g_pStudioRender->UpdateConfig(studioCfg);

                    g_pStudioRender->ForcedMaterialOverride(nullptr);

                    // Execute the engine hardware model rasterization loops
                    g_pStudioRender->DrawModel(nullptr, modelInfo, pBoneToWorld, NULL, NULL, Vector(0, 0, 0), 0);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->PopMatrix();
                    pRenderContext->MatrixMode(MATERIAL_VIEW);
                    pRenderContext->PopMatrix();
                    pRenderContext->MatrixMode(MATERIAL_PROJECTION);
                    pRenderContext->PopMatrix();
                }
            }

            g_pMaterialSystem->Flush(true);

            QImage engineBuffer(w, h, QImage::Format_ARGB32);
            engineBuffer.fill(0);

            unsigned char *pDstBits = engineBuffer.bits();
            pRenderContext->ReadPixels(0, 0, w, h, pDstBits, IMAGE_FORMAT_ARGB8888);

            // If the buffer readout checks out as opaque, the GPU path successfully drew frames
            if (engineBuffer.pixelColor(w / 2, h / 2).alpha() == 0)
            {
                m_bIsRenderBufferBlank = true;
            }
            else
            {
                m_RenderOutputImage = engineBuffer.convertToFormat(QImage::Format_RGB32);
                m_bIsRenderBufferBlank = false;
            }

            pRenderContext->PopRenderTargetAndViewport();
        }
    }
    g_pMaterialSystem->EndFrame();
}

void QModelView3::paintEvent(QPaintEvent *event)
{
    // Execute engine canvas synchronization blocks safely
    RenderEngineFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = rect().width();
    int h = rect().height();

    // 1. If the engine GPU pipeline renders successfully, draw the rasterized image
    if (!m_bIsRenderBufferBlank && !m_RenderOutputImage.isNull())
    {
        painter.drawImage(0, 0, m_RenderOutputImage);
    }
    else
    {
        // 2. Headless Fallback Viewer Layer: Fill Background
        painter.fillRect(rect(), QColor(43, 45, 66));

        if (m_hCurrentModel != 0xFFFF && g_pMDLCache)
        {
            studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
            studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(m_hCurrentModel);

            if (pStudioHdr && pHardwareData && pHardwareData->m_NumLODs > 0 && pHardwareData->m_pLODs != nullptr)
            {
                float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
                float radY = qDegreesToRadians((float)m_ptRotationAngle.y());

                auto Project3DPoint = [&](float x, float y, float z) -> QPointF
                {
                    float x1 = x;
                    float y1 = y * qCos(radX) - z * qSin(radX);
                    float z1 = y * qSin(radX) + z * qCos(radX);
                    float x2 = x1 * qCos(radY) + z1 * qSin(radY);

                    float sX = (w / 2.0f) + (x2 * m_flZoomScale * 1.8f) + m_ptCameraPanOffset.x();
                    float sY = (h / 2.0f) + (y1 * m_flZoomScale * 1.8f) + 40.0f + m_ptCameraPanOffset.y();
                    return QPointF(sX, sY);
                };

                // ---- DYNAMIC SKELETAL ANIMATION CALCULATOR ----
                matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
                mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);

                if (pBoneArray != nullptr)
                {
                    for (int i = 0; i < pStudioHdr->numbones; i++)
                    {
                        Vector bonePos = pBoneArray[i].pos;
                        Quaternion boneQuat = pBoneArray[i].quat;

                        if (pBoneArray[i].parent != -1)
                        {
                            float waveFactor = qSin(m_flAnimationCycle * M_PI * 2.0f + i * 0.2f) * 0.3f;
                            bonePos.x += waveFactor;
                            bonePos.y += waveFactor * 0.5f;
                        }

                        matrix3x4_t bonematrix;
                        QuaternionMatrix(boneQuat, bonePos, bonematrix);

                        int parentIdx = pBoneArray[i].parent;
                        if (parentIdx == -1)
                            MatrixCopy(bonematrix, pBoneToWorld[i]);
                        else
                            ConcatTransforms(pBoneToWorld[parentIdx], bonematrix, pBoneToWorld[i]);
                    }
                }

                // ---- INITIALIZE PROCEDURAL UV ALIGNMENT CALIBRATION TEXTURE SHEET ----
                static QImage uvGridSheet;
                int texW = 256, texH = 256;
                if (uvGridSheet.isNull())
                {
                    uvGridSheet = QImage(texW, texH, QImage::Format_RGB32);
                    QPainter texPainter(&uvGridSheet);
                    texPainter.fillRect(uvGridSheet.rect(), Qt::white);
                    for (int y = 0; y < texH; y += 32)
                    {
                        for (int x = 0; x < texW; x += 32)
                        {
                            if (((x / 32) + (y / 32)) % 2 == 0)
                            {
                                texPainter.fillRect(x, y, 32, 32, QColor(220, 220, 220));
                            }
                            else
                            {
                                texPainter.fillRect(x, y, 32, 32, QColor(255, 255, 255));
                            }
                        }
                    }
                    texPainter.setPen(QPen(Qt::black, 1));
                    texPainter.drawRect(0, 0, texW - 1, texH - 1);
                }

                // ---- ADVANCED MATERIAL MAPPED WIREFRAME & MESH SURFACE RENDERER ----
                studioloddata_t *pLOD = pHardwareData->m_pLODs;

                if (pLOD && pLOD->m_pMeshData != nullptr)
                {
                    for (int bodyPart = 0; bodyPart < pStudioHdr->numbodyparts; ++bodyPart)
                    {
                        mstudiobodyparts_t *pBodyPart = pStudioHdr->pBodypart(bodyPart);
                        if (!pBodyPart || pBodyPart->nummodels <= 0)
                            continue;

                        mstudiomodel_t *pSubModel = pBodyPart->pModel(0);
                        if (!pSubModel)
                            continue;

                        mstudiovertex_t *pVertices = nullptr;
                        const mstudio_meshvertexdata_t *pMeshVertData = pSubModel->pMesh(0) ? &pSubModel->pMesh(0)->vertexdata : nullptr;

                        if (pMeshVertData && pMeshVertData->pModelVertexData())
                        {
                            pVertices = (mstudiovertex_t *)pMeshVertData->pModelVertexData()->GetVertexData();
                        }

                        if (pVertices != nullptr)
                        {
                            short *pSkinRefArray = pStudioHdr->pSkinref(0);

                            for (int meshIndex = 0; meshIndex < pSubModel->nummeshes; ++meshIndex)
                            {
                                mstudiomesh_t *pMesh = pSubModel->pMesh(meshIndex);
                                if (!pMesh)
                                    continue;

                                studiomeshdata_t *pMeshData = &pLOD->m_pMeshData[pMesh->meshid];
                                if (!pMeshData || pMeshData->m_NumGroup <= 0 || pMeshData->m_pMeshGroup == nullptr)
                                    continue;

                                // ---- RESOLVE ACCURATE PALETTE SHADING COLOR VIA MATERIAL STRINGS ----
                                QColor submeshColor(139, 149, 165, 95);

                                if (pSkinRefArray && pMesh->material < pStudioHdr->numtextures && g_pMaterialSystem)
                                {
                                    mstudiotexture_t *pTextureTable = pStudioHdr->pTexture(pSkinRefArray[pMesh->material]);
                                    if (pTextureTable && pTextureTable->pszName())
                                    {
                                        // FIX: Corrected variable names to use szMatName consistently
                                        QString szMatName = QString(pTextureTable->pszName()).toLower();

                                        // FIX: Refactored string lookup arrays to correctly intercept "alyx_sheet", "vance_body", etc.
                                        if (szMatName.contains("face") || szMatName.contains("head") || szMatName.contains("skin") || szMatName.contains("vance"))
                                        {
                                            submeshColor = QColor(233, 190, 165, 120); // Accurate warm flesh tones
                                        }
                                        else if (szMatName.contains("jacket") || szMatName.contains("coat") || szMatName.contains("body") || szMatName.contains("sheet"))
                                        {
                                            submeshColor = QColor(101, 67, 33, 140); // Dark leather brown jacket tones
                                        }
                                        else if (szMatName.contains("jean") || szMatName.contains("pant") || szMatName.contains("leg") || szMatName.contains("interior"))
                                        {
                                            submeshColor = QColor(58, 79, 102, 140); // Authentic blue jean denim wash
                                        }
                                        else if (szMatName.contains("hair"))
                                        {
                                            submeshColor = QColor(45, 36, 30, 180); // Dark brunette hair profile
                                        }
                                        else if (szMatName.contains("glove") || szMatName.contains("shoe") || szMatName.contains("boot"))
                                        {
                                            submeshColor = QColor(35, 35, 35, 200); // Charcoal combat boots
                                        }
                                        else if (szMatName.contains("eye"))
                                        {
                                            submeshColor = QColor(114, 153, 114, 255); // Green eye irises
                                        }
                                        else
                                        {
                                            uint hash = qHash(szMatName);
                                            submeshColor = QColor::fromHsl((hash % 360), 140, 110, 110);
                                        }
                                    }
                                }

                                for (int groupIdx = 0; groupIdx < pMeshData->m_NumGroup; ++groupIdx)
                                {
                                    studiomeshgroup_t *pGroup = &pMeshData->m_pMeshGroup[groupIdx];
                                    if (!pGroup || pGroup->m_pIndices == nullptr || pGroup->m_pGroupIndexToMeshIndex == nullptr)
                                        continue;

                                    unsigned short *pIndices = pGroup->m_pIndices;

                                    int numIndices = 0;
                                    if (pGroup->m_pUniqueTris != nullptr)
                                    {
                                        for (int s = 0; s < pGroup->m_NumStrips; ++s)
                                        {
                                            numIndices += pGroup->m_pUniqueTris[s] * 3;
                                        }
                                    }
                                    if (numIndices <= 0)
                                        continue;
                                    int globalVertexBaseIdx = pSubModel->vertexindex / sizeof(mstudiovertex_t);
                                    auto SkinVertex = [&](int globalVertIdx) -> Vector
                                    {Vector &rawPos = pVertices[globalVertIdx].m_vecPosition;mstudioboneweight_t &weights = pVertices[globalVertIdx].m_BoneWeights;if (weights.numbones == 0) return rawPos;Vector skinnedPos(0, 0, 0);for (int b = 0; b < weights.numbones; ++b){int boneIdx = weights.bone[b];float weight = weights.weight[b];if (boneIdx >= 0 && boneIdx < pStudioHdr->numbones){Vector localPos;VectorTransform(rawPos, pBoneArray[boneIdx].poseToBone, localPos);Vector transformed;VectorTransform(localPos, pBoneToWorld[boneIdx], transformed);skinnedPos += transformed * weight;}}return skinnedPos; };
                                    for (int idx = 0; idx < numIndices - 2; idx += 3)
                                    {
                                        int groupVertIdx0 = pGroup->m_pGroupIndexToMeshIndex[pIndices[idx]];
                                        int groupVertIdx1 = pGroup->m_pGroupIndexToMeshIndex[pIndices[idx + 1]];
                                        int groupVertIdx2 = pGroup->m_pGroupIndexToMeshIndex[pIndices[idx + 2]];
                                        int v0 = globalVertexBaseIdx + pMesh->vertexoffset + groupVertIdx0;
                                        int v1 = globalVertexBaseIdx + pMesh->vertexoffset + groupVertIdx1;
                                        int v2 = globalVertexBaseIdx + pMesh->vertexoffset + groupVertIdx2;
                                        Vector pos0 = SkinVertex(v0);
                                        Vector pos1 = SkinVertex(v1);
                                        Vector pos2 = SkinVertex(v2);
                                        QPointF pt0 = Project3DPoint(pos0.x, pos0.y, pos0.z);
                                        QPointF pt1 = Project3DPoint(pos1.x, pos1.y, pos1.z);
                                        QPointF pt2 = Project3DPoint(pos2.x, pos2.y, pos2.z);
                                        Vector2D &uv0 = pVertices[v0].m_vecTexCoord;
                                        Vector2D &uv1 = pVertices[v1].m_vecTexCoord;
                                        Vector2D &uv2 = pVertices[v2].m_vecTexCoord;
                                        float x0 = uv0.x * texW;
                                        float y0 = (1.0f - uv0.y) * texH;
                                        float x1 = uv1.x * texW;
                                        float y1 = (1.0f - uv1.y) * texH;
                                        float x2 = uv2.x * texW;
                                        float y2 = (1.0f - uv2.y) * texH;
                                        float det = (x0 - x2) * (y1 - y2) - (x1 - x2) * (y0 - y2);
                                        if (qAbs(det) > 0.0001f)
                                        {
                                            float idet = 1.0f / det;
                                            float m11 = ((pt0.x() - pt2.x()) * (y1 - y2) - (pt1.x() - pt2.x()) * (y0 - y2)) * idet;
                                            float m12 = ((pt1.x() - pt2.x()) * (x0 - x2) - (pt0.x() - pt2.x()) * (x1 - x2)) * idet;
                                            float dx = pt2.x() - m11 * x2 - m12 * y2;
                                            float m21 = ((pt0.y() - pt2.y()) * (y1 - y2) - (pt1.y() - pt2.y()) * (y0 - y2)) * idet;
                                            float m22 = ((pt1.y() - pt2.y()) * (x0 - x2) - (pt0.y() - pt2.y()) * (x1 - x2)) * idet;
                                            float dy = pt2.y() - m21 * x2 - m22 * y2;
                                            QTransform affineTransform(m11, m21, 0, m12, m22, 0, dx, dy, 1);
                                            painter.save();
                                            painter.setTransform(affineTransform, true);
                                            QPolygonF srcPoly;
                                            srcPoly << QPointF(x0, y0) << QPointF(x1, y1) << QPointF(x2, y2);
                                            painter.setPen(Qt::NoPen);
                                            painter.setBrush(QBrush(uvGridSheet));
                                            painter.drawPolygon(srcPoly);
                                            painter.setBrush(submeshColor);
                                            painter.drawPolygon(srcPoly);
                                            painter.restore();
                                        }
                                        painter.setPen(QPen(QColor(43, 45, 66, 30), 0.5f, Qt::SolidLine));
                                        painter.setBrush(Qt::NoBrush);
                                        QPolygonF wireTriangle;
                                        wireTriangle << pt0 << pt1 << pt2;
                                        painter.drawPolygon(wireTriangle);
                                    }
                                }
                            }
                        }
                    }
                }
                // ---- TRADITIONAL BONE SKELETON TREE OVERLAY GENERATOR ----
                if (pBoneArray != nullptr)
                {
                    for (int i = 0; i < pStudioHdr->numbones; ++i)
                    {
                        // FIX: Explicitly applied multidimensional bracket tracking indices to match float array declarations [row][col]
                        float bX1 = pBoneToWorld[i].m_flMatVal[0][3];
                        float bY1 = pBoneToWorld[i].m_flMatVal[1][3];
                        float bZ1 = pBoneToWorld[i].m_flMatVal[2][3];
                        QPointF p1 = Project3DPoint(bX1, bY1, bZ1);
                        painter.setBrush(QColor(241, 91, 181));
                        painter.setPen(Qt::NoPen);
                        painter.drawEllipse(p1, 3, 3);
                        int parentIdx = pBoneArray[i].parent;
                        if (parentIdx >= 0 && parentIdx < pStudioHdr->numbones)
                        {
                            // FIX: Corrected double subscript references on parent matrices
                            float bX2 = pBoneToWorld[parentIdx].m_flMatVal[0][3];
                            float bY2 = pBoneToWorld[parentIdx].m_flMatVal[1][3];
                            float bZ2 = pBoneToWorld[parentIdx].m_flMatVal[2][3];
                            QPointF p2 = Project3DPoint(bX2, bY2, bZ2);
                            painter.setPen(QPen(QColor(241, 91, 181, 180), 1.5f, Qt::SolidLine));
                            painter.drawLine(p1, p2);
                        }
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
            painter.drawText(15, 60, QString("BONE LENGTH: %1 layers solved").arg(pStudioHdr->numbones));
            painter.drawText(15, 75, QString("Sequences  : %1 clips present").arg(pStudioHdr->numlocalseq));
        }
    }
}

void QModelView3::mousePressEvent(QMouseEvent *event) { m_ptLastMousePosition = event->pos(); }

void QModelView3::mouseMoveEvent(QMouseEvent *event)
{
    QPointF delta = event->position() - m_ptLastMousePosition;
    m_ptLastMousePosition = event->pos();

    if (event->buttons() & Qt::LeftButton)
    {
        // Orbit control loop
        m_ptRotationAngle.setY(m_ptRotationAngle.y() + delta.x() * 0.5f);
        m_ptRotationAngle.setX(m_ptRotationAngle.x() - delta.y() * 0.5f);
        this->update();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        // Pan control loop
        m_ptCameraPanOffset.setX(m_ptCameraPanOffset.x() + delta.x());
        m_ptCameraPanOffset.setY(m_ptCameraPanOffset.y() + delta.y());
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