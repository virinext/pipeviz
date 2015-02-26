#ifndef CUSTOM_SETTINGS_H_
#define CUSTOM_SETTINGS_H_

#include <QString>
#include <QByteArray>


namespace CustomSettings
{
	void saveLastIODirectory(const QString &name);
	QString lastIODirectory();

	void saveMainWindowGeometry(const QByteArray &geometry);
	QByteArray mainWindowGeometry();
}



#endif
