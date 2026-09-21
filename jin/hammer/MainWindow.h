#pragma once

#include <QMainWindow>
#include <QTreeView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include "widgets/ModelView.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onEntityTreeSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onEntityFilterChanged(const QString &text);

private:
    QModelView              *m_pModelViewerWidget; // The primary, inline viewport
    QTreeView               *m_pEntityTreeView;
    QLineEdit               *m_pEntityFilterEdit;
    QStandardItemModel      *m_pEntityTreeModel;
    QSortFilterProxyModel   *m_pEntityFilterProxyModel;

    void createRightEntityBrowser();
};
