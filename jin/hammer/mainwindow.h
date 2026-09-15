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
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSlider>
#include <QPushButton>

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
    void createStatusBar();

    QMenuBar *m_menuBar;
    QToolBar *m_mainToolBar;
    QToolBar *m_viewToolBar;
    QStatusBar *m_statusBar;

    QModelView3 *m_pModelViewer;
    QTreeWidget *m_pModelTreeWidget;

    // Utility functions to parse virtual VPK index pathways
    void PopulateModelTreeFromVPK();
    void AddVirtualPathToTree(const QString &szVirtualPath);
};

#endif // MAINWINDOW_H