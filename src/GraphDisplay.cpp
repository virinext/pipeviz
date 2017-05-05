#include "GraphDisplay.h"

#include <cassert>
#include <math.h>

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QTableWidget>
#include <QVariant>

#include <QDebug>

#include "ElementProperties.h"
#include "PadProperties.h"
#include "CustomMenuAction.h"

#define PAD_SIZE 8
#define PAD_SIZE_ACTION 16


GraphDisplay::GraphDisplay(QWidget *parent, Qt::WindowFlags f):
QWidget(parent, f)
{
	setFocusPolicy(Qt::WheelFocus);
	setMouseTracking(true);
}

ElementInfo* GraphDisplay::getElement(std::size_t elementId)
{
	ElementInfo* element = NULL;
	std::size_t i=0;
	for(; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == elementId) {
			element = &m_info[i];
			break;
		}
	}
	return element;
}

PadInfo* GraphDisplay::getPad(ElementInfo* element, std::size_t padId)
{
  PadInfo* pad = NULL;
  std::size_t j=0;
  for(; j<element->m_pads.size(); j++)
	  if(element->m_pads[j].m_id == padId) {
	      pad = &element->m_pads[j];
	      break;
	  }
  return pad;
}

void GraphDisplay::updateDisplayInfoIds()
{
	for(std::size_t i=0; i<m_info.size(); i++)
	{
		for(std::size_t j=0; j<m_displayInfo.size(); j++)
		{
			if(m_info[i].m_name == m_displayInfo[j].m_name)
			{
				m_displayInfo[j].m_id = m_info[i].m_id;
				for(std::size_t k=0; k<m_displayInfo.size(); k++)
				{
					if(k != j && m_displayInfo[j].m_id == m_displayInfo[k].m_id)
						m_displayInfo[k].m_id = (size_t)-1;
				}
			}
		}
	}
}


void GraphDisplay::update(const std::vector <ElementInfo> &info)
{
	bool needUpdate = false;

	if(m_info.size() != info.size())
		needUpdate = true;

	if(!needUpdate)
	{
		for(std::size_t i=0; i<info.size(); i++)
		{
			std::size_t j=0;

			for(; j<m_info.size(); j++)
			{
				if(info[i].m_name == m_info[j].m_name)
					break;
			}

			if(j == m_info.size())
			{
				needUpdate = true;
				break;
			}

			if(info[i].m_pads != m_info[j].m_pads)
			{
				needUpdate = true;
				break;
			}

			if(info[i].m_connections != m_info[j].m_connections)
			{
				needUpdate = true;
				break;
			}
		}
	}

	if(needUpdate)
	{
		m_info = info;
		updateDisplayInfoIds();
		calculatePositions();
		repaint();
	}
}


void GraphDisplay::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QPen defaultPen = painter.pen();
	for(std::size_t i=0; i<m_displayInfo.size(); i++)
	{
		if(m_displayInfo[i].m_isSelected)
			painter.setPen(QPen(Qt::blue));

		painter.drawRect(m_displayInfo[i].m_rect);

		painter.setPen(defaultPen);

		for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
		{

			QPoint point = getPadPosition(m_info[i].m_id, m_info[i].m_pads[j].m_id);

			int xPos, yPos;
			xPos = point.x();
			yPos = point.y();

			xPos -= PAD_SIZE / 2;
			yPos -= PAD_SIZE / 2;

			painter.fillRect(xPos, yPos, PAD_SIZE, PAD_SIZE, Qt::black);

			QPoint textPos;

			QRect rect = painter.boundingRect(0, 0, width(), height(), Qt::AlignLeft | Qt::AlignTop, QString(m_info[i].m_pads[j].m_name.c_str()));
			if(m_info[i].m_pads[j].m_type == PadInfo::Out)
				textPos = QPoint(point.x() - PAD_SIZE - rect.width(), point.y() + PAD_SIZE / 2);
			else if(m_info[i].m_pads[j].m_type == PadInfo::In)
				textPos = QPoint(point.x() + PAD_SIZE, point.y() + PAD_SIZE / 2);
			painter.drawText(textPos, QString(m_info[i].m_pads[j].m_name.c_str()));

			if(m_info[i].m_connections[j].m_elementId != ((size_t)-1) && m_info[i].m_connections[j].m_padId != ((size_t)-1))
			{
				xPos = point.x();
				yPos = point.y();

				point = getPadPosition(m_info[i].m_connections[j].m_elementId, m_info[i].m_connections[j].m_padId);
				int xPosPeer, yPosPeer;

				xPosPeer = point.x();
				yPosPeer = point.y();

				painter.drawLine(xPos, yPos, xPosPeer, yPosPeer);
			}

		}

		painter.drawText(m_displayInfo[i].m_rect.topLeft() + QPoint(10, 15), QString(m_displayInfo[i].m_name.c_str()));
	}

	if(m_moveInfo.m_action == MakeConnect)
	{
		painter.drawLine(m_moveInfo.m_position, m_moveInfo.m_startPosition);
	}
	else if(m_moveInfo.m_action == Select)
	{
		if(!m_moveInfo.m_position.isNull())
		{
			painter.setPen(Qt::DashLine);
			painter.drawRect(QRect(m_moveInfo.m_startPosition, m_moveInfo.m_position));
		}
	}
}


