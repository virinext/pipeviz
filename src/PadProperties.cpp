#include "PadProperties.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QScrollArea>

#include <gst/gst.h>

PadProperties::PadProperties(QSharedPointer<GraphManager> pGraphManager, const char *elementName, const char *padName,
		QWidget *parent, Qt::WindowFlags flags):
QWidget(parent, flags)
{
	setWindowTitle(QString(elementName) + "::" + padName + " properties");
	GstElement *element = gst_bin_get_by_name (GST_BIN(pGraphManager -> m_pGraph), elementName);

	if(!element)
		return;

	GstPad *pad = gst_element_get_static_pad(GST_ELEMENT(element), padName);

	QGridLayout  *play = new QGridLayout;

	play -> addWidget(new QLabel("Name"), 0, 0);

	QLabel *plbl = new QLabel(padName);
	plbl -> setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	play -> addWidget(plbl, 0, 1);

	play -> addWidget(new QLabel("All caps:"), 1, 0);
	GstCaps *caps = gst_pad_query_caps(pad, NULL);
	gchar *str;
	gchar *noSpecified = (gchar *)"not specified";
	if(caps)
		str = gst_caps_to_string(caps);
	else
		str = noSpecified;

	plbl = new QLabel(QString(str));
	plbl -> setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	play -> addWidget(plbl, 1, 1);
	if(caps)
	{
		g_free(str);
		gst_caps_unref(caps);
	}

	play -> addWidget(new QLabel("Allowed caps:"), 2, 0);
	caps = gst_pad_get_allowed_caps(pad);
	str = NULL;
	if(caps)
		str = gst_caps_to_string(caps);
	else
		str = noSpecified;

	plbl = new QLabel(QString(str));
	plbl -> setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	play -> addWidget(plbl, 2, 1);
	if(caps)
	{
		g_free(str);
		gst_caps_unref(caps);
	}

	play -> addWidget(new QLabel("Current caps"), 3, 0);
	caps = gst_pad_get_current_caps(pad);
	str = NULL;
	if(caps)
		str = gst_caps_to_string(caps);
	else
		str = noSpecified;

	plbl = new QLabel(QString(str));
	plbl -> setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	play -> addWidget(plbl, 3, 1);
	if(caps)
	{
		g_free(str);
		gst_caps_unref(caps);
	}

	gst_object_unref(element);
	gst_object_unref (pad);

	QVBoxLayout *pvblay = new QVBoxLayout;
	QWidget *pwgt = new QWidget(this);
	pwgt -> setLayout(play);
	QScrollArea *pscroll = new QScrollArea(this);
	pscroll -> setWidget(pwgt);

	pvblay -> addWidget(pscroll);

	setLayout(pvblay);
}