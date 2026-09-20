#include "MapView.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QFile>
#include <QTextStream>
#include <algorithm>

QMapView::QMapView(QWidget *parent)
    : QWidget(parent), m_flZoomScale(1.5f), m_ptCameraPanOffset(QPointF(0, 0))
{
    // Configure natural 3D isometric look-at angles matching classic Hammer viewports
    m_ptRotationAngle = QPoint(25, -45); 
}

QMapView::~QMapView() {}

void QMapView::LoadVMFFile(const QString &szFilePath)
{
    m_szLoadedFilePath = szFilePath;
    ParseVMFTextStream();
    this->update(); // Request immediate repaint sweep
}

// ---- HIGH-PERFORMANCE DIRECT TEXT-BASED VMF PLANE SCANNER ENGINE ----
void QMapView::ParseVMFTextStream()
{
    m_CompiledFaces.clear();

    if (m_szLoadedFilePath.isEmpty())
        return;

    QFile file(m_szLoadedFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    // Baseline viewport parameters to satisfy lambda setup bindings
    float radX = qDegreesToRadians((float)m_ptRotationAngle.x());
    float radY = qDegreesToRadians((float)m_ptRotationAngle.y());
    int w = qMax(64, rect().width());
    int h = qMax(64, rect().height());

    // Unified 3D Screen Space projection utility
    auto Project3DNode = [this, w, h, radX, radY](float x, float y, float z, float &outDepth) -> QPointF
    {
        float rotatedX = x * qCos(radY) - y * qSin(radY);
        float rotatedY = x * qSin(radY) + y * qCos(radY);

        float finalX = rotatedX;
        float finalY = rotatedY * qCos(radX) - z * qSin(radX);
        outDepth     = rotatedY * qSin(radX) + z * qCos(radX);

        float sX = (w / 2.0f) + (finalX * m_flZoomScale) + m_ptCameraPanOffset.x();
        float sY = (h / 2.0f) - (finalY * m_flZoomScale) + m_ptCameraPanOffset.y();
        return QPointF(sX, sY);
    };

    // Directional studio light setup
    float lx = 0.4f, ly = -0.5f, lz = 0.7f;
    float lLen = qSqrt(lx*lx + ly*ly + lz*lz);
    lx /= lLen; ly /= lLen; lz /= lLen;

    // Loop through map syntax tokens line-by-line
    while (!stream.atEnd())
    {
        QString line = stream.readLine().trimmed();

        // Detect raw plane definitions inside brush structural nodes
        if (line.startsWith("\"plane\""))
        {
            // Typical format schema: "plane" "(0 0 64) (128 0 64) (128 128 64)"
            int firstParen = line.indexOf('(');
            if (firstParen == -1) continue;

            QString cleanCoordinates = line.mid(firstParen).replace("\"", "");
            QStringList pointTokens = cleanCoordinates.split(')', Qt::SkipEmptyParts);

            if (pointTokens.size() >= 3)
            {
                QPolygonF tri;
                float cumulativeDepth = 0.0f;
                MapVector_t vertices[3]; // Fixed scalar structure sizing layout
                bool bValidPoints = true;

                for (int i = 0; i < 3; ++i)
                {
                    QString pStr = pointTokens[i].mid(pointTokens[i].indexOf('(') + 1).trimmed();
                    QStringList axisValues = pStr.split(' ', Qt::SkipEmptyParts);
                    
                    if (axisValues.size() == 3)
                    {
                        vertices[i].x = axisValues[0].toFloat();
                        vertices[i].y = axisValues[1].toFloat();
                        vertices[i].z = axisValues[2].toFloat();

                        float nodeDepth = 0.0f;
                        tri << Project3DNode(vertices[i].x, vertices[i].y, vertices[i].z, nodeDepth);
                        cumulativeDepth += nodeDepth;
                    }
                    else
                    {
                        bValidPoints = false;
                    }
                }

                if (!bValidPoints) continue;

                // ---- REAL-TIME CPU LAMBERTIAN LIGHT RESOLVER ----
                // FIX: Restored zero-based scalar indices explicitly, [1], and [2] to prevent pointer overruns
                float e1x = vertices[1].x - vertices[0].x;
                float e1y = vertices[1].y - vertices[0].y;
                float e1z = vertices[1].z - vertices[0].z;

                float e2x = vertices[2].x - vertices[0].x;
                float e2y = vertices[2].y - vertices[0].y;
                float e2z = vertices[2].z - vertices[0].z;

                // Perform cross-product equations to resolve the true surface normal vector
                float nx = e1y * e2z - e1z * e2y;
                float ny = e1z * e2x - e1x * e2z;
                float nz = e1x * e2y - e1y * e2x;
                float nLen = qSqrt(nx*nx + ny*ny + nz*nz);
                if (nLen > 0.001f) { nx /= nLen; ny /= nLen; nz /= nLen; }

                // Compute ambient-diffuse lighting balance ratios
                float dotProduct = nx * lx + ny * ly + nz * lz;
                float lightIntensity = qMax(0.0f, dotProduct) * 0.65f + 0.35f; // 35% global fill light

                // Apply dynamic lighting to classic Hammer dev-orange layout tones
                QColor baseColor(225, 145, 90); 
                QColor shadedColor(
                    qBound(0, (int)(baseColor.red() * lightIntensity), 255),
                    qBound(0, (int)(baseColor.green() * lightIntensity), 255),
                    qBound(0, (int)(baseColor.blue() * lightIntensity), 255)
                );

                ParsedFace_t faceItem;
                faceItem.poly = tri;
                faceItem.color = shadedColor;
                faceItem.avgDepth = cumulativeDepth / 3.0f;

                m_CompiledFaces.append(faceItem);
            }
        }
    }
    file.close();
}

void QMapView::paintEvent(QPaintEvent *event)
{
    // Re-parse strings during viewing angle transformations to keep screen coordinates synced
    ParseVMFTextStream();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Deep design-grid charcoal theme backdrop
    painter.fillRect(rect(), QColor(32, 34, 42));

    if (m_CompiledFaces.isEmpty())
    {
        painter.setPen(QColor(140, 145, 160));
        painter.setFont(QFont("Arial", 10));
        painter.drawText(rect(), Qt::AlignCenter, "No Map Geometry Active\n\nGo to File -> Open Map (.vmf) to load and render brush planes.");
        return;
    }

    // Sort collected face polygons from back to front to ensure clean layer rendering
    std::sort(m_CompiledFaces.begin(), m_CompiledFaces.end(), [](const ParsedFace_t &a, const ParsedFace_t &b) {
        return a.avgDepth < b.avgDepth; 
    });

    // Flush polygons directly down onto our widget workspace
    for (const auto &face : m_CompiledFaces)
    {
        painter.setPen(QPen(face.color.darker(120), 0.5f, Qt::SolidLine)); // Natural structural wire edges
        painter.setBrush(face.color);
        painter.drawPolygon(face.poly);
    }

    // Status Banner Overlay
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(15, 25, QString("MapView (Pure Option-A Pipeline) - Polygons Buffered: %1").arg(m_CompiledFaces.size()));
}

void QMapView::mousePressEvent(QMouseEvent *event) { m_ptLastMousePosition = event->pos(); }
void QMapView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF delta = event->position() - m_ptLastMousePosition;
    m_ptLastMousePosition = event->pos();

    if (event->buttons() & Qt::LeftButton)
    {
        // Orbit drag adjustments
        m_ptRotationAngle.setY(m_ptRotationAngle.y() + delta.x() * 0.5f);
        m_ptRotationAngle.setX(m_ptRotationAngle.x() - delta.y() * 0.5f);
        this->update();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        // Viewport panning translations
        m_ptCameraPanOffset.setX(m_ptCameraPanOffset.x() + delta.x());
        m_ptCameraPanOffset.setY(m_ptCameraPanOffset.y() + delta.y());
        this->update();
    }
}
void QMapView::wheelEvent(QWheelEvent *event)
{
    m_flZoomScale += event->angleDelta().y() > 0 ? 0.05f : -0.05f;
    m_flZoomScale = qBound(0.01f, m_flZoomScale, 10.0f);
    this->update();
}
void QMapView::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); }
