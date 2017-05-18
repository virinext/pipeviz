#include "GraphManager.h"
#include "PluginsList.h"
#include <QDebug>

#include <QString>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>

#include "CustomSettings.h"

#define MAX_STR_CAPS_SIZE 150
gchar* get_str_caps_limited(gchar* str)
{
  gchar* result;
  if(strlen(str) > MAX_STR_CAPS_SIZE) {
      result = g_strndup(str, MAX_STR_CAPS_SIZE);
      for(int i = strlen(result)-1;i > strlen(result)-4 ;i--)
	result[i] = '.';
  } else
    result = g_strdup(str);
  return result;
}

GraphManager::GraphManager()
{
	m_pGraph = gst_pipeline_new ("pipeline");
	m_pluginsList = new PluginsList();
}

GraphManager::~GraphManager()
{
  delete m_pluginsList;
}

QString GraphManager::getPadCaps(ElementInfo* elementInfo, PadInfo* padInfo, ePadCapsSubset subset,  bool afTruncated)
{
	QString padCaps = "";
	if(!elementInfo || !padInfo)
	  return padCaps;

	GstElement *element = gst_bin_get_by_name (GST_BIN(m_pGraph), elementInfo->m_name.c_str());

	if(!element)
		return padCaps;
	GstPad *pad = gst_element_get_static_pad(GST_ELEMENT(element), padInfo->m_name.c_str());
	if(!pad) {
	    gst_object_unref(element);
	    return padCaps;
	}
	GstCaps *caps;
	switch(subset)
	{
	  case PAD_CAPS_ALLOWED:
	    caps =  gst_pad_get_allowed_caps(pad);
	    break;
	  case PAD_CAPS_NEGOCIATED:
#if GST_VERSION_MAJOR >= 1
	caps = gst_pad_get_current_caps(pad);
#else
	caps = gst_pad_get_negotiated_caps(pad);
#endif
	  break;
	  case PAD_CAPS_ALL:
	  default:
#if GST_VERSION_MAJOR >= 1
	    caps = gst_pad_query_caps(pad, NULL);
#else
	    caps = gst_pad_get_caps(pad);
#endif
	}
#if GST_VERSION_MAJOR >= 1
	caps = gst_pad_query_caps(pad, NULL);
#else
	caps = gst_pad_get_caps(pad);
#endif
	if(caps) {
	    gchar* str = gst_caps_to_string(caps);
	    if (afTruncated) {
		    gchar* str_limited = get_str_caps_limited(str);
		    g_free(str);
		    padCaps = str_limited;
		    g_free(str_limited);
	    } else {
		 padCaps = str;
		 g_free(str);
	    }

	}

	gst_object_unref(element);
	gst_object_unref (pad);
	return padCaps;
}

bool GraphManager::AddPlugin(const char *plugin, const char *name)
{
	GstElement *pel = gst_element_factory_make(plugin, name);
	if(!pel)
		return false;

	if(GST_IS_URI_HANDLER(pel))
	{
		static const gchar *const *protocols;
		protocols = gst_uri_handler_get_protocols(GST_URI_HANDLER(pel));

		bool isFile = false;

		for(std::size_t i=0; protocols[i] != NULL; i++)
		{
			if(strcmp("file", protocols[i]) == 0)
			{
				isFile = true;
				break;
			}
		}

		if(isFile)
		{
			QString path;
			QString dir = CustomSettings::lastIODirectory();

			if(gst_uri_handler_get_uri_type(GST_URI_HANDLER(pel)) == GST_URI_SRC) 
				path = QFileDialog::getOpenFileName(NULL, "Open Source File...", dir);
			else
				path = QFileDialog::getSaveFileName(NULL, "Open Sink File...", dir);

			if(!path.isEmpty())
			{
				gchar *uri = gst_filename_to_uri(path.toStdString().c_str(), NULL);
				if(uri)
				{
					qDebug() << "Set uri: " << uri;
#if GST_VERSION_MAJOR >= 1
					gst_uri_handler_set_uri(GST_URI_HANDLER(pel), uri, NULL);
#else
					gst_uri_handler_set_uri(GST_URI_HANDLER(pel), uri);
#endif
					g_free(uri);

					QString dir = QFileInfo(path).absoluteDir().absolutePath();
					CustomSettings::saveLastIODirectory(dir);
				}
			}
		}
		else
		{
			QString uri = QInputDialog::getText(NULL, "Uri...", "Uri:");

			if(!uri.isEmpty())
			{
				qDebug() << "Set uri: " << uri;
#if GST_VERSION_MAJOR >= 1
				gst_uri_handler_set_uri(GST_URI_HANDLER(pel), uri.toStdString().c_str(), NULL);
#else
				gst_uri_handler_set_uri(GST_URI_HANDLER(pel), uri.toStdString().c_str());
#endif
			}

		}
	}

	bool res = gst_bin_add(GST_BIN(m_pGraph), pel);
	if(res)
		gst_element_sync_state_with_parent(pel);

	return res;
}


