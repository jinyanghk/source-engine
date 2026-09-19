#include "MainWindow.h"
#include "widgets/Map2DView.h"
#include "widgets/Map3DView.h"
#include "VmfIO.h"
#include "FgdManager.h"

#include <QScrollBar>
#include <cmath>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
      m_selectedBrushId(-1),
      m_activeTool(TOOL_SELECT), 
      m_currentEntityClass("info_player_start"), 
      m_bIsSingle3DMode(false),
      m_pGridScene(nullptr),
      m_p3DViewport(nullptr),
      m_pEntityTreeView(nullptr),
      m_pEntityFilterEdit(nullptr),
      m_pEntityTreeModel(nullptr),
      m_pEntityFilterProxyModel(nullptr),
      m_pToolActionGroup(nullptr),
      m_hMainSplitter(nullptr),
      m_pActView3D(nullptr),
      m_pActView4Way(nullptr)
{
    // 1. Core Data Initializations: Load the compiled game data configurations (FGD)
    if (!FgdManager::Instance().LoadFgdFile("halflife2.fgd")) 
    {
        qDebug() << "[HammerEditor] WARNING: Failed to locate or parse halflife2.fgd!";
    }

    // 2. Setup standard global application scene context models
    m_pGridScene = new QGraphicsScene(this);
    m_pGridScene->setSceneRect(-8192, -8192, 16384, 16384);

    // 3. Establish the base central widget layouts container
    QWidget *mainContainer = new QWidget(this);
    setCentralWidget(mainContainer);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 4. Instantiate our high-level layout partitioning controls splitter
    m_hMainSplitter = new QSplitter(Qt::Horizontal, mainContainer);
    m_hMainSplitter->setStyleSheet("QSplitter::handle { background-color: #3c3c3c; width: 4px; }");
    mainLayout->addWidget(m_hMainSplitter);

    // 5. Construct the central Grid Layout viewport array context container
    QSplitter *vLeftSplitter = new QSplitter(Qt::Vertical, m_hMainSplitter);
    QSplitter *hTopGridSplitter = new QSplitter(Qt::Horizontal, vLeftSplitter);
    QSplitter *hBottomGridSplitter = new QSplitter(Qt::Horizontal, vLeftSplitter);

    vLeftSplitter->addWidget(hTopGridSplitter);
    vLeftSplitter->addWidget(hBottomGridSplitter);
    m_hMainSplitter->addWidget(vLeftSplitter);

    // 6. Allocate our 2D Viewports matching classic Hammer layout dimensions
    // Top Row viewports layout splits
    Map2DView *pTopXY = new Map2DView(Map2DView::VIEW_TOP, hTopGridSplitter);
    pTopXY->setScene(m_pGridScene);
    m_views.append(pTopXY);
    hTopGridSplitter->addWidget(pTopXY);

    // Top Right: Allocate our accelerated, sequence-stable 3D graphics workspace view
    m_p3DViewport = new Map3DView(hTopGridSplitter);
    hTopGridSplitter->addWidget(m_p3DViewport);

    // Bottom Row viewports layout splits
    Map2DView *pFrontXZ = new Map2DView(Map2DView::VIEW_FRONT, hBottomGridSplitter);
    pFrontXZ->setScene(m_pGridScene);
    m_views.append(pFrontXZ);
    hBottomGridSplitter->addWidget(pFrontXZ);

    Map2DView *pSideYZ = new Map2DView(Map2DView::VIEW_SIDE, hBottomGridSplitter);
    pSideYZ->setScene(m_pGridScene);
    m_views.append(pSideYZ);
    hBottomGridSplitter->addWidget(pSideYZ);

    // Connect all our cross-viewport modification signals straight down to our slots
    for (auto view : m_views)
    {
        connect(view, &Map2DView::brushCreated, this, &MainWindow::onBrushCreated);
        connect(view, &Map2DView::brushSelected, this, &MainWindow::onBrushSelected);
        connect(view, &Map2DView::brushMoved, this, &MainWindow::onBrushMoved);
        connect(view, &Map2DView::brushResized, this, &MainWindow::onBrushResized);
        connect(view, &Map2DView::entityPlaced, this, &MainWindow::onEntityPlaced);
    }

    // Set stable default grid split sizing distributions inside our nested viewports panels
    hTopGridSplitter->setSizes(QList<int>() << 400 << 400);
    hBottomGridSplitter->setSizes(QList<int>() << 400 << 400);
    vLeftSplitter->setSizes(QList<int>() << 300 << 300);

    // 7. Initialize standard framework actions and UI sub-dock controllers
    createMenuBarActions();
    createViewMenuActions();
    createSidebarToolbox();  // Stands up the tool selections (Select, Block, Entity)
    
    // CALL THE RIGHT BROWSER: Build and dock our clean category prefix tree system!
    createRightEntityBrowser();

    // 8. Configure high level base window frame geometries
    resize(1280, 800);
    statusBar()->showMessage(tr("Qt6 Hammer Engine Ready."));

    // Populate initial state maps from our document collections
    generateMockBrushes();
    syncAllViews();
}

