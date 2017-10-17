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

#ifndef LOG_LEVEL
#   define LOG_LEVEL 1
#endif

#define MAX_LOG_LEVEL 3

#ifndef CLASS_LOGGER_TAG
#   define CLASS_LOGGER_TAG   __BASE_FILE__
#endif


class Logger : public QThread {
  Q_OBJECT
public:
  enum TimeStampFlag {
       UseTimeStamp = 0,
       NoTimeStamp,
   };

  Logger();
  void configure_logger();

  static Logger& instance();
  void Quit();

  void processLog(const QString& line);
  void createLog(Logger::TimeStampFlag, const char* format, ...);
  int getLevel() const { return m_level; }
  void incrementLogLevel();

protected:
  void processGstLog(gchar* line);
  gchar* readGstLine(FILE *file);
  GList* parseGstLine(gchar* line, gchar ch = ' ', int max_field = -1);

signals:
  void sendLog(const QString& message, int category);

private:
  void run();
  bool m_fExit;
  int m_level;
};

#define log_debug(LEVEL,TYPE,FMT, ARGS...) do { \
    if (LEVEL <= Logger::instance().getLevel()) \
        Logger::instance().createLog(Logger::UseTimeStamp, "" TYPE ": " FMT "", ## ARGS); \
    } while (0)
#define log_debug_no_time_stderr(LEVEL,TYPE,FMT, ARGS...) do { \
        Logger::instance().createLog(Logger::NoTimeStamp, "" FMT "", ## ARGS); \
    } while (0)

#define LOG_IMPORTANT(FMT, ARGS...) log_debug_no_time_stderr(0, "",FMT ,## ARGS)

#ifndef DISABLE_LOG
#   define LOG_ERROR(FMT, ARGS...) log_debug(0, "ERROR\t" CLASS_LOGGER_TAG ":%d:%s", FMT,__LINE__, __func__, ## ARGS)
#   define LOG_WARNING(FMT, ARGS...) log_debug(1, "WARN\t" CLASS_LOGGER_TAG ":%d:%s", FMT, __LINE__, __func__, ## ARGS)
#   define LOG_INFO(FMT, ARGS...) log_debug(2, "INFO\t" CLASS_LOGGER_TAG ":%d:%s", FMT, __LINE__, __func__, ## ARGS)
#   define LOG_DEBUG(FMT, ARGS...) log_debug(3, "DEBUG\t" CLASS_LOGGER_TAG ":%d:%s", FMT, __LINE__, __func__, ## ARGS)
#else
#   define LOG_ERROR(FMT, ARGS...)
#   define LOG_WARNING(FMT, ARGS...)
#   define LOG_INFO(FMT, ARGS...)
#   define LOG_DEBUG(FMT, ARGS...)
#endif

#endif /* LOGGER_H_ */
