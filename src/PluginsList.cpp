#include "PluginsList.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>

#include <gst/gst.h>

#include <QDebug>

namespace 
{
	void InitPluginsList(QListWidget *plwgt)
	{
		std::size_t num = 0;
		GList *plugins;
		GstRegistry *registry;
#if GST_VERSION_MAJOR >= 1
		registry = gst_registry_get();
#else
		registry = gst_registry_get_default();
#endif
		plugins = gst_registry_get_plugin_list(registry);
		while(plugins) 
		{
			GstPlugin *plugin;
			plugin = (GstPlugin *) (plugins->data);
			plugins = g_list_next (plugins);
#if GST_VERSION_MAJOR >= 1
			registry = gst_registry_get();
#else
			registry = gst_registry_get_default();
#endif
			GList *features = gst_registry_get_feature_list_by_plugin (registry,
				gst_plugin_get_name (plugin));

			while(features) 
			{
				GstPluginFeature *feature;
				feature = GST_PLUGIN_FEATURE (features->data);
				if(GST_IS_ELEMENT_FACTORY (feature)) 
				{
					GstElementFactory *factory;
					factory = GST_ELEMENT_FACTORY (feature);
					plwgt -> addItem(GST_OBJECT_NAME (factory));
					num++;
				}

				features = g_list_next (features);
			}
		}
	}
}

PluginsList::PluginsList(QWidget *pwgt, Qt::WindowFlags f):
QDialog(pwgt, f)
,m_pGraph(NULL)
{
	m_pPlugins = new QListWidget;
	m_plblInfo = new QLabel;

	m_plblInfo -> setTextInteractionFlags(Qt::TextSelectableByMouse);
	m_plblInfo -> setAlignment(Qt::AlignLeft | Qt::AlignTop);
	QScrollArea *pscroll = new QScrollArea;
	pscroll -> setWidget(m_plblInfo);
	m_plblInfo -> resize(pscroll -> size());

	QHBoxLayout *phblay = new QHBoxLayout;

	phblay -> addWidget(m_pPlugins, 1);
	phblay -> addWidget(pscroll, 2);

	InitPluginsList(m_pPlugins);

	QHBoxLayout *phblayFind = new QHBoxLayout;

	QLineEdit *ple = new QLineEdit;
	phblayFind -> addWidget(ple);
	phblayFind -> addStretch(1);

	ple -> setPlaceholderText("Search...");

	QVBoxLayout *pvblay = new QVBoxLayout;
	pvblay -> addLayout(phblayFind);
	pvblay -> addLayout(phblay);

	setLayout(pvblay);

	setWindowTitle("Add plugin");

	QObject::connect(m_pPlugins, SIGNAL(currentItemChanged  (QListWidgetItem *, QListWidgetItem *)),
		this, SLOT(showInfo(QListWidgetItem *, QListWidgetItem *)));

	QObject::connect(m_pPlugins, SIGNAL(itemDoubleClicked (QListWidgetItem *)),
		this, SLOT(insert(QListWidgetItem *)));

	QObject::connect(ple, SIGNAL(textChanged(const QString &)), this, SLOT(filterPlagins(const QString &)));

	installEventFilter(this);
}


void PluginsList::showInfo(QListWidgetItem *pitem, QListWidgetItem *previous)
{
	qDebug() << "Show Info: " << pitem -> text();
	m_plblInfo -> clear();
	QString descr;
	descr += "<b>Plugin details</b><hr>";

	GstElementFactory *factory = gst_element_factory_find(pitem -> text().toStdString().c_str());
	if(!factory)
	{
		qDebug() << "warning: " << pitem -> text() << " Not Found";
		return;		
	}

	factory = GST_ELEMENT_FACTORY (gst_plugin_feature_load (GST_PLUGIN_FEATURE(factory)));
	if(!factory)
	{
		qDebug() << "warning: " << pitem -> text() << " Not Found";
		return;		
	}
#if GST_VERSION_MAJOR >= 1
	GstPlugin *plugin = gst_plugin_feature_get_plugin (GST_PLUGIN_FEATURE (factory));
#else
	const gchar* plugin_name = GST_PLUGIN_FEATURE(factory)->plugin_name;
	if (!plugin_name) {
		return;
	}
	GstPlugin* plugin = gst_default_registry_find_plugin(plugin_name);
#endif
	if(!plugin)
	{
		qDebug() << "warning: " << pitem -> text() << " Not Found";
		return;
	}

#if GST_VERSION_MAJOR >= 1
	const gchar *release_date = gst_plugin_get_release_date_string (plugin);
#else
	const gchar *release_date = (plugin->desc.release_datetime) ? plugin->desc.release_datetime : "";
#endif
	const gchar *filename = gst_plugin_get_filename(plugin);

	descr += "<b>Name</b>: " + QString(gst_plugin_get_name(plugin)) + "<br>";
	descr += "<b>Description</b>: " + QString(gst_plugin_get_description(plugin)) + "<br>";
	descr += "<b>Filename</b>: " + QString((filename != NULL) ? filename : "(null)") + "<br>";
	descr += "<b>Version</b>: " + QString(gst_plugin_get_version (plugin)) + "<br>";
	descr += "<b>License</b>: " + QString(gst_plugin_get_license (plugin)) + "<br>";
	descr += "<b>Source module</b>: " + QString(gst_plugin_get_source (plugin)) + "<br>";

	if (release_date != NULL) 
	{
		const gchar *tz = "(UTC)";
		gchar *str, *sep;

		str = g_strdup (release_date);
		sep = strstr (str, "T");
		if (sep != NULL) 
		{
			*sep = ' ';
			sep = strstr (sep + 1, "Z");
			if (sep != NULL)
				*sep = ' ';
		} 
		else 
		{
			tz = "";
		}
		descr += "<b>Source release date</b>: " + QString(str) + " " +  QString(tz) + "<br>";
		g_free (str);
	}
	descr += "<b>Binary package</b>: " + QString(gst_plugin_get_package (plugin)) + "<br>";
	descr += "<b>Origin URL</b>: " + QString(gst_plugin_get_origin (plugin)) + "<br>";

	m_plblInfo -> setText(descr);
}


void PluginsList::insert(QListWidgetItem *pitem)
{
	qDebug() << "Insert: " << pitem -> text();

	if(!m_pGraph || !m_pGraph -> AddPlugin(pitem -> text().toStdString().c_str(), NULL))
	{
		QMessageBox::warning(this, "Plugin addition problem", "Plugin `" + pitem -> text() + "` insertion was FAILED");
		qDebug() << "Plugin `" << pitem -> text() << "` insertion FAILED";
		return;
	}
}



bool PluginsList::eventFilter(QObject *obj, QEvent *event)
{
	if (event -> type() == QEvent::KeyPress) 
	{
		QKeyEvent *key = static_cast<QKeyEvent*>(event);

		if((key -> key() == Qt::Key_Enter) || (key -> key() == Qt::Key_Return))
		{
			insert(m_pPlugins -> currentItem());
			return true;
		}
	} 

	return QDialog::eventFilter(obj, event);
}


void PluginsList::filterPlagins(const QString &text)
{
	for(std::size_t i=0; i<m_pPlugins -> count(); i++)
	{
		QListWidgetItem *pitem = m_pPlugins -> item(i);

		if(pitem -> text().contains(text))
			pitem -> setHidden(false);
		else
			pitem -> setHidden(true);
	}

}
