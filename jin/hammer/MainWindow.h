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
#include <QString>
#include <QToolBar>
#include <QComboBox>
#include "mathlib/vector.h"

#include "widgets/ModelView.h"

// Unified Edit Tool Enum matching traditional Valve Hammer operations
enum EditTool {
    TOOL_SELECT,
    TOOL_BLOCK,
    TOOL_ENTITY
};

struct MapBrush {
    int id;
    Vector mins;
    Vector maxs;
    QColor color;
};

// New structure tracking Point Entities positions and target classnames
struct MapEntity {
    int id;
    QString classname;
    Vector origin;
    QColor color;
};

// REMOVED: ModelViewEntity has been deleted from here since it is already defined in widgets/ModelView4.h

//-----------------------------------------------------------------------------
// Hammer2DGridView: Multi-projection orthographic editor canvas
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// MainWindow Layout Structure
//-----------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool loadFromVMF(const QString& filePath);
    bool saveToVMF(const QString& filePath);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onBrushCreated(const Vector& mins, const Vector& maxs);
    void onBrushSelected(int brushId);
    void onBrushMoved(const Vector& delta3D);
    void onBrushResized(int brushId, const Vector& newMins, const Vector& newMaxs);
    void onEntityPlaced(const Vector& origin, const QString& classname);
    
    void triggerOpenDialog();
    void triggerSaveDialog();
    
    void onToolChanged(EditTool tool);
    void onEntityClassChanged(const QString& classname);

private:
    QGraphicsScene *m_pGridScene;
    QVector<Hammer2DGridView*> m_views;
    QModelView *m_p3DViewport; 
    
    QVector<MapBrush> m_mapBrushes;
    QVector<MapEntity> m_mapEntities;
    int m_nextBrushId;
    int m_nextEntityId;
    int m_selectedBrushId;

    EditTool m_activeTool;
    QString m_currentEntityClass;
    QComboBox *m_pEntityClassCombo;
    QActionGroup *m_pToolActionGroup;

    void createMenuBarActions();
    void createSidebarToolbox();
    void generateMockBrushes();
    void deleteSelectedBrush();
    void syncAllViews();
};
