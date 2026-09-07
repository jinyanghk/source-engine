#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QShowEvent>   // <-- ADD THIS INCLUDE
#include <QResizeEvent> // <-- ADD THIS INCLUDE

class CEngineView : public QWidget
{
    Q_OBJECT
public:
    explicit CEngineView(QWidget *parent = nullptr);
    virtual ~CEngineView();

protected:
    // Virtual method overrides from QWidget
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    
    // FIX: Declare the missing events implemented in your .cpp file
    void showEvent(QShowEvent *event) override;     // <-- ADD THIS DECLARATION
    void resizeEvent(QResizeEvent *event) override; // <-- ADD THIS DECLARATION

    // Tells Qt to leave this canvas layout completely to the engine
    QPaintEngine* paintEngine() const override { return nullptr; }

private:
    void RenderFrame();
};
