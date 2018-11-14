/*
 * Logger.cpp
 *
 *  Created on: 31 mai 2017
 *      Author: scerveau
 */
#include "Logger.h"

#include "CustomSettings.h"

#include <QDebug>
#include <QStringList>
#include <QTime>

#include <assert.h>
#include <unistd.h>
#include <thread>

G_LOCK_DEFINE_STATIC(logger);

static long int getThreadID() {
    return quintptr(QThread::currentThreadId());
}

void
Logger::configure_logger ()
{
  QString lastGstDebugString = CustomSettings::lastGstDebugString ();
  qputenv(lastGstDebugString.split("=").at(0).toStdString ().c_str(),
          lastGstDebugString.split("=").at(1).toLocal8Bit());
  qputenv("GST_DEBUG_NO_COLOR", QByteArray("1"));
  qputenv("GST_DEBUG_FILE",     QByteArray("gst_pipeviz.txt"));
}

Logger::Logger()
: QThread(),
m_fExit(false),
m_level(MAX_LOG_LEVEL)
{
}

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::Quit()
{
    m_fExit = true;
    wait();
}

void Logger::createLog(TimeStampFlag flag, const char* format, ...)
{
    // first check if we should add the timestamp before the string
    char* new_fmt = NULL;
    if (flag == Logger::UseTimeStamp) {
        QString szTimestamp(QTime::currentTime().toString("HH:mm:ss:zzz"));

        const char* fmt_template = "%s %d 0x%x %s";
        int len = snprintf(NULL, 0, fmt_template, szTimestamp.toStdString().c_str(), getThreadID(), std::this_thread::get_id(), format);
        if (len < 0) {
            // cannot parse the string
            return;
        }
        len++; // add 1 byte for the additional terminating null character
        new_fmt = static_cast<char*>(malloc(sizeof(char) * len));
        snprintf(new_fmt, len, fmt_template, szTimestamp.toStdString().c_str(), getThreadID(), std::this_thread::get_id(), format);
    }

    // create the actual string (timestamp + format...)
    const char* formatString = new_fmt ? new_fmt : format;
    assert(formatString);

    va_list aptr;
    va_start(aptr, format);
    va_list argcopy; // copy the va_list and use vsnprintf to get the length of the final string
    va_copy(argcopy, aptr);
    int len = vsnprintf(NULL, 0, formatString , argcopy);
    va_end(argcopy);
    if (len < 0) {
        // cannot parse the string
        g_free(new_fmt);
        va_end(aptr);
        return;
    }

    len++; // add 1 byte for the additional terminating null character.
    char* buffer = static_cast<char*>(malloc(sizeof(char) * len));
    int actualLen = vsprintf(buffer, formatString, aptr);
    va_end(aptr);
    formatString = 0;
    g_free (new_fmt);
    if (actualLen < 0) {
        g_free (buffer);
        return;
    }
    // if lengths are different, our code is bugged
    assert((actualLen + 1) == len);

    // dump the final string
    G_LOCK(logger);
    processLog(buffer);
    G_UNLOCK(logger);

    // free the buffer and flush the logs
    g_free(buffer);
}

void Logger::incrementLogLevel() {
    m_level++;
    if (m_level > MAX_LOG_LEVEL)
        m_level = MAX_LOG_LEVEL;
    char buffer[32];
    sprintf(buffer,"logger log level %d\n",m_level);
    processLog(buffer);
}

void Logger::processLog(const QString& line)
{
  emit sendLog(line,eLOG_CATEGORY_INTERNAL);
}

//#define GST_TIME_FORMAT "u:%02u:%02u.%09u"
#define GST_TIME_FORMAT "%s"
#define PRINT_FMT " "PID_FMT" "PTR_FMT" %s "CAT_FMT" %s\n"

struct GSTLog {
  gchar* date;
  gchar* pid;
  gchar* level;
  gchar* category;
  gchar* file;
  gchar* line;
  gchar* function;
  gchar* obj;
  gchar* message;
};

