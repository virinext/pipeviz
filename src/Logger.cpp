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

void
Logger::configure_logger ()
{
  QString lastGstDebugString = CustomSettings::lastGstDebugString ();
  setenv (lastGstDebugString.split ("=").at (0).toStdString ().c_str (), lastGstDebugString.split ("=").at (1).toStdString ().c_str (), 1);
  setenv ("GST_DEBUG_NO_COLOR", "1", 1);
  setenv ("GST_DEBUG_FILE", "/tmp/gst_pipeviz.txt", 1);
}

Logger::Logger()
: QThread(),
m_fExit(false)
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

  file = fopen("/tmp/gst_pipeviz.txt", "r");
  if(!file)
    return;

  while (!m_fExit) {
    line = readGstLine(file);
    if (line != NULL) {
      processGstLog(line);
      g_free(line);
    }
    //usleep(10);
  }

  qDebug() << "Logger closed";

  fclose(file);

}
