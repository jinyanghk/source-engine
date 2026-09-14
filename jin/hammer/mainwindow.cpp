#include "mainwindow.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QGridLayout>
#include <QFileDialog>

#include "dialogs/FaceEditSheet.h"
#include "dialogs/RunMapNormal.h"
#include "widgets/EngineView.h"

#include "filesystem.h"        // Canonical g_pFileSystem hooks

extern IFileSystem *g_pFileSystem;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1280, 720);
    setWindowTitle("Hammer Qt6 - Standalone Asset Browser");

    QSplitter *pMainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(pMainSplitter);

    // Left Side: VPK Tree Panel Sidebar
    m_pModelTreeWidget = new QTreeWidget(pMainSplitter);
    m_pModelTreeWidget->setHeaderLabel("Game VPK Model Repository");
    m_pModelTreeWidget->setAnimated(true);
    m_pModelTreeWidget->setSortingEnabled(true);

    // Right Side: Viewport Container Shell Widget
    QWidget *pRightContainerShell = new QWidget(pMainSplitter);
    QVBoxLayout *pRightLayout = new QVBoxLayout(pRightContainerShell);
    pRightLayout->setContentsMargins(4, 4, 4, 4);

    // Instantiate our active animated canvas widget
    m_pModelViewer = new QModelView3(pRightContainerShell);
    pRightLayout->addWidget(m_pModelViewer, 1); // Stretch to fill layout space maps

    // ---- ADVANCED TIMELINE CONTROLS TOOLBAR PANEL ----
    QHBoxLayout *pControlToolbarLayout = new QHBoxLayout();
    pRightLayout->addLayout(pControlToolbarLayout);

    // 1. Sequence Combobox Selector
    QLabel *pSeqLabel = new QLabel("Active Clip:", pRightContainerShell);
    QComboBox *pSeqComboBox = new QComboBox(pRightContainerShell);
    pSeqComboBox->setMinimumWidth(180);
    pControlToolbarLayout->addWidget(pSeqLabel);
    pControlToolbarLayout->addWidget(pSeqComboBox);

    // 2. Play / Pause Button Toggle
    QPushButton *pPlayPauseBtn = new QPushButton("Pause", pRightContainerShell);
    pPlayPauseBtn->setCheckable(true);
    pControlToolbarLayout->addWidget(pPlayPauseBtn);

    // 3. Normalized Scrubbing Timeline Slider
    QSlider *pTimelineSlider = new QSlider(Qt::Horizontal, pRightContainerShell);
    pTimelineSlider->setRange(0, 100); // Maps 0% to 100% of cycle indices
    pControlToolbarLayout->addWidget(pTimelineSlider);

    // Attach panels into the layout structures
    pMainSplitter->addWidget(m_pModelTreeWidget);
    pMainSplitter->addWidget(pRightContainerShell);
    pMainSplitter->setSizes(QList<int>() << 320 << 960);

    // ---- CONNECT INTERACTIVE SIGNALS AND TIMELINE TRACKERS ----
    
    // Automatically populate and refresh the sequence selection list when a new MDL is loaded
    auto RefreshSequenceDropdownList = [=]() {
        pSeqComboBox->clear();
        int seqCount = m_pModelViewer->GetSequenceCount();
        for (int i = 0; i < seqCount; ++i) {
            pSeqComboBox->addItem(m_pModelViewer->GetSequenceName(i));
        }
    };

    connect(m_pModelTreeWidget, &QTreeWidget::itemDoubleClicked, this, [=](QTreeWidgetItem *pItem, int col) {
        QString szModelPath = pItem->data(0, Qt::UserRole).toString();
        if (!szModelPath.isEmpty() && szModelPath.endsWith(".mdl")) {
            m_pModelViewer->LoadModelFile(szModelPath);
            RefreshSequenceDropdownList(); // Pull the new model's animation tags instantly
            pPlayPauseBtn->setChecked(false);
            pPlayPauseBtn->setText("Pause");
            m_pModelViewer->SetPlaybackPaused(false);
        }
    });

    // Handle Active Dropdown Sequence Swapping
    connect(pSeqComboBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        if (index >= 0) {
            m_pModelViewer->SetActiveSequence(index);
        }
    });

    // Handle Playback Pausing State Machine
    connect(pPlayPauseBtn, &QPushButton::toggled, this, [=](bool bIsChecked) {
        pPlayPauseBtn->setText(bIsChecked ? "Play" : "Pause");
        m_pModelViewer->SetPlaybackPaused(bIsChecked);
    });

    // Handle Manual Slider Scrubbing
    connect(pTimelineSlider, &QSlider::sliderMoved, this, [=](int val) {
        // Force playback pause when manual scrubbing occurs to match standard animation editors
        pPlayPauseBtn->setChecked(true);
        pPlayPauseBtn->setText("Play");
        m_pModelViewer->SetPlaybackPaused(true);

        float flTargetPercentage = (float)val / 100.0f;
        m_pModelViewer->SetAnimationCycle(flTargetPercentage);
    });

    // Setup initial workspace populations
    PopulateModelTreeFromVPK();

    // FIX: Trigger a deferred initial refresh execution sweep.
    // This allows the viewport to load models/alyx.mdl by default at startup,
    // and immediately populates the animation clip list so it is interactive on launch!
    QTimer::singleShot(250, this, [=]() {
        RefreshSequenceDropdownList();
        pPlayPauseBtn->setChecked(false);
        pPlayPauseBtn->setText("Pause");
    });
}

