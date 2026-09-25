#pragma once

#include <QWindow>
#include <QPaintEvent>
#include <QFocusEvent>

class EngineViewWindow : public QWindow
{
    Q_OBJECT
public:
    explicit EngineViewWindow(QWindow *parent = nullptr);
    ~EngineViewWindow() override;

    void RenderFrame();

protected:
    void exposeEvent(QExposeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
};
