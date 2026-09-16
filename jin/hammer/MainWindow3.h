#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QVector>
#include <QPen>
#include <QBrush>
#include <QSplitter>
#include <QLabel>
#include <QVBoxLayout>
#include "mathlib/vector.h"

#include "widgets/ModelView4.h"

struct MapBrush {
    int id;
    Vector mins;
    Vector maxs;
    QColor color;
};

//-----------------------------------------------------------------------------
// Hammer2DGridView: Multi-projection orthographic editor canvas
//-----------------------------------------------------------------------------
class Hammer2DGridView : public QGraphicsView
{
    Q_OBJECT
public:
    enum ViewOrientation {
        VIEW_TOP,   // XY
        VIEW_FRONT, // XZ
        VIEW_SIDE   // YZ
    };

    explicit Hammer2DGridView(ViewOrientation orientation, QWidget *parent = nullptr);
    
    void setGridSize(int size);
    int gridSize() const { return m_gridSize; }
    void updateBrushes(const QVector<MapBrush>& brushes, int selectedId);
    ViewOrientation orientation() const { return m_orientation; }

signals:
    void brushCreated(const Vector& mins, const Vector& maxs);
    void brushSelected(int brushId);
    void brushMoved(const Vector& delta3D);
    void brushResized(int brushId, const Vector& newMins, const Vector& newMaxs);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    
    // Explicit painter logic drawing map volumes uniquely per window context
    void paintEvent(QPaintEvent *event) override;
    
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    enum EditMode {
        MODE_NONE,
        MODE_DRAWING,
        MODE_TRANSLATING,
        MODE_RESIZING
    };

    enum HandleIndex {
        HANDLE_NONE = -1,
        HANDLE_TOP_LEFT, HANDLE_TOP, HANDLE_TOP_RIGHT,
        HANDLE_RIGHT,
        HANDLE_BOTTOM_RIGHT, HANDLE_BOTTOM, HANDLE_BOTTOM_LEFT,
        HANDLE_LEFT
    };

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

//-----------------------------------------------------------------------------
// MainWindow3 Layout Structure containing embedded 3D context engine slots
//-----------------------------------------------------------------------------
class MainWindow3 : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow3(QWidget *parent = nullptr);
    ~MainWindow3();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onBrushCreated(const Vector& mins, const Vector& maxs);
    void onBrushSelected(int brushId);
    void onBrushMoved(const Vector& delta3D);
    void onBrushResized(int brushId, const Vector& newMins, const Vector& newMaxs);

private:
    QGraphicsScene *m_pGridScene;
    QVector<Hammer2DGridView*> m_views;
    
    // Live 3D engine viewport container
    QModelView4 *m_p3DViewport; 
    
    QVector<MapBrush> m_mapBrushes;
    int m_nextBrushId;
    int m_selectedBrushId;

    void generateMockBrushes();
    void deleteSelectedBrush();
    void syncAllViews();
};