GraphDisplay::ElementDisplayInfo GraphDisplay::calculateOnePosition(const ElementInfo &info)
{
	ElementDisplayInfo displayInfo;
	displayInfo.m_id = info.m_id;
	displayInfo.m_name = info.m_name;
	displayInfo.m_isSelected = false;

	int width = 150;
	int height = 50;

	int numInPads, numOutPads;
	numInPads = numOutPads = 0;

	for(std::size_t j=0; j<info.m_pads.size(); j++)
	{
		if(info.m_pads[j].m_type == PadInfo::Out)
			numOutPads++;
		else if(info.m_pads[j].m_type == PadInfo::In)
			numInPads++;
	}

	if(std::max(numInPads, numOutPads) >= 1)
		height += (std::max(numInPads, numOutPads) - 1) * 25;

	int curX, curY;
	curX = curY = 10;

	QRect rect(curX, curY, width, height);

	while(true)
	{
		rect = QRect(curX, curY, width, height);
		QRect rectTest(curX, curY - 15, width + 15, height + 15);
		bool noIntersects = true;
		for(std::size_t i=0; i<m_displayInfo.size(); i++)
		{
			if(rectTest.intersects(m_displayInfo[i].m_rect))
			{
				noIntersects = false;
				break;
			}
		}

		if(noIntersects)
			break;

		curY += 25;
	}

	displayInfo.m_rect = rect;

	return displayInfo;

}

void GraphDisplay::calculatePositions()
{
	while(true)
	{
		std::size_t i=0;
		for(; i<m_displayInfo.size(); i++)
		{
			std::size_t j=0;
			for(; j<m_info.size(); j++)
			{
				if(m_displayInfo[i].m_id == m_info[j].m_id)
					break;
			}

			if(j == m_info.size())
			{
				m_displayInfo.erase(m_displayInfo.begin() + i);
				break;
			}
		}

		if(i >= m_displayInfo.size())
			break;
	}


	std::size_t i=0;
	for(; i<m_info.size(); i++)
	{
		std::size_t j=0;
		for(; j<m_displayInfo.size(); j++)
		{
			if(m_displayInfo[j].m_id == m_info[i].m_id)
				break;
		}

		if(j == m_displayInfo.size())
			m_displayInfo.push_back(calculateOnePosition(m_info[i]));
	}

	std::vector <ElementDisplayInfo> reorderedDisplayInfo(m_info.size());
	for(std::size_t i=0; i<m_info.size(); i++)
	{
		for(std::size_t j=0; j<m_displayInfo.size(); j++)
		{
			if(m_displayInfo[j].m_id == m_info[i].m_id)
			{
				reorderedDisplayInfo[i] = m_displayInfo[j];
				break;
			}
		}
	}

	m_displayInfo = reorderedDisplayInfo;
}


