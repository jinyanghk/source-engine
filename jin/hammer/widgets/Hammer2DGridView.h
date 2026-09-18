#pragma once

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QVector>
#include <QRectF>
#include <QPointF>
#include "HammerUITypes.h"

class Hammer2DGridView : public QGraphicsView
{
    Q_OBJECT
public:
    enum ViewOrientation { VIEW_TOP, VIEW_FRONT, VIEW_SIDE };

    explicit Hammer2DGridView(ViewOrientation orientation, QWidget *parent = nullptr);
    void setGridSize(int size);
    int gridSize() const { return m_gridSize; }
    
    void updateSceneData(const QVector<MapBrush>& brushes, const QVector<MapEntity>& entities, int selectedBrushId, EditTool activeTool, const QString& entityClass);
    ViewOrientation orientation() const { return m_orientation; }

signals:
    void brushCreated(const Vector& mins, const Vector& maxs);
    void brushSelected(int brushId);
    void brushMoved(const Vector& delta3D);
    void brushResized(int brushId, const Vector& newMins, const Vector& newMaxs);
    void entityPlaced(const Vector& origin, const QString& classname);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum EditMode { MODE_NONE, MODE_DRAWING, MODE_TRANSLATING, MODE_RESIZING };
    enum HandleIndex { HANDLE_NONE = -1, HANDLE_TOP_LEFT, HANDLE_TOP, HANDLE_TOP_RIGHT, HANDLE_RIGHT, HANDLE_BOTTOM_RIGHT, HANDLE_BOTTOM, HANDLE_BOTTOM_LEFT, HANDLE_LEFT };

    ViewOrientation m_orientation;
    int m_gridSize;
    bool m_isPanning;
    EditMode m_editMode;
    QPoint m_lastMousePos;
    QPointF m_drawStartScene;
    QPointF m_drawCurrentScene;
    int m_selectedBrushId;
    QPointF m_translateStartScene;
    QPointF m_translateCurrentScene;
    HandleIndex m_activeHandle;
    QRectF m_selectedBrushRect;
    QRectF m_handleRects[8];

    QVector<MapBrush> m_brushes;
    QVector<MapEntity> m_entities;
    EditTool m_activeTool;
    QString m_currentEntityClass;

    void updateHandlePositions();
    HandleIndex hitTestHandles(const QPointF& scenePos);
    void updateCursorForHandle(HandleIndex handle);
    int getBrushIdAtPosition(const QPointF& scenePos);

public:
    void projectTo2D(const Vector& mins, const Vector& maxs, qreal& x, qreal& y, qreal& w, qreal& h);
    void unprojectFrom2D(qreal x, qreal y, qreal w, qreal h, Vector& targetMins, Vector& targetMaxs);
    void convertDeltaTo3D(const QPointF& delta2D, Vector& outDelta3D);
    float snapToGrid(float value) const;
    QPointF snapToGrid(const QPointF& scenePos) const;
};
