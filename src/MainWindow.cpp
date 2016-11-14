#include "MainWindow.h"

#include "PluginsList.h"

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QFileDialog>
#include <QMessageBox>
#include <QScopedArrayPointer>
#include <QDebug>
#include <QScrollArea>
#include <QLabel>
#include <QScrollArea>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QColor>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QInputDialog>
#include <QSettings>

#include <QDebug>

#include "GraphDisplay.h"
#include "PipelineIE.h"
#include "CustomSettings.h"
#include "SeekSlider.h"

#include "version_info.h"

#include <gst/gst.h>

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags):
	QMainWindow(parent, flags)
	,m_pGraph(new GraphManager)
{
	QToolBar *ptb = addToolBar("Menu");

	QAction *pactAdd = ptb -> addAction("Add...");
	pactAdd -> setShortcut(QKeySequence("Ctrl+F"));
	connect(pactAdd, SIGNAL(triggered()), SLOT(AddPlugin()));


	QAction *pactOpenFile = ptb -> addAction("Open File...");
	connect(pactOpenFile, SIGNAL(triggered()), SLOT(OpenFile()));

	ptb -> addSeparator();

	QPixmap pxPlay(24, 24);
	pxPlay.fill(QColor(0, 0, 0, 0));
	QPainter pntrPlay(&pxPlay);
	pntrPlay.setPen(Qt::darkGreen);
	pntrPlay.setBrush(QBrush(Qt::darkGreen));

	QPolygon polygon(3);
	polygon.setPoint(0, 4, 4);
	polygon.setPoint(1, 4, 20);
	polygon.setPoint(2, 20, 12);

	pntrPlay.drawPolygon(polygon, Qt::WindingFill);
	
	QAction *pactPlay = ptb -> addAction(QIcon(pxPlay), "Play");
	connect(pactPlay, SIGNAL(triggered()), SLOT(Play()));

	QPixmap pxPause(24, 24);
	pxPause.fill(QColor(0, 0, 0, 0));
	QPainter pntrPause(&pxPause);
	pntrPause.setPen(Qt::darkGray);
	pntrPause.setBrush(QBrush(Qt::darkGray));

	pntrPause.drawRect(8, 4, 3, 16);
	pntrPause.drawRect(13, 4, 3, 16);

	QAction *pactPause = ptb -> addAction(QIcon(pxPause), "Pause");
	connect(pactPause, SIGNAL(triggered()), SLOT(Pause()));

	QPixmap pxStop(24, 24);
	pxStop.fill(QColor(0, 0, 0, 0));
	QPainter pntrStop(&pxStop);
	pntrStop.setPen(Qt::darkRed);
	pntrStop.setBrush(QBrush(Qt::darkRed));

	pntrStop.drawRect(6, 6, 12, 12);

	QAction *pactStop = ptb -> addAction(QIcon(pxStop), "Stop");
	connect(pactStop, SIGNAL(triggered()), SLOT(Stop()));

	QPixmap pxFulsh(24, 24);
	pxFulsh.fill(QColor(0, 0, 0, 0));
	QPainter pntrFlush(&pxFulsh);
	pntrFlush.setPen(Qt::darkGreen);
	pntrFlush.setBrush(QBrush(Qt::darkGreen));

	pntrFlush.drawRect(3, 4, 3, 16);

	polygon = QPolygon(3);
	polygon.setPoint(0, 9, 4);
	polygon.setPoint(1, 9, 20);
	polygon.setPoint(2, 21, 12);

	pntrFlush.drawPolygon(polygon, Qt::WindingFill);

	QAction *pactFlush = ptb -> addAction(QIcon(pxFulsh), "Flush");
	connect(pactFlush, SIGNAL(triggered()), SLOT(Flush()));


	ptb -> addSeparator();

	m_pslider = new SeekSlider();
	m_pslider -> setOrientation(Qt::Horizontal);
	m_pslider -> setRange(0, 10000);
	m_pslider -> setTracking(false);

	connect(m_pslider, SIGNAL(valueChanged(int)), SLOT(Seek(int)));
	ptb -> addWidget(m_pslider);

	QMenu *pmenu = menuBar() -> addMenu("&File");

	QAction *pactOpen = pmenu -> addAction ("Open...", this, SLOT(Open()), QKeySequence::Open);
	addAction (pactOpen);

	QAction *pactSave = pmenu -> addAction ("Save", this, SLOT(Save()), QKeySequence::Save);
	addAction (pactSave);

	QAction *pactSaveAs = pmenu -> addAction ("Save As...", this, SLOT(SaveAs()), QKeySequence::SaveAs);
	addAction (pactSaveAs);

	pmenu -> addSeparator();
	pmenu -> addAction("Exit", this, SLOT(close()));


	pmenu = menuBar() -> addMenu("&Graph");

	pmenu -> addAction(pactAdd);
	pmenu -> addAction(pactOpenFile);
	pmenu -> addAction ("Open Uri...", this, SLOT(OpenUri()));
	pmenu -> addSeparator();
	pmenu -> addAction(pactPlay);
	pmenu -> addAction(pactPause);
	pmenu -> addAction(pactStop);
	pmenu -> addAction(pactFlush);


	pmenu = menuBar() -> addMenu("&Help");

	pmenu -> addAction ("About pipeviz...", this, SLOT(About()));


	m_pGraphDisplay = new GraphDisplay;

	QScrollArea *pscroll = new QScrollArea;
	pscroll -> setWidget(m_pGraphDisplay);
	pscroll -> setWidgetResizable(false);
	m_pGraphDisplay -> resize(10000, 10000);
	m_pGraphDisplay -> m_pGraph = m_pGraph;
	setCentralWidget(pscroll);
	m_pstatusBar = new QStatusBar;
	setStatusBar(m_pstatusBar);

	restoreGeometry(CustomSettings::mainWindowGeometry());
	startTimer(100);
}


