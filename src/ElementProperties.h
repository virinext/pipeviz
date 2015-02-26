#ifndef ELEMENT_PROPERTIES_H_
#define ELEMENT_PROPERTIES_H_

#include <QWidget>
#include <QSharedPointer>
#include <QMap>
#include <QString>
#include <QLineEdit>

#include "GraphManager.h"

class ElementProperties: public QWidget
{
Q_OBJECT
public:
	ElementProperties(QSharedPointer<GraphManager> pGraphManager, const char *name, 
		QWidget *parent = 0, Qt::WindowFlags flags = 0);


private slots:
	void applyClicked();
	void okClicked();

private:
	QSharedPointer<GraphManager>         m_pGraphManager;
	QMap<QString, QLineEdit *>           m_values;
	QString                              m_name;

	void create();
};


#endif
