#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QTimer>
#include <QImage>

typedef unsigned short MDLHandle_t;
// Forward declaration of engine texture interface
class ITexture;

class QModelView3 : public QWidget
{
    Q_OBJECT

public:
    explicit QModelView3(QWidget *parent = nullptr);
    virtual ~QModelView3();

    void LoadModelFile(const QString &szFilePath);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void RenderEngineFrame();

    QString      m_szCurrentModelPath;
    MDLHandle_t  m_hCurrentModel;
    float        m_flAnimationCycle;

    // Viewport camera tracking metrics
    float   m_flZoomScale;
    QPoint  m_ptRotationAngle;
    QPoint  m_ptLastMousePosition;
    bool    m_bIsDragging;
    
    QTimer*  m_pAnimationFrameTimer;
    ITexture* m_pOffscreenRenderTarget; // Engine-side Render Target to render the 3D mesh
    QImage   m_RenderOutputImage;       // Thread-safe image buffer to pass pixels to QPainter
    bool     m_bIsRenderBufferBlank; // Tracker to handle headless driver software fallbacks

};
