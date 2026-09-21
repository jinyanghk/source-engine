#include "ModelView.h"
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

#include "materialsystem/imaterialsystem.h"
#include "istudiorender.h"
#include "datacache/imdlcache.h"

extern IMDLCache *g_pMDLCache;

extern "C" {
    void Hammer_RegisterModelViewerCanvas(void* pQtWindowToken, void* pOSWindowHandle, int w, int h);
    void Hammer_NotifyModelViewerResize(void* pQtWindowToken, int w, int h);
    void Hammer_PassModelViewerInput(void* pQtWindowToken, int eventType, int x, int y, int delta);
    void Hammer_AttachEngineContextToNativeWindow(void* pOSWindowHandle, int w, int h);
    void Hammer_SetModelViewerActiveAsset(void* pQtWindowToken, unsigned short hModel);
}

// Update this constructor section at the top of ModelView.cpp:
QModelView::QModelView(QWidget *parent)
    : QWidget(parent), 
      m_szCurrentModelPath(""),
      m_hCurrentModel(0xFFFF), 
      m_flAnimationCycle(0.0f), 
      m_nActiveSequenceIndex(0), 
      m_bPlaybackPaused(false),
      m_pForeignWindow(nullptr), 
      m_pEmbeddedWidget(nullptr),
      m_pEngineTickTimer(nullptr),
      m_bContextAttached(false)
{
    setAttribute(Qt::WA_NativeWindow, true);

    QVBoxLayout* pMainLayout = new QVBoxLayout(this);
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->setSpacing(0);

    this->installEventFilter(this);
}

QModelView::~QModelView()
{
    Hammer_RegisterModelViewerCanvas(reinterpret_cast<void*>(this), nullptr, 0, 0);
}

void QModelView::TriggerEngineBinding()
{
    if (m_bContextAttached) return;
    m_bContextAttached = true;

    // FIX: Removed the single-shot timer. QOpenGLWidget guarantees immediate local window handles!
    int startWidth = qMax(64, this->width());
    int startHeight = qMax(64, this->height());
    WId localWinId = this->winId();

    // Register and hijack the valid OpenGL hardware handle context directly
    Hammer_RegisterModelViewerCanvas(reinterpret_cast<void*>(this), reinterpret_cast<void*>(localWinId), startWidth, startHeight);
    Hammer_AttachEngineContextToNativeWindow(reinterpret_cast<void*>(localWinId), startWidth, startHeight);

    if (!m_szCurrentModelPath.isEmpty())
    {
        this->LoadModelFile(m_szCurrentModelPath);
    }
}

// FIX: Completely deleted the custom paintEvent override to let QOpenGLWidget manage its internal framebuffers safely

void QModelView::LoadModelFile(const QString &szPath)
{
    m_szCurrentModelPath = szPath;
    
    if (!g_pMDLCache || m_szCurrentModelPath.isEmpty())
        return;

    m_hCurrentModel = g_pMDLCache->FindMDL(m_szCurrentModelPath.toUtf8().constData());
    Hammer_SetModelViewerActiveAsset(reinterpret_cast<void*>(this), m_hCurrentModel);
    
    // Signal a clean viewport paint refresh tick loop update pass
    this->update();
}

void QModelView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int w = qMax(64, width());
    int h = qMax(64, height());
    Hammer_NotifyModelViewerResize(reinterpret_cast<void*>(this), w, h);
}

bool QModelView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this)
    {
        switch (event->type())
        {
            case QEvent::MouseButtonPress:
            {
                QMouseEvent* pMouse = static_cast<QMouseEvent*>(event);
                Hammer_PassModelViewerInput(reinterpret_cast<void*>(this), 1, pMouse->position().x(), pMouse->position().y(), 0);
                return true;
            }
            case QEvent::MouseButtonRelease:
            {
                QMouseEvent* pMouse = static_cast<QMouseEvent*>(event);
                Hammer_PassModelViewerInput(reinterpret_cast<void*>(this), 2, pMouse->position().x(), pMouse->position().y(), 0);
                return true;
            }
            case QEvent::MouseMove:
            {
                QMouseEvent* pMouse = static_cast<QMouseEvent*>(event);
                Hammer_PassModelViewerInput(reinterpret_cast<void*>(this), 3, pMouse->position().x(), pMouse->position().y(), 0);
                return true;
            }
            case QEvent::Wheel:
            {
                QWheelEvent* pWheel = static_cast<QWheelEvent*>(event);
                Hammer_PassModelViewerInput(reinterpret_cast<void*>(this), 4, 0, 0, pWheel->angleDelta().y());
                return true;
            }
            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

int QModelView::GetSequenceCount() { return 0; }
const char* QModelView::GetSequenceName(int index) { Q_UNUSED(index); return ""; }
void QModelView::SetActiveSequence(int index) { Q_UNUSED(index); }
void QModelView::SetAnimationCycle(float flCycle) { Q_UNUSED(flCycle); }
void QModelView::SetPlaybackPaused(bool bPaused) { Q_UNUSED(bPaused); }
