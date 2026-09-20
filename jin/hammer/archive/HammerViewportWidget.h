#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>

class CHammerViewportWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    CHammerViewportWidget(int viewportType, QWidget *parent = nullptr);
    virtual ~CHammerViewportWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_iViewportType; // 0 = 3D, 1 = Top, 2 = Front, 3 = Side
    QTimer *m_pRenderTimer;
    bool m_bContextReady; // Fix: Tracks when OpenGL pointers are safely mapped
};
