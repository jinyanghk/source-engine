#include "mainwindow.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "dialogs/FaceEditSheet.h"
#include "dialogs/RunMapNormal.h"
#include "widgets/PrefSlider.h"

using namespace ui;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Hammer Qt6");
    resize(1200, 800);

    createMenuBar();
    createToolBars();
    createDockWidgets();
    createCentralWidget();
    createStatusBar();
    //applyStyleSheet();
}

MainWindow::~MainWindow() {}

void MainWindow::createMenuBar()
{
    m_menuBar = menuBar();

    // File Menu
    QMenu *fileMenu = m_menuBar->addMenu("File");
    fileMenu->addAction("New");
    fileMenu->addAction("Open");
    fileMenu->addAction("Save");
    fileMenu->addAction("Save As");
    fileMenu->addSeparator();
    fileMenu->addAction("Export");
    fileMenu->addSeparator();
    fileMenu->addAction("Quit", this, &QWidget::close);

    // Edit menu
    QMenu *editMenu = m_menuBar->addMenu("Edit");
    editMenu->addAction("Undo");
    editMenu->addAction("Redo");
    editMenu->addSeparator();
    editMenu->addAction("Cut");
    editMenu->addAction("Copy");
    editMenu->addAction("Paste");
    editMenu->addAction("Delete");
    editMenu->addSeparator();
    editMenu->addAction("Select All");
    editMenu->addAction("Find");

    // Map menu
    QMenu *mapMenu = m_menuBar->addMenu("Map");
    mapMenu->addAction("Entity");
    mapMenu->addAction("Texture");
    mapMenu->addAction("Displacement");
    mapMenu->addSeparator();

    QAction *faceEditAction = new QAction("Face Edit", this);
    connect(faceEditAction, &QAction::triggered, this, [this]()
            {
		ui::CFaceEditSheet dialog(this);
		dialog.exec(); });
    mapMenu->addAction(faceEditAction);

    QAction *runMapAction = new QAction("Run Map", this);
    connect(runMapAction, &QAction::triggered, this, [this]()
            {
		ui::CRunMapNormal dialog(this);
		dialog.exec(); });
    mapMenu->addAction(runMapAction);

    // View Menu
    QMenu *viewMenu = m_menuBar->addMenu("View");
    viewMenu->addAction("3D View");
    viewMenu->addAction("Top View");
    viewMenu->addAction("Front View");
    viewMenu->addAction("Side View");
    viewMenu->addSeparator();
    viewMenu->addAction("Zoom OUt");
    viewMenu->addAction("Zoom In");
    viewMenu->addAction("Fit View");

    // Tools menu
    QMenu *toolsMenu = m_menuBar->addMenu("Tools");
    toolsMenu->addAction("Select");
    toolsMenu->addAction("Move");
    toolsMenu->addAction("Rotate");
    toolsMenu->addAction("Scale");
    toolsMenu->addAction("Cut");
    toolsMenu->addSeparator();
    toolsMenu->addAction("Option...");

    // Instancing menu
    QMenu *instanceMenu = m_menuBar->addMenu("Instancing");

    // Instancing menu
    QMenu *windowMenu = m_menuBar->addMenu("Window");

    // Help menu
    QMenu *helpMenu = m_menuBar->addMenu("Help");
    helpMenu->addAction("About");
}

void MainWindow::createToolBars()
{
    // Main toolbar
    m_mainToolBar = new QToolBar("Main Toolbar", this);
    m_mainToolBar->setOrientation(Qt::Vertical); 
    m_mainToolBar->setIconSize(QSize(32, 32));
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QStringList mainTools = {"Select", "Move", "Rotate", "Scale", "Texture"};
    for (const QString &text : mainTools)
    {
        m_mainToolBar->addAction(text);
    }
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction("Run");
    m_mainToolBar->addAction("Option");

    addToolBar(Qt::LeftToolBarArea, m_mainToolBar);

    // view control toolbar
    m_viewToolBar = new QToolBar("View Control", this);
    m_viewToolBar->setIconSize(QSize(20, 20));
    QStringList viewTools = {"Top", "Front", "Side", "3D", "Fit"};
    for (const QString &text : viewTools)
    {
        m_viewToolBar->addAction(text);
    }
    addToolBar(Qt::TopToolBarArea, m_viewToolBar);
    m_viewToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
}

