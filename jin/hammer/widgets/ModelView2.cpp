#include "ModelView2.h"
#include "materialsystem/imaterialsystem.h"
#include "filesystem.h" 
#include "studio.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

extern IMaterialSystem* g_pMaterialSystem;
extern IFileSystem*     g_pFileSystem; 

QModelView2::QModelView2(QWidget *parent)
    : QWidget(parent)
    , m_nNumBonesParsed(0)
    , m_nNumTrianglesParsed(0)
    , m_flZoomScale(1.0f)
    , m_ptRotationAngle(30, -45)
    , m_bIsDragging(false)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setFocusPolicy(Qt::StrongFocus);

    LoadModelFile("models/alyx.mdl");
}

QModelView2::~QModelView2()
{
}

void QModelView2::LoadModelFile(const QString &szFilePath)
{
    m_szCurrentModelPath = szFilePath;
    m_MeshVertices.clear(); 
    m_MeshEdges.clear();    

    if (!g_pFileSystem || szFilePath.isEmpty())
        return;

    // Convert to a safe engine-relative layout path format
    QString szEnginePath = szFilePath;
    szEnginePath.replace("\\", "/");
    QByteArray pathBytes = szEnginePath.toLatin1();
    const char* pRelativePath = pathBytes.constData();

    // 1. Open the file straight out of the mounted VPK context paths
    FileHandle_t hFile = g_pFileSystem->Open(pRelativePath, "rb", "GAME");
    if (hFile != FILESYSTEM_INVALID_HANDLE) {
        int nFileSize = g_pFileSystem->Size(hFile);
        
        if (nFileSize > (int)sizeof(studiohdr_t)) {
            // 2. Allocate a temporary buffer chunk to stream the raw binary stream bytes
            QVector<char> fileBuffer(nFileSize);
            g_pFileSystem->Read(fileBuffer.data(), nFileSize, hFile);

            studiohdr_t* pHeader = reinterpret_cast<studiohdr_t*>(fileBuffer.data());
            
            // 3. Verify Valve 'IDST' compiled signature
            if (pHeader->id == 0x54534449) {
                m_szModelNameHeader = QString::fromLatin1(pHeader->name);
                m_nNumBonesParsed = pHeader->numbones;
                m_nNumTrianglesParsed = pHeader->numbodyparts;

                // Cache bounding box vector structures safely at indexes 0 and 1
                m_MeshVertices.append({QVector3D(pHeader->hull_min.x, pHeader->hull_min.y, pHeader->hull_min.z)}); 
                m_MeshVertices.append({QVector3D(pHeader->hull_max.x, pHeader->hull_max.y, pHeader->hull_max.z)}); 

                // 4. Extract physical skeleton geometry straight out of byte offset allocations
                if (pHeader->boneindex > 0 && pHeader->numbones > 0) {
                    char* pBasePtr = fileBuffer.data();
                    mstudiobone_t* pBoneArray = reinterpret_cast<mstudiobone_t*>(pBasePtr + pHeader->boneindex);

                    int baseBoneVertOffset = m_MeshVertices.size();
                    for (int i = 0; i < pHeader->numbones; ++i) {
                        mstudiobone_t* pBone = &pBoneArray[i];
                        
                        m_MeshVertices.append({QVector3D(pBone->pos.x, pBone->pos.y, pBone->pos.z)});

                        if (pBone->parent >= 0 && pBone->parent < pHeader->numbones) {
                            m_MeshEdges.append({baseBoneVertOffset + i, baseBoneVertOffset + pBone->parent});
                        }
                    }
                }
            }
        }
        g_pFileSystem->Close(hFile);
    }

    this->update();
}

void QModelView2::RenderEngineFrame()
{
    if (!g_pMaterialSystem || !isVisible()) 
        return;

    g_pMaterialSystem->BeginFrame(0.0f);
    g_pMaterialSystem->SetView((void*)winId());
    
    CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
    if (pRenderContext) {
        pRenderContext->ClearColor4ub(43, 45, 66, 255); 
        pRenderContext->ClearBuffers(true, true);
    }
    
    g_pMaterialSystem->EndFrame();
    g_pMaterialSystem->SwapBuffers();
}

