#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QVector3D>
#include <QVector>

class QModelView : public QWidget
{
    Q_OBJECT

public:
    explicit QModelView(QWidget *parent = nullptr);
    virtual ~QModelView();

    // Call this function when an asset is selected in the sidebar file tree
    void LoadModelFile(const QString &szFilePath);

protected:
    // Native Qt drawing and mouse viewport manipulation handlers
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void RenderEngineFrame(); // Safely pumps the engine's caching timeline tickers
    void GenerateMockWireframeMesh(); // Builds a clean fallback preview mesh based on parsed dimensions

    QString m_szCurrentModelPath;
    QString m_szModelNameHeader;
    int m_nNumBonesParsed;
    int m_nNumTrianglesParsed;

    // Viewport layout tracking variables (Panning & Zoom limits)
    float m_flZoomScale;
    QPoint m_ptRotationAngle;
    QPoint m_ptLastMousePosition;
    bool m_bIsDragging;

    // Lightweight mock structure for wireframe visualization tracking
    struct WireframeVertex_t {
        QVector3D position;
    };
    struct WireframeEdge_t {
        int v1, v2;
    };

    QVector<WireframeVertex_t> m_MeshVertices;
    QVector<WireframeEdge_t>   m_MeshEdges;
};