bool GraphManager::RemovePlugin(const char *name)
{
	GstElement *element = gst_bin_get_by_name (GST_BIN(m_pGraph), name);

	if(!element)
		return false;

	bool res = gst_bin_remove (GST_BIN(m_pGraph), element);
	gst_object_unref(element);

	return res;
}


bool GraphManager::OpenUri(const char *uri, const char *name)
{
#if GST_VERSION_MAJOR >= 1
	GstElement *element = gst_element_make_from_uri(GST_URI_SRC, uri, name, NULL);
#else
	GstElement *element = gst_element_make_from_uri(GST_URI_SRC, uri, name);
#endif
	if(!element)
		return false;

	bool res = gst_bin_add(GST_BIN(m_pGraph), element);
	if(res)
		gst_element_sync_state_with_parent(element);

	return res;
}


bool GraphManager::Connect(const char *srcElement, const char *srcPad,
	const char *dstElement, const char *dstPad)
{
	GstElement *src = gst_bin_get_by_name (GST_BIN(m_pGraph), srcElement);
	GstElement *dst = gst_bin_get_by_name (GST_BIN(m_pGraph), dstElement);

	bool res = gst_element_link_pads(src, srcPad, dst, dstPad);

	gboolean seekRes = gst_element_seek_simple(m_pGraph, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 0);

	gst_object_unref(src);
	gst_object_unref(dst);

	return res;
}



bool GraphManager::Disconnect(const char *srcElement, const char *srcPad,
	const char *dstElement, const char *dstPad)
{
	GstElement *src = gst_bin_get_by_name (GST_BIN(m_pGraph), srcElement);
	GstElement *dst = gst_bin_get_by_name (GST_BIN(m_pGraph), dstElement);


	gst_element_unlink_pads(src, srcPad, dst, dstPad);

	gst_object_unref(src);
	gst_object_unref(dst);

	return true;
}