void GraphDisplay::mousePressEvent(QMouseEvent *event)
{
	std::size_t elementId, padId;
	getIdByPosition(event -> pos(), elementId, padId);


	if(event -> buttons() & Qt::RightButton)
	{
		showContextMenu(event);
	}
	else
	{
		if(padId != ((size_t)-1))
		{
			m_moveInfo.m_padId = padId;
			m_moveInfo.m_elementId = elementId;
			m_moveInfo.m_position = event -> pos(); 
			m_moveInfo.m_action = MakeConnect;
			m_moveInfo.m_startPosition = event -> pos();
		}
		else if(elementId != ((size_t)-1))
		{
			m_moveInfo.m_elementId = elementId;
			m_moveInfo.m_padId = -1;
			m_moveInfo.m_position = event -> pos(); 
			m_moveInfo.m_action = MoveComponent;
			m_moveInfo.m_startPosition = event -> pos();
		}
		else
		{
			m_moveInfo.m_startPosition = event -> pos(); 
			m_moveInfo.m_action = Select;
			m_moveInfo.m_position = QPoint();
		}
	}

	for(std::size_t i=0; i<m_displayInfo.size(); i++)
		m_displayInfo[i].m_isSelected = false;
}



void GraphDisplay::mouseReleaseEvent (QMouseEvent *event)
{
	if(m_moveInfo.m_action == MakeConnect)
	{
		std::size_t elementId, padId;
		getIdByPosition(event -> pos(), elementId, padId);
	
		if(elementId != ((size_t)-1) && padId != ((size_t)-1))
		{
			ElementInfo infoSrc, infoDst;
			const char *srcPad, *dstPad;
			srcPad = NULL;
			dstPad = NULL;

			for(std::size_t i=0; i<m_info.size(); i++)
			{
				if(m_info[i].m_id == m_moveInfo.m_elementId)
				{
					infoSrc = m_info[i];
					for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
					{
						if(m_info[i].m_pads[j].m_id == m_moveInfo.m_padId)
						{
							srcPad = m_info[i].m_pads[j].m_name.c_str();
							break;
						}
					}
				}
				if(m_info[i].m_id == elementId)
				{
					infoDst = m_info[i];
					for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
					{
						if(m_info[i].m_pads[j].m_id == padId)
						{
							dstPad = m_info[i].m_pads[j].m_name.c_str();
							break;
						}
					}
				}

				if(srcPad != NULL && dstPad != NULL)
					break;
			}
			if(!infoSrc.m_name.compare(infoDst.m_name)) {
			      qDebug() << "infoSrc == infoDst. No need to connect anything.";
			      goto exit;

			}

			assert(srcPad != NULL && dstPad != NULL);

			qDebug() << "Connection from " << infoSrc.m_name.c_str() << ":" << srcPad
				<< " to " << infoDst.m_name.c_str() << ":" << dstPad;

			if(!m_pGraph -> Connect(infoSrc.m_name.c_str(), srcPad, infoDst.m_name.c_str(), dstPad))
			{
				QString msg;
				msg = "Connection ";
				msg += QString(infoSrc.m_name.c_str()) + ":" + srcPad;
				msg += " => ";
				msg += QString(infoDst.m_name.c_str()) + ":" + dstPad;
				msg += " was FAILED";

				QMessageBox::warning(this, "Coonection failed", msg);
			}

			m_info = m_pGraph -> GetInfo();
			updateDisplayInfoIds();
			if(g_str_has_prefix(infoDst.m_name.c_str(),"decodebin")) {
				m_pGraph->Play();
				qDebug() << "Launch play to discover the new pad";
			}
		}
	}
	else if(m_moveInfo.m_action == Select)
	{
		std::size_t width = std::abs(m_moveInfo.m_position.x() - m_moveInfo.m_startPosition.x());
		std::size_t height = std::abs(m_moveInfo.m_position.y() - m_moveInfo.m_startPosition.y());

		if(!m_moveInfo.m_position.isNull() && width * height > 5)
		{
			QRect selectionRect(m_moveInfo.m_startPosition, m_moveInfo.m_position);
			for(std::size_t i=0; i<m_displayInfo.size(); i++)
			{
				if(selectionRect.intersects(m_displayInfo[i].m_rect))
					m_displayInfo[i].m_isSelected = true;
			}
		}

		repaint();
	}
	else if(m_moveInfo.m_action == MoveComponent)
	{
		int dx = event -> x() - m_moveInfo.m_startPosition.x();
		int dy = event -> y() - m_moveInfo.m_startPosition.y();

		if(dx == dy && dy == 0)
		{
			for(std::size_t i=0; i<m_displayInfo.size(); i++)
			{
				if(m_displayInfo[i].m_id == m_moveInfo.m_elementId)
				{
					m_displayInfo[i].m_isSelected = true;
					repaint();
					break;
				}
			}
		}

	}
exit:
	m_moveInfo.m_action = None;
	m_moveInfo.m_elementId = -1;
	m_moveInfo.m_padId = -1;
	m_moveInfo.m_startPosition = QPoint();
	m_moveInfo.m_position = QPoint();
	repaint();
}


