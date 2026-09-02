#pragma once

#include <QDialog>

class CSmoothingGroups : public QDialog
{
	Q_OBJECT
public:
	CSmoothingGroups(QWidget *pParent);

private slots:
	void onSmoothPressed(int value);
	void onHardPressed(int value);
};