// 2. Add this new layout initialization helper method:
void MainWindow::createViewMenuActions()
{
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QActionGroup *viewGroup = new QActionGroup(this);
    viewGroup->setExclusive(true);

    m_pActView4Way = viewMenu->addAction(tr("4 Viewports Split Layout"));
    m_pActView4Way->setCheckable(true);
    m_pActView4Way->setChecked(true); // Default active
    m_pActView4Way->setShortcut(QKeySequence(Qt::Key_F2)); // F2 maps to 4-Way
    viewGroup->addAction(m_pActView4Way);
    connect(m_pActView4Way, &QAction::triggered, this, &MainWindow::toggleViewModeSplit4Way);

    m_pActView3D = viewMenu->addAction(tr("Single 3D Perspective Viewport"));
    m_pActView3D->setCheckable(true);
    m_pActView3D->setShortcut(QKeySequence(Qt::Key_F3)); // F3 maps to full 3D Alyx view
    viewGroup->addAction(m_pActView3D);
    connect(m_pActView3D, &QAction::triggered, this, &MainWindow::toggleViewModeSingle3D);
}

// 3. Add the layout swapper slots to the bottom of MainWindow.cpp:
void MainWindow::toggleViewModeSingle3D(bool checked)
{
    if (!checked || m_bIsSingle3DMode) return;
    m_bIsSingle3DMode = true;

    statusBar()->showMessage(tr("Switched to Maximized 3D perspective viewport."), 2000);

    // FIXED: Find and cache splitter proportions before modifying parent trees
    QSplitter *vSplitterLeft = qobject_cast<QSplitter*>(m_hMainSplitter->widget(0));
    QSplitter *vSplitterRight = qobject_cast<QSplitter*>(m_hMainSplitter->widget(1));
    if (vSplitterLeft)  m_cachedLeftSizes  = vSplitterLeft->sizes();
    if (vSplitterRight) m_cachedRightSizes = vSplitterRight->sizes();

    m_hMainSplitter->hide();
    m_p3DViewport->setParent(nullptr);

    QWidget *pOldCentral = centralWidget();
    if (pOldCentral) {
        pOldCentral->setParent(nullptr);
    }

    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_p3DViewport);
    
    setCentralWidget(container);
    m_p3DViewport->show();
    m_p3DViewport->update();
}

void MainWindow::toggleViewModeSplit4Way(bool checked)
{
    if (!checked || !m_bIsSingle3DMode) return;
    m_bIsSingle3DMode = false;

    statusBar()->showMessage(tr("Restored traditional 4-way multi-split layout."), 2000);

    m_p3DViewport->setParent(nullptr);

    QSplitter *vSplitterLeft = qobject_cast<QSplitter*>(m_hMainSplitter->widget(0));
    QSplitter *vSplitterRight = qobject_cast<QSplitter*>(m_hMainSplitter->widget(1));

    if (vSplitterRight) {
        vSplitterRight->insertWidget(0, m_p3DViewport);
    }

    // FIXED: Re-apply the cached sizing metrics back onto the splitter matrices immediately!
    if (vSplitterLeft && !m_cachedLeftSizes.isEmpty()) {
        vSplitterLeft->setSizes(m_cachedLeftSizes);
    }
    if (vSplitterRight && !m_cachedRightSizes.isEmpty()) {
        vSplitterRight->setSizes(m_cachedRightSizes);
    }

    QWidget *pOldCentral = centralWidget();
    if (pOldCentral) {
        pOldCentral->setParent(nullptr);
        delete pOldCentral;
    }

    QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_hMainSplitter);
    
    m_hMainSplitter->show();
    setCentralWidget(container);
    
    syncAllViews();
}

