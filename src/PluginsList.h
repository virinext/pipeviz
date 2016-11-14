#ifndef PLUGINS_LIST_H_
#define PLUGINS_LIST_H_



#include <QDialog>
#include <QLabel>
#include <QListWidgetItem>


#include "GraphManager.h"


class PluginsList: public QDialog
{
Q_OBJECT
public:
	PluginsList(QWidget *pwgt = NULL, Qt::WindowFlags f = Qt::Window);

	GraphManager    *m_pGraph;

protected:
	bool eventFilter(QObject *obj, QEvent *ev);


public slots:
	void showInfo(QListWidgetItem *current, QListWidgetItem *previous);
	void insert(QListWidgetItem *);

private slots:
	void filterPlagins(const QString &text);

private:
	QLabel          *m_plblInfo;
	QListWidget     *m_pPlugins;
};




#endif
