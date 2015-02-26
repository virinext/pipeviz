#include "CustomSettings.h"

#include <QSettings>

#define COMPANY_NAME              "virinext"
#define APPLICATION_NAME          "pipeviz"

void CustomSettings::saveLastIODirectory(const QString &name)
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	settings.setValue("last_directory", name);
}


QString CustomSettings::lastIODirectory()
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	QString res = settings.value("last_directory").toString();

	if(res.isEmpty())
		res = "./";

	return res;
}


void CustomSettings::saveMainWindowGeometry(const QByteArray &geometry)
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	settings.setValue("geometry", geometry);
}


QByteArray CustomSettings::mainWindowGeometry()
{
	QSettings settings(COMPANY_NAME, APPLICATION_NAME);
	return settings.value("geometry").toByteArray();
}
