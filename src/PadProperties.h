#ifndef PAD_PROPERTIES_H_
#define PAD_PROPERTIES_H_

#include <QWidget>
#include <QSharedPointer>

#include "GraphManager.h"

class PadProperties: public QWidget
{
public:
  PadProperties(QSharedPointer<GraphManager> pGraphManager, const char *element, const char *pad
      , QWidget *parent = 0, Qt::WindowFlags flags = 0);
};

#endif
