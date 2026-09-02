
#include <QApplication>

#include "mainwindow.h"

int main(int argc, char** argv)  {
	QApplication app(argc, argv);

	auto pWin = new ui::MainWindow(nullptr);
    pWin->setAttribute(Qt::WA_DeleteOnClose);
    pWin->show();
	
	return QApplication::exec();
}