void QModelView2::paintEvent(QPaintEvent *event)
{
    RenderEngineFrame();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = rect().width();
    int h = rect().height();
    painter.fillRect(rect(), QColor(43, 45, 66));

    float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
    float radY = qDegreesToRadians((float)m_ptRotationAngle.y());

    auto Project3DPointTo2D = [&](const QVector3D &vertex) -> QPointF {
        float x1 = vertex.x();
        float y1 = vertex.y() * qCos(radX) - vertex.z() * qSin(radX);
        float z1 = vertex.y() * qSin(radX) + vertex.z() * qCos(radX);

        float x2 = x1 * qCos(radY) + z1 * qSin(radY);
        float y2 = y1;

        // Visual layout anchor multiplier zoom scaling configurations
        float screenX = (w / 2.0f) + (x2 * m_flZoomScale * 3.5f);
        float screenY = (h / 2.0f) + (y2 * m_flZoomScale * 3.5f);
        return QPointF(screenX, screenY);
    };

    if (!m_szCurrentModelPath.isEmpty() && m_MeshVertices.size() >= 2) {
        // DRAW SKELETON: Render the extracted skeletal links
        painter.setPen(QPen(QColor(0, 180, 216), 2.0f, Qt::SolidLine)); 
        for (const auto &edge : m_MeshEdges) {
            if (edge.v1 < m_MeshVertices.size() && edge.v2 < m_MeshVertices.size()) {
                QPointF p1 = Project3DPointTo2D(m_MeshVertices[edge.v1].position);
                QPointF p2 = Project3DPointTo2D(m_MeshVertices[edge.v2].position);
                painter.drawLine(p1, p2);
            }
        }

        // Draw bone joint dots
        painter.setBrush(QColor(241, 91, 181)); 
        painter.setPen(Qt::NoPen);
        for (int i = 2; i < m_MeshVertices.size(); ++i) {
            QPointF p = Project3DPointTo2D(m_MeshVertices[i].position);
            painter.drawEllipse(p, 3, 3);
        }

        // DRAW BOUNDING BOX FRAMEWORK
        painter.setPen(QPen(QColor(255, 159, 67, 120), 1.5f, Qt::DashLine)); 
        QVector3D modelMin = m_MeshVertices[0].position;
        QVector3D modelMax = m_MeshVertices[1].position;

        QPointF b000 = Project3DPointTo2D(QVector3D(modelMin.x(), modelMin.y(), modelMin.z()));
        QPointF b100 = Project3DPointTo2D(QVector3D(modelMax.x(), modelMin.y(), modelMin.z()));
        QPointF b010 = Project3DPointTo2D(QVector3D(modelMin.x(), modelMax.y(), modelMin.z()));
        QPointF b110 = Project3DPointTo2D(QVector3D(modelMax.x(), modelMax.y(), modelMin.z()));
        QPointF b001 = Project3DPointTo2D(QVector3D(modelMin.x(), modelMin.y(), modelMax.z()));
        QPointF b101 = Project3DPointTo2D(QVector3D(modelMax.x(), modelMin.y(), modelMax.z()));
        QPointF b011 = Project3DPointTo2D(QVector3D(modelMin.x(), modelMax.y(), modelMax.z()));
        QPointF b111 = Project3DPointTo2D(QVector3D(modelMax.x(), modelMax.y(), modelMax.z()));

        painter.drawLine(b000, b100); painter.drawLine(b100, b110); painter.drawLine(b110, b010); painter.drawLine(b010, b000);
        painter.drawLine(b001, b101); painter.drawLine(b101, b111); painter.drawLine(b111, b011); painter.drawLine(b011, b001);
        painter.drawLine(b000, b001); painter.drawLine(b100, b101); painter.drawLine(b110, b111); painter.drawLine(b010, b011);
    }

    // Top UI Panel Status Output Text overlays
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(15, 25, m_szCurrentModelPath.isEmpty() ? "StudioModel View: IDLE" : "StudioModel View: VPK LIVE DIRECTORY STREAM");
    
    painter.setFont(QFont("Courier New", 9));
    painter.setPen(QColor(200, 214, 229));
    if (!m_szCurrentModelPath.isEmpty()) {
        painter.drawText(15, 45, QString("Header Name : %1").arg(m_szModelNameHeader.left(32)));
        painter.drawText(15, 60, QString("Bones Linked: %1 parsed").arg(m_nNumBonesParsed));
        painter.drawText(15, 75, QString("Body Parts  : %1 sections").arg(m_nNumTrianglesParsed));
    }
}

void QModelView2::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bIsDragging = true;
        m_ptLastMousePosition = event->pos();
    }
}

void QModelView2::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bIsDragging && (event->buttons() & Qt::LeftButton)) {
        QPoint delta = event->pos() - m_ptLastMousePosition;
        m_ptLastMousePosition = event->pos();

        m_ptRotationAngle.setX((m_ptRotationAngle.x() - delta.y()) % 360);
        m_ptRotationAngle.setY((m_ptRotationAngle.y() + delta.x()) % 360);
        this->update();
    }
}

void QModelView2::wheelEvent(QWheelEvent *event)
{
    float numDegrees = event->angleDelta().y() / 8.0f;
    float numSteps = numDegrees / 15.0f;

    m_flZoomScale += numSteps * 0.15f;
    if (m_flZoomScale < 0.05f) m_flZoomScale = 0.05f;
    if (m_flZoomScale > 20.0f) m_flZoomScale = 20.0f;

    this->update();
}

void QModelView2::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    this->update();
}