void GraphDisplay::mouseMoveEvent(QMouseEvent *event)
{
	if(m_moveInfo.m_action == MoveComponent)
	{
		int dx = event -> x() - m_moveInfo.m_position.x();
		int dy = event -> y() - m_moveInfo.m_position.y();

		for(std::size_t i=0; i<m_displayInfo.size(); i++)
		{
			if(m_displayInfo[i].m_id == m_moveInfo.m_elementId)
			{
				QRect newRect = m_displayInfo[i].m_rect;
				newRect.adjust(dx, dy, dx, dy);

				if(contentsRect().contains(newRect))
					m_displayInfo[i].m_rect = newRect;

				break;
			}
		}

	}

	if(m_moveInfo.m_action != None)
	{
		m_moveInfo.m_position = event -> pos();
		repaint();
	} else {
	    std::size_t elementId, padId;
	    getIdByPosition(event -> pos(), elementId, padId);
	    if (padId != ((size_t)-1)) {
		ElementInfo* element = getElement(elementId);
		PadInfo* pad = getPad(element, padId);
		QString caps = m_pGraph->getPadCaps(element,pad,PAD_CAPS_ALL,true);
		setToolTip(caps);
	    }
	    else
	      setToolTip("");

	}
}



void GraphDisplay::keyPressEvent(QKeyEvent* event)
{
	if(event -> key() == Qt::Key_Delete)
		removeSelected();

	return QWidget::keyPressEvent(event);
}


void GraphDisplay::showContextMenu(QMouseEvent *event)
{
	QMenu menu;

	std::size_t elementId, padId;
	getIdByPosition(event -> pos(), elementId, padId);

	GstState state;
	GstStateChangeReturn res = gst_element_get_state (m_pGraph -> m_pGraph, &state, NULL, GST_MSECOND);

	bool isActive = false;

	if(res != GST_STATE_CHANGE_SUCCESS || state == GST_STATE_PAUSED || state == GST_STATE_PLAYING)
		isActive = true;

	int selectedCount = 0;
	for(std::size_t i=0; i<m_displayInfo.size(); i++)
		if(m_displayInfo[i].m_isSelected)
			selectedCount++;

	if(selectedCount > 1)
	{
		CustomMenuAction *pact = new CustomMenuAction("Remove selected", &menu);
		menu.addAction(pact);
		if(isActive)
			pact -> setDisabled(true);

	}
	else if(padId != ((size_t)-1))
		menu.addAction(new CustomMenuAction("Pad properties", &menu));
	else if(elementId != ((size_t)-1))
	{
		menu.addAction(new CustomMenuAction("Element properties", &menu));
		QAction *pact = new CustomMenuAction("Remove", &menu);
		menu.addAction(pact);

		if(isActive)
			pact -> setDisabled(true);

		pact = new CustomMenuAction("Request pad...", &menu);
		menu.addAction(pact);
	}
	else
	{

		for(std::size_t i=0; i<m_info.size(); i++)
		{
			for(std::size_t j=0; j<m_info[i].m_connections.size(); j++)
			{

				QPoint point = getPadPosition(m_info[i].m_id, m_info[i].m_pads[j].m_id);

				double x1, y1;
				x1 = point.x();
				y1 = point.y();

				if(m_info[i].m_connections[j].m_elementId != ((size_t)-1) && m_info[i].m_connections[j].m_padId != ((size_t)-1))
				{
					point = getPadPosition(m_info[i].m_connections[j].m_elementId, m_info[i].m_connections[j].m_padId);
					double x2, y2;

					x2 = point.x();
					y2 = point.y();

					double dy = y2 - y1;
					double dx = x2 - x1;

					double x0 = event -> pos().x();
					double y0 = event -> pos().y();

					double distance = std::abs((int)(dy * x0 - dx * y0 + x2 * y1 - y2 * x1));
					distance = distance / sqrt(dy * dy + dx * dx);

					if(distance < 5)
					{
						elementId = m_info[i].m_id;
						padId = m_info[i].m_pads[j].m_id;

						QAction *pact = new CustomMenuAction("Disconnect", &menu);
						menu.addAction(pact);


						if(isActive)
							pact -> setDisabled(true);
						break;
					}
				}
			}
			if(!menu.isEmpty())
				break;
		}	
	}

	if(!menu.isEmpty())
	{
		CustomMenuAction *pact = (CustomMenuAction*)menu.exec(event -> globalPos());
		if(pact)
		{
			if(pact -> getName() == "Remove")
				removePlugin(elementId);
			else if(pact -> getName() == "Element properties")
				showElementProperties(elementId);
			else if(pact -> getName() == "Pad properties")
				showPadProperties(elementId, padId);
			else if(pact -> getName() == "Disconnect")
				disconnect(elementId, padId);
			else if(pact -> getName() == "Request pad...")
				requestPad(elementId);
			else if(pact -> getName() == "Remove selected")
				removeSelected();
		}
	}
}


