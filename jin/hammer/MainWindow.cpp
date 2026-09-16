#include "MainWindow.h"
#include <QScrollBar>
#include <cmath>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStatusBar>
#include <QActionGroup>

//-----------------------------------------------------------------------------
// Hammer2DGridView Implementation
//-----------------------------------------------------------------------------
Hammer2DGridView::Hammer2DGridView(ViewOrientation orientation, QWidget *parent)
    : QGraphicsView(parent), m_orientation(orientation), m_gridSize(32),
      m_isPanning(false), m_editMode(MODE_NONE), m_selectedBrushId(-1), m_activeHandle(HANDLE_NONE),
      m_activeTool(TOOL_SELECT), m_currentEntityClass("info_player_start")
{
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::TextAntialiasing, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Crucial: Use standard scene routing backends to make items selectable
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setBackgroundBrush(QColor(30, 30, 30));
    setMouseTracking(true);
}

float Hammer2DGridView::snapToGrid(float value) const { return std::round(value / m_gridSize) * m_gridSize; }
QPointF Hammer2DGridView::snapToGrid(const QPointF &scenePos) const { return QPointF(snapToGrid(scenePos.x()), snapToGrid(scenePos.y())); }

void Hammer2DGridView::setGridSize(int size)
{
    if (size >= 1 && size <= 1024)
    {
        m_gridSize = size;
        updateHandlePositions();
        viewport()->update();
    }
}

void Hammer2DGridView::projectTo2D(const Vector &mins, const Vector &maxs, qreal &x, qreal &y, qreal &w, qreal &h)
{
    switch (m_orientation)
    {
    case VIEW_TOP:
        x = mins.x;
        y = -maxs.y;
        w = maxs.x - mins.x;
        h = maxs.y - mins.y;
        break;
    case VIEW_FRONT:
        x = mins.x;
        y = -maxs.z;
        w = maxs.x - mins.x;
        h = maxs.z - mins.z;
        break;
    case VIEW_SIDE:
        x = mins.y;
        y = -maxs.z;
        w = maxs.y - mins.y;
        h = maxs.z - mins.z;
        break;
    }
}

void Hammer2DGridView::unprojectFrom2D(qreal x, qreal y, qreal w, qreal h, Vector &targetMins, Vector &targetMaxs)
{
    qreal x1 = x;
    qreal x2 = x + w;
    qreal localY1 = -y;
    qreal localY2 = -(y + h);

    float outX1 = std::min(x1, x2);
    float outX2 = std::max(x1, x2);
    float outY1 = std::min(localY1, localY2);
    float outY2 = std::max(localY1, localY2);

    switch (m_orientation)
    {
    case VIEW_TOP:
        targetMins.x = outX1;
        targetMaxs.x = outX2;
        targetMins.y = outY1;
        targetMaxs.y = outY2;
        break;
    case VIEW_FRONT:
        targetMins.x = outX1;
        targetMaxs.x = outX2;
        targetMins.z = outY1;
        targetMaxs.z = outY2;
        break;
    case VIEW_SIDE:
        targetMins.y = outX1;
        targetMaxs.y = outX2;
        targetMins.z = outY1;
        targetMaxs.z = outY2;
        break;
    }
}

void Hammer2DGridView::convertDeltaTo3D(const QPointF &delta2D, Vector &outDelta3D)
{
    outDelta3D.Init(0, 0, 0);
    switch (m_orientation)
    {
    case VIEW_TOP:
        outDelta3D.Init(delta2D.x(), -delta2D.y(), 0);
        break;
    case VIEW_FRONT:
        outDelta3D.Init(delta2D.x(), 0, -delta2D.y());
        break;
    case VIEW_SIDE:
        outDelta3D.Init(0, delta2D.x(), -delta2D.y());
        break;
    }
}

