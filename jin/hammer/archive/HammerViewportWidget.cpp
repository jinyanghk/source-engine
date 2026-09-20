#include "HammerViewportWidget.h"
#include <QMouseEvent>

// Core Source Engine Interface Headers
#include "materialsystem/imaterialsystem.h"

// Reference globals managed by your main application entry point
extern IMaterialSystem* g_pMaterialSystem;

CHammerViewportWidget::CHammerViewportWidget(int viewportType, QWidget *parent)
    : QOpenGLWidget(parent), m_iViewportType(viewportType), m_bContextReady(false)
{
    m_pRenderTimer = new QTimer(this);
    connect(m_pRenderTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    m_pRenderTimer->start(16); // ~60 FPS
}

CHammerViewportWidget::~CHammerViewportWidget()
{
}

void CHammerViewportWidget::initializeGL()
{
    // Initialize the OpenGL function lookup tables for this specific context
    initializeOpenGLFunctions();

    if ( !g_pMaterialSystem )
        return;

    // Fix: Pass simple integer arguments to match the Linux engine method signature!
    // This stops pointer contamination errors dead in their tracks.
    g_pMaterialSystem->SetAdapter( 0, 0 ); 

    // Signal that functions are bound and safe to execute
    m_bContextReady = true; 
}

void CHammerViewportWidget::resizeGL(int w, int h)
{
}

void CHammerViewportWidget::paintGL()
{
    // Block paint rendering updates completely until initializeGL finishes
    if ( !m_bContextReady || !g_pMaterialSystem )
        return;

    // Fetch IMatRenderContext directly from the loaded material system interface
    IMatRenderContext *pRenderContext = g_pMaterialSystem->GetRenderContext();
    if ( !pRenderContext )
        return;

    // Call internal engine context tracking bounds safely
    pRenderContext->Viewport( 0, 0, width(), height() );
    
    // Clear to dark gray editor grid background
    pRenderContext->ClearColor3ub( 45, 45, 45 );
    pRenderContext->ClearBuffers( true, true );

    g_pMaterialSystem->BeginFrame( 0 );
    g_pMaterialSystem->EndFrame();
}

void CHammerViewportWidget::mousePressEvent(QMouseEvent *event) {}
void CHammerViewportWidget::mouseMoveEvent(QMouseEvent *event) {}
