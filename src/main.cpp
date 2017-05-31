#include <QApplication>
#include "MainWindow.h"

#include <gst/gst.h>

int
main (int argc, char **argv)
{
  gst_init (&argc, &argv);

  GstRegistry *registry;
#if GST_VERSION_MAJOR >= 1
  registry = gst_registry_get();
#else
  registry = gst_registry_get_default ();
#endif
  gst_registry_scan_path (registry, "./plugins");

  QApplication app (argc, argv);

  MainWindow wgt;
  wgt.show ();

  return app.exec ();
}