void Hammer2DGridView::updateSceneData(const QVector<MapBrush> &brushes, const QVector<MapEntity> &entities, int selectedId, EditTool activeTool, const QString &entityClass)
{
    m_brushes = brushes;
    m_entities = entities;
    m_selectedBrushId = selectedId;
    m_activeTool = activeTool;
    m_currentEntityClass = entityClass;
    updateHandlePositions();
    viewport()->update();
}

// FIXED: Use standard view paint pipeline. Brushes and shapes draw on screen cleanly.
void Hammer2DGridView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    QPainter painter(viewport());
    painter.setTransform(transform());

    // 1. Draw Map Brushes Geometry
    for (const auto &brush : m_brushes)
    {
        qreal x, y, w, h;
        projectTo2D(brush.mins, brush.maxs, x, y, w, h);
        QRectF brushRect(x, y, w, h);
        
        bool isSelected = (brush.id == m_selectedBrushId);
        QColor displayColor = isSelected ? QColor(255, 0, 0) : brush.color;
        
        painter.setPen(QPen(displayColor, isSelected ? 2 : 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        painter.setBrush(QBrush(QColor(displayColor.red(), displayColor.green(), displayColor.blue(), isSelected ? 45 : 25)));
        painter.drawRect(brushRect);
    }

    // 2. Draw Point Entities Natively
    for (const auto &ent : m_entities)
    {
        qreal scrX = 0, scrY = 0;
        switch (m_orientation)
        {
            case VIEW_TOP:   scrX = ent.origin.x; scrY = -ent.origin.y; break;
            case VIEW_FRONT: scrX = ent.origin.x; scrY = -ent.origin.z; break;
            case VIEW_SIDE:  scrX = ent.origin.y; scrY = -ent.origin.z; break;
        }

        qreal size = 16.0; qreal half = size / 2.0;
        QRectF entRect(scrX - half, scrY - half, size, size);

        painter.setPen(QPen(ent.color, 1.5f, Qt::SolidLine));
        painter.setBrush(QBrush(QColor(ent.color.red(), ent.color.green(), ent.color.blue(), 60)));
        painter.drawRect(entRect);

        painter.drawLine(QPointF(scrX - half - 4, scrY), QPointF(scrX + half + 4, scrY));
        painter.drawLine(QPointF(scrX, scrY - half - 4), QPointF(scrX, scrY + half + 4));
    }

    // FIXED: Draw white handles inside the local transformation matrix loop!
    // This stops them from drifting away from the red bounding boxes during panning actions.
    if (m_selectedBrushId != -1 && m_activeTool == TOOL_SELECT)
    {
        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(Qt::white);
        for (int i = 0; i < 8; ++i)
        {
            if (!m_handleRects[i].isNull()) {
                painter.drawRect(m_handleRects[i]);
            }
        }
    }
}

void Hammer2DGridView::updateHandlePositions()
{
    if (m_selectedBrushId == -1)
    {
        for (int i = 0; i < 8; ++i)
            m_handleRects[i] = QRectF();
        return;
    }

    MapBrush selectedBrush;
    bool found = false;
    for (const auto &b : m_brushes)
    {
        if (b.id == m_selectedBrushId)
        {
            selectedBrush = b;
            found = true;
            break;
        }
    }
    if (!found)
        return;

    qreal x, y, w, h;
    projectTo2D(selectedBrush.mins, selectedBrush.maxs, x, y, w, h);
    m_selectedBrushRect = QRectF(x, y, w, h);

    qreal handleSize = 6.0 / transform().m11();
    qreal half = handleSize / 2.0;
    QRectF r = m_selectedBrushRect;

    // Map control handles perfectly around the active 2D selection rectangle boundary
    m_handleRects[HANDLE_TOP_LEFT] = QRectF(r.left() - half, r.top() - half, handleSize, handleSize);
    m_handleRects[HANDLE_TOP] = QRectF(r.center().x() - half, r.top() - half, handleSize, handleSize);
    m_handleRects[HANDLE_TOP_RIGHT] = QRectF(r.right() - half, r.top() - half, handleSize, handleSize);
    m_handleRects[HANDLE_RIGHT] = QRectF(r.right() - half, r.center().y() - half, handleSize, handleSize);
    m_handleRects[HANDLE_BOTTOM_RIGHT] = QRectF(r.right() - half, r.bottom() - half, handleSize, handleSize);
    m_handleRects[HANDLE_BOTTOM] = QRectF(r.center().x() - half, r.bottom() - half, handleSize, handleSize);
    m_handleRects[HANDLE_BOTTOM_LEFT] = QRectF(r.left() - half, r.bottom() - half, handleSize, handleSize);
    m_handleRects[HANDLE_LEFT] = QRectF(r.left() - half, r.center().y() - half, handleSize, handleSize);
}

int Hammer2DGridView::getBrushIdAtPosition(const QPointF &scenePos)
{
    // Test Entity selection bounds
    for (int i = m_entities.size() - 1; i >= 0; --i)
    {
        qreal scrX = 0, scrY = 0;
        switch (m_orientation)
        {
        case VIEW_TOP:
            scrX = m_entities[i].origin.x;
            scrY = -m_entities[i].origin.y;
            break;
        case VIEW_FRONT:
            scrX = m_entities[i].origin.x;
            scrY = -m_entities[i].origin.z;
            break;
        case VIEW_SIDE:
            scrX = m_entities[i].origin.y;
            scrY = -m_entities[i].origin.z;
            break;
        }
        if (QRectF(scrX - 10, scrY - 10, 20, 20).contains(scenePos))
        {
            return -(m_entities[i].id + 1000);
        }
    }

    // Test Brush block intersection bounds
    for (int i = m_brushes.size() - 1; i >= 0; --i)
    {
        qreal x, y, w, h;
        projectTo2D(m_brushes[i].mins, m_brushes[i].maxs, x, y, w, h);
        if (QRectF(x, y, w, h).contains(scenePos))
            return m_brushes[i].id;
    }
    return -1;
}

Hammer2DGridView::HandleIndex Hammer2DGridView::hitTestHandles(const QPointF &scenePos)
{
    if (m_selectedBrushId == -1 || m_activeTool != TOOL_SELECT)
        return HANDLE_NONE;
    for (int i = 0; i < 8; ++i)
    {
        if (m_handleRects[i].contains(scenePos))
            return static_cast<HandleIndex>(i);
    }
    return HANDLE_NONE;
}

void Hammer2DGridView::updateCursorForHandle(HandleIndex handle)
{
    switch (handle)
    {
    case HANDLE_TOP_LEFT:
    case HANDLE_BOTTOM_RIGHT:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case HANDLE_TOP_RIGHT:
    case HANDLE_BOTTOM_LEFT:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case HANDLE_TOP:
    case HANDLE_BOTTOM:
        setCursor(Qt::SizeVerCursor);
        break;
    case HANDLE_LEFT:
    case HANDLE_RIGHT:
        setCursor(Qt::SizeHorCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

void Hammer2DGridView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    qreal left = rect.left(); qreal top = rect.top(); qreal right = rect.right(); qreal bottom = rect.bottom();
    qreal firstX = std::floor(left / m_gridSize) * m_gridSize; qreal firstY = std::floor(top / m_gridSize) * m_gridSize;
    QPen customGridPen(QColor(55, 55, 55), 1.0); QPen majorGridPen(QColor(80, 80, 80), 1.0); QPen axisPen(QColor(180, 70, 70), 1.0);

    for (qreal x = firstX; x <= right; x += m_gridSize)
    {
        int intX = static_cast<int>(std::round(x));
        if (intX == 0) painter->setPen(axisPen);
        else if (intX % (m_gridSize * 8) == 0) painter->setPen(majorGridPen);
        else painter->setPen(customGridPen);
        painter->drawLine(QPointF(x, top), QPointF(x, bottom));
    }
    for (qreal y = firstY; y <= bottom; y += m_gridSize)
    {
        // FIXED: Explicitly provide the required template type target to the compiler
        int intY = static_cast<int>(std::round(y));
        if (intY == 0) painter->setPen(axisPen);
        else if (intY % (m_gridSize * 8) == 0) painter->setPen(majorGridPen);
        else painter->setPen(customGridPen);
        painter->drawLine(QPointF(left, y), QPointF(right, y));
    }
}

void Hammer2DGridView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);
    painter->setPen(Qt::yellow);
    QFont font = painter->font();
    font.setPixelSize(12);
    font.setBold(true);
    painter->setFont(font);
    QString labelText = "2D TOP (XY)";
    if (m_orientation == VIEW_FRONT)
        labelText = "2D FRONT (XZ)";
    if (m_orientation == VIEW_SIDE)
        labelText = "2D SIDE (YZ)";
    painter->drawText(mapToScene(QPoint(10, 20)), labelText);
    if (m_editMode == MODE_DRAWING && m_activeTool == TOOL_BLOCK)
    {
        qreal x = std::min(m_drawStartScene.x(), m_drawCurrentScene.x());
        qreal y = std::min(m_drawStartScene.y(), m_drawCurrentScene.y());
        qreal w = std::abs(m_drawStartScene.x() - m_drawCurrentScene.x());
        qreal h = std::abs(m_drawStartScene.y() - m_drawCurrentScene.y());
        painter->setPen(QPen(QColor(220, 40, 40), 1.5, Qt::DashLine));
        painter->setBrush(QBrush(QColor(220, 40, 40, 20)));
        painter->drawRect(QRectF(x, y, w, h));
    }
    // Draw white alignment squares natively over the bounding box overlay pass
    if (m_selectedBrushId != -1 && m_activeTool == TOOL_SELECT)
    {
        painter->setPen(QPen(Qt::white, 1));
        painter->setBrush(Qt::white);
        for (int i = 0; i < 8; ++i)
        {
            if (!m_handleRects[i].isNull())
                painter->drawRect(m_handleRects[i]);
        }
    }
}
void Hammer2DGridView::wheelEvent(QWheelEvent *event)
{
    qreal scaleFactor = 1.15;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    updateHandlePositions();
    viewport()->update();
}
void Hammer2DGridView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier)))
    {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton)
    {
        QPointF scenePos = mapToScene(event->pos());
        if (m_activeTool == TOOL_ENTITY)
        {
            QPointF snapped = snapToGrid(scenePos);
            Vector spawnOrigin(0, 0, 0);
            switch (m_orientation)
            {
            case VIEW_TOP:
                spawnOrigin.Init(snapped.x(), -snapped.y(), 0.0f);
                break;
            case VIEW_FRONT:
                spawnOrigin.Init(snapped.x(), 0.0f, -snapped.y());
                break;
            case VIEW_SIDE:
                spawnOrigin.Init(0.0f, snapped.x(), -snapped.y());
                break;
            }
            emit entityPlaced(spawnOrigin, m_currentEntityClass);
            event->accept();
            return;
        }
        if (m_activeTool == TOOL_SELECT)
        {
            HandleIndex clickedHandle = hitTestHandles(scenePos);
            if (clickedHandle != HANDLE_NONE)
            {
                m_editMode = MODE_RESIZING;
                m_activeHandle = clickedHandle;
                event->accept();
                return;
            }
            int clickedBrushId = getBrushIdAtPosition(scenePos);
            if (clickedBrushId != -1)
            {
                emit brushSelected(clickedBrushId);
                m_editMode = MODE_TRANSLATING;
                m_translateStartScene = snapToGrid(scenePos);
                m_translateCurrentScene = m_translateStartScene;
                event->accept();
                return;
            }
            emit brushSelected(-1);
        }
        if (m_activeTool == TOOL_BLOCK)
        {
            m_editMode = MODE_DRAWING;
            m_drawStartScene = snapToGrid(scenePos);
            m_drawCurrentScene = m_drawStartScene;
            viewport()->update();
            event->accept();
            return;
        }
    }
    QGraphicsView::mousePressEvent(event);
}
void Hammer2DGridView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePos = mapToScene(event->pos());
    if (m_isPanning)
    {
        QPointF delta = mapToScene(m_lastMousePos) - scenePos;
        m_lastMousePos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
        updateHandlePositions();
        event->accept();
        return;
    }
    if (m_editMode == MODE_NONE && m_activeTool == TOOL_SELECT)
    {
        HandleIndex hoveredHandle = hitTestHandles(scenePos);
        updateCursorForHandle(hoveredHandle);
    }
    if (m_editMode == MODE_DRAWING && m_activeTool == TOOL_BLOCK)
    {
        m_drawCurrentScene = snapToGrid(scenePos);
        viewport()->update();
        event->accept();
        return;
    }
    if (m_editMode == MODE_TRANSLATING && m_selectedBrushId != -1 && m_activeTool == TOOL_SELECT)
    {
        QPointF snappedScenePos = snapToGrid(scenePos);
        if (snappedScenePos != m_translateCurrentScene)
        {
            QPointF delta = snappedScenePos - m_translateCurrentScene;
            m_translateCurrentScene = snappedScenePos;
            Vector delta3D;
            convertDeltaTo3D(delta, delta3D);
            emit brushMoved(delta3D);
        }
        event->accept();
        return;
    }
    if (m_editMode == MODE_RESIZING && m_selectedBrushId != -1 && m_activeTool == TOOL_SELECT)
    {
        QPointF snappedMouse = snapToGrid(scenePos);
        QRectF newRect = m_selectedBrushRect;
        switch (m_activeHandle)
        {
        case HANDLE_TOP_LEFT:
            newRect.setTopLeft(snappedMouse);
            break;
        case HANDLE_TOP:
            newRect.setTop(snappedMouse.y());
            break;
        case HANDLE_TOP_RIGHT:
            newRect.setTopRight(snappedMouse);
            break;
        case HANDLE_RIGHT:
            newRect.setRight(snappedMouse.x());
            break;
        case HANDLE_BOTTOM_RIGHT:
            newRect.setBottomRight(snappedMouse);
            break;
        case HANDLE_BOTTOM:
            newRect.setBottom(snappedMouse.y());
            break;
        case HANDLE_BOTTOM_LEFT:
            newRect.setBottomLeft(snappedMouse);
            break;
        case HANDLE_LEFT:
            newRect.setLeft(snappedMouse.x());
            break;
        default:
            break;
        }
        if (newRect.width() >= m_gridSize && newRect.height() >= m_gridSize)
        {
            MapBrush currentBrush;
            for (const auto &b : m_brushes)
            {
                if (b.id == m_selectedBrushId)
                    currentBrush = b;
            }
            Vector finalMins = currentBrush.mins;
            Vector finalMaxs = currentBrush.maxs;
            unprojectFrom2D(newRect.x(), newRect.y(), newRect.width(), newRect.height(), finalMins, finalMaxs);
            emit brushResized(m_selectedBrushId, finalMins, finalMaxs);
        }
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}
void Hammer2DGridView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPanning)
    {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton)
    {
        if (m_editMode == MODE_DRAWING && m_activeTool == TOOL_BLOCK)
        {
            m_editMode = MODE_NONE;
            m_drawCurrentScene = snapToGrid(mapToScene(event->pos()));
            qreal x = std::min(m_drawStartScene.x(), m_drawCurrentScene.x());
            qreal w = std::abs(m_drawStartScene.x() - m_drawCurrentScene.x());
            qreal y = std::min(m_drawStartScene.y(), m_drawCurrentScene.y());
            qreal h = std::abs(m_drawStartScene.y() - m_drawCurrentScene.y());
            if (w > 0.1 && h > 0.1)
            {
                Vector mins(-64, -64, -64);
                Vector maxs(64, 64, 64);
                unprojectFrom2D(x, y, w, h, mins, maxs);
                emit brushCreated(mins, maxs);
            }
        }
        else if (m_editMode == MODE_TRANSLATING || m_editMode == MODE_RESIZING)
        {
            m_editMode = MODE_NONE;
            m_activeHandle = HANDLE_NONE;
            setCursor(Qt::ArrowCursor);
        }
        viewport()->update();
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}
//-----------------------------------------------------------------------------
// MainWindow Framework Methods
//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) // (Adjust if renamed to MainWindow)
    : QMainWindow(parent), m_nextBrushId(3), m_nextEntityId(1), m_selectedBrushId(-1), m_activeTool(TOOL_SELECT), m_currentEntityClass("info_player_start")
{
    resize(1300, 850);
    setWindowTitle(tr("Qt6 Hammer Engine - Multi-Viewport Split Grid System"));
    
    m_pGridScene = new QGraphicsScene(this);
    m_pGridScene->setSceneRect(-16384, -16384, 32768, 32768);
    
    Hammer2DGridView *topView   = new Hammer2DGridView(Hammer2DGridView::VIEW_TOP, this);
    Hammer2DGridView *frontView = new Hammer2DGridView(Hammer2DGridView::VIEW_FRONT, this);
    Hammer2DGridView *sideView  = new Hammer2DGridView(Hammer2DGridView::VIEW_SIDE, this);
    
    topView->setScene(m_pGridScene); 
    frontView->setScene(m_pGridScene); 
    sideView->setScene(m_pGridScene);
    
    m_views.append(topView); 
    m_views.append(frontView); 
    m_views.append(sideView);
    
    m_p3DViewport = new QModelView(this); // Swapped to ModelView5
    
    QSplitter *vSplitterLeft = new QSplitter(Qt::Vertical, this);
    vSplitterLeft->addWidget(topView); 
    vSplitterLeft->addWidget(frontView); 
    vSplitterLeft->setSizes(QList<int>({400, 400}));
    
    QSplitter *vSplitterRight = new QSplitter(Qt::Vertical, this);
    vSplitterRight->addWidget(m_p3DViewport); 
    vSplitterRight->addWidget(sideView); 
    vSplitterRight->setSizes(QList<int>({400, 400}));
    
    QSplitter *hMainSplitter = new QSplitter(Qt::Horizontal, this);
    hMainSplitter->addWidget(vSplitterLeft); 
    hMainSplitter->addWidget(vSplitterRight); 
    hMainSplitter->setSizes(QList<int>({600, 600}));

    for (auto view : m_views)
    {
        connect(view, &Hammer2DGridView::brushCreated, this, &MainWindow::onBrushCreated);
        connect(view, &Hammer2DGridView::brushSelected, this, &MainWindow::onBrushSelected);
        connect(view, &Hammer2DGridView::brushMoved, this, &MainWindow::onBrushMoved);
        connect(view, &Hammer2DGridView::brushResized, this, &MainWindow::onBrushResized);
        connect(view, &Hammer2DGridView::entityPlaced, this, &MainWindow::onEntityPlaced);
    }
    createMenuBarActions();
    createSidebarToolbox();

    QWidget *mainContainer = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);
    mainLayout->addWidget(hMainSplitter, 1);
    setCentralWidget(mainContainer);
    
    generateMockBrushes();
    syncAllViews();
    
    // FIXED: Instead of forcing a rigid centerOn calculation that breaks local coordinates transforms,
    // we center on the active world center and apply our 1.5x zoom factor natively.
    for (auto view : m_views) {
        view->resetTransform();
        view->scale(1.5, 1.5);
        view->centerOn(0, 0);
    }
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
    MapEntity ent;
    ent.id = m_nextEntityId++;
    ent.classname = classname;
    ent.origin = origin;
    if (classname == "info_player_start")
        ent.color = QColor(0, 255, 0);
    else if (classname == "light")
        ent.color = QColor(255, 255, 0);
    else
        ent.color = QColor(230, 0, 255);
    m_mapEntities.append(ent);
    syncAllViews();
}
void MainWindow::syncAllViews()
{
    for (auto view : m_views)
    {
        view->updateSceneData(m_mapBrushes, m_mapEntities, m_selectedBrushId, m_activeTool, m_currentEntityClass);
    }
    if (m_p3DViewport)
    {
        m_p3DViewport->updateBrushes(m_mapBrushes.constData(), m_mapBrushes.size(), m_selectedBrushId);
        m_p3DViewport->updateEntities(reinterpret_cast<const ModelViewEntity *>(m_mapEntities.constData()), m_mapEntities.size());
    }
    m_pGridScene->update();
    // FORCE scene tree hierarchy to repaint completely
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
        if (loadFromVMF(filePath)) {
            statusBar()->showMessage(tr("Map loaded successfully."), 3000);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to parse VMF map structure."));
        }
    }
}

