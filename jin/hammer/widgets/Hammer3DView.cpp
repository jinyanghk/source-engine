#include "Hammer3DView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QTimer>
#include <algorithm>

#include "materialsystem/imaterialsystem.h"
#include "istudiorender.h"
#include "datacache/imdlcache.h"
#include "materialsystem/itexture.h"

extern IMaterialSystem *g_pMaterialSystem;
extern IStudioRender *g_pStudioRender;
extern IMDLCache *g_pMDLCache;

extern "C" void Hammer_SetLauncherWindowContext(void *pWindowRef, int width, int height);
static matrix3x4_t s_InterceptedBoneTransforms[MAXSTUDIOBONES];

Hammer3DView::Hammer3DView(QWidget *parent)
    : QWidget(parent), m_flAnimationCycle(0.0f), m_hCurrentModel(0xFFFF), m_szCurrentModelPath(""),
      m_flZoomScale(2.5f), m_pOffscreenRenderTarget(nullptr), m_bIsRenderBufferBlank(true),
      m_ptCameraPanOffset(QPointF(0, 0)), m_selectedBrushId(-1)
{
    m_ptRotationAngle = QPoint(-20, 45); // Classic Hammer oblique view angle rules orientation setup
    m_nActiveSequenceIndex = 0;
    m_bPlaybackPaused = false;

    m_pAnimationFrameTimer = new QTimer(this);
    connect(m_pAnimationFrameTimer, &QTimer::timeout, this, [=]()
            {
        // FORCE the animation cycle state parameter to stay permanently at zero!
        m_flAnimationCycle = 0.0f; 

        // Keep updating the viewport window at 30 FPS natively
        this->update(); });
    m_pAnimationFrameTimer->start(33); // 30 FPS Refresh Thread Tick Loop

    QTimer::singleShot(200, this, [=]()
                       { LoadModelFile("models/alyx.mdl"); });
}

Hammer3DView::~Hammer3DView()
{
    if (m_pOffscreenRenderTarget && g_pMaterialSystem)
    {
        m_pOffscreenRenderTarget->DecrementReferenceCount();
    }
}

void Hammer3DView::updateBrushes(const MapBrush *pBrushes, int count, int selectedId)
{
    m_mapBrushes.clear();
    m_selectedBrushId = selectedId;

    // Convert elements safely via layout field matching copy pass
    if (pBrushes && count > 0)
    {
        m_mapBrushes.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            ModelViewBrush b;
            b.id = pBrushes[i].id;

            // Map raw Source Vector objects directly
            b.mins = pBrushes[i].mins;
            b.maxs = pBrushes[i].maxs;
            b.color = pBrushes[i].color;

            m_mapBrushes.append(b);
        }
    }
    this->update(); // Enforce layout refresh pass
}

void Hammer3DView::LoadModelFile(const QString &szPath)
{
    if (!g_pMDLCache)
        return;
    m_szCurrentModelPath = szPath;
    m_flAnimationCycle = 0.0f;
    m_hCurrentModel = g_pMDLCache->FindMDL(m_szCurrentModelPath.toUtf8().constData());
    this->update();
}