void MainWindow::createDockWidgets()
{
    // 1. Objects
    m_hierarchyDock = new QDockWidget("Objects", this);
    m_hierarchyDock->setAllowedAreas(/*Qt::LeftDockWidgetArea | */Qt::RightDockWidgetArea);
    //m_hierarchyDock->setFeatures(QDockWidget::DockWidgetMovable);
    //m_hierarchyDock->setFeatures(m_hierarchyDock->features() & ~QDockWidget::DockWidgetClosable);
    //m_hierarchyDock->setFeatures(m_hierarchyDock->features() & ~QDockWidget::DockWidgetFloatable);

    QTreeView *hierarchyTree = new QTreeView();
    QStandardItemModel *model = new QStandardItemModel();
    model->setHorizontalHeaderLabels({"Objects"});

    QStandardItem *rootItem = model->invisibleRootItem();
    QStringList items = {"worldspawn", "func_detail", "Solid (Vertex)", "Solid Entity"};
    for (const QString &name : items)
    {
        QStandardItem *item = new QStandardItem(name);
        item->appendRow(new QStandardItem("  " + name + "_child"));
        rootItem->appendRow(item);
    }
    hierarchyTree->setModel(model);
    m_hierarchyDock->setWidget(hierarchyTree);
    addDockWidget(Qt::RightDockWidgetArea, m_hierarchyDock);

    // 2. Texture Browser
    m_textureDock = new QDockWidget("Texture Browser", this);
    m_textureDock->setAllowedAreas(/*Qt::LeftDockWidgetArea | */Qt::RightDockWidgetArea);

    QListWidget *textureList = new QListWidget();
    QStringList textures = {"brick_001", "concrete_01", "metal_plate", "wood_floor", "skybox_01"};
    textureList->addItems(textures);
    m_textureDock->setWidget(textureList);
    addDockWidget(Qt::RightDockWidgetArea, m_textureDock);

    // 3. Property
    m_propertyDock = new QDockWidget("Property", this);
    m_propertyDock->setAllowedAreas(Qt::RightDockWidgetArea);

    QTableWidget *propTable = new QTableWidget(5, 2);
    propTable->setHorizontalHeaderLabels({"Property", "Value"});
    propTable->setAlternatingRowColors(true);

    QList<QPair<QString, QString>> props = {
        {"class", "worldspawn"}, {"origin", "0 0 0"}, {"angles", "0 0 0"}, {"scale", "1.0"}, {"model", "none"}};
    for (int i = 0; i < props.size(); ++i)
    {
        propTable->setItem(i, 0, new QTableWidgetItem(props[i].first));
        propTable->setItem(i, 1, new QTableWidgetItem(props[i].second));
    }
    m_propertyDock->setWidget(propTable);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);

    // 4. console
    //m_consoleDock = new QDockWidget("console", this);
    //m_consoleDock->setAllowedAreas(Qt::BottomDockWidgetArea);

    //QTextEdit *console = new QTextEdit();
    //console->setReadOnly(true);
    //console->append("Hammer Started.");
    //console->append("> Ready.");
    //m_consoleDock->setWidget(console);
    //addDockWidget(Qt::BottomDockWidgetArea, m_consoleDock);
}

