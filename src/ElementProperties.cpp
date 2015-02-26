#include "ElementProperties.h"

#include <QDebug>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QString>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>

#include <gst/gst.h>

ElementProperties::ElementProperties(QSharedPointer<GraphManager> pGraph, const char *name, 
		QWidget *parent, Qt::WindowFlags flags):
QWidget(parent, flags),
m_pGraphManager(pGraph),
m_name(name)
{
	setWindowTitle(QString(name) + " properties");

	create();
}



void ElementProperties::create()
{
	GstElement *element = gst_bin_get_by_name (GST_BIN(m_pGraphManager -> m_pGraph), m_name.toStdString().c_str());

	if(!element)
		return;

	QGridLayout  *play = new QGridLayout;

	GParamSpec **prop_specs;
	guint num_props;

	prop_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (element), 
		&num_props);

	for(std::size_t i = 0; i<num_props; i++) 
	{
		bool readOnly = true;
		GParamSpec *param = prop_specs[i];
		GValue value = { 0 };

		g_value_init (&value, param -> value_type);
		if(param -> flags & G_PARAM_READABLE) 
			g_object_get_property (G_OBJECT(element), param -> name, &value);
		else
		{
			const GValue *valueDef = g_param_spec_get_default_value(param);
			g_value_copy(valueDef, &value);
		}

		if(param->flags & G_PARAM_WRITABLE)
			readOnly = false;

		QString propertyName = g_param_spec_get_name (param);
		QString propertyValue;

		bool skip = false;

		switch (G_VALUE_TYPE (&value))
		{
			case G_TYPE_STRING:
			{
				const char *string_val = g_value_get_string (&value);
				propertyValue = string_val;
				break;
			}
			case G_TYPE_BOOLEAN:
			{
				gboolean bool_val = g_value_get_boolean (&value);
				propertyValue = QString::number(bool_val);
				break;
			}
			case G_TYPE_ULONG:
			{
				propertyValue = QString::number(g_value_get_ulong(&value));
				break;
			}
			case G_TYPE_LONG:
			{
				propertyValue = QString::number(g_value_get_long(&value));
				break;
			}
			case G_TYPE_UINT:
			{
				propertyValue = QString::number(g_value_get_uint(&value));
				break;
			}
			case G_TYPE_INT:
			{
				propertyValue = QString::number(g_value_get_int(&value));
				break;
			}
			case G_TYPE_UINT64:
			{
				propertyValue = QString::number(g_value_get_uint64(&value));
				break;
			}
			case G_TYPE_INT64:
			{
				propertyValue = QString::number(g_value_get_int64(&value));
				break;
			}
			case G_TYPE_FLOAT:
			{
				propertyValue = QString::number(g_value_get_float(&value));
				break;
			}
			case G_TYPE_DOUBLE:
			{
				propertyValue = QString::number(g_value_get_double(&value));
				break;
			}
			default:	
			{
				skip = true;
				qDebug() << "property " << propertyName << " not supported";
				break;
			}
		};

		play -> addWidget(new QLabel(propertyName), i, 0);

		QLineEdit *ple = new QLineEdit(propertyValue);
		ple -> setReadOnly(readOnly);
		play -> addWidget(ple, i, 1);
		if(!skip)
			m_values.insert(propertyName, ple);
		else
			ple -> setReadOnly(true);
	}

	QVBoxLayout *pvblay = new QVBoxLayout;
	QWidget *pwgt = new QWidget(this);
	pwgt -> setLayout(play);
	QScrollArea *pscroll = new QScrollArea(this);
	pscroll -> setWidget(pwgt);

	pvblay -> addWidget(pscroll);

	QHBoxLayout *phblay = new QHBoxLayout;

	QPushButton *pcmdApply = new QPushButton("Apply");
	QPushButton *pcmdOk = new QPushButton("OK");
	QPushButton *pcmdCancel = new QPushButton("Cancel");


	phblay -> addStretch(1);
	phblay -> addWidget(pcmdApply);
	phblay -> addWidget(pcmdCancel);
	phblay -> addWidget(pcmdOk);

	pvblay -> addLayout(phblay);

	QObject::connect(pcmdApply, SIGNAL(clicked()), this, SLOT(applyClicked()));
	QObject::connect(pcmdCancel, SIGNAL(clicked()), this, SLOT(close()));
	QObject::connect(pcmdOk, SIGNAL(clicked()), this, SLOT(okClicked()));

	setLayout(pvblay);

	g_free(prop_specs);
	gst_object_unref(element);

}

void ElementProperties::applyClicked()
{
	GstElement *element = gst_bin_get_by_name (GST_BIN(m_pGraphManager -> m_pGraph), 
		m_name.toStdString().c_str());

	if(!element)
		return;

	QMap<QString, QLineEdit *>::iterator itr = m_values.begin();

	for(;itr != m_values.end(); itr++)
	{
		GParamSpec *param = g_object_class_find_property(G_OBJECT_GET_CLASS (element),
			itr.key().toStdString().c_str());

		if(!param)
		{
			qDebug() << "problem with setting " << itr.key() << " property";
			continue;
		}

		if(!(param -> flags & G_PARAM_WRITABLE))
			continue;

		QString valStr = itr.value() -> text();
		std::string tmpStr = itr.key().toStdString();
		const char *propName = tmpStr.c_str();
		switch (param -> value_type)
		{
			case G_TYPE_STRING:
			{
				g_object_set(G_OBJECT(element), propName, valStr.toStdString().c_str(), NULL);
				break;
			}
			case G_TYPE_BOOLEAN:
			{
				gboolean val = valStr.toInt();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_ULONG:
			{
				gulong val = valStr.toULong();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_LONG:
			{
				glong val = valStr.toLong();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_UINT:
			{
				guint val = valStr.toUInt();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_INT:
			{
				gint val = valStr.toInt();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_UINT64:
			{
				guint64 val = valStr.toULongLong();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_INT64:
			{
				gint64 val = valStr.toLongLong();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_FLOAT:
			{
				gfloat val = valStr.toFloat();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			case G_TYPE_DOUBLE:
			{
				gdouble val = valStr.toDouble();
				g_object_set(G_OBJECT(element), propName, val, NULL);
				break;
			}
			default:	
			{
				qDebug() << "property " << itr.key() << " not supported";
				break;
			}
		};
	}

	gst_object_unref(element);

	delete layout();
	qDeleteAll(children());


	create();
}

void ElementProperties::okClicked()
{
	applyClicked();
	close();
}