void MainWindow::createSidebarToolbox()
{
    QToolBar *sidebar = new QToolBar(tr("Toolbox"), this);
    sidebar->setMovable(false);
    sidebar->setOrientation(Qt::Vertical);
    sidebar->setIconSize(QSize(24, 24));
    sidebar->setStyleSheet("background-color: #252526; border-right: 1px solid #3c3c3c; padding: 4px;");

    m_pToolActionGroup = new QActionGroup(this);
    m_pToolActionGroup->setExclusive(true);

    QAction *actSelect = sidebar->addAction(tr("Select"));
    actSelect->setCheckable(true);
    actSelect->setChecked(true);
    m_pToolActionGroup->addAction(actSelect);
    connect(actSelect, &QAction::triggered, this, [=]() { onToolChanged(TOOL_SELECT); });

    QAction *actBlock = sidebar->addAction(tr("Block"));
    actBlock->setCheckable(true);
    m_pToolActionGroup->addAction(actBlock);
    connect(actBlock, &QAction::triggered, this, [=]() { onToolChanged(TOOL_BLOCK); });

    QAction *actEntity = sidebar->addAction(tr("Entity"));
    actEntity->setCheckable(true);
    m_pToolActionGroup->addAction(actEntity);
    connect(actEntity, &QAction::triggered, this, [=]() { onToolChanged(TOOL_ENTITY); });

    // FIXED: The old m_pEntityClassCombo logic, lblClass label, and item loops 
    // are completely removed since they are now beautifully handled by your Right Browser tree view!

    addToolBar(Qt::LeftToolBarArea, sidebar);
}

void MainWindow::onToolChanged(EditTool tool)
{
    m_activeTool = tool;
    statusBar()->showMessage(tr("Active Tool Switched."), 2000);
    syncAllViews();
}

void MainWindow::onEntityClassChanged(const QString &classname) { m_currentEntityClass = classname; }

void MainWindow::onEntityPlaced(const Vector &origin, const QString &classname)
{
    // 1. Pipe properties safely down to our pure C++ backend data container
    m_document.CreateNewEntity(classname.toUtf8().constData(), origin);
    
    // 2. Query our fgdlib wrapper template definitions to map visual profiles
    FgdEntityTemplate entTemplate;
    QColor displayColor(0, 255, 0); // Default fallback profile color
    
    if (FgdManager::Instance().FindTemplate(classname.toUtf8().constData(), entTemplate))
    {
        // Extract the explicit editor color profile defined in the FGD schema
        displayColor = QColor(entTemplate.r, entTemplate.g, entTemplate.b);
    }
    
    // 3. Track presentation metrics locally so viewports paint unique attributes
    MapEntity visualEnt;
    visualEnt.id = m_mapEntities.size() + 1; // Temporary presentation ID tracking
    visualEnt.classname = classname;
    visualEnt.origin = origin;
    visualEnt.color = displayColor;
    
    m_mapEntities.append(visualEnt);
    
    syncAllViews();
}

