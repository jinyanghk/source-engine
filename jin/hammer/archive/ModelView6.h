//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Qt6 Window Container wrapping a native SDL2 / Source Engine viewport.
//          Bypasses software rasterization and handles native hardware output.
//
//=============================================================================//

#ifndef MODELVIEW_H
#define MODELVIEW_H
#pragma once

#include <QWidget>
#include <QString>
#include <QWindow>
#include <QTimer>

typedef unsigned short MDLHandle_t;

class QModelView : public QWidget 
{
    Q_OBJECT

public:
    explicit QModelView(QWidget *parent = nullptr);
    virtual ~QModelView();

    // Model management interface
    void LoadModelFile(const QString &szFilePath);
    
    // Animation controls mapped directly to Source engine parameters
    int GetSequenceCount();
    const char* GetSequenceName(int index);
    void SetActiveSequence(int index);
    void SetAnimationCycle(float flCycle);
    void SetPlaybackPaused(bool bPaused);
        void TriggerEngineBinding();

protected:
    // Replaced traditional paintEvent with native container geometry sync handles
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

// FIX: Completely delete the private slots section and SetupNativeEmbeddedWindow() from here!

private:
    QString      m_szCurrentModelPath;
    MDLHandle_t  m_hCurrentModel;
    float        m_flAnimationCycle;

    int          m_nActiveSequenceIndex;
    bool         m_bPlaybackPaused;

    // Heavyweight widget containers bridging Qt6 and SDL2 contexts
    QWindow*     m_pForeignWindow;
    QWidget*     m_pEmbeddedWidget;
    
    // Sync timer forcing the underlying Source Engine window context tick loop
    QTimer*      m_pEngineTickTimer;
    bool         m_bContextAttached;
};

#endif // MODELVIEW_H
