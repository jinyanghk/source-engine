#include "MainWindow.h"
#include "FgdManager.h"

#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QWindow>

#include <SDL2/SDL.h>

#include "appframework/ilaunchermgr.h"
#include "materialsystem/imaterialsystem.h"
#include "istudiorender.h"
#include "datacache/imdlcache.h"

#include "EngineViewWindow.h"

extern IMaterialSystem *g_pMaterialSystem;
extern IStudioRender *g_pStudioRender;
extern IMDLCache *g_pMDLCache;

MDLHandle_t g_hActiveHammerModel = 0xFFFF;

extern "C" unsigned long long ExtractX11WindowFromLauncher(void *pSDLMgrSystemRef);
extern ILauncherMgr *g_pLauncherMgr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
      m_pEngineContainerWidget(nullptr),
      m_pStrataEngineViewWindow(nullptr), // Initialize pointer to null
      m_pEntityTreeView(nullptr),
      m_pEntityFilterEdit(nullptr),
      m_pEntityTreeModel(nullptr),
      m_pEntityFilterProxyModel(nullptr)
{
    qDebug() << "[HammerEditor] Spawning Strata Source Viewport Setup Context.";

    if (!FgdManager::Instance().LoadFgdFile("halflife2.fgd")) {
        qDebug() << "[HammerEditor] WARNING: Failed to parse halflife2.fgd!";
    }

    QWidget *mainContainer = new QWidget(this);
    setCentralWidget(mainContainer);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 1. INITIALIZE OUR SPECIALIZED STRATA ENGINE WINDOW
    m_pStrataEngineViewWindow = new EngineViewWindow();

    // 2. CONVERT THE NATIVE QWINDOW SURFACE INTO A STANDARD QWIDGET UI COMPONENT
    m_pEngineContainerWidget = QWidget::createWindowContainer(m_pStrataEngineViewWindow, mainContainer);
    mainLayout->addWidget(m_pEngineContainerWidget, 7); // Takes up 70% layout space grid block

    createRightEntityBrowser();
    mainLayout->addWidget(m_pEntityTreeView->parentWidget(), 3); 

    resize(1280, 800); 
    this->show();

    // 3. PERSISTENT TIMEOUT LOOP DRIVER
    QTimer* pFrameTimer = new QTimer(this);
    connect(pFrameTimer, &QTimer::timeout, this, [this]() {
        // Drain events to keep input processing humming smoothly
        SDL_Event event;
        while (SDL_PollEvent(&event)) {}

        // Fire the viewport's paint routine explicitly
        if (m_pStrataEngineViewWindow) {
            m_pStrataEngineViewWindow->RenderFrame();
        }
    });
    pFrameTimer->start(16); 
}

void MainWindow::onRenderTimerTick()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // Drain events to keep window responsive
    }

    // Call our execution loop method
    executeEngineFrame();
}

void MainWindow::executeEngineFrame()
{
    static int loggerThrottle = 0;
    bool bShouldLog = ((loggerThrottle++ % 120) == 0);

    if (!g_pMaterialSystem || !g_pStudioRender || !g_pMDLCache) return;
    if (g_hActiveHammerModel == 0xFFFF) return;

    int width = m_pEngineContainerWidget ? qMax(64, m_pEngineContainerWidget->width()) : 800;
    int height = m_pEngineContainerWidget ? qMax(64, m_pEngineContainerWidget->height()) : 600;

    // 1. FORCE THE UNDERLYING HARDWARE VIEWPORT AND CLEAR USING DIRECT SYSTEM STATES
    // This bypasses the engine context validation loops to force the left side area to turn gray!
    CMatRenderContextPtr pRenderContext(g_pMaterialSystem);
    if (pRenderContext)
    {
        pRenderContext->Viewport(0, 0, width, height);
        pRenderContext->ClearColor4ub(160, 165, 180, 255); // Soft slate gray
        pRenderContext->ClearBuffers(true, true);
    }

    // 2. RUN NATIVE ENGINE MODEL BLOCKS
    g_pMaterialSystem->BeginFrame(0.016f);
    {
        if (pRenderContext)
        {
            // Configure full-bright ambient vertex coefficients
            pRenderContext->SetAmbientLight(1.0f, 1.0f, 1.0f);

            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->PushMatrix();
            pRenderContext->LoadIdentity();
            pRenderContext->PerspectiveX(65.0, (float)width / (float)height, 1.0, 1000.0);

            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->PushMatrix();
            pRenderContext->LoadIdentity();
            // Pull the eye position back slightly so the model bounds sit directly in the frame center
            pRenderContext->Translate(0.0f, -20.0f, -60.0f); 

            pRenderContext->MatrixMode(MATERIAL_MODEL);
            pRenderContext->PushMatrix();
            pRenderContext->LoadIdentity();

            studiohdr_t *pStudioHdr = g_pMDLCache->GetStudioHdr(g_hActiveHammerModel);
            studiohwdata_t *pHardwareData = g_pMDLCache->GetHardwareData(g_hActiveHammerModel);

            if (pStudioHdr && pHardwareData)
            {
                if (bShouldLog) {
                    qDebug() << "[Telemetry] Drawing embedded asset:" << pStudioHdr->name;
                }

                DrawModelInfo_t modelInfo;
                modelInfo.m_pStudioHdr = pStudioHdr;
                modelInfo.m_pHardwareData = pHardwareData;
                modelInfo.m_Skin = 0;
                modelInfo.m_Body = 0;
                modelInfo.m_HitboxSet = 0;

                matrix3x4_t identityBones[MAXSTUDIOBONES];
                for(int i = 0; i != MAXSTUDIOBONES; ++i) 
                {
                    SetIdentityMatrix(identityBones[i]);
                }

                g_pStudioRender->SetLocalLights(0, nullptr);

                ::StudioRenderConfig_t studioCfg;
                memset(&studioCfg, 0, sizeof(::StudioRenderConfig_t));
                studioCfg.drawEntities = 1;
                studioCfg.bSoftwareLighting = false;
                
                g_pStudioRender->UpdateConfig(studioCfg);
                g_pStudioRender->ForcedMaterialOverride(nullptr);
                
                // Draw the asset
                g_pStudioRender->DrawModel(nullptr, modelInfo, identityBones, NULL, NULL, Vector(0, 0, 0), 0);
            }

            pRenderContext->MatrixMode(MATERIAL_MODEL);
            pRenderContext->PopMatrix();
            pRenderContext->MatrixMode(MATERIAL_VIEW);
            pRenderContext->PopMatrix();
            pRenderContext->MatrixMode(MATERIAL_PROJECTION);
            pRenderContext->PopMatrix();
        }
    }
    g_pMaterialSystem->EndFrame();

    // 3. FORCE HARDWARE SWAP
    // Instructs the underlying engine driver to immediately swap its frame surface buffer
    g_pMaterialSystem->Flush(true);
}