void MainWindow::syncAllViews()
{
    // 1. Clear out old cached presentation caches to prevent duplicate drawing
    m_mapBrushes.clear();
    m_mapEntities.clear();

    // 2. Convert decoupled C++ backend solids into viewport presentation structures
    const auto& backendSolids = m_document.GetSolids();
    for (const auto& solid : backendSolids)
    {
        MapBrush visualBrush;
        visualBrush.id = solid.id;
        
        // Staging coordinates during this refactoring phase. 
        // These will pull dynamically from face arrays as brush math matures.
        visualBrush.mins.Init(-64, -64, -64); 
        visualBrush.maxs.Init(64, 64, 64);
        
        // Highlight selection colors at the presentation layer
        visualBrush.color = (solid.id == m_selectedBrushId) ? QColor(255, 0, 0) : QColor(0, 255, 100);

        m_mapBrushes.append(visualBrush);
    }

    // 3. Convert decoupled C++ backend entities into viewport presentation structures
    const auto& backendEntities = m_document.GetEntities();
    for (const auto& ent : backendEntities)
    {
        MapEntity visualEnt;
        visualEnt.id = ent.id;
        visualEnt.classname = QString::fromStdString(ent.classname);
        visualEnt.origin.Init(ent.origin.x, ent.origin.y, ent.origin.z);
        
        // DYNAMIC FGD INTERPRETATION: Pull unique display color specifications
        FgdEntityTemplate entTemplate;
        if (FgdManager::Instance().FindTemplate(ent.classname, entTemplate))
        {
            visualEnt.color = QColor(entTemplate.r, entTemplate.g, entTemplate.b);
        }
        else
        {
            visualEnt.color = QColor(230, 0, 255); // Fallback color magenta
        }

        m_mapEntities.append(visualEnt);
    }

    // 4. Dispatch the synchronized dataset across all 2D Orthographic Grid Viewports
    for (auto view : m_views)
    {
        view->updateSceneData(m_mapBrushes, m_mapEntities, m_selectedBrushId, m_activeTool, m_currentEntityClass);
    }
    
    // 5. Stream the presentation data to update nillerusr's source-engine rendering context
    if (m_p3DViewport)
    {
        m_p3DViewport->updateBrushes(m_mapBrushes.constData(), m_mapBrushes.size(), m_selectedBrushId);
        m_p3DViewport->updateEntities(m_mapEntities.constData(), m_mapEntities.size()); // NEW: Direct, clean passing matching our new structures
    }

    // 6. Force the underlying Qt Graphics Scene hierarchy to schedule a full display repaint
    m_pGridScene->update();
}

// Remainder data logic blocks
void MainWindow::createMenuBarActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    
    // FIXED: Restore the required pointer asterisk types matching addAction return arrays
    QAction *openAct = fileMenu->addAction(tr("&Open VMF..."), this, &MainWindow::triggerOpenDialog);
    openAct->setShortcut(QKeySequence::Open);
    
    QAction *saveAct = fileMenu->addAction(tr("&Save VMF..."), this, &MainWindow::triggerSaveDialog);
    saveAct->setShortcut(QKeySequence::Save);
}

void MainWindow::triggerOpenDialog()
{
    // FIXED: Changed (.vmf) to (*.vmf) so the operating system displays your files cleanly
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Valve Map File"), "", tr("Valve Map Files (*.vmf)"));
    if (!filePath.isEmpty())
    {
        if (VmfIO::LoadFromFile(filePath.toUtf8().constData(), m_document)) {
            statusBar()->showMessage(tr("Map loaded successfully."), 3000);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to parse VMF map structure."));
        }
    }
}

void MainWindow::triggerSaveDialog()
{
    // FIX: Change "Valve Map Files (.vmf)" to "Valve Map Files (*.vmf)"
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Valve Map File"), "", tr("Valve Map Files (*.vmf)"));
    if (!filePath.isEmpty())
    {
        if (VmfIO::SaveToFile(filePath.toUtf8().constData(), m_document))
            statusBar()->showMessage(tr("Map saved successfully."), 3000);
    }
}

void MainWindow::generateMockBrushes()
{
    MapBrush b1;
    b1.id = 1;
    b1.mins.Init(-256, -256, -128);
    b1.maxs.Init(256, 256, 128);
    b1.color = QColor(255, 128, 0);
    m_mapBrushes.append(b1);
    MapBrush b2;
    b2.id = 2;
    b2.mins.Init(64, 64, -128);
    b2.maxs.Init(128, 128, 128);
    b2.color = QColor(0, 180, 255);
    m_mapBrushes.append(b2);
}

void MainWindow::onBrushCreated(const Vector &mins, const Vector &maxs)
{
    // 1. Pass the geometric allocation down to our decoupled backend data engine
    int newSolidId = m_document.CreateNewSolid(mins, maxs);
    
    // 2. Automatically select the newly created structural element
    m_selectedBrushId = newSolidId;
    
    // 3. Synchronize data layers across our viewports
    syncAllViews();
}