void MainWindow::triggerSaveDialog()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Valve Map File"), "", tr("Valve Map Files (.vmf)"));
    if (!filePath.isEmpty())
    {
        if (saveToVMF(filePath))
            statusBar()->showMessage(tr("Map saved successfully."), 3000);
    }
}

bool MainWindow::loadFromVMF(const QString &filePath) // (Ensure class name matches MainWindow3/MainWindow4)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    m_mapBrushes.clear();
    m_mapEntities.clear();
    m_selectedBrushId = -1;

    QTextStream in(&file);
    
    // Parser state tracking flags
    bool inSolid = false;
    bool inEntity = false;
    
    MapBrush currentBrush;
    MapEntity currentEntity;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        // --- CHECK BLOCK BEGINNINGS ---
        if (line == "solid")
        {
            inSolid = true;
            currentBrush = MapBrush();
            currentBrush.color = QColor(0, 255, 100); // Green placeholder default
            continue;
        }
        if (line == "entity")
        {
            inEntity = true;
            currentEntity = MapEntity();
            currentEntity.color = QColor(230, 0, 255); // Magenta fallback default
            continue;
        }

        // --- CHECK SOLID BRUSH SCOPE ---
        if (inSolid)
        {
            if (line == "}")
            {
                m_mapBrushes.append(currentBrush);
                m_nextBrushId = std::max(m_nextBrushId, currentBrush.id + 1);
                inSolid = false;
                continue;
            }

            QStringList tokens = line.split(" ", Qt::SkipEmptyParts);
            if (tokens.size() >= 2)
            {
                QString key = tokens[0].remove('"');
                if (key == "id")
                {
                    currentBrush.id = tokens[1].remove('"').toInt();
                }
                else if (key == "mins" && tokens.size() >= 4)
                {
                    float x = tokens[1].remove('"').toFloat();
                    float y = tokens[2].remove('"').toFloat();
                    float z = tokens[3].remove('"').toFloat();
                    currentBrush.mins.Init(x, y, z);
                }
                else if (key == "maxs" && tokens.size() >= 4)
                {
                    float x = tokens[1].remove('"').toFloat();
                    float y = tokens[2].remove('"').toFloat();
                    float z = tokens[3].remove('"').toFloat();
                    currentBrush.maxs.Init(x, y, z);
                }
                else if (key == "color" && tokens.size() >= 4)
                {
                    int r = tokens[1].remove('"').toInt();
                    int g = tokens[2].remove('"').toInt();
                    int b = tokens[3].remove('"').toInt();
                    currentBrush.color = QColor(r, g, b);
                }
            }
        }

        // --- CHECK POINT ENTITY SCOPE ---
        if (inEntity)
        {
            if (line == "}")
            {
                m_mapEntities.append(currentEntity);
                m_nextEntityId = std::max(m_nextEntityId, currentEntity.id + 1);
                inEntity = false;
                continue;
            }

            QStringList tokens = line.split(" ", Qt::SkipEmptyParts);
            if (tokens.size() >= 2)
            {
                QString key = tokens[0].remove('"');
                if (key == "id")
                {
                    currentEntity.id = tokens[1].remove('"').toInt();
                }
                else if (key == "classname")
                {
                    currentEntity.classname = tokens[1].remove('"');
                    if (currentEntity.classname == "info_player_start") currentEntity.color = QColor(0, 255, 0);
                    else if (currentEntity.classname == "light")         currentEntity.color = QColor(255, 255, 0);
                }
                else if (key == "origin" && tokens.size() >= 4)
                {
                    float x = tokens[1].remove('"').toFloat();
                    float y = tokens[2].remove('"').toFloat();
                    float z = tokens[3].remove('"').toFloat();
                    currentEntity.origin.Init(x, y, z);
                }
            }
        }
    }

    file.close();
    syncAllViews(); // Repaint every screen view node completely
    return true;
}

