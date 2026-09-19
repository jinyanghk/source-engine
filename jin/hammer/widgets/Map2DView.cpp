#include "Map2DView.h"
#include <QScrollBar>
#include <QPainter>
#include <cmath>
#include <algorithm>

Map2DView::Map2DView(ViewOrientation orientation, QWidget *parent)
    : QGraphicsView(parent), m_orientation(orientation), m_gridSize(32),
      m_isPanning(false), m_editMode(MODE_NONE), m_selectedBrushId(-1), m_activeHandle(HANDLE_NONE),
      m_activeTool(TOOL_SELECT), m_currentEntityClass("info_player_start")
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

float Map2DView::snapToGrid(float value) const { return std::round(value / m_gridSize) * m_gridSize; }
QPointF Map2DView::snapToGrid(const QPointF &scenePos) const { return QPointF(snapToGrid(scenePos.x()), snapToGrid(scenePos.y())); }

void Map2DView::setGridSize(int size)
{
    if (size >= 1 && size <= 1024)
    {
        m_gridSize = size;
        updateHandlePositions();
        viewport()->update();
    }
}

void Map2DView::projectTo2D(const Vector &mins, const Vector &maxs, qreal &x, qreal &y, qreal &w, qreal &h)
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

void Map2DView::unprojectFrom2D(qreal x, qreal y, qreal w, qreal h, Vector &targetMins, Vector &targetMaxs)
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

void Map2DView::convertDeltaTo3D(const QPointF &delta2D, Vector &outDelta3D)
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

void Map2DView::updateSceneData(const QVector<MapBrush> &brushes, const QVector<MapEntity> &entities, int selectedId, EditTool activeTool, const QString &entityClass)
{
    m_brushes = brushes;
    m_entities = entities;
    m_selectedBrushId = selectedId;
    m_activeTool = activeTool;
    m_currentEntityClass = entityClass;
    updateHandlePositions();
    viewport()->update();
}

void Map2DView::paintEvent(QPaintEvent *event)
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

    // 2. Draw Point Entities Natively aligned to World Axis Transformations
    for (const auto &ent : m_entities)
    {
        qreal scrX = 0, scrY = 0;
        switch (m_orientation)
        {
        case VIEW_TOP:   
            scrX = ent.origin.x; 
            scrY = -ent.origin.y; 
            break;
        case VIEW_FRONT: 
            scrX = ent.origin.x; 
            scrY = -ent.origin.z; 
            break;
        case VIEW_SIDE:  
            scrX = ent.origin.y; 
            scrY = -ent.origin.z; 
            break;
        }

        // FIX: Draw a robust 3D bounding box footprint mapped directly to world coordinates
        // so it scales, centers, and anchors perfectly on your grid crosshair line paths!
        qreal size = 16.0; 
        qreal half = size / 2.0;
        QRectF entRect(scrX - half, scrY - half, size, size);

        // Map colors driven dynamically from your FGD configuration template definitions
        painter.setPen(QPen(ent.color, 1.5f, Qt::SolidLine));
        painter.setBrush(QBrush(QColor(ent.color.red(), ent.color.green(), ent.color.blue(), 45)));
        
        // Paint the point box outline footprint
        painter.drawRect(entRect);

        // Draw internal target alignment crosshair markers centered within the block
        painter.drawLine(QPointF(scrX - half - 4, scrY), QPointF(scrX + half + 4, scrY));
        painter.drawLine(QPointF(scrX, scrY - half - 4), QPointF(scrX, scrY + half + 4));
    }

    if (m_selectedBrushId != -1 && m_activeTool == TOOL_SELECT)
    {
        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(Qt::white);
        for (int i = 0; i < 8; ++i)
        {
            if (!m_handleRects[i].isNull())
            {
                painter.drawRect(m_handleRects[i]);
            }
        }
    }
}

void Map2DView::updateHandlePositions()
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

int Map2DView::getBrushIdAtPosition(const QPointF &scenePos)
{
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

    for (int i = m_brushes.size() - 1; i >= 0; --i)
    {
        qreal x, y, w, h;
        projectTo2D(m_brushes[i].mins, m_brushes[i].maxs, x, y, w, h);
        if (QRectF(x, y, w, h).contains(scenePos))
            return m_brushes[i].id;
    }
    return -1;
}

Map2DView::HandleIndex Map2DView::hitTestHandles(const QPointF &scenePos)
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

void Map2DView::updateCursorForHandle(HandleIndex handle)
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

void Map2DView::drawBackground(QPainter *painter, const QRectF &rect)
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
void Map2DView::drawForeground(QPainter *painter, const QRectF &rect)
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
void Map2DView::wheelEvent(QWheelEvent *event)
{
    qreal scaleFactor = 1.15;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    updateHandlePositions();
    viewport()->update();
}
void Map2DView::mousePressEvent(QMouseEvent *event)
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
void Map2DView::mouseMoveEvent(QMouseEvent *event)
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
void Map2DView::mouseReleaseEvent(QMouseEvent *event)
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