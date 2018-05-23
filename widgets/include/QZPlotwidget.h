#ifndef QZPLOTWIDGET_H
#define QZPLOTWIDGET_H

#include<map>

#include <QWidget>
#include <QtGui/QResizeEvent>
#include <QtGui/QDragMoveEvent>
#include "qcustomplot.h"

using namespace std;

class ForwardMouseTracking: public QObject
{
	Q_OBJECT

	private:

	protected:
		bool eventFilter(QObject* obj, QEvent* event);

	public:
		ForwardMouseTracking()
		{
		}
		~ForwardMouseTracking()
		{
		}
};

namespace Ui
{
	class QZPlotwidget;
}

class QZPlotwidget: public QWidget
{
	Q_OBJECT

	private:
		Ui::QZPlotwidget *ui;

		map<QString, QSize> standardSizes;

		ForwardMouseTracking* fMt;

	protected:
		void MakeColorPalette();
		virtual void resizeEvent(QResizeEvent* event);

	public:
		explicit QZPlotwidget(QWidget *parent = 0);
		~QZPlotwidget();

		QCustomPlot* GetQCustomPlot()
		{
			return findChild<QCustomPlot*>("customPlot");
		}

		int graphCount = 0;

		void RegisterStandardSizes();
		void RegisterInitialSliderHandlesPos();
		void RegisterMaxRanges(bool forceUpdate);

		QTime* refreshTime;

		QColor colorPalette[10];

		map<QString, QPoint> initialSliderHandlesPos;
		double xSliderRange;
		double ySliderRange;

		double xRangeBeforeDrag;
		double yRangeBeforeDrag;

		double xMinBeforeDrag;
		double xMaxBeforeDrag;

		double yMaxBeforeDrag;
		double yMinBeforeDrag;

		double origXMin;
		double origXMax;
		double origYMin;
		double origYMax;

		double xAxisMin;
		double xAxisMax;

		double yAxisMax;
		double yAxisMin;

	signals:

	public slots:
		void AddGraph(string title, vector<double> x_, vector<double> y_, double xMin_, double xMax_, string xAxisLabel, string yAxisLabel, bool countGraph = true);

		void ToogleLogScale(int isOn);
		void ToogleAllowDrag(int isOn);
		void ToogleAllowDragX(int isOn);
		void ToogleAllowDragY(int isOn);

		void RecenterAxis();

		void RescaleXAxis();
		void RescaleYAxis();

		void UpdateXAxisRangeBoxes(const QCPRange& newRange);
		void UpdateYAxisRangeBoxes(const QCPRange& newRange);

		void ToogleGraphToolsGB();

		void ChangeLineSize(int newSize);
};

QWidget* GetFirstParent(QWidget* widget);

#endif // QZPLOTWIDGET_H

