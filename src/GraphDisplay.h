#ifndef GRAPH_DISPLAY_H_
#define GRAPH_DISPLAY_H_

#include <vector>

#include <QWidget>
#include <QSharedPointer>
#include <QPoint>


#include "GraphManager.h"
#include <vector>




class GraphDisplay: public QWidget
{
Q_OBJECT

public:
	GraphDisplay(QWidget *parent=0, Qt::WindowFlags f=0);
	void update(const std::vector <ElementInfo> &info);
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

	void keyPressEvent(QKeyEvent* event);


	QSharedPointer<GraphManager>         m_pGraph;

private slots:
	void addRequestPad(int row, int collumn);

private:

	enum MoveAction
	{
		None = 0,
		MoveComponent,
		MakeConnect,
		Select
	};

	struct MoveInfo
	{
		MoveInfo(): m_action(None)
		{
		}

		MoveAction        m_action;
		size_t            m_elementId;
		size_t            m_padId;
		QPoint            m_position;
		QPoint            m_startPosition;
	};


	struct ElementDisplayInfo
	{
		QRect              m_rect;
		size_t             m_id;
		std::string        m_name;
		bool               m_isSelected;
	};

	void calculatePositions();
	void updateDisplayInfoIds();
	ElementDisplayInfo calculateOnePosition(const ElementInfo &info);
	void showContextMenu(QMouseEvent *event);
	void showElementProperties(std::size_t id);
	void showPadProperties(std::size_t elementId, std::size_t padId);
	void renderPad(std::size_t elementId, std::size_t padId, bool capsAny);
	void removePlugin(std::size_t id);
	void removeSelected();
	void getIdByPosition(const QPoint &pos, std::size_t &elementId, std::size_t &padId);
	QPoint getPadPosition(std::size_t elementId, std::size_t padId);
	void disconnect(std::size_t elementId, std::size_t padId);
	void requestPad(std::size_t elementId);
	void addPlugin(const QString& name);

	ElementInfo* getElement(std::size_t elementId);
	PadInfo* getPad(std::size_t elementId, std::size_t padId);

	std::vector <ElementInfo>            m_info;
	std::vector <ElementDisplayInfo>     m_displayInfo;

	MoveInfo                             m_moveInfo;
};


#endif
