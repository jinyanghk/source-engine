#pragma once

#include <QMainWindow>
#include <QTreeView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

class EngineViewWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // --- ADD THIS PUBLIC GETTER OVERRIDE METHOD ---
    unsigned long long GetViewportWindowID() const {
        return m_pEngineContainerWidget ? (unsigned long long)m_pEngineContainerWidget->winId() : 0;
    }

private slots:
    void onEntityTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onEntityFilterChanged(const QString &text);
    void onRenderTimerTick();

private:
    QWidget                 *m_pEngineContainerWidget; 
    QTreeView               *m_pEntityTreeView;
    QLineEdit               *m_pEntityFilterEdit;
    QStandardItemModel      *m_pEntityTreeModel;
    QSortFilterProxyModel   *m_pEntityFilterProxyModel;

    EngineViewWindow *m_pStrataEngineViewWindow;

    void createRightEntityBrowser();
    void executeEngineFrame();
};
