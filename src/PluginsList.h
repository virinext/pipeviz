#ifndef PLUGINS_LIST_H_
#define PLUGINS_LIST_H_

#include <QDialog>
#include <QLabel>
#include <QListWidgetItem>
#include <QPushButton>

#include "GraphManager.h"

class Plugin
{
public:
  Plugin(const QString& name, int rank):
  m_name(name),
  m_rank(rank)
  {
  }
  const QString getName() {return m_name;}
  int getRank() {return m_rank;}

private:
  const QString m_name;
  int m_rank;

};

class PluginsList
{
public:
  PluginsList();
  ~PluginsList();

  static PluginsList& instance();

  GList* getList() {return m_pluginsList;}
  GList* getSortedByRank();
  GList* getPluginListByCaps(GstPadDirection direction, GstCaps* caps);
  GList* getPluginListFavorite();

private:
  void init();

  GList* m_pluginsList; //list of plugins
};

class MainWindow;

class PluginsListDialog: public QDialog
{
  Q_OBJECT
public:
  PluginsListDialog(QWidget *pwgt = NULL, Qt::WindowFlags f = Qt::Window);
  ~PluginsListDialog();

  void setGraph(GraphManager* graph) {m_pGraph = graph;}

protected:
  bool eventFilter(QObject *obj, QEvent *ev);

private:
  void InitPluginsList();

public slots:
  void showInfo(QListWidgetItem *current, QListWidgetItem *previous);
  void insert(QListWidgetItem *);

private slots:
  void filterPlugins(const QString &text);
  void favoritesClicked();
  void ProvideContextMenu(const QPoint &pos);

signals:
  void signalAddPluginToFav (const QString &plugin_name);
  void signalRemPluginToFav (const QString &plugin_name);

private:
  MainWindow * m_main;
  QLabel *m_plblInfo;
  QListWidget *m_pPlugins;
  GraphManager *m_pGraph;
  QPushButton*m_favoriteListButton;
};

#endif
