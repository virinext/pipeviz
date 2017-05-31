/*
 * Logger.h
 *
 *  Created on: 31 mai 2017
 *      Author: scerveau
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <QThread>
#include "glib.h"

enum eLogCategory {
  eLOG_CATEGORY_INTERNAL,
  eLOG_CATEGORY_GST,
  eLOG_CATEGORY_EVENTS,
};

#define LOG_INFO(x) Logger::instance().processLog(x)

class Logger : public QThread {
  Q_OBJECT
public:
  Logger();
  void configure_logger();

  static Logger& instance();
  void Quit();

  void processLog(const QString& line);

protected:
  void processGstLog(gchar* line);
  gchar* readGstLine(FILE *file);
  GList* parseGstLine(gchar* line, gchar ch = ' ', int max_field = -1);

signals:
  void sendLog(const QString& message, int category);

private:
  void run();
  bool m_fExit;
};

#endif /* LOGGER_H_ */