MainWindow::~MainWindow()
{
	CustomSettings::saveMainWindowGeometry(saveGeometry());
}


void MainWindow::AddPlugin()
{
	PluginsList lst(this);
	lst.m_pGraph = m_pGraph.data();

	lst.exec();
	std::vector<ElementInfo> info = m_pGraph -> GetInfo();
	m_pGraphDisplay -> update(info);
}



void MainWindow::OpenFile()
{
	QString dir = CustomSettings::lastIODirectory();

	QString path = QFileDialog::getOpenFileName(this, "Open File...", dir );
	if(!path.isEmpty())
	{
		gchar *uri = gst_filename_to_uri(path.toStdString().c_str(), NULL);
		if(uri)
		{
			qDebug() << "Open Source file: " << path;

			m_pGraph -> OpenUri(uri, NULL);
			g_free(uri);

			std::vector<ElementInfo> info = m_pGraph -> GetInfo();
			m_pGraphDisplay -> update(info);

			QString dir = QFileInfo(path).absoluteDir().absolutePath();
			CustomSettings::saveLastIODirectory(dir);
		}
	}
}


void MainWindow::OpenUri()
{
	QString uri = QInputDialog::getText(this, "Open Uri...", "Uri:");

	if(!uri.isEmpty())
	{
		qDebug() << "Open uri: " << uri;
		m_pGraph -> OpenUri(uri.toStdString().c_str(), NULL);

		std::vector<ElementInfo> info = m_pGraph -> GetInfo();
		m_pGraphDisplay -> update(info);
	}

}


void MainWindow::Play()
{
	qDebug() << "Play";
	m_pGraph -> Play();
}


void MainWindow::Pause()
{
	qDebug() << "Pause";
	m_pGraph -> Pause();
}


void MainWindow::Stop()
{
	qDebug() << "Stop";
	m_pGraph -> Stop();
}


void MainWindow::Flush()
{
	qDebug() << "Flush";

	if(m_pGraph -> m_pGraph)
	{
		gst_element_send_event(GST_ELEMENT(m_pGraph -> m_pGraph), gst_event_new_flush_start());
#if GST_VERSION_MAJOR >= 1
		gst_element_send_event(GST_ELEMENT(m_pGraph -> m_pGraph), gst_event_new_flush_stop(true));
#else
		gst_element_send_event(GST_ELEMENT(m_pGraph -> m_pGraph), gst_event_new_flush_stop());
#endif
	}
}


void MainWindow::Seek(int val)
{
	if(m_pGraph -> SetPosition((double)(val) / m_pslider -> maximum()))
		qDebug() << "Seek to" << val; 
	else
		qDebug() << "Seek to" << val << "was FAILED"; 
}


void MainWindow::timerEvent(QTimerEvent *)
{
	GstState state;
	GstStateChangeReturn res = gst_element_get_state (m_pGraph -> m_pGraph, &state, NULL, GST_MSECOND);

	if(res == GST_STATE_CHANGE_SUCCESS)
	{
		QString str;
		switch(state)
		{
			case GST_STATE_VOID_PENDING:
				str = "Pending";
				break;
			case GST_STATE_NULL:
				str = "Null";
				break;
			case GST_STATE_READY:
				str = "Ready";
				break;
			case GST_STATE_PAUSED:
				str = "Paused";
				break;
			case GST_STATE_PLAYING:
				str = "Playing";
				break;
		};

		m_pstatusBar -> showMessage(str);
	}
	else
	{
		m_pstatusBar -> showMessage(QString(gst_element_state_change_return_get_name(res)));
	}

	double pos = m_pGraph -> GetPosition();

	if(m_pslider -> value() != (int)(m_pslider -> maximum() * pos))
		m_pslider -> setSliderPosition(m_pslider -> maximum() * pos);


	m_pGraphDisplay -> update(m_pGraph -> GetInfo());
}



void MainWindow::Save()
{
	if(m_fileName.isEmpty())
		SaveAs();
	else
		PipelineIE::Export(m_pGraph, m_fileName);
}


void MainWindow::SaveAs()
{
	QString dir = CustomSettings::lastIODirectory();

	QString path = QFileDialog::getSaveFileName(this, "Save As...", dir);

	if(!path.isEmpty())
	{
		m_fileName = path;
		Save();

		QString dir = QFileInfo(path).absoluteDir().absolutePath();
		CustomSettings::saveLastIODirectory(dir);
	}
}


void MainWindow::Open()
{
	QString dir = CustomSettings::lastIODirectory();

	QString path = QFileDialog::getOpenFileName(this, "Open...", dir);

	if(!path.isEmpty())
	{
		if(PipelineIE::Import(m_pGraph, path))
			m_fileName = path;

		QString dir = QFileInfo(path).absoluteDir().absolutePath();
		CustomSettings::saveLastIODirectory(dir);
	}
}


void MainWindow::About()
{
	QString message;
	message = "<center><b>pipeviz</b></center><br>";
	message = "<center>virinext@gmail.com</center><br>";
	message += QString("<center>Version: ") + VERSION_STR + "</center><br>";
	message += "<center>GUI Based on Qt</center>";
	message += "<center>using ";
	message += gst_version_string();
	message += "</center>";

	QMessageBox::about(this, "About", message);
}