std::vector <ElementInfo> GraphManager::GetInfo()
{
	std::vector <ElementInfo> res;

	GstIterator *iter;
	iter = gst_bin_iterate_elements (GST_BIN (m_pGraph));
	GstElement* element = NULL;
	bool done = false;
	size_t id = 0;
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
				ElementInfo elementInfo;
				elementInfo.m_id = id;
				id++;


				gchar *name = gst_element_get_name(element);
				elementInfo.m_name = name;
				g_free(name);

				GstElementFactory *pfactory =
					gst_element_get_factory(element);

				elementInfo.m_pluginName =
					gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(pfactory));

				GstIterator *padItr = gst_element_iterate_pads (element);
				bool padDone = FALSE;
				std::size_t padId = 0;
				GstPad *pad;
				while (!padDone) 
				{
#if GST_VERSION_MAJOR >= 1
					GValue padVal = { 0 };
					switch (gst_iterator_next (padItr, &padVal))
					{
						case GST_ITERATOR_OK:
						{
							pad = GST_PAD(g_value_get_object(&padVal));
#else
					switch (gst_iterator_next (padItr, (gpointer *)&pad))
					{
						case GST_ITERATOR_OK:
						{
#endif
							PadInfo padInfo;
							padInfo.m_id = padId;

							gchar *pad_name = gst_pad_get_name(pad);
							padInfo.m_name = pad_name;
							g_free(pad_name);

							GstPadDirection direction = gst_pad_get_direction(pad);
							if(direction == GST_PAD_SRC)
								padInfo.m_type = PadInfo::Out;
							else if(direction == GST_PAD_SINK)
								padInfo.m_type = PadInfo::In;
							else
								padInfo.m_type = PadInfo::None;

							elementInfo.m_pads.push_back(padInfo);
#if GST_VERSION_MAJOR >= 1
							g_value_reset (&padVal);
#endif
							break;
						}
						case GST_ITERATOR_RESYNC:
						case GST_ITERATOR_ERROR:
						case GST_ITERATOR_DONE:
							padDone = TRUE;
							break;
					};
					padId++;
				}
#if GST_VERSION_MAJOR >= 1
				g_value_reset (&value);
#endif
				res.push_back(elementInfo);
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

	for(std::size_t i=0; i<res.size(); i++)
	{
		res[i].m_connections.resize(res[i].m_pads.size());

		GstElement *element = gst_bin_get_by_name (GST_BIN(m_pGraph), res[i].m_name.c_str());

		for(std::size_t j=0; j<res[i].m_pads.size(); j++)
		{
			res[i].m_connections[j].m_elementId = -1;
			res[i].m_connections[j].m_padId = -1;

			GstPad *pad = gst_element_get_static_pad (element, res[i].m_pads[j].m_name.c_str());
			GstPad *peerPad = gst_pad_get_peer(pad);

			if(peerPad)
			{
				GstElement *peerElement = GST_ELEMENT(gst_pad_get_parent(peerPad));

				gchar *peerName = gst_element_get_name(peerElement);
				gchar *peerPadName = gst_pad_get_name(peerPad);

				for(std::size_t k=0; k<res.size(); k++)
				{
					if(res[k].m_name == peerName)
					{
						for(std::size_t l=0; l<res[k].m_pads.size(); l++)
						{
							if(res[k].m_pads[l].m_name == peerPadName)
							{
								res[i].m_connections[j].m_elementId = res[k].m_id;
								res[i].m_connections[j].m_padId = res[k].m_pads[l].m_id;
								break;
							}
						}	
					}
				}

				g_free(peerName);
				g_free(peerPadName);

				gst_object_unref(peerPad);
				gst_object_unref(peerElement);
			}

			gst_object_unref(pad);
		}
		gst_object_unref(element);

	}

	return res;
}


bool GraphManager::Play()
{
	GstStateChangeReturn res;
	gst_element_set_state(m_pGraph, GST_STATE_PLAYING);

	GstState state;
	res = gst_element_get_state (m_pGraph, &state, NULL, GST_SECOND);

	if(res != GST_STATE_CHANGE_SUCCESS)
	{
		gst_element_abort_state(m_pGraph);
		qDebug() << "state changing to Play was FAILED";
	}

	return res == GST_STATE_PLAYING;
}


bool GraphManager::Pause()
{
	GstStateChangeReturn res;

	GstState state;
	gst_element_set_state(m_pGraph, GST_STATE_PAUSED);
	res = gst_element_get_state (m_pGraph, &state, NULL, GST_SECOND);
	if(res != GST_STATE_CHANGE_SUCCESS)
	{
		gst_element_abort_state(m_pGraph);
		qDebug() << "state changing to Pause was FAILED";
	}

	return res == GST_STATE_PAUSED;
}


bool GraphManager::Stop()
{
	GstStateChangeReturn res = gst_element_set_state(m_pGraph, GST_STATE_READY);
	return res == GST_STATE_CHANGE_SUCCESS;
}

double GraphManager::GetPosition()
{
	gint64 current, duration;
	GstFormat fmt = GST_FORMAT_TIME;
#if GST_VERSION_MAJOR >= 1
	if(!gst_element_query_position(m_pGraph, fmt, &current))
		return 0;
#else
	if(!gst_element_query_position(m_pGraph, &fmt, &current))
		return 0;
#endif

#if GST_VERSION_MAJOR >= 1
	if(!gst_element_query_duration(m_pGraph, fmt, &duration))
		return 0;
#else
	if(!gst_element_query_duration(m_pGraph, &fmt, &duration))
		return 0;
#endif

	if(duration < 0 || current < 0)
		return 0;

	return (double) current / duration;
}


bool GraphManager::SetPosition(double pos)
{
	GstQuery *query = gst_query_new_seeking(GST_FORMAT_TIME);
	GstFormat fmt = GST_FORMAT_TIME;
	if(!query)
		return false;

	if(!gst_element_query(m_pGraph, query))
		return false;

	gboolean seekable;
	gst_query_parse_seeking(query, NULL, &seekable, NULL, NULL);

	gst_query_unref(query);

	if(!seekable)
		return false;

	gint64 duration;

#if GST_VERSION_MAJOR >= 1
	if(!gst_element_query_duration(m_pGraph, fmt, &duration))
		return 0;
#else
	if(!gst_element_query_duration(m_pGraph, &fmt, &duration))
		return 0;
#endif

	if(duration < 0)
		return 0;

	gboolean seekRes = gst_element_seek_simple(m_pGraph, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, duration * pos);

	return seekRes;
}
