#ifndef GRAPH_MANAGER_H_
#define GRAPH_MANAGER_H_

#include <gst/gst.h>

#include <string>
#include <vector>

class QString;
class PluginsList;

enum ePadCapsSubset {
  PAD_CAPS_ALLOWED = 0,
  PAD_CAPS_NEGOCIATED,
  PAD_CAPS_ALL
};

struct PadInfo
{
	public:
	enum PadType
	{
		None,
		Out,
		In
	};

	size_t        m_id;
	PadType       m_type;
	std::string   m_name;


	bool operator == (const PadInfo &obj) const
	{
		if(this == &obj)
			return true;

		if(m_id != obj.m_id)
			return false;

		if(m_type != obj.m_type)
			return false;

		if(m_name != obj.m_name)
			return false;

		return true;
	}
};

struct ElementInfo
{
	struct Connection
	{
		size_t           m_padId;
		size_t           m_elementId;

		bool operator == (const Connection &obj) const
		{
			if(this == &obj)
				return true;

			if(m_padId != obj.m_padId)
				return false;

			if(m_elementId != obj.m_elementId)
				return false;

			return true;
		}
	};


	size_t                       m_id;
	std::string                  m_name;
	std::string                  m_pluginName;
	std::vector<PadInfo>         m_pads;
	std::vector<Connection>      m_connections;
};


class GraphManager
{
public:
	GraphManager();
	~GraphManager();

	bool AddPlugin(const char *plugin, const char *name);
	bool RemovePlugin(const char *name);
	bool Connect(const char *srcElement, const char *srcPad,
		const char *dstElement, const char *dstPad);
	bool Disconnect(const char *srcElement, const char *srcPad,
		const char *dstElement, const char *dstPad);
	std::vector <ElementInfo> GetInfo();

	bool OpenUri(const char *uri, const char *name);

	double GetPosition();
	bool SetPosition(double);

	PluginsList* getPluginsList() { return m_pluginsList;}

	bool Play();
	bool Pause();
	bool Stop();

	QString getPadCaps(ElementInfo* elementInfo, PadInfo* padInfo, ePadCapsSubset subset, bool afTruncated = false);

	GstElement       *m_pGraph;
	PluginsList       *m_pluginsList;

};


#endif
