#include <QApplication>
#include "MainWindow.h"

#include <gst/gst.h>

int main(int argc, char **argv)
{
	gst_init (&argc, &argv);

	GstRegistry *registry;
	registry = gst_registry_get();
	gst_registry_scan_path(registry, "./plugins");

	QApplication app(argc, argv);

	MainWindow wgt;
	wgt.show();

	return app.exec();
}