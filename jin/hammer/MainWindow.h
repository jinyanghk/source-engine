#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QSplitter>
#include <QComboBox>
#include <QActionGroup>
#include <QAction>
#include <QList>

#include "mathlib/vector.h"
#include "MapDocument.h"
#include "HammerUITypes.h"

// Forward declarations of our clean UI components
class Hammer2DGridView;
class Hammer3DView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onBrushCreated(const Vector &mins, const Vector &maxs);
    void onBrushSelected(int brushId);
    void onBrushMoved(const Vector &delta3D);
    void onBrushResized(int brushId, const Vector &newMins, const Vector &newMaxs);
    void onEntityPlaced(const Vector &origin, const QString &classname);
    
    void triggerOpenDialog();
    void triggerSaveDialog();
    
    void onToolChanged(EditTool tool);
    void onEntityClassChanged(const QString &classname);

    void toggleViewModeSingle3D(bool checked);
    void toggleViewModeSplit4Way(bool checked);

private:
    QGraphicsScene *m_pGridScene;
    QVector<Hammer2DGridView*> m_views;
    Hammer3DView *m_p3DViewport; // NEW: Pointing to our animation-free viewport panel
    
    QVector<MapBrush> m_mapBrushes;
    QVector<MapEntity> m_mapEntities;
    int m_nextBrushId;
    int m_nextEntityId;
    int m_selectedBrushId;

    EditTool m_activeTool;
    QString m_currentEntityClass;
    QComboBox *m_pEntityClassCombo;
    QActionGroup *m_pToolActionGroup;

    QSplitter *m_hMainSplitter;
    QAction *m_pActView3D;
    QAction *m_pActView4Way;
    bool m_bIsSingle3DMode;

    QList<int> m_cachedLeftSizes;
    QList<int> m_cachedRightSizes;

    MapDocument m_document;

    void createViewMenuActions();
    void createMenuBarActions();
    void createSidebarToolbox();
    void generateMockBrushes();
    void deleteSelectedBrush();
    void syncAllViews();
};
