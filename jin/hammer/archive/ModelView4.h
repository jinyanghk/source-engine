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

// Forward declaration to bypass include loops safely
struct MapBrush;

struct ModelViewBrush {
    int id;
    Vector mins;
    Vector maxs;
    QColor color;
};

class QModelView4 : public QWidget
{
    Q_OBJECT

public:
    explicit QModelView4(QWidget *parent = nullptr);
    virtual ~QModelView4();

    void LoadModelFile(const QString &szFilePath);

    // FIXED: Accepts sequential pointers directly from our main map data layer
    void updateBrushes(const MapBrush* pBrushes, int count, int selectedId);

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

    QVector<ModelViewBrush> m_mapBrushes;
    int m_selectedBrushId = -1;
};
