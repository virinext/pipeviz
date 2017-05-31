#ifndef PLUGINS_LIST_H_
#define PLUGINS_LIST_H_

#include <QDialog>
#include <QLabel>
#include <QListWidgetItem>

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

  GList* getList() {return m_pluginsList;}
  GList* getSortedByRank();
  GList* getPluginListByCaps(GstPadDirection direction, GstCaps* caps);

private:
  void init();

  GList* m_pluginsList;
};

class PluginsListDialog: public QDialog
{
  Q_OBJECT
public:
  PluginsListDialog(PluginsList* pluginList, QWidget *pwgt = NULL, Qt::WindowFlags f = Qt::Window);
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
  void filterPlagins(const QString &text);

private:
  QLabel *m_plblInfo;
  QListWidget *m_pPlugins;
  PluginsList *m_pPluginsList;
  GraphManager *m_pGraph;
};

#endif
