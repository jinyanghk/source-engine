#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QVector3D>
#include <QVector>

class QModelView2 : public QWidget
{
    Q_OBJECT

public:
    explicit QModelView2(QWidget *parent = nullptr);
    virtual ~QModelView2();

    void LoadModelFile(const QString &szFilePath);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void RenderEngineFrame();

    QString m_szCurrentModelPath;
    QString m_szModelNameHeader;
    int     m_nNumBonesParsed;
    int     m_nNumTrianglesParsed;

    // Viewport camera tracking metrics
    float   m_flZoomScale;
    QPoint  m_ptRotationAngle;
    QPoint  m_ptLastMousePosition;
    bool    m_bIsDragging;

    // Structural vertex node trackers for thread-safe UI projection drawing
    struct BoneVertex_t {
        QVector3D position;
    };
    struct BoneEdge_t {
        int v1, v2;
    };

    QVector<BoneVertex_t> m_MeshVertices;
    QVector<BoneEdge_t>   m_MeshEdges;
};
