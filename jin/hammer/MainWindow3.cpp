#include "MainWindow3.h"
#include <QScrollBar>
#include <cmath>

//-----------------------------------------------------------------------------
// Hammer2DGridView Implementation
//-----------------------------------------------------------------------------
Hammer2DGridView::Hammer2DGridView(ViewOrientation orientation, QWidget *parent)
    : QGraphicsView(parent), m_orientation(orientation), m_gridSize(32),
      m_isPanning(false), m_editMode(MODE_NONE), m_selectedBrushId(-1), m_activeHandle(HANDLE_NONE)
{
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::TextAntialiasing, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setBackgroundBrush(QColor(30, 30, 30));
    setMouseTracking(true);
}

float Hammer2DGridView::snapToGrid(float value) const
{
    return std::round(value / m_gridSize) * m_gridSize;
}

QPointF Hammer2DGridView::snapToGrid(const QPointF &scenePos) const
{
    return QPointF(snapToGrid(scenePos.x()), snapToGrid(scenePos.y()));
}

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
    qreal y1 = -y;
    qreal y2 = -(y + h);

    float outX1 = std::min(x1, x2);
    float outX2 = std::max(x1, x2);
    float outY1 = std::min(y1, y2);
    float outY2 = std::max(y1, y2);

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

void Hammer2DGridView::updateBrushes(const QVector<MapBrush> &brushes, int selectedId)
{
    m_brushes = brushes;
    m_selectedBrushId = selectedId;
    updateHandlePositions();
    viewport()->update();
}

