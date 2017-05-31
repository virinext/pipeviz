#ifndef CUSTOM_MENU_ACTION_H_
#define CUSTOM_MENU_ACTION_H_

#include <QAction>

class CustomMenuAction: public QAction
{
public:
  CustomMenuAction(const QString& displayName, QObject * parent);
  CustomMenuAction(const QString& displayName, const QString& name, QObject * parent);

  QString getName() {return m_name;}

private:
  QString m_name;
};

#endif //CUSTOM_MENU_ACTION_H_
