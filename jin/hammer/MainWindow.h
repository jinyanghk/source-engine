#pragma once

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QSplitter>
#include <QTreeView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QActionGroup>
#include <QAction>
#include <QList>

#include "mathlib/vector.h"
#include "MapDocument.h"
#include "HammerUITypes.h"
#include "widgets/ModelView.h"

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
    void onEntityClassChanged(const QString &classname); // Re-added to match your cpp slot

    void onEntityTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onEntityFilterChanged(const QString &text);
    void onEntityTreeDoubleClicked(const QModelIndex &index); 

    void toggleViewModeSingle3D(bool checked);
    void toggleViewModeSplit4Way(bool checked);

private:
    QGraphicsScene *m_pGridScene;
    QVector<Hammer2DGridView*> m_views;
    Hammer3DView *m_p3DViewport; 
    
    QVector<MapBrush> m_mapBrushes;
    QVector<MapEntity> m_mapEntities;
    int m_selectedBrushId;

    EditTool m_activeTool;
    QString m_currentEntityClass;

    QTreeView               *m_pEntityTreeView;
    QLineEdit               *m_pEntityFilterEdit;
    QStandardItemModel      *m_pEntityTreeModel;
    QSortFilterProxyModel   *m_pEntityFilterProxyModel;

    QActionGroup *m_pToolActionGroup;
    QSplitter *m_hMainSplitter;
    QAction *m_pActView3D;
    QAction *m_pActView4Way;
    bool m_bIsSingle3DMode;

    // Re-added size tracking caches required by your view toggle methods
    QList<int> m_cachedLeftSizes;
    QList<int> m_cachedRightSizes;

    MapDocument m_document;

    void createViewMenuActions();
    void createMenuBarActions();
    void createSidebarToolbox(); // Kept original name matching your cpp definition
    void createRightEntityBrowser();
    
    void generateMockBrushes();
    void deleteSelectedBrush();
    void syncAllViews();
};