void MainWindow::onBrushSelected(int id)
{
    if (id <= -1000)
    {
        int realEntityId = std::abs(id) - 1000;
        statusBar()->showMessage(tr("Selected Entity ID: %1").arg(realEntityId), 2000);
        return;
    }
    m_selectedBrushId = id;
    syncAllViews();
}

void MainWindow::onBrushMoved(const Vector &delta3D)
{
    if (m_selectedBrushId != -1)
    {
        // Execute the pure 3D transformation via the document model controller
        if (m_document.TranslateSolid(m_selectedBrushId, delta3D))
        {
            syncAllViews();
        }
    }
}

void MainWindow::onBrushResized(int id, const Vector &mi, const Vector &ma)
{
    // Forward boundary updates into the isolated document data array
    if (m_document.ResizeSolid(id, mi, ma))
    {
        syncAllViews();
    }
}

void MainWindow::deleteSelectedBrush()
{
    if (m_selectedBrushId != -1)
    {
        // Remove the solid structure from the isolated backend container
        if (m_document.DeleteSolid(m_selectedBrushId))
        {
            m_selectedBrushId = -1;
            syncAllViews();
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int c = m_views.first()->gridSize();
    if (event->key() == Qt::Key_BracketLeft)
    {
        for (auto v : m_views)
            v->setGridSize(c / 2);
    }
    else if (event->key() == Qt::Key_BracketRight)
    {
        for (auto v : m_views)
            v->setGridSize(c * 2);
    }
    else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        deleteSelectedBrush();
    }
    else
        QMainWindow::keyPressEvent(event);
}

void MainWindow::createRightEntityBrowser()
{
    // 1. Create a dedicated container panel widget for the right side dock area
    QWidget *rightContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    rightLayout->setSpacing(4);
    rightContainer->setStyleSheet("background-color: #252526; border-left: 1px solid #3c3c3c;");

    // 2. Instantiate our fast search line edit control
    m_pEntityFilterEdit = new QLineEdit(this);
    m_pEntityFilterEdit->setPlaceholderText(tr("Filter entities..."));
    m_pEntityFilterEdit->setStyleSheet("background-color: #3c3c3c; color: white; border: 1px solid #555; padding: 4px; border-radius: 2px;");
    connect(m_pEntityFilterEdit, &QLineEdit::textChanged, this, &MainWindow::onEntityFilterChanged);
    rightLayout->addWidget(m_pEntityFilterEdit);

    // 3. Build the underlying model tree architecture
    m_pEntityTreeModel = new QStandardItemModel(this);
    m_pEntityTreeModel->setHorizontalHeaderLabels(QStringList() << tr("Entity Classes"));

    // Fetch our dynamic FGD class strings array directly from fgdlib
    std::vector<std::string> rawPointClasses = FgdManager::Instance().GetAvailablePointClasses();
    
    // Fallback staging values if FGD didn't return tokens
    if(rawPointClasses.empty()) {
        rawPointClasses = {"info_player_start", "light", "env_shake", "env_spark", "trigger_once"};
    }

    // Map helper structures to keep track of folder nodes we create dynamically
    QMap<QString, QStandardItem*> folderCache;

    for (const auto& classnameStr : rawPointClasses)
    {
        QString classname = QString::fromStdString(classnameStr);
        QStandardItem *item = new QStandardItem(classname);
        item->setEditable(false);

        // Intelligently determine a prefix folder classification name (e.g., "env", "info", "trigger")
        int prefixIndex = classname.indexOf('_');
        if (prefixIndex > 0)
        {
            QString prefix = classname.left(prefixIndex).toUpper();
            
            // If the folder item doesn't exist yet in our view tree model, create it!
            if (!folderCache.contains(prefix))
            {
                QStandardItem *folderNode = new QStandardItem(prefix);
                folderNode->setEditable(false);
                folderNode->setFont(QFont("Arial", 9, QFont::Bold));
                m_pEntityTreeModel->invisibleRootItem()->appendRow(folderNode);
                folderCache.insert(prefix, folderNode);
            }
            
            // Append the class item as a leaf under its parent prefix group node
            folderCache[prefix]->appendRow(item);
        }
        else
        {
            // If the entity has no underscore prefix, place it cleanly at the root level
            m_pEntityTreeModel->invisibleRootItem()->appendRow(item);
        }
    }

    // 4. Configure our live regex search filter proxy model layers
    m_pEntityFilterProxyModel = new QSortFilterProxyModel(this);
    m_pEntityFilterProxyModel->setSourceModel(m_pEntityTreeModel);
    m_pEntityFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pEntityFilterProxyModel->setRecursiveFilteringEnabled(true); // Ensures parent folders stay visible during deep leaf searches

    // 5. Build and populate our actual Tree View viewport widget panel
    m_pEntityTreeView = new QTreeView(this);
    m_pEntityTreeView->setModel(m_pEntityFilterProxyModel);
    m_pEntityTreeView->setHeaderHidden(true);
    m_pEntityTreeView->setAnimated(true);
    m_pEntityTreeView->setStyleSheet(
        "QTreeView { background-color: #1e1e1e; color: #cccccc; border: 1px solid #3c3c3c; }"
        "QTreeView::item:selected { background-color: #094771; color: white; }"
        "QTreeView::item:hover { background-color: #2a2d2e; }"
    );
    
    connect(m_pEntityTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onEntityTreeSelectionChanged);
    connect(m_pEntityTreeView, &QTreeView::doubleClicked, this, &MainWindow::onEntityTreeDoubleClicked);
    rightLayout->addWidget(m_pEntityTreeView);

    // 6. Integrate this container into your central splitter layout hierarchy!
    // Simply add the container widget as a new pane to the main horizontal window layout splitter
    m_hMainSplitter->addWidget(rightContainer);
    
    // Set appropriate starting horizontal panel sizing distributions
    m_hMainSplitter->setStretchFactor(m_hMainSplitter->indexOf(rightContainer), 0);
}

void MainWindow::onEntityFilterChanged(const QString &text)
{
    if (m_pEntityFilterProxyModel)
    {
        m_pEntityFilterProxyModel->setFilterFixedString(text);
        
        // Auto-expand folder nodes when actively filtering text strings to show hidden matches
        if (!text.isEmpty() && m_pEntityTreeView) {
            m_pEntityTreeView->expandAll();
        }
    }
}

void MainWindow::onEntityTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!current.isValid()) return;

    // Translate the proxy index back to the true underlying source item string
    QModelIndex sourceIndex = m_pEntityFilterProxyModel->mapToSource(current);
    QStandardItem *item = m_pEntityTreeModel->itemFromIndex(sourceIndex);
    
    if (item && !item->hasChildren()) // Ensure the user didn't accidentally select a group folder header node
    {
        m_currentEntityClass = item->text();
        statusBar()->showMessage(tr("Active Entity Tool Target: %1").arg(m_currentEntityClass), 2000);
    }
}

void MainWindow::onEntityTreeDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid() || !m_pEntityFilterProxyModel || !m_pEntityTreeModel || !m_p3DViewport) 
        return;

    QModelIndex sourceIndex = m_pEntityFilterProxyModel->mapToSource(index);
    QStandardItem *item = m_pEntityTreeModel->itemFromIndex(sourceIndex);
    
    if (item && !item->hasChildren())
    {
        QString classname = item->text();
        std::string targetModelPath = FgdManager::Instance().GetModelPathForClass(classname.toUtf8().constData());
        
        QModelView* pModelViewerWindow = new QModelView(nullptr);
        pModelViewerWindow->resize(640, 480);
        pModelViewerWindow->setAttribute(Qt::WA_DeleteOnClose);

        pModelViewerWindow->show();
        pModelViewerWindow->raise();
        pModelViewerWindow->activateWindow();

        QTimer::singleShot(200, pModelViewerWindow, [pModelViewerWindow, targetModelPath]() {
            pModelViewerWindow->LoadModelFile(QString::fromStdString(targetModelPath));
        });

    }
}

MainWindow::~MainWindow(){}