void Hammer3DView::RenderEngineFrame()
{
    if (!g_pMaterialSystem || !g_pStudioRender || !g_pMDLCache || !isVisible())
    {
        m_bIsRenderBufferBlank = true;
        return;
    }
    int w = qMax(64, rect().width());
    int h = qMax(64, rect().height());
    Hammer_SetLauncherWindowContext(reinterpret_cast<void *>(this->winId()), w, h);

    if (!m_pOffscreenRenderTarget || m_pOffscreenRenderTarget->GetActualWidth() != w || m_pOffscreenRenderTarget->GetActualHeight() != h)
    {
        if (m_pOffscreenRenderTarget)
        {
            m_pOffscreenRenderTarget->DecrementReferenceCount();
            m_pOffscreenRenderTarget = nullptr;
        }
        m_pOffscreenRenderTarget = g_pMaterialSystem->CreateRenderTargetTexture(w, h, RT_SIZE_NO_CHANGE, IMAGE_FORMAT_RGBA8888, MATERIAL_RT_DEPTH_SHARED);
        if (m_pOffscreenRenderTarget)
            m_pOffscreenRenderTarget->IncrementReferenceCount();
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

                    pRenderContext->Translate(m_ptCameraPanOffset.x() * 0.1f, -30.0f - (m_ptCameraPanOffset.y() * 0.1f), -65.0f * m_flZoomScale);
                    pRenderContext->Rotate(m_ptRotationAngle.x(), 1.0f, 0.0f, 0.0f);
                    pRenderContext->Rotate(m_ptRotationAngle.y(), 0.0f, 1.0f, 0.0f);

                    pRenderContext->MatrixMode(MATERIAL_MODEL);
                    pRenderContext->PushMatrix();
                    pRenderContext->LoadIdentity();

                    matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
                    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
                    int nSequenceIndex = qBound(0, m_nActiveSequenceIndex, pStudioHdr->numlocalseq - 1);

                    if (pBoneArray != nullptr)
                    {
                        for (int i = 0; i < pStudioHdr->numbones; i++)
                        {
                            Vector bonePos = pBoneArray[i].pos;
                            Quaternion boneQuat = pBoneArray[i].quat;

                            if (pBoneArray[i].parent != -1)
                            {
                                float timelineFactor = (m_flAnimationCycle * M_PI * 2.0f);
                                if (nSequenceIndex > 0)
                                {
                                    float sequencePoseAngle = (float)nSequenceIndex * 0.35f;
                                    boneQuat.x += qSin(timelineFactor + i * 0.1f) * 0.08f + qMin(0.2f, sequencePoseAngle * 0.05f);
                                    boneQuat.y += qCos(timelineFactor + i * 0.1f) * 0.05f;
                                    if (i > 10 && i % 2 == 0)
                                    {
                                        bonePos.z += qMax(-3.0f, -((float)nSequenceIndex * 0.6f));
                                        bonePos.y += qMin(4.0f, ((float)nSequenceIndex * 0.4f));
                                    }
                                }
                                else
                                {
                                    float waveFactor = qSin(timelineFactor + i * 0.2f) * 0.2f;
                                    bonePos.x += waveFactor;
                                    bonePos.y += waveFactor * 0.5f;
                                }
                            }

                            matrix3x4_t bonematrix;
                            QuaternionMatrix(boneQuat, bonePos, bonematrix);

                            int parentIdx = pBoneArray[i].parent;
                            if (parentIdx == -1)
                                MatrixCopy(bonematrix, pBoneToWorld[i]);
                            else
                                ConcatTransforms(pBoneToWorld[parentIdx], bonematrix, pBoneToWorld[i]);

                            MatrixCopy(pBoneToWorld[i], s_InterceptedBoneTransforms[i]);
                        }
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

            if (engineBuffer.pixelColor(w / 2, h / 2).alpha() != 0)
            {
                m_RenderOutputImage = engineBuffer.convertToFormat(QImage::Format_RGB32);
                m_bIsRenderBufferBlank = false;
            }
            else
            {
                m_bIsRenderBufferBlank = true;
            }
            pRenderContext->PopRenderTargetAndViewport();
        }
    }
    g_pMaterialSystem->EndFrame();
}
void Hammer3DView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    RenderEngineFrame();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int w = rect().width();
    int h = rect().height();
    painter.fillRect(rect(), QColor(30, 32, 44));
    float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
    float radY = qDegreesToRadians((float)m_ptRotationAngle.y());

    auto Project3DPointEx = [this, w, h, radX, radY](float x, float y, float z) -> QPointF
    {
        float rotatedX = x * qCos(radY) - y * qSin(radY);
        float rotatedY = x * qSin(radY) + y * qCos(radY);
        float finalX = rotatedX;
        float finalY = rotatedY * qCos(radX) - z * qSin(radX);
        float sX = (w / 2.0f) + (finalX * m_flZoomScale * 1.8f) + m_ptCameraPanOffset.x();
        float sY = (h / 2.0f) - (finalY * m_flZoomScale * 1.8f) + m_ptCameraPanOffset.y();
        return QPointF(sX, sY);
    };

    if (m_hCurrentModel != 0xFFFF && g_pMDLCache)
    {
        studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
        studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(m_hCurrentModel);
        if (pStudioHdr && pHardwareData && pHardwareData->m_NumLODs > 0 && pHardwareData->m_pLODs != nullptr)
        {
            matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
            mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);
            for (int b = 0; b < qMin(pStudioHdr->numbones, MAXSTUDIOBONES); ++b)
            {
                MatrixCopy(s_InterceptedBoneTransforms[b], pBoneToWorld[b]);
            }
            Vector vecLightDir(0.5f, -0.4f, 0.7f);
            vecLightDir.NormalizeInPlace();
            struct SortableTriangle_t
            {
                QPolygonF poly;
                QColor color;
                float avgDepth;
            };
            QList<SortableTriangle_t> triangleDrawList;
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
                        pVertices = (mstudiovertex_t *)pMeshVertData->pModelVertexData()->GetVertexData();
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
                            QColor baseColor(145, 150, 160);
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
                                        numIndices += pGroup->m_pUniqueTris[s] * 3;
                                }
                                if (numIndices <= 0)
                                    continue;
                                int globalVertexBaseIdx = pSubModel->vertexindex / sizeof(mstudiovertex_t);
                                auto SkinVertex = [&](int globalVertIdx) -> Vector
                                {Vector &rawPos = pVertices[globalVertIdx].m_vecPosition;mstudioboneweight_t &weights = pVertices[globalVertIdx].m_BoneWeights;if (weights.numbones == 0) return rawPos; Vector skinnedPos(0, 0, 0);for (int b = 0; b < weights.numbones; ++b) {int boneIdx = (int)weights.bone[b]; float weight = weights.weight[b];if (boneIdx >= 0 && boneIdx < pStudioHdr->numbones && pBoneArray != nullptr) {Vector localPos, transformed;VectorTransform(rawPos, pBoneArray[boneIdx].poseToBone, localPos);VectorTransform(localPos, pBoneToWorld[boneIdx], transformed);skinnedPos += transformed * weight;}}return skinnedPos; };
                                for (int idx = 0; idx < numIndices - 2; idx += 3)
                                {
                                    int gv0 = pGroup->m_pGroupIndexToMeshIndex[pIndices[idx]];
                                    int gv1 = pGroup->m_pGroupIndexToMeshIndex[pIndices[idx + 1]];
                                    int gv2 = pGroup->m_pGroupIndexToMeshIndex[pIndices[idx + 2]];
                                    Vector pos0 = SkinVertex(globalVertexBaseIdx + pMesh->vertexoffset + gv0);
                                    Vector pos1 = SkinVertex(globalVertexBaseIdx + pMesh->vertexoffset + gv1);
                                    Vector pos2 = SkinVertex(globalVertexBaseIdx + pMesh->vertexoffset + gv2);
                                    Vector edge1 = pos1 - pos0;
                                    Vector edge2 = pos2 - pos0;
                                    Vector faceNormal;
                                    CrossProduct(edge1, edge2, faceNormal);
                                    faceNormal.NormalizeInPlace();
                                    float lightIntensity = qMax(0.0f, faceNormal.Dot(vecLightDir)) * 0.70f + 0.30f;
                                    QColor shadedColor(qBound(0, (int)(baseColor.red() * lightIntensity), 255), qBound(0, (int)(baseColor.green() * lightIntensity), 255), qBound(0, (int)(baseColor.blue() * lightIntensity), 255));
                                    QPointF pt0 = Project3DPointEx(pos0.x, pos0.y, pos0.z);
                                    QPointF pt1 = Project3DPointEx(pos1.x, pos1.y, pos1.z);
                                    QPointF pt2 = Project3DPointEx(pos2.x, pos2.y, pos2.z);
                                    if (((pt1.x() - pt0.x()) * (pt2.y() - pt0.y()) - (pt1.y() - pt0.y()) * (pt2.x() - pt0.x())) < 0.0f)
                                        continue;
                                    SortableTriangle_t tri;
                                    tri.poly << pt0 << pt1 << pt2;
                                    tri.color = shadedColor;
                                    tri.avgDepth = (pos0.y + pos1.y + pos2.y) / 3.0f;
                                    triangleDrawList.append(tri);
                                }
                            }
                        }
                    }
                }
            }
            std::sort(triangleDrawList.begin(), triangleDrawList.end(), [](const SortableTriangle_t &a, const SortableTriangle_t &b)
                      { return a.avgDepth > b.avgDepth; });
            for (const auto &tri : triangleDrawList)
            {
                painter.setPen(QPen(tri.color.darker(110), 0.3f, Qt::SolidLine));
                painter.setBrush(tri.color);
                painter.drawPolygon(tri.poly);
            }
        }
    }
    // ---- FIX 2: OVERLAY WIREFRAME MAP BRUSHES PARALLEL LAYER ----
    for (const auto &brush : m_mapBrushes)
    {
        bool isSelected = (brush.id == m_selectedBrushId);
        QColor displayColor = isSelected ? QColor(255, 0, 0) : brush.color;
        painter.setPen(QPen(displayColor, isSelected ? 2.0f : 1.0f, Qt::SolidLine));
        QPointF p0 = Project3DPointEx(brush.mins.x, brush.mins.y, brush.mins.z);
        QPointF p1 = Project3DPointEx(brush.maxs.x, brush.mins.y, brush.mins.z);
        QPointF p2 = Project3DPointEx(brush.maxs.x, brush.maxs.y, brush.mins.z);
        QPointF p3 = Project3DPointEx(brush.mins.x, brush.maxs.y, brush.mins.z);
        QPointF p4 = Project3DPointEx(brush.mins.x, brush.mins.y, brush.maxs.z);
        QPointF p5 = Project3DPointEx(brush.maxs.x, brush.mins.y, brush.maxs.z);
        QPointF p6 = Project3DPointEx(brush.maxs.x, brush.maxs.y, brush.maxs.z);
        QPointF p7 = Project3DPointEx(brush.mins.x, brush.maxs.y, brush.maxs.z);
        painter.drawLine(p0, p1);
        painter.drawLine(p1, p2);
        painter.drawLine(p2, p3);
        painter.drawLine(p3, p0);
        painter.drawLine(p4, p5);
        painter.drawLine(p5, p6);
        painter.drawLine(p6, p7);
        painter.drawLine(p7, p4);
        painter.drawLine(p0, p4);
        painter.drawLine(p1, p5);
        painter.drawLine(p2, p6);
        painter.drawLine(p3, p7);
    }
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(15, 25, "ModelView: 3D MAP WORKSPACE WIREFRAME ACTIVE");
}
void Hammer3DView::mousePressEvent(QMouseEvent *event) { m_ptLastMousePosition = event->pos(); }
void Hammer3DView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF delta = event->position() - m_ptLastMousePosition;
    m_ptLastMousePosition = event->pos();
    if (event->buttons() & Qt::LeftButton)
    {
        m_ptRotationAngle.setY(m_ptRotationAngle.y() + delta.x() * 0.5f);
        m_ptRotationAngle.setX(m_ptRotationAngle.x() - delta.y() * 0.5f);
        this->update();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        m_ptCameraPanOffset.setX(m_ptCameraPanOffset.x() + delta.x());
        m_ptCameraPanOffset.setY(m_ptCameraPanOffset.y() + delta.y());
        this->update();
    }
}
void Hammer3DView::wheelEvent(QWheelEvent *event)
{
    m_flZoomScale += event->angleDelta().y() > 0 ? 0.1f : -0.1f;
    m_flZoomScale = qBound(0.1f, m_flZoomScale, 10.0f);
    this->update();
}
void Hammer3DView::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); }
int Hammer3DView::GetSequenceCount() { return 0; }
const char *Hammer3DView::GetSequenceName(int index)
{
    Q_UNUSED(index);
    return "";
}
void Hammer3DView::SetActiveSequence(int index) { Q_UNUSED(index); }
void Hammer3DView::SetAnimationCycle(float flCycle) { Q_UNUSED(flCycle); }
void Hammer3DView::SetPlaybackPaused(bool bPaused) { Q_UNUSED(bPaused); }

// Replace the updateEntities function block in widgets/ModelView4.cpp with this:
void Hammer3DView::updateEntities(const MapEntity *pEntities, int count)
{
    m_mapEntities.clear();
    if (pEntities && count > 0)
    {
        m_mapEntities.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            m_mapEntities.append(pEntities[i]);
        }
    }
    this->update(); // Re-trigger the 3D paintEvent pass
}
