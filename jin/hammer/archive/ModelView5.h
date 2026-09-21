#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QTimer>
#include <QImage>
#include <QVector>
#include <QColor>
#include "mathlib/vector.h"

typedef unsigned short MDLHandle_t;
class ITexture;

class QModelView : public QWidget
{
    Q_OBJECT

public:
    explicit QModelView(QWidget *parent = nullptr);
    virtual ~QModelView();

    void LoadModelFile(const QString &szFilePath);
    int GetSequenceCount();
    const char* GetSequenceName(int index);
    void SetActiveSequence(int index);
    void SetAnimationCycle(float flCycle);
    void SetPlaybackPaused(bool bPaused);

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

    float   m_flZoomScale;
    QPoint  m_ptRotationAngle;
    QPoint  m_ptLastMousePosition;
    bool    m_bIsDragging;
    
    QTimer*  m_pAnimationFrameTimer;
    ITexture* m_pOffscreenRenderTarget;
    QImage   m_RenderOutputImage;      
    bool     m_bIsRenderBufferBlank; 

    QPointF m_ptCameraPanOffset; 
    QMap<QString, QImage> m_MaterialTextureCache;

    int m_nActiveSequenceIndex = 0;
    bool m_bPlaybackPaused = false;
};
