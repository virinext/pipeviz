#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>
#include <QVBoxLayout>
#include <QSharedPointer>
#include <QStatusBar>
#include <QAction>
#include <QSlider>

#include <gst/gstbuffer.h>
#include <gst/gstevent.h>
#include <gst/gstcaps.h>

#include "GraphManager.h"

class GraphDisplay;
class PluginsList;

class MainWindow: public QMainWindow
{
	Q_OBJECT
	public:
		MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
		~MainWindow();

	protected:
		void timerEvent(QTimerEvent *);

	private slots:
		void AddPlugin();
		void OpenMediaFile();
		void OpenMediaUri();
		void Play();
		void Pause();
		void Stop();
		void Flush();
		void Seek(int);

		void Save();
		void SaveAs();
		void Open();

		void ClearGraph();
		void About();

	private:
		QSharedPointer<GraphManager>     m_pGraph;
		GraphDisplay                    *m_pGraphDisplay;

		QStatusBar                      *m_pstatusBar;
		QSlider                         *m_pslider;

		QString                          m_fileName;
		PluginsList			*m_pluginList;
};


#endif