bool MainWindow::saveToVMF(const QString &filePath) // (Ensure class name matches MainWindow3/MainWindow4)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);

    // Standard Valve Hammer editor environmental blocks header
    out << "versioninfo\n{\n\t\"editorversion\" \"400\"\n\t\"editorbuild\" \"9030\"\n}\n";
    out << "viewsettings\n{\n\t\"bSnapToGrid\" \"1\"\n\t\"enumGridSpacing\" \"32\"\n}\n";
    
    // Begin World block container
    out << "world\n{\n\t\"id\" \"1\"\n\t\"mapversion\" \"1\"\n\t\"classname\" \"worldspawn\"\n";

    // 1. Serialize solid Brushes
    for (const auto &brush : m_mapBrushes)
    {
        out << "\tsolid\n\t{\n";
        out << "\t\t\"id\" \"" << brush.id << "\"\n";
        out << "\t\t\"mins\" \"" << brush.mins.x << " " << brush.mins.y << " " << brush.mins.z << "\"\n";
        out << "\t\t\"maxs\" \"" << brush.maxs.x << " " << brush.maxs.y << " " << brush.maxs.z << "\"\n";
        out << "\t\t\"color\" \"" << brush.color.red() << " " << brush.color.green() << " " << brush.color.blue() << "\"\n";
        out << "\t}\n";
    }
    out << "}\n"; // Close World block container

    // 2. Serialize point Entities outside worldspawn block
    for (const auto &ent : m_mapEntities)
    {
        out << "entity\n{\n";
        out << "\t\"id\" \"" << ent.id << "\"\n";
        out << "\t\"classname\" \"" << ent.classname << "\"\n";
        out << "\t\"origin\" \"" << ent.origin.x << " " << ent.origin.y << " " << ent.origin.z << "\"\n";
        out << "}\n";
    }

    file.close();
    return true;
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
    MapBrush b;
    b.id = m_nextBrushId++;
    b.mins = mins;
    b.maxs = maxs;
    b.color = QColor(0, 255, 100);
    m_mapBrushes.append(b);
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
        for (auto &b : m_mapBrushes)
        {
            if (b.id == m_selectedBrushId)
            {
                b.mins += delta3D;
                b.maxs += delta3D;
                break;
            }
        }
        syncAllViews();
    }
}
void MainWindow::onBrushResized(int id, const Vector &mi, const Vector &ma)
{
    for (auto &b : m_mapBrushes)
    {
        if (b.id == id)
        {
            b.mins = mi;
            b.maxs = ma;
            break;
        }
    }
    syncAllViews();
}
void MainWindow::deleteSelectedBrush()
{
    if (m_selectedBrushId != -1)
    {
        for (int i = 0; i < m_mapBrushes.size(); ++i)
        {
            if (m_mapBrushes[i].id == m_selectedBrushId)
            {
                m_mapBrushes.removeAt(i);
                break;
            }
        }
        m_selectedBrushId = -1;
        syncAllViews();
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