void GraphDisplay::removeSelected()
{
	GstState state;
	GstStateChangeReturn res = gst_element_get_state (m_pGraph -> m_pGraph, &state, NULL, GST_MSECOND);

	if(res != GST_STATE_CHANGE_SUCCESS || state == GST_STATE_PAUSED || state == GST_STATE_PLAYING)
		return;

	while(true)
	{
		std::size_t i=0;
		for(; i<m_displayInfo.size(); i++)
		{
			if(m_displayInfo[i].m_isSelected)
				break;
		}

		if(i != m_displayInfo.size())
			removePlugin(m_displayInfo[i].m_id);
		else 
			break;
	}
}

void GraphDisplay::removePlugin(std::size_t id)
{
	std::size_t i=0;
	for(; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == id)
			break;
	}

	if(i < m_info.size())
	{
		if(m_pGraph -> RemovePlugin(m_info[i].m_name.c_str()))
		{
			std::vector<ElementInfo> info = m_pGraph -> GetInfo();
			update(info);
		}
		else
			QMessageBox::warning(this, "Element removing problem", "Element `" + QString(m_info[i].m_name.c_str()) + "` remowing was FAILED");

	}
}


void GraphDisplay::showElementProperties(std::size_t id)
{
	std::size_t i=0;
	for(; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == id)
			break;
	}

	if(i < m_info.size())
	{
		ElementProperties *pprops = new ElementProperties(m_pGraph, m_info[i].m_name.c_str());
		pprops -> setAttribute(Qt::WA_QuitOnClose, false);
		pprops -> show();
	}
}

void GraphDisplay::showPadProperties(std::size_t elementId, std::size_t padId)
{
	std::size_t i=0;
	for(; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == elementId)
			break;
	}

	if(i < m_info.size())
	{
		std::size_t j=0;
		for(; j<m_info[i].m_pads.size(); j++)
			if(m_info[i].m_pads[j].m_id == padId)
				break;

		if(j < m_info[i].m_pads.size())
		{
			PadProperties *pprops = new PadProperties(m_pGraph, m_info[i].m_name.c_str(), m_info[i].m_pads[j].m_name.c_str());
			pprops -> setAttribute(Qt::WA_QuitOnClose, false);
			pprops -> show();
		}
	}
}

