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

class Map3DView : public QWidget
{
    Q_OBJECT

public:
    explicit Map3DView(QWidget *parent = nullptr);
    virtual ~Map3DView();

    void LoadModelFile(const QString &szFilePath);
    void updateBrushes(const MapBrush* pBrushes, int count, int selectedId);
    void updateEntities(const MapEntity* pEntities, int count);

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

    float   m_flZoomScale;
    QPoint  m_ptRotationAngle;
    QPoint  m_ptLastMousePosition;
    
    ITexture* m_pOffscreenRenderTarget;
    QImage   m_RenderOutputImage;      
    bool     m_bIsRenderBufferBlank; 

    QPointF m_ptCameraPanOffset; 
    // QMap<QString, QImage> m_MaterialTextureCache; // not ready for this yet

    QVector<MapBrush> m_mapBrushes;
    QVector<MapEntity> m_mapEntities;
    int m_selectedBrushId = -1;
};
