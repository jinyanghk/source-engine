#include "MainWindow2.h"
#include "widgets/MapView.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow2::MainWindow2(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1280, 720);
    setWindowTitle("Hammer Qt6 - Dedicated Map Geometry View (Option-A Engine)");

    // Instantiate and bind our clean software geometry viewport widget
    m_pMapView3D = new QMapView(this);
    setCentralWidget(m_pMapView3D);

    createMenuBar();
    createToolBars();
    createStatusBar();
    applyStyleSheet();
}

MainWindow2::~MainWindow2() {}

void MainWindow2::createMenuBar()
{
    m_menuBar = menuBar();

    QMenu *fileMenu = m_menuBar->addMenu("File");
    QAction *openAction = fileMenu->addAction("Open Map (.vmf)...");
    fileMenu->addSeparator();
    fileMenu->addAction("Exit Workspace", this, &QWidget::close);

    connect(openAction, &QAction::triggered, this, &MainWindow2::OnOpenMapFileSlot);

    m_menuBar->addMenu("Edit");
    m_menuBar->addMenu("Map");
    m_menuBar->addMenu("View");
    m_menuBar->addMenu("Tools");
    m_menuBar->addMenu("Help");
}

void MainWindow2::OnOpenMapFileSlot()
{
    QString szSelectedFile = QFileDialog::getOpenFileName(
        this, 
        "Load Valve VMF Text Document", 
        "", 
        "Valve Map Format (*.vmf)"
    );

    if (szSelectedFile.isEmpty())
        return;

    // Stream the chosen VMF text document straight down into our clean parser
    m_pMapView3D->LoadVMFFile(szSelectedFile);
    m_statusBar->showMessage(QString("Loaded map node document: %1").arg(szSelectedFile));
}

void MainWindow2::createToolBars()
{
    m_mainToolBar = new QToolBar("Editor Tools", this);
    m_mainToolBar->addAction("Selection");
    m_mainToolBar->addAction("Brush Creator");
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction("Compile Map");
    addToolBar(Qt::LeftToolBarArea, m_mainToolBar);
}

void MainWindow2::createStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage("Ready | Load a .vmf file to parse and intersect map geometry brushes.");
}

void MainWindow2::applyStyleSheet()
{
    setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; }"
        "QMenuBar { background-color: #3c3c3c; color: #eee; }"
        "QMenuBar::item:selected { background-color: #5a5a5a; }"
        "QToolBar { background-color: #3c3c3c; border: none; spacing: 3px; color: #ddd; }"
        "QStatusBar { background-color: #3c3c3c; color: #bbb; }"
        "QLabel { color: #ddd; }");
}