void GraphDisplay::disconnect(size_t elementId, size_t padId)
{
	std::string src, dst, srcPad, dstPad;

	for(std::size_t i=0; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == elementId)
		{
			for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
			{
				if(m_info[i].m_pads[j].m_id == padId)
				{
					if(m_info[i].m_pads[j].m_type == PadInfo::Out)
					{
						src = m_info[i].m_name;
						srcPad = m_info[i].m_pads[j].m_name.c_str();
						elementId = m_info[i].m_connections[j].m_elementId;
						padId = m_info[i].m_connections[j].m_padId;
					}
					else
					{
						dst = m_info[i].m_name;
						dstPad = m_info[i].m_pads[j].m_name.c_str();
						elementId = m_info[i].m_connections[j].m_elementId;
						padId = m_info[i].m_connections[j].m_padId;
					}
					break;
				}
			}

			break;
		}
	}

	for(std::size_t i=0; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == elementId)
		{
			for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
			{
				if(m_info[i].m_pads[j].m_id == padId)
				{
					if(m_info[i].m_pads[j].m_type == PadInfo::Out)
					{
						src = m_info[i].m_name;
						srcPad = m_info[i].m_pads[j].m_name.c_str();
					}
					else
					{
						dst = m_info[i].m_name;
						dstPad = m_info[i].m_pads[j].m_name.c_str();
					}
					break;
				}
			}
			break;
		}
	}

	qDebug() << "Disconnect " << src.c_str() << ":" << srcPad.c_str()
				<< " <-> " << dst.c_str() << ":" << dstPad.c_str();

	if(src.empty() || dst.empty() || srcPad.empty() || dstPad.empty())
		return;


	m_pGraph -> Disconnect(src.c_str(), srcPad.c_str(), dst.c_str(), dstPad.c_str());

	m_info = m_pGraph -> GetInfo();
	updateDisplayInfoIds();
	repaint();
}


void GraphDisplay::requestPad(std::size_t elementId)
{
	QStringList labels;
	labels.push_back("Template name");
	labels.push_back("Caps");
	labels.push_back("Direction");

	QTableWidget *ptwgt = new QTableWidget();
	ptwgt -> setColumnCount(3);
	ptwgt -> setHorizontalHeaderLabels(labels);
	ptwgt -> setSelectionBehavior(QAbstractItemView::SelectRows);
	ptwgt -> setEditTriggers(QAbstractItemView::NoEditTriggers);

	GstElement *element = NULL;
	for(std::size_t i=0; i<m_info.size(); i++)
	{
		if(m_info[i].m_id == elementId)
		{
			element = gst_bin_get_by_name(GST_BIN(m_pGraph -> m_pGraph), m_info[i].m_name.c_str());
			break;
		}
	}

	if(!element)
	{
		QMessageBox::warning(this, "Request pad failed", "Request pad list obtaining was failed");
		return;
	}

	GstElementClass *klass;
	klass = GST_ELEMENT_GET_CLASS(element);

	GList *lst = gst_element_class_get_pad_template_list(klass);

	std::size_t k = 0;
	while (lst != NULL) 
	{
		GstPadTemplate *templ;
		templ = (GstPadTemplate *) lst -> data;

		if(GST_PAD_TEMPLATE_PRESENCE(templ) == GST_PAD_REQUEST)
		{
			ptwgt -> setRowCount(k + 1);
			ptwgt -> setItem(k, 0, new QTableWidgetItem(GST_PAD_TEMPLATE_NAME_TEMPLATE(templ)));

			GstCaps *caps = gst_pad_template_get_caps(templ);
			gchar *capsStr = gst_caps_to_string(caps);
			ptwgt -> setItem(k, 1, new QTableWidgetItem(capsStr));
			g_free(capsStr);
			gst_caps_unref(caps);

			const char *directionSrc = "SRC";
			const char *directionSink = "SINK";
			const char *directionUnknown = "UNKNOWN";

			QString direction;
			switch(GST_PAD_TEMPLATE_DIRECTION(templ))
			{
				case GST_PAD_UNKNOWN:
					direction = directionUnknown;
					break;

				case GST_PAD_SRC:
					direction = directionSrc;
					break;

				case GST_PAD_SINK:
					direction = directionSink;
					break;
			};

			ptwgt -> setItem(k, 2, new QTableWidgetItem(direction));
			k++;
		}

		lst = g_list_next(lst);
	}

	qulonglong v((qulonglong)element);
	ptwgt -> setProperty("element", v);

	connect(ptwgt, SIGNAL(cellActivated(int, int)), SLOT(addRequestPad(int, int)));

	ptwgt -> setAttribute(Qt::WA_QuitOnClose, false);
	ptwgt -> show();
}

