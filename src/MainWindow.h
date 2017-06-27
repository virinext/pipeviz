#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QVBoxLayout>
#include <QSharedPointer>
#include <QStatusBar>
#include <QAction>
#include <QSlider>
#include <QListWidget>

#include <gst/gstbuffer.h>
#include <gst/gstevent.h>
#include <gst/gstcaps.h>

#include "GraphManager.h"
#include "Logger.h"

class GraphDisplay;
class PluginsListDialog;


class MainWindow: public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~MainWindow();

  static MainWindow& instance();

protected:
  void timerEvent(QTimerEvent *);
  void createDockWindows();

public slots:
    void InsertLogLine(const QString& line, int category);

private slots:
  void AddPlugin();
  void OpenMediaFile();
  void OpenMediaUri();
  void Play();
  void Pause();
  void Stop();
  void Flush();
  void Seek(int);

  void Save();
  void SaveAs();
  void Open();

  void ClearGraph();
  void About();

private:
  QSharedPointer<GraphManager> m_pGraph;

  GraphDisplay *m_pGraphDisplay;

  QStatusBar *m_pstatusBar;
  QSlider *m_pslider;

  QString m_fileName;
  PluginsListDialog *m_pluginListDlg;
  QMenu *m_menu;
  QListWidget* m_logList;
};

#endif
