#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QTreeView>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QSplitter>

#include "widgets/HammerViewportWidget.h"
#include "widgets/MaterialPreview.h"
#include "widgets/ModelView3.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createMenuBar();
    void createToolBars();
    void createDockWidgets();
    void createCentralWidget();
    void createStatusBar();
    void applyStyleSheet();

    QMenuBar *m_menuBar;
    QToolBar *m_mainToolBar;
    QToolBar *m_viewToolBar;
    QStatusBar *m_statusBar;

    QDockWidget *m_hierarchyDock;
    QDockWidget *m_textureDock;
    QDockWidget *m_propertyDock;
    QDockWidget *m_consoleDock;

    QWidget *m_centralWidget;
    QGridLayout *m_viewLayout;
    QLabel *m_view3D;
    QLabel *m_viewTop;
    QLabel *m_viewFront;
    QLabel *m_viewSide;

    QSplitter *m_mainSplitter;
    QSplitter *m_leftSplitter;
    QSplitter *m_rightSplitter;

    QModelView3 *m_modelView;
};

#endif // MAINWINDOW_H