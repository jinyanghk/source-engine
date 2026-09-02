#pragma once

#include <QDialog>

class CMapInformation : public QDialog
{
	Q_OBJECT
public:
	CMapInformation(QWidget *pParent);

private slots:
	void onClosePressed();
};
