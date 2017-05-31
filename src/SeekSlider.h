#ifndef SEEK_SLIDER_H_
#define SEEK_SLIDER_H_

#include <QSlider>
#include <QMouseEvent>

class SeekSlider: public QSlider
{
protected:
  void mousePressEvent(QMouseEvent *);
};

#endif
