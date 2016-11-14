#include "PipelineIE.h"

#include <QFile>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QDomDocument>

#include <vector>

#include <QDebug>

static void clearPipeline(GstElement *pipeline)
{
	if(!pipeline)
		return;

	GstIterator *iter;
	iter = gst_bin_iterate_elements (GST_BIN (pipeline));
	GstElement *element  = NULL;
	bool done = false;
	while (!done) 
	{
#if GST_VERSION_MAJOR >= 1
		GValue value = { 0 };
		switch (gst_iterator_next (iter, &value)) 
		{
			case GST_ITERATOR_OK:
			{
				element = GST_ELEMENT(g_value_get_object(&value));
#else
		switch (gst_iterator_next (iter, (gpointer *)&element)) 
		{
			case GST_ITERATOR_OK:
			{
#endif
				gst_bin_remove(GST_BIN(pipeline), element);
#if GST_VERSION_MAJOR >= 1
				g_value_reset (&value);
#endif
				iter = gst_bin_iterate_elements (GST_BIN (pipeline));
				if(!iter)
					done = true;

				break;
			}
			case GST_ITERATOR_DONE:
			case GST_ITERATOR_RESYNC:
			case GST_ITERATOR_ERROR:
			{
				done = true;
				break;
			}
		};
	}

	gst_iterator_free (iter);
}

namespace 
{
	struct Connection
	{
		std::string element1;
		std::string pad1;
		std::string element2;
		std::string pad2;
	};
}


static void writeProperties(QXmlStreamWriter &xmlWriter, const GstElement *element)
{
	GParamSpec **prop_specs;
	guint num_props;

	prop_specs = g_object_class_list_properties(G_OBJECT_GET_CLASS (element), 
		&num_props);

	if(!num_props)
		return;

	xmlWriter.writeStartElement("properties");

	for(std::size_t i = 0; i<num_props; i++) 
	{
		GParamSpec *param = prop_specs[i];

		if((param -> flags & G_PARAM_READABLE) && (param -> flags & G_PARAM_WRITABLE))
		{
			GValue value = { 0 };
			g_value_init (&value, param -> value_type);

			g_object_get_property (G_OBJECT(element), param -> name, &value);

			if(!g_param_value_defaults(param, &value))
			{
				bool skip = false;
				QString propertyName = g_param_spec_get_name (param);
				QString propertyValue;


				switch (G_VALUE_TYPE (&value))
				{
					case G_TYPE_STRING:
					{

						const char *string_val = g_value_get_string (&value);
						if(string_val)
							propertyValue = string_val;
						else
							skip = true;
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
						gchar *elementName = gst_element_get_name(element);

						qDebug() << "property `" << propertyName << "` for `" 
							<< elementName << "` not supported";
						g_free(elementName);

						skip = true;
						break;
					}
				};

				if(!skip)
				{
					xmlWriter.writeStartElement("property");
					xmlWriter.writeAttribute("name", propertyName);
					xmlWriter.writeAttribute("value", propertyValue);
					xmlWriter.writeEndElement();
				}
				g_value_reset(&value);

			}
		}
	}

	xmlWriter.writeEndElement();

	g_free(prop_specs);
}



static void loadProperties(QDomElement node, GstElement *element)
{
	QDomNode child = node.firstChild();
	while(!child.isNull())
	{
		if(child.toElement().tagName() == "property")
		{
			QString name = child.toElement().attribute("name");
			QString value = child.toElement().attribute("value");

			GParamSpec *param = g_object_class_find_property(G_OBJECT_GET_CLASS (element),
				name.toStdString().c_str());

			if(!param)
			{
				gchar *elementName = gst_element_get_name(element);
				qDebug() << "problem with setting property `" << name << "` for `" << elementName << "`";
				g_free(elementName);
				continue;
			}

			if(!(param -> flags & G_PARAM_WRITABLE))
				continue;

			std::string tmpStr = name.toStdString();
			const char *propName = tmpStr.c_str();
			switch (param -> value_type)
			{
				case G_TYPE_STRING:
				{
					g_object_set(G_OBJECT(element), propName, value.toStdString().c_str(), NULL);
					break;
				}
				case G_TYPE_BOOLEAN:
				{
					gboolean val = value.toInt();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_ULONG:
				{
					gulong val = value.toULong();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_LONG:
				{
					glong val = value.toLong();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_UINT:
				{
					guint val = value.toUInt();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_INT:
				{
					gint val = value.toInt();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_UINT64:
				{
					guint64 val = value.toULongLong();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_INT64:
				{
					gint64 val = value.toLongLong();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_FLOAT:
				{
					gfloat val = value.toFloat();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				case G_TYPE_DOUBLE:
				{
					gdouble val = value.toDouble();
					g_object_set(G_OBJECT(element), propName, val, NULL);
					break;
				}
				default:	
				{
					gchar *elementName = gst_element_get_name(element);
					qDebug() << "property `" << name << "` for `" << QString(elementName) << "` not supported";
					g_free(elementName);
					break;
				}
			};
		}

		child = child.nextSibling();
	}

}


static void create_requst_pad(GstElement *element, const QString &templateName, const QString &padName)
{
	GstElementClass *klass = GST_ELEMENT_GET_CLASS(element);

	GstPadTemplate *templ = gst_element_class_get_pad_template(klass, templateName.toStdString().c_str());

	gst_element_request_pad(element, templ, padName.toStdString().c_str(), NULL);
}

bool PipelineIE::Export(QSharedPointer<GraphManager> pgraph, const QString &fileName)
{
	QFile file(fileName);
 
	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(0, "Read only", "The file is in read only mode");
		return false;
	}

	std::vector <ElementInfo> info = pgraph -> GetInfo();

	QXmlStreamWriter xmlWriter;
	xmlWriter.setDevice(&file);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("pipeline");

	for(std::size_t i=0; i<info.size(); i++)
	{
		xmlWriter.writeStartElement("element");

		xmlWriter.writeAttribute("name", info[i].m_name.c_str());
		xmlWriter.writeAttribute("plugin-name", info[i].m_pluginName.c_str());

		GstElement *element = gst_bin_get_by_name (GST_BIN(pgraph -> m_pGraph), info[i].m_name.c_str());

		for(std::size_t j=0; j<info[i].m_pads.size(); j++)
		{
			xmlWriter.writeStartElement("pad");

			xmlWriter.writeAttribute("name", info[i].m_pads[j].m_name.c_str());

			GstPad *pad = gst_element_get_static_pad(element, info[i].m_pads[j].m_name.c_str());

			GstPadTemplate *templ = gst_pad_get_pad_template(pad);
			if (templ) {
				QString presence;
				switch(GST_PAD_TEMPLATE_PRESENCE(templ))
				{
					case GST_PAD_ALWAYS:
						presence = "always";
						break;

					case GST_PAD_SOMETIMES:
						presence = "sometimes";
						break;

					case GST_PAD_REQUEST:
						presence = "request";
						break;
				};

				xmlWriter.writeAttribute("presence", presence);
				xmlWriter.writeAttribute("template-name", GST_PAD_TEMPLATE_NAME_TEMPLATE(templ));
			} else {
				qDebug() << "Unable to find a template for" << info[i].m_pads[j].m_name.c_str();
				xmlWriter.writeAttribute("presence", "always");
				xmlWriter.writeAttribute("template-name", "");
			}
			gst_object_unref(pad);

			if(info[i].m_connections[j].m_elementId != (size_t)-1 &&
				info[i].m_connections[j].m_padId != (size_t)-1)
			{
				std::size_t elementPos, padPos;
				for(elementPos = 0; elementPos < info.size(); elementPos++)
				{
					if(info[elementPos].m_id == info[i].m_connections[j].m_elementId)
					{
						for(padPos = 0; padPos < info[elementPos].m_pads.size(); padPos++)
							if(info[elementPos].m_pads[padPos].m_id == info[i].m_connections[j].m_padId)
								break;

						if(padPos < info[elementPos].m_pads.size())
							break;
					}
				}
				if(elementPos < info.size() && padPos < info[elementPos].m_pads.size())
				{
					xmlWriter.writeStartElement("connected-to");
					xmlWriter.writeAttribute("element-name", info[elementPos].m_name.c_str());
					xmlWriter.writeAttribute("pad-name", info[elementPos].m_pads[padPos].m_name.c_str());
					xmlWriter.writeEndElement();
				}
			}

			xmlWriter.writeEndElement();
		}

		writeProperties(xmlWriter, element);
		gst_object_unref(element);

		xmlWriter.writeEndElement();
	}

	xmlWriter.writeEndElement();
	xmlWriter.writeEndDocument();

	return true;
}


bool PipelineIE::Import(QSharedPointer<GraphManager> pgraph, const QString &fileName)
{
	GstElement *pipeline = pgraph -> m_pGraph;
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly | QFile::Text)) 
	{
		QMessageBox::warning(0, "Open failed", 
			QString("Cannot read file ") + fileName + 
			": " + file.errorString());

		return false;
	}

	QString errorStr;
	int errorLine;
	int errorColumn;

	QDomDocument doc;
	if(!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) 
	{
		QMessageBox::warning(0, "Xml parsing failed", 
			QString("Parse error at line ") + QString::number(errorLine) + ", "
			"column " + QString::number(errorColumn) + ": " + errorStr);
		return false;
	}

	clearPipeline(pipeline);

    QDomElement root = doc.documentElement();

	if(root.tagName() != "pipeline") 
	{
		QMessageBox::warning(0, "Parsing failed", "Is invalid pipeline file");
		return false;
	}

	QDomNode child = root.firstChild();

	std::vector<Connection> connections;
	while(!child.isNull()) 
	{
		if(child.toElement().tagName() == "element")
		{
			QDomElement elNode = child.toElement();
			

			GstElement *pel = gst_element_factory_make(elNode.attribute("plugin-name").toStdString().c_str(), 
				elNode.attribute("name").toStdString().c_str());

			if(!pel)
			{
				QMessageBox::warning(0, "Element creation failed", 
					QString("Could not create element of `") + 
					elNode.attribute("plugin-name") + "` with name `" + 
					elNode.attribute("name") + "`");

				child = child.nextSibling();
				continue;
			}

			bool res = gst_bin_add(GST_BIN(pipeline), pel);

			if(!res)
			{
				QMessageBox::warning(0, "Element insertion failed", 
					QString("Could not insert element `") + 
					elNode.attribute("name") + "` to pipeline");

				child = child.nextSibling();
				continue;
			}

			gst_element_sync_state_with_parent(pel);


			QDomNode elementChild = elNode.firstChild();
			while(!elementChild.isNull())
			{
				if(elementChild.toElement().tagName() == "pad")
				{
					QDomNode padChild = elementChild.firstChild();
					QDomElement elPad = elementChild.toElement();
//					GstPadPresence presence = GST_PAD_ALWAYS;

					QString templaneName;
					if(elPad.attribute("presence") == "request")
						create_requst_pad(pel, elPad.attribute("template-name"), elPad.attribute("name"));

					while(!padChild.isNull())
					{
						if(padChild.toElement().tagName() == "connected-to")
						{
							bool isExists = false;
							QDomElement elCoonnectedTo = padChild.toElement();

							for(std::size_t i=0; i<connections.size(); i++)
							{
								if((connections[i].element1 == elNode.attribute("name").toStdString() &&
									connections[i].element2 == elCoonnectedTo.attribute("element-name").toStdString() &&
									connections[i].pad1 == elPad.attribute("name").toStdString() &&
									connections[i].pad2 == elCoonnectedTo.attribute("pad-name").toStdString())
									||
									(connections[i].element2 == elNode.attribute("name").toStdString() &&
									connections[i].element1 == elCoonnectedTo.attribute("element-name").toStdString() &&
									connections[i].pad2 == elPad.attribute("name").toStdString() &&
									connections[i].pad1 ==elCoonnectedTo.attribute("pad-name").toStdString())
								)
								{
									isExists = true;
								}
							}

							if(!isExists)
							{
								Connection newConnetion;
								newConnetion.element1 = elNode.attribute("name").toStdString();
								newConnetion.element2 = padChild.toElement().attribute("element-name").toStdString();
								newConnetion.pad1 = elementChild.toElement().attribute("name").toStdString();
								newConnetion.pad2 = padChild.toElement().attribute("pad-name").toStdString();

								connections.push_back(newConnetion);
							}
						}
						padChild = padChild.nextSibling();
					}
				}
				else if(elementChild.toElement().tagName() == "properties")
				{
					loadProperties(elementChild.toElement(), pel);
				}
				elementChild = elementChild.nextSibling();
			}
		}
		child = child.nextSibling();
	}

	std::size_t maxStarts = 5;
	bool setReady = true;
	for(std::size_t k=0; k<maxStarts; k++)	
	{
		if(connections.empty())
			break;
	
		if(k > 0)
			setReady = false;

		while(true)
		{
			std::size_t i=0;
			for(; i<connections.size(); i++)
			{
				GstElement *el1 = gst_bin_get_by_name (GST_BIN(pipeline), connections[i].element1.c_str());
				GstElement *el2 = gst_bin_get_by_name (GST_BIN(pipeline), connections[i].element2.c_str());

				if(!el1 || !el2)
				{
					QMessageBox::warning(0, "Internal error", 
						QString("Could not find one of elements `") + 
						QString(connections[i].element1.c_str()) + "`, `" + 
						QString(connections[i].element2.c_str()) + "`");				

					gst_object_unref(el1);
					gst_object_unref(el2);

					return false;
				}
			
				GstPad *pad1 = gst_element_get_static_pad(el1, connections[i].pad1.c_str());
				GstPad *pad2 = gst_element_get_static_pad(el2, connections[i].pad2.c_str());

				if(pad1 && pad2)
				{
					if(GST_PAD_IS_SRC(pad1))
						gst_element_link_pads(el1, connections[i].pad1.c_str(), el2, connections[i].pad2.c_str());
					else
						gst_element_link_pads(el2, connections[i].pad2.c_str(), el1, connections[i].pad1.c_str());

					gst_object_unref(pad1);
					gst_object_unref(pad2);
					connections.erase(connections.begin() + i);

					gst_object_unref(el1);
					gst_object_unref(el2);

					break;
				}

				if(pad1)
					gst_object_unref(pad1);
		
				if(pad2)
					gst_object_unref(pad2);

				gst_object_unref(el1);
				gst_object_unref(el2);
			}

			if(i == connections.size())
				break;
		}

		if(!connections.empty())
		{
			gst_element_set_state(pipeline, GST_STATE_PAUSED);

			GstState state;
			gst_element_get_state (pipeline, &state, NULL, GST_MSECOND * 2500);
		}

	}

	if(setReady)
		gst_element_set_state(pipeline, GST_STATE_READY);

	return true;
}
