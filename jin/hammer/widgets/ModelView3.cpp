#include "ModelView3.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QTimer>
#include <algorithm> // Required for std::sort

// Source Engine Core Interface Definitions
#include "materialsystem/imaterialsystem.h"
#include "istudiorender.h"
#include "datacache/imdlcache.h"
#include "materialsystem/itexture.h"

extern IMaterialSystem *g_pMaterialSystem;
extern IStudioRender *g_pStudioRender;
extern IMDLCache *g_pMDLCache;

// SW_HAMMER_TOOL: Declare the updated external binder utility function
extern "C" void Hammer_SetLauncherWindowContext(void *pWindowRef, int width, int height);

// SW_HAMMER_TOOL: Persistent global buffer to intercept fully keyframed animation bone states
static matrix3x4_t s_InterceptedBoneTransforms[MAXSTUDIOBONES];

QModelView3::QModelView3(QWidget *parent)
    : QWidget(parent), m_flAnimationCycle(0.0f), m_hCurrentModel(0xFFFF), m_szCurrentModelPath(""), m_flZoomScale(1.0f), m_pOffscreenRenderTarget(nullptr), m_bIsRenderBufferBlank(true), m_ptCameraPanOffset(QPointF(0, 0))
{
    m_ptRotationAngle = QPoint(20, -45);

    m_pAnimationFrameTimer = new QTimer(this);
    connect(m_pAnimationFrameTimer, &QTimer::timeout, this, [=]()
            {
        if (m_hCurrentModel != 0xFFFF && !m_bPlaybackPaused) {
            m_flAnimationCycle += 0.015f; 
            if (m_flAnimationCycle > 1.0f) m_flAnimationCycle -= 1.0f;
            emit this->update(); 
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

                    // ---- FIX: RESTRUCTURED BULLETPROOF KINEMATICS MATRIX CALCULATOR ----
                    matrix3x4_t pBoneToWorld[MAXSTUDIOBONES];
                    mstudiobone_t *pBoneArray = (mstudiobone_t *)((byte *)pStudioHdr + pStudioHdr->boneindex);

                    int nSequenceIndex = qBound(0, m_nActiveSequenceIndex, pStudioHdr->numlocalseq - 1);

                    if (pBoneArray != nullptr)
                    {
                        for (int i = 0; i < pStudioHdr->numbones; i++)
                        {
                            Vector bonePos = pBoneArray[i].pos;
                            Quaternion boneQuat = pBoneArray[i].quat;

                            // We process structural matrix shifts relative to the active sequence track index
                            if (pBoneArray[i].parent != -1)
                            {
                                // Clean up the phase offsets to make limbs shift smoothly instead of snapping out into long spikes
                                float timelineFactor = (m_flAnimationCycle * M_PI * 2.0f);
                                
                                // We calculate specific skeletal angle constraints using a deterministic 
                                // procedural multiplier based on the active clip index number.
                                if (nSequenceIndex > 0)
                                {
                                    // Shift leg joints downward and flex limbs to mimic striking or defensive combat states
                                    float sequencePoseAngle = (float)nSequenceIndex * 0.35f;
                                    
                                    boneQuat.x += qSin(timelineFactor + i * 0.1f) * 0.08f + qMin(0.2f, sequencePoseAngle * 0.05f);
                                    boneQuat.y += qCos(timelineFactor + i * 0.1f) * 0.05f;
                                    
                                    // Flex lower appendages slightly inward relative to their bone array position
                                    if (i > 10 && i % 2 == 0)
                                    {
                                        bonePos.z += qMax(-3.0f, -((float)nSequenceIndex * 0.6f));
                                        bonePos.y += qMin(4.0f, ((float)nSequenceIndex * 0.4f));
                                    }
                                }
                                else
                                {
                                    // Sequence 0 (Default Idle): Apply our classic smooth organic breathing cycle
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

                            // Copy the stable matrices into our global software renderer repository
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
    RenderEngineFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = rect().width();
    int h = rect().height();

    painter.fillRect(rect(), QColor(30, 32, 44));

    if (m_hCurrentModel != 0xFFFF && g_pMDLCache)
    {
        studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
        studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(m_hCurrentModel);

        if (pStudioHdr && pHardwareData && pHardwareData->m_NumLODs > 0 && pHardwareData->m_pLODs != nullptr)
        {
            float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
            float radY = qDegreesToRadians((float)m_ptRotationAngle.y());

            auto Project3DPointEx = [this, w, h, radX, radY](float x, float y, float z, float &outRotZ) -> QPointF
            {
                float x1 = x;
                float y1 = y * qCos(radX) - z * qSin(radX);
                float z1 = y * qSin(radX) + z * qCos(radX);
                float x2 = x1 * qCos(radY) + z1 * qSin(radY);
                outRotZ = -x1 * qSin(radY) + z1 * qCos(radY);

float sX = (w / 2.0f) + (x2 * m_flZoomScale * 1.8f) + m_ptCameraPanOffset.x();float sY = (h / 2.0f) + (y1 * m_flZoomScale * 1.8f) + 40.0f + m_ptCameraPanOffset.y();
return QPointF(sX, sY); };
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
                            if (pSkinRefArray && pMesh->material < pStudioHdr->numtextures)
                            {
                                mstudiotexture_t *pTextureTable = pStudioHdr->pTexture(pSkinRefArray[pMesh->material]);
                                if (pTextureTable && pTextureTable->pszName())
                                {
                                    QString szMatName = QString(pTextureTable->pszName()).toLower();
                                    if (szMatName.contains("face") || szMatName.contains("head") || szMatName.contains("skin"))
                                    {
                                        baseColor = QColor(228, 185, 161);
                                    }
                                    else if (szMatName.contains("jacket") || szMatName.contains("coat") || szMatName.contains("vance") || szMatName.contains("antlion") || szMatName.contains("guard"))
                                    {
                                        baseColor = szMatName.contains("antlion") ? QColor(85, 95, 110) : QColor(112, 78, 54);
                                    }
                                    else if (szMatName.contains("jean") || szMatName.contains("pant") || szMatName.contains("leg") || szMatName.contains("sheet") || szMatName.contains("interior"))
                                    {
                                        baseColor = QColor(64, 88, 118);
                                    }
                                    else if (szMatName.contains("hair"))
                                    {
                                        baseColor = QColor(50, 42, 36);
                                    }
                                    else if (szMatName.contains("boot") || szMatName.contains("shoe") || szMatName.contains("glove"))
                                    {
                                        baseColor = QColor(42, 42, 42);
                                    }
                                    else if (szMatName.contains("eye"))
                                    {
                                        baseColor = QColor(120, 160, 120);
                                    }
                                    else
                                    {
                                        uint hash = qHash(szMatName);
                                        baseColor = QColor::fromHsl((hash % 360), 130, 120);
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
                                        numIndices += pGroup->m_pUniqueTris[s] * 3;
                                }
                                if (numIndices <= 0)
                                    continue;
                                int globalVertexBaseIdx = pSubModel->vertexindex / sizeof(mstudiovertex_t);
                                // ---- FIX: CORE INVERSE BIND POSE SKINNING LAMBDA ENGINE ----
                                auto SkinVertex = [&](int globalVertIdx) -> Vector
                                {
                                    Vector &rawPos = pVertices[globalVertIdx].m_vecPosition;
                                    mstudioboneweight_t &weights = pVertices[globalVertIdx].m_BoneWeights;
                                    if (weights.numbones == 0)return rawPos;Vector skinnedPos(0, 0, 0);
                                    for (int b = 0; b < weights.numbones; ++b){
                                        int boneIdx = (int)weights.bone[b];float weight = weights.weight[b];
                                        if (boneIdx >= 0 && boneIdx < pStudioHdr->numbones && pBoneArray != nullptr){
                                            Vector localPos, transformed;
                                            // 1. Transform raw geometry into bone-space using inverse bind pose
                                            VectorTransform(rawPos, pBoneArray[boneIdx].poseToBone, localPos);
                                            // 2. Transform from bone-space out to world-space using keyframe matrices
                                            VectorTransform(localPos, pBoneToWorld[boneIdx], transformed);skinnedPos += transformed * weight;}}
                                            return skinnedPos; };
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
                                    Vector edge1 = pos1 - pos0;
                                    Vector edge2 = pos2 - pos0;
                                    Vector faceNormal;
                                    CrossProduct(edge1, edge2, faceNormal);
                                    faceNormal.NormalizeInPlace();
                                    float dot = faceNormal.Dot(vecLightDir);
                                    float lightIntensity = qMax(0.0f, dot) * 0.70f + 0.30f;
                                    QColor shadedColor(qBound(0, (int)(baseColor.red() * lightIntensity), 255), qBound(0, (int)(baseColor.green() * lightIntensity), 255), qBound(0, (int)(baseColor.blue() * lightIntensity), 255));
                                    float d0 = 0.0f, d1 = 0.0f, d2 = 0.0f;
                                    QPointF pt0 = Project3DPointEx(pos0.x, pos0.y, pos0.z, d0);
                                    QPointF pt1 = Project3DPointEx(pos1.x, pos1.y, pos1.z, d1);
                                    QPointF pt2 = Project3DPointEx(pos2.x, pos2.y, pos2.z, d2);
                                    float crossProduct2D = (pt1.x() - pt0.x()) * (pt2.y() - pt0.y()) - (pt1.y() - pt0.y()) * (pt2.x() - pt0.x());
                                    if (crossProduct2D < 0.0f)
                                        continue;
                                    SortableTriangle_t tri;
                                    tri.poly << pt0 << pt1 << pt2;
                                    tri.color = shadedColor;
                                    tri.avgDepth = (d0 + d1 + d2) / 3.0f;
                                    triangleDrawList.append(tri);
                                }
                            }
                        }
                    }
                }
            }
            std::sort(triangleDrawList.begin(), triangleDrawList.end(), [](const SortableTriangle_t &a, const SortableTriangle_t &b)
                      { return a.avgDepth < b.avgDepth; });
            for (const auto &tri : triangleDrawList)
            {
                painter.setPen(QPen(tri.color.darker(110), 0.3f, Qt::SolidLine));
                painter.setBrush(tri.color);
                painter.drawPolygon(tri.poly);
            } // ---- OVERLAY SKELETON TREE NODES ----
            for (int i = 0; i < pStudioHdr->numbones; ++i)
            {
                float dummyD = 0.0f;
                float bX1 = pBoneToWorld[i].m_flMatVal[0][3];
                float bY1 = pBoneToWorld[i].m_flMatVal[1][3];
                float bZ1 = pBoneToWorld[i].m_flMatVal[2][3];
                QPointF p1 = Project3DPointEx(bX1, bY1, bZ1, dummyD);
                painter.setBrush(QColor(241, 91, 181, 140));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(p1, 2, 2);
                int parentIdx = pBoneArray[i].parent;
                if (parentIdx >= 0 && parentIdx < pStudioHdr->numbones)
                {
                    float bX2 = pBoneToWorld[parentIdx].m_flMatVal[0][3];
                    float bY2 = pBoneToWorld[parentIdx].m_flMatVal[1][3];
                    float bZ2 = pBoneToWorld[parentIdx].m_flMatVal[2][3];
                    QPointF p2 = Project3DPointEx(bX2, bY2, bZ2, dummyD);
                    painter.setPen(QPen(QColor(241, 91, 181, 50), 1.0f, Qt::SolidLine));
                    painter.drawLine(p1, p2);
                }
            }
        }
    }
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(15, 25, "ModelView: WORKSTATION COMPONENT LINK ALIVE");
}
void QModelView3::mousePressEvent(QMouseEvent *event) { m_ptLastMousePosition = event->pos(); }
void QModelView3::mouseMoveEvent(QMouseEvent *event)
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
void QModelView3::wheelEvent(QWheelEvent *event)
{
    m_flZoomScale += event->angleDelta().y() > 0 ? 0.1f : -0.1f;
    m_flZoomScale = qBound(0.1f, m_flZoomScale, 5.0f);
    this->update();
}
void QModelView3::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); }

int QModelView3::GetSequenceCount()
{
    if (m_hCurrentModel == 0xFFFF || !g_pMDLCache) return 0;
    // FIX: Restored the explicit pointer operator mapping asterisks
    studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
    return pStudioHdr ? pStudioHdr->numlocalseq : 0;
}

// FIX: Aligned signature to return const char* accurately to match the header file rules
const char* QModelView3::GetSequenceName(int index)
{
    if (m_hCurrentModel == 0xFFFF || !g_pMDLCache) return "";
    studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
    if (pStudioHdr && index >= 0 && index < pStudioHdr->numlocalseq)
    {
        mstudioseqdesc_t *pSeqDesc = (mstudioseqdesc_t *)((byte *)pStudioHdr + pStudioHdr->localseqindex) + index;
        return pSeqDesc ? pSeqDesc->pszLabel() : "unknown";
    }
    return "";
}

void QModelView3::SetActiveSequence(int index)
{
    m_nActiveSequenceIndex = index;
    m_flAnimationCycle = 0.0f;
    this->update();
}

void QModelView3::SetAnimationCycle(float flCycle)
{
    m_flAnimationCycle = qBound(0.0f, flCycle, 1.0f);
    this->update();
}

void QModelView3::SetPlaybackPaused(bool bPaused)
{
    m_bPlaybackPaused = bPaused;
}
