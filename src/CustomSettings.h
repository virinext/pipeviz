#ifndef CUSTOM_SETTINGS_H_
#define CUSTOM_SETTINGS_H_

#include <QString>
#include <QStringList>
#include <QByteArray>

namespace CustomSettings
{
  void saveLastIODirectory(const QString &name);
  QString lastIODirectory();
  void saveFavoriteList(const QStringList &name);
  QStringList loadFavoriteList();

  void saveGstDebugString(const QString &name);
  QString lastGstDebugString();

  void saveMainWindowGeometry(const QByteArray &geometry);
  QByteArray mainWindowGeometry();
}

#endif
