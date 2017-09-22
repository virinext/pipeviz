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
class FavoritesList;

class MainWindow: public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~MainWindow();

  static MainWindow& instance();
  FavoritesList* getFavoritesList();

protected:
  void timerEvent(QTimerEvent *);
  void createDockWindows();

public slots:
  void InsertLogLine(const QString& line, int category);
  void AddPlugin();
  void ClearGraph();
  void AddPluginToFavorites(const QString& plugin_name);
  void RemovePluginToFavorites(const QString& plugin_name);

private slots:
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

  void About();


  void onFavoriteListItemDoubleClicked(QListWidgetItem* item);
  void ProvideContextMenu(const QPoint &pos);

private:
  QSharedPointer<GraphManager> m_pGraph;

  GraphDisplay *m_pGraphDisplay;

  QStatusBar *m_pstatusBar;
  QSlider *m_pslider;

  QString m_fileName;
  PluginsListDialog *m_pluginListDlg;
  QMenu *m_menu;
  QListWidget* m_logList;
  FavoritesList* m_favoriteList;
};

#endif
