#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

class QMapView; // Forward declaration of our clean view layout

class MainWindow2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow2(QWidget *parent = nullptr);
    ~MainWindow2();

private:
    void createMenuBar();
    void createToolBars();
    void createStatusBar();
    void applyStyleSheet();

    QMenuBar    *m_menuBar;
    QToolBar    *m_mainToolBar;
    QStatusBar  *m_statusBar;
    QMapView    *m_pMapView3D; // Core interactive canvas reference tracker

private slots:
    void OnOpenMapFileSlot();  // Triggers the safe text file loader pipeline
};

#endif // MAINWINDOW2_H
