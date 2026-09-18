#include "MainWindow.h"
#include "widgets/Hammer2DGridView.h"
#include "widgets/Hammer3DView.h"
#include "VmfIO.h"

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
      m_nextBrushId(3), 
      m_nextEntityId(1), 
      m_selectedBrushId(-1), 
      m_activeTool(TOOL_SELECT), 
      m_currentEntityClass("info_player_start"), 
      m_bIsSingle3DMode(false)
{
    // 1. Configure the primary window frame metrics
    resize(1300, 850);
    setWindowTitle(tr("Qt6 Hammer Engine - Multi-Viewport Split Grid System"));
    
    // 2. Allocate the unified background grid manager scene graph
    m_pGridScene = new QGraphicsScene(this);
    m_pGridScene->setSceneRect(-16384, -16384, 32768, 32768);
    
    // 3. Instantiate orthographic viewports
    Hammer2DGridView *topView   = new Hammer2DGridView(Hammer2DGridView::VIEW_TOP, this);
    Hammer2DGridView *frontView = new Hammer2DGridView(Hammer2DGridView::VIEW_FRONT, this);
    Hammer2DGridView *sideView  = new Hammer2DGridView(Hammer2DGridView::VIEW_SIDE, this);
    
    // 4. Assign the shared graph manager context
    topView->setScene(m_pGridScene); 
    frontView->setScene(m_pGridScene); 
    sideView->setScene(m_pGridScene);
    
    // 5. Append instances to our central class member tracker list
    m_views.append(topView); 
    m_views.append(frontView); 
    m_views.append(sideView);
    
    // 6. Allocate your live skeletal animation 3D engine canvas
    m_p3DViewport = new Hammer3DView(this);
    
    // 7. Assemble split structural layout containers
    QSplitter *vSplitterLeft = new QSplitter(Qt::Vertical, this);
    vSplitterLeft->addWidget(topView); 
    vSplitterLeft->addWidget(frontView); 
    vSplitterLeft->setSizes(QList<int>({400, 400}));
    
    QSplitter *vSplitterRight = new QSplitter(Qt::Vertical, this);
    vSplitterRight->addWidget(m_p3DViewport); 
    vSplitterRight->addWidget(sideView); 
    vSplitterRight->setSizes(QList<int>({400, 400}));
    
    // 8. Capture the root frame splitter reference into our class member pointer
    m_hMainSplitter = new QSplitter(Qt::Horizontal, this);
    m_hMainSplitter->addWidget(vSplitterLeft); 
    m_hMainSplitter->addWidget(vSplitterRight);
    m_hMainSplitter->setSizes(QList<int>({600, 600}));

    // 9. Attach layout signals network routing paths
    for (auto view : m_views)
    {
        connect(view, &Hammer2DGridView::brushCreated,  this, &MainWindow::onBrushCreated);
        connect(view, &Hammer2DGridView::brushSelected, this, &MainWindow::onBrushSelected);
        connect(view, &Hammer2DGridView::brushMoved,    this, &MainWindow::onBrushMoved);
        connect(view, &Hammer2DGridView::brushResized,  this, &MainWindow::onBrushResized);
        connect(view, &Hammer2DGridView::entityPlaced,  this, &MainWindow::onEntityPlaced);
    }
    
    // 10. Generate control menus and toolbox items widgets
    createMenuBarActions();
    createSidebarToolbox();
    createViewMenuActions();

    // 11. Wrap components inside our central main widget container layout
    QWidget *mainContainer = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    
    // Default launch state: Start with the traditional 4-Way multi-split layout
    mainLayout->addWidget(m_hMainSplitter, 1);
    setCentralWidget(mainContainer);

    // 12. Seed mock records and synchronize structural visibility passes
    generateMockBrushes();
    syncAllViews();
    
    // 13. Initialize unique camera zoom focus rules on the world center axis origin lines
    for (auto view : m_views) 
    {
        view->resetTransform();
        view->scale(1.5, 1.5);
        view->centerOn(0, 0);
    }
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
    addToolBar(Qt::LeftToolBarArea, sidebar);
    m_pToolActionGroup = new QActionGroup(this);
    m_pToolActionGroup->setExclusive(true);

    // FIXED: Clean, type-safe C++ lambda closure definitions
    QAction *actSelect = sidebar->addAction(tr("Select"));
    actSelect->setCheckable(true); actSelect->setChecked(true);
    m_pToolActionGroup->addAction(actSelect);
    connect(actSelect, &QAction::triggered, this, [this](){ onToolChanged(TOOL_SELECT); });

    QAction *actBlock = sidebar->addAction(tr("Block"));
    actBlock->setCheckable(true);
    m_pToolActionGroup->addAction(actBlock);
    connect(actBlock, &QAction::triggered, this, [this](){ onToolChanged(TOOL_BLOCK); });

    QAction *actEntity = sidebar->addAction(tr("Entity"));
    actEntity->setCheckable(true);
    m_pToolActionGroup->addAction(actEntity);
    connect(actEntity, &QAction::triggered, this, [this](){ onToolChanged(TOOL_ENTITY); });
    
    sidebar->addSeparator();

    QLabel *lblClass = new QLabel(tr(" Entity Class:"), this);
    lblClass->setStyleSheet("color: #ffaa00; font-weight: bold; margin-top: 10px;");
    sidebar->addWidget(lblClass);
    m_pEntityClassCombo = new QComboBox(this);
    m_pEntityClassCombo->addItem("info_player_start");
    m_pEntityClassCombo->addItem("light");
    m_pEntityClassCombo->addItem("ambient_generic");
    m_pEntityClassCombo->addItem("npc_zombie");
    m_pEntityClassCombo->setStyleSheet("background-color: #3c3c3c; color: white; margin: 4px; padding: 2px;");
    sidebar->addWidget(m_pEntityClassCombo);
    connect(m_pEntityClassCombo, &QComboBox::currentTextChanged, this, &MainWindow::onEntityClassChanged);
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
    // Pass placement properties safely down to our pure C++ backend data container
    m_document.CreateNewEntity(classname.toUtf8().constData(), origin);
    
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
        
        // Determine viewport display wireframe color profiles based on entity classification
        if (visualEnt.classname == "info_player_start") {
            visualEnt.color = QColor(0, 255, 0);
        } else if (visualEnt.classname == "light") {
            visualEnt.color = QColor(255, 255, 0);
        } else {
            visualEnt.color = QColor(230, 0, 255);
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

MainWindow::~MainWindow(){}