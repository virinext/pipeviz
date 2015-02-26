#include <QApplication>
#include "MainWindow.h"

#include <gst/gst.h>

int main(int argc, char **argv)
{
	gst_init (&argc, &argv);
	QApplication app(argc, argv);

	MainWindow wgt;
	wgt.show();

	return app.exec();
}