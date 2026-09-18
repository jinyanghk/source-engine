#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QTimer>
#include <QImage>
#include <QVector>
#include <QColor>
#include "mathlib/vector.h"

#include "HammerUITypes.h"

typedef unsigned short MDLHandle_t;
class ITexture;

struct ModelViewBrush {
    int id;
    Vector mins;
    Vector maxs;
    QColor color;
};

struct ModelViewEntity {
    int id;
    QString classname;
    Vector origin;
    QColor color;
};

class Hammer3DView : public QWidget
{
    Q_OBJECT

public:
    explicit Hammer3DView(QWidget *parent = nullptr);
    virtual ~Hammer3DView();

    void LoadModelFile(const QString &szFilePath);
    void updateBrushes(const MapBrush* pBrushes, int count, int selectedId);
    
    // EXPLICIT FIXED DECLARATION: Makes the method visible to MainWindow3
    void updateEntities(const MapEntity* pEntities, int count);

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
    QVector<MapEntity> m_mapEntities; // Array to retain entity nodes locally
    int m_selectedBrushId = -1;
};