void Logger::processGstLog(gchar* log)
{
//  GSTLog gstLog;
  if(!g_strrstr(log,"WARN") && !g_strrstr(log,"ERROR"))
    return;
/*  GList* stringList = parseGstLine(log, ' ', 6);
  GList* l;

  gstLog.date = g_strdup((gchar*)g_list_nth_data(stringList, 0));
  gstLog.pid = g_strdup((gchar*)g_list_nth_data(stringList, 1));
  gstLog.level = g_strdup((gchar*)g_list_nth_data(stringList, 2));
  gstLog.category = g_strdup((gchar*)g_list_nth_data(stringList, 3));

  gstLog.file = g_strdup((gchar*)g_list_nth_data(stringList, 4));
  gstLog.line = g_strdup((gchar*)g_list_nth_data(stringList, 5));
  gstLog.function = g_strdup((gchar*)g_list_nth_data(stringList, 6));
  gstLog.obj = g_strdup((gchar*)g_list_nth_data(stringList, 7));

  QString message = "";
  for (l = stringList; l != NULL; l = l->next) {
    message += (gchar*)l->data;
    message += " ";
  }
*/
  emit sendLog(log,eLOG_CATEGORY_GST);
}

#define MAX_LINE_LENGTH 128

GList* Logger::parseGstLine(gchar* line, gchar delimiter, int max_fields)
{
  int count = 0;
  size_t pos = 0;
  char *lineBuffer = NULL;

  gboolean new_string=TRUE;
  GList* strList = NULL;
  gint n_fields = 0;

  int maximumLineLength = MAX_LINE_LENGTH;

  if (line == NULL)
      return NULL;

  do {
    if (new_string) {
      if (line[pos] != delimiter) {
        new_string = FALSE;
        lineBuffer = (gchar *)malloc(sizeof(char) * maximumLineLength);
        count=0;
        lineBuffer[count++] = line[pos];
      }
    }
    else {
      if (count == MAX_LINE_LENGTH) {
        maximumLineLength += MAX_LINE_LENGTH;
        lineBuffer = (gchar*)realloc(lineBuffer, maximumLineLength);
      }
      if (line[pos] == delimiter || line[pos] == '\0') {
        lineBuffer[count] = '\0';
        new_string= TRUE;
        strList = g_list_append(strList, g_strdup(lineBuffer));
        g_free(lineBuffer);
        n_fields++;
        if (max_fields != -1 && n_fields == max_fields) {
          pos++;
          if (pos < strlen(line))
            strList = g_list_append(strList, g_strdup(&line[pos]));
          break;
        }
      } else {
        lineBuffer[count++] = line[pos];
      }

    }
    pos++;
  } while (pos < strlen(line));


  return strList;
}

gchar *Logger::readGstLine(FILE *file)
{
  int count = 0;
  char ch;
  char *line = NULL;
  gchar *lineBuffer = NULL;
  int maximumLineLength = MAX_LINE_LENGTH;

  if (file == NULL)
      goto beach;

  lineBuffer = (gchar *)malloc(sizeof(char) * maximumLineLength);
  if (lineBuffer == NULL)
    goto beach;


  ch = getc(file);
  while ((ch != '\n') && (ch != EOF)) {
      if (count == maximumLineLength) {
          maximumLineLength += MAX_LINE_LENGTH;
          lineBuffer = (gchar*)realloc(lineBuffer, maximumLineLength);
          if (lineBuffer == NULL)
              goto beach;
      }
      lineBuffer[count] = ch;
      count++;

      ch = getc(file);
  }
  if(count==0)
    goto beach;

  lineBuffer[count] = '\0';
  lineBuffer = (gchar*)realloc(lineBuffer, count + 1);
  line = g_strdup(lineBuffer);
  //qDebug() << lineBuffer;
  //qDebug() << line;
beach:
  g_free(lineBuffer);
  return line;
}

void Logger::run()
{
  FILE* file;
  gchar* line;

  file = fopen("gst_pipeviz.txt", "r");
  if(!file)
    return;

  while (!m_fExit) {
    line = readGstLine(file);
    if (line != NULL) {
      processGstLog(line);
      g_free(line);
    } else
      usleep(10000);
  }

  qDebug() << "Logger closed";

  fclose(file);

}
