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
    // Execute frame safety checks
    RenderEngineFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = rect().width();
    int h = rect().height();

    // Clear background canvas space smoothly
    painter.fillRect(rect(), QColor(30, 32, 44));

    if (m_hCurrentModel != 0xFFFF && g_pMDLCache)
    {
        studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(m_hCurrentModel);
        studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(m_hCurrentModel);

        if (pStudioHdr && pHardwareData && pHardwareData->m_NumLODs > 0 && pHardwareData->m_pLODs != nullptr)
        {
            float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
            float radY = qDegreesToRadians((float)m_ptRotationAngle.y());

            // 3D Screen Space Projector with explicit depth extraction return parameters
            auto Project3DPointEx = [this, w, h, radX, radY](float x, float y, float z, float &outRotZ) -> QPointF
            {
                float x1 = x;
                float y1 = y * qCos(radX) - z * qSin(radX);
                float z1 = y * qSin(radX) + z * qCos(radX);
                float x2 = x1 * qCos(radY) + z1 * qSin(radY);
                outRotZ = -x1 * qSin(radY) + z1 * qCos(radY); // Depth tracking variable

                float sX = (w / 2.0f) + (x2 * m_flZoomScale * 1.8f) + m_ptCameraPanOffset.x();
                float sY = (h / 2.0f) + (y1 * m_flZoomScale * 1.8f) + 40.0f + m_ptCameraPanOffset.y();
                return QPointF(sX, sY);
            };

            // ---- HIGH PERFORMANCE FORWARD KINEMATICS CHAIN ----
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

            // Directional studio key light vector direction
            Vector vecLightDir(0.5f, -0.4f, 0.7f);
            vecLightDir.NormalizeInPlace();

            // Collect all triangles to run our Painter's Depth Sorting pass
            struct SortableTriangle_t
            {
                QPolygonF poly;
                QColor color;
                float avgDepth;
            };
            QList<SortableTriangle_t> triangleDrawList;

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

                            // ---- RESOLVE ACCURATE PALETTE SHADING COLOR VIA MATERIAL STRINGS ----
                            QColor baseColor(145, 150, 160); // Neutral baseline grey
                            
                            if (pSkinRefArray && pMesh->material < pStudioHdr->numtextures)
                            {
                                mstudiotexture_t *pTextureTable = pStudioHdr->pTexture(pSkinRefArray[pMesh->material]);
                                if (pTextureTable && pTextureTable->pszName())
                                {
                                    QString szMatName = QString(pTextureTable->pszName()).toLower();
                                    
                                    // FIX: Catch her true texture sheet names like "alyx_sheet", "vance_body", and "alyx_faceandbody"
                                    if (szMatName.contains("face") || szMatName.contains("head") || szMatName.contains("skin"))
                                    {
                                        baseColor = QColor(228, 185, 161); // Clear Skin Flush Tone
                                    }
                                    // If the texture represents her combined body sheet, or her leather jacket assets
                                    else if (szMatName.contains("jacket") || szMatName.contains("coat") || szMatName.contains("vance"))
                                    {
                                        baseColor = QColor(112, 78, 54);    // Leather Brown Jacket
                                    }
                                    // Catch her lower body denim sheets like "alyx_sheet" or "alyx_interior"
                                    else if (szMatName.contains("jean") || szMatName.contains("pant") || szMatName.contains("leg") || szMatName.contains("sheet") || szMatName.contains("interior"))
                                    {
                                        baseColor = QColor(64, 88, 118);    // Denim Blue Jeans
                                    }
                                    else if (szMatName.contains("hair"))
                                    {
                                        baseColor = QColor(50, 42, 36);     // Dark Brunette Hair
                                    }
                                    else if (szMatName.contains("boot") || szMatName.contains("shoe") || szMatName.contains("glove"))
                                    {
                                        baseColor = QColor(42, 42, 42);     // Charcoal Combat Items
                                    }
                                    else if (szMatName.contains("eye"))
                                    {
                                        baseColor = QColor(120, 160, 120);  // Green Eyes
                                    }
                                    else
                                    {
                                        // Fallback procedural hashing
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

                                auto SkinVertex = [&](int globalVertIdx) -> Vector
                                {
                                    Vector &rawPos = pVertices[globalVertIdx].m_vecPosition;
                                    mstudioboneweight_t &weights = pVertices[globalVertIdx].m_BoneWeights;

                                    if (weights.numbones == 0)
                                        return rawPos;

                                    Vector skinnedPos(0, 0, 0);
                                    for (int b = 0; b < weights.numbones; ++b)
                                    {
                                        int boneIdx = (int)weights.bone[b];
                                        float weight = weights.weight[b];

                                        if (boneIdx >= 0 && boneIdx < pStudioHdr->numbones)
                                        {
                                            Vector localPos, transformed;
                                            VectorTransform(rawPos, pBoneArray[boneIdx].poseToBone, localPos);
                                            VectorTransform(localPos, pBoneToWorld[boneIdx], transformed);
                                            skinnedPos += transformed * weight;
                                        }
                                    }
                                    return skinnedPos;
                                };
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
                                    // Calculate face normal and light intensity
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
                                    // Backface culling engine
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
            // Execute Depth Sorting
            std::sort(triangleDrawList.begin(), triangleDrawList.end(), [](const SortableTriangle_t &a, const SortableTriangle_t &b)
                      { return a.avgDepth < b.avgDepth; });
            // Draw the sorted polygons
            for (const auto &tri : triangleDrawList)
            {
                painter.setPen(QPen(tri.color.darker(110), 0.3f, Qt::SolidLine));
                painter.setBrush(tri.color);
                painter.drawPolygon(tri.poly);
            }
            // ---- OVERLAY SKELETON TREE NODES ----
            for (int i = 0; i < pStudioHdr->numbones; ++i)
            {
                float dummyD = 0.0f;
                // FIX: Explicitly applied matrix subscripts [row][col] to extract the position coordinates from column 3
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
                    // FIX: Explicitly applied parent matrix subscripts to resolve row layouts safely from column 3
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
    // Status Text Overlays
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