void MainWindow::createCentralWidget()
{
    // central widget, use QVBoxLayout to hold splitters
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    // ---- create 4 views one 3D view and 3 2D views ----
    auto createViewLabel = [](const QString &text) -> QLabel *
    {
        QLabel *label = new QLabel(text);
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumSize(100, 80); // min size
        label->setStyleSheet(
            "background-color: #1a1a1a; "
            "border: 1px solid #444; "
            "color: #888; "
            "font-size: 16px; "
            "font-weight: bold;");
        return label;
    };

    m_view3D = createViewLabel("3D View");
    m_viewTop = createViewLabel("Top View (XZ)");
    m_viewFront = createViewLabel("Front View (XY)");
    m_viewSide = createViewLabel("Side View (YZ)");

    // ---- construct split view ----
    // 1. lefe side view split: 3D (top) + Top View (bottom)
    m_leftSplitter = new QSplitter(Qt::Vertical);
    m_leftSplitter->addWidget(m_view3D);
    m_leftSplitter->addWidget(m_viewTop);
    m_leftSplitter->setSizes({300, 200}); // init size

    // 2. righ side view split: Front View (top) + Side View (bottom)
    m_rightSplitter = new QSplitter(Qt::Vertical);
    m_rightSplitter->addWidget(m_viewFront);
    m_rightSplitter->addWidget(m_viewSide);
    m_rightSplitter->setSizes({300, 200});

    // 3. horizontal split: left side view + right side view
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->addWidget(m_leftSplitter);
    m_mainSplitter->addWidget(m_rightSplitter);
    m_mainSplitter->setSizes({400, 400}); // init size

    // sync left right splitter
    QObject::connect(m_leftSplitter, &QSplitter::splitterMoved,
                     [this](int pos, int index)
                     {
                         QList<int> sizes = m_rightSplitter->sizes();
                         int total = sizes[0] + sizes[1];

                         QList<int> leftSizes = m_leftSplitter->sizes();
                         int leftTotal = leftSizes[0] + leftSizes[1];

                         if (leftTotal > 0)
                         {
                             int newSize = (leftSizes[0] * total) / leftTotal;
                             m_rightSplitter->setSizes({newSize, total - newSize});
                         }
                     });

    // sync left right splitter
    connect(m_rightSplitter, &QSplitter::splitterMoved,
            [this](int pos, int index)
            {
                QList<int> sizes = m_leftSplitter->sizes();
                int total = sizes[0] + sizes[1];

                QList<int> rightSizes = m_rightSplitter->sizes();
                int rightTotal = rightSizes[0] + rightSizes[1];

                if (rightTotal > 0)
                {
                    int newSize = (rightSizes[0] * total) / rightTotal;
                    m_leftSplitter->setSizes({newSize, total - newSize});
                }
            });

    // add main splitter
    mainLayout->addWidget(m_mainSplitter);

    // set central window
    setCentralWidget(centralWidget);

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
}

void MainWindow::createStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage("Ready | Selected: 0 Object");

    QLabel *gridLabel = new QLabel("Grid: 64");
    QLabel *sepLabel = new QLabel(" | ");
    QLabel *textureLabel = new QLabel("Texture: default");

    m_statusBar->addPermanentWidget(gridLabel);
    m_statusBar->addPermanentWidget(sepLabel);
    m_statusBar->addPermanentWidget(textureLabel);
}

void MainWindow::applyStyleSheet()
{
    // apply dark style
    setStyleSheet(
        "QMainWindow { background-color: #2b2b2b; }"
        "QMenuBar { background-color: #3c3c3c; color: #eee; }"
        "QMenuBar::item:selected { background-color: #5a5a5a; }"
        "QToolBar { background-color: #3c3c3c; border: none; spacing: 3px; }"
        "QStatusBar { background-color: #3c3c3c; color: #bbb; }"
        "QDockWidget { titlebar-close-icon: url(none); }"
        "QDockWidget::title { background-color: #3c3c3c; color: #ddd; }"
        "QTreeView, QListView, QTableWidget { background-color: #1e1e1e; color: #ddd; alternate-background-color: #2a2a2a; }"
        "QTabWidget::pane { background-color: #2b2b2b; border: 1px solid #444; }"
        "QTabBar::tab { background-color: #3c3c3c; color: #bbb; padding: 5px 10px; }"
        "QTabBar::tab:selected { background-color: #4a4a4a; color: white; }"
        "QPushButton { background-color: #4a4a4a; color: #eee; border: 1px solid #555; padding: 5px; }"
        "QPushButton:hover { background-color: #5a5a5a; }"
        "QLineEdit, QComboBox, QSpinBox { background-color: #1e1e1e; color: #ddd; border: 1px solid #444; padding: 3px; }"
        "QLabel { color: #ddd; }");
}