// ---- ADVANCED VIRTUAL REPOSITORY TREE PARSER ----
void MainWindow::PopulateModelTreeFromVPK()
{
    if (!g_pFileSystem)
        return;

    m_pModelTreeWidget->clear();
    
    // We query the mounted engine file system cache across all active VPK packages 
    // to discover any compiled .mdl character files hidden in the archives.
    FileFindHandle_t findHandle;
    const char *pFirstFile = g_pFileSystem->FindFirstEx("models/*.mdl", "GAME", &findHandle);
    
    while (pFirstFile != nullptr)
    {
        // Ignore dummy asset signatures or placeholder configurations
        if (pFirstFile[0] != '.' && !QString(pFirstFile).contains("vphysics"))
        {
            // Reconstruct the virtual package lookup key (e.g., "models/player/alyx.mdl")
            QString szAbsoluteVirtualPath = QString("models/%1").arg(pFirstFile);
            AddVirtualPathToTree(szAbsoluteVirtualPath);
        }
        pFirstFile = g_pFileSystem->FindNext(findHandle);
    }
    g_pFileSystem->FindClose(findHandle);
}

// Helper lambda parser to split virtual paths into nested Qt Tree folders cleanly
void MainWindow::AddVirtualPathToTree(const QString &szVirtualPath)
{
    QStringList pathTokens = szVirtualPath.split('/');
    QTreeWidgetItem *pCurrentParentNode = nullptr;

    // Loop through the folders to construct hierarchical tree structures on the fly
    for (int i = 0; i < pathTokens.size(); ++i)
    {
        QString szCurrentToken = pathTokens[i];
        bool bIsFileNode = (i == pathTokens.size() - 1);

        // Check if this directory layer item node already exists under the current parent
        bool bNodeFound = false;
        int childCount = pCurrentParentNode ? pCurrentParentNode->childCount() : m_pModelTreeWidget->topLevelItemCount();
        
        for (int c = 0; c < childCount; ++c)
        {
            QTreeWidgetItem *pChildItem = pCurrentParentNode ? pCurrentParentNode->child(c) : m_pModelTreeWidget->topLevelItem(c);
            if (pChildItem->text(0) == szCurrentToken)
            {
                pCurrentParentNode = pChildItem;
                bNodeFound = true;
                break;
            }
        }

        // If the path directory layer node is brand new, spawn it into the collection list
        if (!bNodeFound)
        {
            QTreeWidgetItem *pNewNode = new QTreeWidgetItem();
            pNewNode->setText(0, szCurrentToken);

            if (bIsFileNode)
            {
                // Attach the full lookup path key as metadata to use when clicked
                pNewNode->setData(0, Qt::UserRole, szVirtualPath);
                pNewNode->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon)); // Clean file sheet graphic
            }
            else
            {
                pNewNode->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));  // Clean folder graphic
            }

            if (pCurrentParentNode)
            {
                pCurrentParentNode->addChild(pNewNode);
            }
            else
            {
                m_pModelTreeWidget->addTopLevelItem(pNewNode);
            }
            pCurrentParentNode = pNewNode;
        }
    }
}

MainWindow::~MainWindow() {}

void MainWindow::createCentralWidget()
{
    m_pModelViewer = new QModelView3(this);
    setCentralWidget(m_pModelViewer);
}

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
		CFaceEditSheet dialog(this);
		dialog.exec(); });
    mapMenu->addAction(faceEditAction);

    QAction *runMapAction = new QAction("Run Map", this);
    connect(runMapAction, &QAction::triggered, this, [this]()
            {
		CRunMapNormal dialog(this);
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
/*
    QAction *previewAction = new QAction("Preview Model...", this);
    connect(previewAction, &QAction::triggered, this, [this]()
    {

    });
    instanceMenu->addAction(previewAction);
*/
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

/*
void MainWindow::createCentralWidget()
{
    m_MatView = new QMaterialPreview(this);
    setCentralWidget(m_MatView);
}

void MainWindow::createCentralWidget()
{
    // 1. Create your newly adjusted QWidget-based viewport
    QEngineView *pEngineView = new QEngineView(this);
    setCentralWidget(pEngineView);

    // 2. Set up a heartbeat timer to continuously invalidate the widget canvas
    QTimer *pRenderTimer = new QTimer(this);
    connect(pRenderTimer, &QTimer::timeout, pEngineView, qOverload<>(&QWidget::update));
    
    // Start ticking every 16ms (~60 FPS) to force paintEvent() calls
    pRenderTimer->start(16); 
}

void MainWindow::createCentralWidget()
{
    QWidget *centralWidget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(centralWidget);

    CHammerViewportWidget *viewTop   = new CHammerViewportWidget(1, this); // Top (XY)
    CHammerViewportWidget *viewSide  = new CHammerViewportWidget(2, this); // Side (YZ)
    CHammerViewportWidget *viewFront = new CHammerViewportWidget(3, this); // Front (XZ)
    CHammerViewportWidget *view3D    = new CHammerViewportWidget(0, this); // Camera 3D

    layout->addWidget(viewTop, 0, 0);
    layout->addWidget(viewSide, 0, 1);
    layout->addWidget(viewFront, 1, 0);
    layout->addWidget(view3D, 1, 1);

    setCentralWidget(centralWidget);
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
*/

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