void Hammer2DGridView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setTransform(transform());

    for (const auto &brush : m_brushes)
    {
        qreal x, y, w, h;
        projectTo2D(brush.mins, brush.maxs, x, y, w, h);
        QRectF brushRect(x, y, w, h);

        bool isSelected = (brush.id == m_selectedBrushId);
        QColor displayColor = isSelected ? QColor(255, 0, 0) : brush.color;

        painter.setPen(QPen(displayColor, isSelected ? 2 : 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        painter.setBrush(QBrush(QColor(displayColor.red(), displayColor.green(), displayColor.blue(), isSelected ? 50 : 35)));
        painter.drawRect(brushRect);
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
    for (int i = m_brushes.size() - 1; i >= 0; --i)
    {
        qreal x, y, w, h;
        projectTo2D(m_brushes[i].mins, m_brushes[i].maxs, x, y, w, h);
        if (QRectF(x, y, w, h).contains(scenePos))
        {
            return m_brushes[i].id;
        }
    }
    return -1;
}

// Fixed Handle Matrix references bounds cleanly
Hammer2DGridView::HandleIndex Hammer2DGridView::hitTestHandles(const QPointF &scenePos)
{
    if (m_selectedBrushId == -1)
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

    qreal left = rect.left();
    qreal top = rect.top();
    qreal right = rect.right();
    qreal bottom = rect.bottom();

    qreal firstX = std::floor(left / m_gridSize) * m_gridSize;
    qreal firstY = std::floor(top / m_gridSize) * m_gridSize;

    QPen customGridPen(QColor(55, 55, 55), 1.0);
    QPen majorGridPen(QColor(80, 80, 80), 1.0);
    QPen axisPen(QColor(180, 70, 70), 1.0);

    for (qreal x = firstX; x <= right; x += m_gridSize)
    {
        int intX = static_cast<int>(std::round(x));
        if (intX == 0)
            painter->setPen(axisPen);
        else if (intX % (m_gridSize * 8) == 0)
            painter->setPen(majorGridPen);
        else
            painter->setPen(customGridPen);
        painter->drawLine(QPointF(x, top), QPointF(x, bottom));
    }

    for (qreal y = firstY; y <= bottom; y += m_gridSize)
    {
        int intY = static_cast<int>(std::round(y));
        if (intY == 0)
            painter->setPen(axisPen);
        else if (intY % (m_gridSize * 8) == 0)
            painter->setPen(majorGridPen);
        else
            painter->setPen(customGridPen);
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

    QPointF textPos = mapToScene(QPoint(10, 20));
    painter->drawText(textPos, labelText);

    if (m_editMode == MODE_DRAWING)
    {
        qreal x = std::min(m_drawStartScene.x(), m_drawCurrentScene.x());
        qreal y = std::min(m_drawStartScene.y(), m_drawCurrentScene.y());
        qreal w = std::abs(m_drawStartScene.x() - m_drawCurrentScene.x());
        qreal h = std::abs(m_drawStartScene.y() - m_drawCurrentScene.y());
        painter->setPen(QPen(QColor(220, 40, 40), 1.5, Qt::DashLine));
        painter->setBrush(QBrush(QColor(220, 40, 40, 20)));
        painter->drawRect(QRectF(x, y, w, h));
    }

    if (m_selectedBrushId != -1)
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
        m_editMode = MODE_DRAWING;
        m_drawStartScene = snapToGrid(scenePos);
        m_drawCurrentScene = m_drawStartScene;
        viewport()->update();
        event->accept();
        return;
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
    if (m_editMode == MODE_NONE)
    {
        HandleIndex hoveredHandle = hitTestHandles(scenePos);
        updateCursorForHandle(hoveredHandle);
    }
    if (m_editMode == MODE_DRAWING)
    {
        m_drawCurrentScene = snapToGrid(scenePos);
        viewport()->update();
        event->accept();
        return;
    }
    if (m_editMode == MODE_TRANSLATING && m_selectedBrushId != -1)
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
    if (m_editMode == MODE_RESIZING && m_selectedBrushId != -1)
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
        if (m_editMode == MODE_DRAWING)
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
// MainWindow3 Layout Structure
//-----------------------------------------------------------------------------
MainWindow3::MainWindow3(QWidget *parent) : QMainWindow(parent), m_nextBrushId(3), m_selectedBrushId(-1)
{
    resize(1200, 850);
    setWindowTitle(tr("Qt6 Hammer Engine - Multi-Viewport Split Grid System"));
    m_pGridScene = new QGraphicsScene(this);
    m_pGridScene->setSceneRect(-16384, -16384, 32768, 32768);
    Hammer2DGridView *topView = new Hammer2DGridView(Hammer2DGridView::VIEW_TOP, this);
    Hammer2DGridView *frontView = new Hammer2DGridView(Hammer2DGridView::VIEW_FRONT, this);
    Hammer2DGridView *sideView = new Hammer2DGridView(Hammer2DGridView::VIEW_SIDE, this);
    topView->setScene(m_pGridScene);
    frontView->setScene(m_pGridScene);
    sideView->setScene(m_pGridScene);
    m_views.append(topView);
    m_views.append(frontView);
    m_views.append(sideView);
    m_p3DViewport = new QModelView4(this);
    QSplitter *vSplitterLeft = new QSplitter(Qt::Vertical, this);
    vSplitterLeft->addWidget(topView);
    vSplitterLeft->addWidget(frontView);
    vSplitterLeft->setSizes(QList({400, 400}));
    QSplitter *vSplitterRight = new QSplitter(Qt::Vertical, this);
    vSplitterRight->addWidget(m_p3DViewport);
    vSplitterRight->addWidget(sideView);
    vSplitterRight->setSizes(QList({400, 400}));
    // FIX 1: Enforce perfect 50/50 sizing blocks layout allocation
    QSplitter *hMainSplitter = new QSplitter(Qt::Horizontal, this);
    hMainSplitter->addWidget(vSplitterLeft);
    hMainSplitter->addWidget(vSplitterRight);
    hMainSplitter->setSizes(QList({600, 600}));
    // FIX 1: Enforce horizontal layout distribution bounds
    setCentralWidget(hMainSplitter);
    for (auto view : m_views)
    {
        connect(view, &Hammer2DGridView::brushCreated, this, &MainWindow3::onBrushCreated);
        connect(view, &Hammer2DGridView::brushSelected, this, &MainWindow3::onBrushSelected);
        connect(view, &Hammer2DGridView::brushMoved, this, &MainWindow3::onBrushMoved);
        connect(view, &Hammer2DGridView::brushResized, this, &MainWindow3::onBrushResized);
    }
    generateMockBrushes();
    syncAllViews();
    for (auto view : m_views)
    {
        view->centerOn(0, 0);
        view->scale(1.5, 1.5);
    }
}
MainWindow3::~MainWindow3() {}
void MainWindow3::generateMockBrushes()
{
    MapBrush spawnRoom;
    spawnRoom.id = 1;
    spawnRoom.mins.Init(-256.0f, -256.0f, -128.0f);
    spawnRoom.maxs.Init(256.0f, 256.0f, 128.0f);
    spawnRoom.color = QColor(255, 128, 0);
    m_mapBrushes.append(spawnRoom);
    MapBrush pillar;
    pillar.id = 2;
    pillar.mins.Init(64.0f, 64.0f, -128.0f);
    pillar.maxs.Init(128.0f, 128.0f, 128.0f);
    pillar.color = QColor(0, 180, 255);
    m_mapBrushes.append(pillar);
}
void MainWindow3::syncAllViews()
{
    // Update orthographic 2D grids
    for (auto view : m_views)
    {
        view->updateBrushes(m_mapBrushes, m_selectedBrushId);
    }
    
    // SAFE FIX: Pass the contiguous data block pointer directly down to the 3D Viewport
    if (m_p3DViewport)
    {
        m_p3DViewport->updateBrushes(m_mapBrushes.constData(), m_mapBrushes.size(), m_selectedBrushId);
    }
}
void MainWindow3::onBrushCreated(const Vector &mins, const Vector &maxs)
{
    MapBrush newBrush;
    newBrush.id = m_nextBrushId++;
    newBrush.mins = mins;
    newBrush.maxs = maxs;
    newBrush.color = QColor(0, 255, 100);
    m_mapBrushes.append(newBrush);
    syncAllViews();
}
void MainWindow3::onBrushSelected(int brushId)
{
    m_selectedBrushId = brushId;
    syncAllViews();
}
void MainWindow3::onBrushMoved(const Vector &delta3D)
{
    if (m_selectedBrushId == -1)
        return;
    for (int i = 0; i < m_mapBrushes.size(); ++i)
    {
        if (m_mapBrushes[i].id == m_selectedBrushId)
        {
            m_mapBrushes[i].mins += delta3D;
            m_mapBrushes[i].maxs += delta3D;
            break;
        }
    }
    syncAllViews();
}
void MainWindow3::onBrushResized(int brushId, const Vector &newMins, const Vector &newMaxs)
{
    for (int i = 0; i < m_mapBrushes.size(); ++i)
    {
        if (m_mapBrushes[i].id == brushId)
        {
            m_mapBrushes[i].mins = newMins;
            m_mapBrushes[i].maxs = newMaxs;
            break;
        }
    }
    syncAllViews();
}
void MainWindow3::deleteSelectedBrush()
{
    if (m_selectedBrushId == -1)
        return;
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
void MainWindow3::keyPressEvent(QKeyEvent *event)
{
    int currentSize = m_views.first()->gridSize();
    if (event->key() == Qt::Key_BracketLeft)
    {
        for (auto view : m_views)
            view->setGridSize(currentSize / 2);
        event->accept();
    }
    else if (event->key() == Qt::Key_BracketRight)
    {
        for (auto view : m_views)
            view->setGridSize(currentSize * 2);
        event->accept();
    }
    else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
        deleteSelectedBrush();
        event->accept();
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}