void MainWindow::createRightEntityBrowser()
{
    QWidget *rightContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(4, 4, 4, 4);
    rightLayout->setSpacing(4);
    rightContainer->setStyleSheet("background-color: #252526; border-left: 1px solid #3c3c3c;");

    m_pEntityFilterEdit = new QLineEdit(this);
    m_pEntityFilterEdit->setPlaceholderText(tr("Filter entities..."));
    m_pEntityFilterEdit->setStyleSheet("background-color: #3c3c3c; color: white; padding: 4px;");
    connect(m_pEntityFilterEdit, &QLineEdit::textChanged, this, &MainWindow::onEntityFilterChanged);
    rightLayout->addWidget(m_pEntityFilterEdit);

    m_pEntityTreeModel = new QStandardItemModel(this);
    m_pEntityTreeModel->setHorizontalHeaderLabels(QStringList() << tr("Entity Classes"));

    std::vector<std::string> rawPointClasses = FgdManager::Instance().GetAvailablePointClasses();
    if (rawPointClasses.empty())
    {
        rawPointClasses = {"info_player_start", "light", "trigger_once"};
    }

    QMap<QString, QStandardItem *> folderCache;
    for (const auto &classnameStr : rawPointClasses)
    {
        QString classname = QString::fromStdString(classnameStr);
        QStandardItem *item = new QStandardItem(classname);
        item->setEditable(false);

        int prefixIndex = classname.indexOf('_');
        if (prefixIndex > 0)
        {
            QString prefix = classname.left(prefixIndex).toUpper();
            if (!folderCache.contains(prefix))
            {
                QStandardItem *folderNode = new QStandardItem(prefix);
                folderNode->setEditable(false);
                m_pEntityTreeModel->invisibleRootItem()->appendRow(folderNode);
                folderCache.insert(prefix, folderNode);
            }
            folderCache[prefix]->appendRow(item);
        }
        else
        {
            m_pEntityTreeModel->invisibleRootItem()->appendRow(item);
        }
    }

    m_pEntityFilterProxyModel = new QSortFilterProxyModel(this);
    m_pEntityFilterProxyModel->setSourceModel(m_pEntityTreeModel);
    m_pEntityFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pEntityFilterProxyModel->setRecursiveFilteringEnabled(true);

    m_pEntityTreeView = new QTreeView(this);
    m_pEntityTreeView->setModel(m_pEntityFilterProxyModel);
    m_pEntityTreeView->setHeaderHidden(true);

    connect(m_pEntityTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onEntityTreeSelectionChanged);
    rightLayout->addWidget(m_pEntityTreeView);
}

void MainWindow::onEntityFilterChanged(const QString &text)
{
    if (m_pEntityFilterProxyModel)
    {
        m_pEntityFilterProxyModel->setFilterFixedString(text);
        if (!text.isEmpty() && m_pEntityTreeView)
        {
            m_pEntityTreeView->expandAll();
        }
    }
}

void MainWindow::onEntityTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!current.isValid() || !m_pEntityFilterProxyModel || !m_pEntityTreeModel)
        return;

    QModelIndex sourceIndex = m_pEntityFilterProxyModel->mapToSource(current);
    QStandardItem *item = m_pEntityTreeModel->itemFromIndex(sourceIndex);

    if (item && !item->hasChildren())
    {
        QString classname = item->text();
        std::string targetModelPath = FgdManager::Instance().GetModelPathForClass(classname.toUtf8().constData());

        qDebug() << "[Selection Fired] Class:" << classname << " | Path:" << QString::fromStdString(targetModelPath);

        if (!targetModelPath.empty() && g_pMDLCache)
        {
            g_hActiveHammerModel = g_pMDLCache->FindMDL(targetModelPath.c_str());
            qDebug() << "[Telemetry] FindMDL returned handle ID:" << g_hActiveHammerModel;
            statusBar()->showMessage(tr("Engine loaded asset: %1").arg(QString::fromStdString(targetModelPath)), 2000);
            if (m_pStrataEngineViewWindow) {
                m_pStrataEngineViewWindow->RenderFrame(); // Re-rasterizes model cache changes instantly
            }
        }
        else
        {
            g_hActiveHammerModel = 0xFFFF;
        }
    }
}
MainWindow::~MainWindow() {}