#ifndef MAPVIEW_H
#define MAPVIEW_H

#pragma once

#include <QWidget>
#include <QPoint>
#include <QPointF>
#include <QList>
#include <QColor>
#include <QPolygonF>

// Lightweight internal structure to store raw 3D vectors without external engine dependencies
struct MapVector_t {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Structural blueprint to hold individual parsed face fragments for rendering
struct ParsedFace_t {
    QPolygonF poly;
    QColor color;
    float avgDepth = 0.0f;
};

class QMapView : public QWidget
{
    Q_OBJECT

public:
    explicit QMapView(QWidget *parent = nullptr);
    virtual ~QMapView();

    // High-performance direct path file loader to clear previous data and stream map nodes
    void LoadVMFFile(const QString &szFilePath);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QString m_szLoadedFilePath;
    QList<ParsedFace_t> m_CompiledFaces;

    // Viewport camera tracking metrics
    float   m_flZoomScale;
    QPoint  m_ptRotationAngle;
    QPoint  m_ptLastMousePosition;
    QPointF m_ptCameraPanOffset;

    // Direct ASCII token parsing runtime
    void ParseVMFTextStream();
};

#endif // MAPVIEW_H
