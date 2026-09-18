#include "mainwindow1.h"

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
    m_pModelViewer = new QModelView(pRightContainerShell);
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
