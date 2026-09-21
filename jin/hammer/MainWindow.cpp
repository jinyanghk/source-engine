#include "MainWindow.h"
#include "FgdManager.h"

#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
      m_pModelViewerWidget(nullptr),
      m_pEntityTreeView(nullptr),
      m_pEntityFilterEdit(nullptr),
      m_pEntityTreeModel(nullptr),
      m_pEntityFilterProxyModel(nullptr)
{
    if (!FgdManager::Instance().LoadFgdFile("halflife2.fgd")) 
    {
        qDebug() << "[HammerEditor] WARNING: Failed to locate or parse halflife2.fgd!";
    }

    QWidget *mainContainer = new QWidget(this);
    setCentralWidget(mainContainer);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // 1. ALLOCATE THE COMPONENT FIRST: 
    m_pModelViewerWidget = new QModelView(mainContainer);
    
    // 2. STACK SEAMLESSLY INTO THE MAIN WINDOW CONTAINER PANEL:
    // This gives the widget valid layout geometry sizes BEFORE the engine tries to bind to it!
    mainLayout->addWidget(m_pModelViewerWidget, 7); // Takes up 70% of horizontal window space

    // 3. Build and append the Right Sidebar Categories tree hierarchy list
    createRightEntityBrowser();
    mainLayout->addWidget(m_pEntityTreeView->parentWidget(), 3); // Takes up 30% of space

    resize(1280, 800); 
    this->show();

    // 5. Connect the sync loop to update BOTH the engine rendering pipeline and the Qt UI frames!
    QTimer* pFrameTimer = new QTimer(this);
    connect(pFrameTimer, &QTimer::timeout, this, [this]() {

        
        // Force the layout widget wrapper to repaint and process frame ticks
        if (m_pModelViewerWidget) {
            m_pModelViewerWidget->update();
        }
    });
    pFrameTimer->start(16); 
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
    m_pEntityFilterEdit->setStyleSheet("background-color: #3c3c3c; color: white; border: 1px solid #555; padding: 4px; border-radius: 2px;");
    connect(m_pEntityFilterEdit, &QLineEdit::textChanged, this, &MainWindow::onEntityFilterChanged);
    rightLayout->addWidget(m_pEntityFilterEdit);

    m_pEntityTreeModel = new QStandardItemModel(this);
    m_pEntityTreeModel->setHorizontalHeaderLabels(QStringList() << tr("Entity Classes"));

    std::vector<std::string> rawPointClasses = FgdManager::Instance().GetAvailablePointClasses();
    if(rawPointClasses.empty()) {
        rawPointClasses = {"info_player_start", "light", "env_shake", "env_spark", "trigger_once"};
    }

    QMap<QString, QStandardItem*> folderCache;
    for (const auto& classnameStr : rawPointClasses)
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
                folderNode->setFont(QFont("Arial", 9, QFont::Bold));
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
    m_pEntityTreeView->setAnimated(true);
    m_pEntityTreeView->setStyleSheet(
        "QTreeView { background-color: #1e1e1e; color: #cccccc; border: 1px solid #3c3c3c; }"
        "QTreeView::item:selected { background-color: #094771; color: white; }"
        "QTreeView::item:hover { background-color: #2a2d2e; }"
    );
    
    connect(m_pEntityTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::onEntityTreeSelectionChanged);

    rightLayout->addWidget(m_pEntityTreeView);
}

void MainWindow::onEntityFilterChanged(const QString &text)
{
    if (m_pEntityFilterProxyModel)
    {
        m_pEntityFilterProxyModel->setFilterFixedString(text);
        if (!text.isEmpty() && m_pEntityTreeView) {
            m_pEntityTreeView->expandAll();
        }
    }
}

void MainWindow::onEntityTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!current.isValid() || !m_pEntityFilterProxyModel || !m_pEntityTreeModel) return;

    QModelIndex sourceIndex = m_pEntityFilterProxyModel->mapToSource(current);
    QStandardItem *item = m_pEntityTreeModel->itemFromIndex(sourceIndex);
    
    if (item && !item->hasChildren()) 
    {
        QString classname = item->text();
        std::string targetModelPath = FgdManager::Instance().GetModelPathForClass(classname.toUtf8().constData());
        
        // STREAM SELECTION STRINGS: Instantly updates the inline viewport context model path parameter!
        if (m_pModelViewerWidget)
        {
            m_pModelViewerWidget->LoadModelFile(QString::fromStdString(targetModelPath));
            statusBar()->showMessage(tr("Selected asset resource target: %1").arg(QString::fromStdString(targetModelPath)), 2000);
        }
    }
}

MainWindow::~MainWindow(){}