void GraphDisplay::addRequestPad(int row, int collumn)
{
	QTableWidget *ptwgt = dynamic_cast<QTableWidget *> (QObject::sender());

	qulonglong v = ptwgt -> property("element").toULongLong();
	GstElement *element = (GstElement *) v;
	GstElementClass *klass = GST_ELEMENT_GET_CLASS(element);

	GstPadTemplate *templ = gst_element_class_get_pad_template(klass, ptwgt -> itemAt(row, 0) -> text().toStdString().c_str());

	gst_element_request_pad(element, templ, NULL, NULL);

	gst_object_unref(element);
	ptwgt -> close();

	std::vector<ElementInfo> info = m_pGraph -> GetInfo();
	update(info);

}

void GraphDisplay::getIdByPosition(const QPoint &pos, std::size_t &elementId, std::size_t &padId)
{
	std::size_t i=0;
	elementId = padId = -1;
	for(; i<m_displayInfo.size(); i++)
	{
		if(elementId != ((size_t)-1))
			break;

		QRect rect(
			m_displayInfo[i].m_rect.x() - 8,
			m_displayInfo[i].m_rect.y() - 6,
			m_displayInfo[i].m_rect.width() + 16,
			m_displayInfo[i].m_rect.height() + 12
			);
		if(rect.contains(pos))
		{
			std::size_t j=0;
			for(; j<m_info[i].m_pads.size(); j++)
			{
				QPoint point = getPadPosition(m_displayInfo[i].m_id, m_info[i].m_pads[j].m_id);

				int xPos, yPos;
				xPos = point.x();
				yPos = point.y();

				xPos -= PAD_SIZE_ACTION / 2;
				yPos -= PAD_SIZE_ACTION / 2;

				QRect rect(xPos, yPos, PAD_SIZE_ACTION, PAD_SIZE_ACTION);
				if(rect.contains(pos))
				{
					padId = m_info[i].m_pads[j].m_id;
					elementId = m_displayInfo[i].m_id;
					break;
				}
			}

			if(j == m_info[i].m_pads.size())
			{
				if(m_displayInfo[i].m_rect.contains(pos))
					elementId = m_displayInfo[i].m_id;
			}
		}
	}
}



QPoint GraphDisplay::getPadPosition(std::size_t elementId, std::size_t padId)
{
	QPoint res;
	if(elementId == ((size_t)-1) || padId == ((size_t)-1))
		return res;

	for(std::size_t i=0; i<m_displayInfo.size(); i++)
	{
		if(m_displayInfo[i].m_id == elementId)
		{
			int numInPads, numOutPads;
			numInPads = numOutPads = 0;
			for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
			{
				if(m_info[i].m_pads[j].m_type == PadInfo::Out)
					numOutPads++;
				else if(m_info[i].m_pads[j].m_type == PadInfo::In)
					numInPads++;
			}

			int inDelta, outDelta, inPos, outPos;

			inDelta = m_displayInfo[i].m_rect.height() / (numInPads + 1);
			outDelta = m_displayInfo[i].m_rect.height() / (numOutPads + 1);

			inPos = inDelta;
			outPos = outDelta;
			for(std::size_t j=0; j<m_info[i].m_pads.size(); j++)
			{
				int xPos, yPos;
				yPos = m_displayInfo[i].m_rect.topRight().y();

				if(m_info[i].m_pads[j].m_type == PadInfo::Out)
				{
					xPos = m_displayInfo[i].m_rect.topRight().x();
					yPos += outPos;
					outPos += outDelta;
				}
				else if(m_info[i].m_pads[j].m_type == PadInfo::In)
				{
					xPos = m_displayInfo[i].m_rect.topLeft().x();
					yPos += inPos;
					inPos += inDelta;
				}

				if(m_info[i].m_pads[j].m_id == padId)
				{
					res = QPoint(xPos, yPos);
					break;
				}
			}

			break;
		}
	}

	return res;
}
