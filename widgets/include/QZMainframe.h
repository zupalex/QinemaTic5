#ifndef QZMAINFRAME_H
#define QZMAINFRAME_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

#include <QEvent>
#include <QtWidgets/QLineEdit>
#include <QMainWindow>

#include "qcustomplot.h"
#include "QZPlotwidget.h"

namespace Ui
{
	class QZMainFrame;
}

class QZMainFrame: public QMainWindow
{
	Q_OBJECT

	private:
		Ui::QZMainFrame *ui;

		void RegisterChildrenDefaultValues();
		bool IsValueOK(QLineEdit* lineEdit);

		bool isRootProcessDone;

		map<string, string> lastProcessedSingleConvert;

	public:
		explicit QZMainFrame(QWidget *parent = 0);
		~QZMainFrame();

		QZPlotwidget* displayWidget;
		void CreateDisplayWidget();

		QLineEdit* activeSingleConvert;

		map<QLineEdit*, QString> defaultChildrenValues;

		bool invertEjecRecoil;
		bool invertLabCMEn;

	protected:
		virtual void closeEvent(QCloseEvent * event);

	public slots:
		void InvertRecoilAndEjectile(int state);
		void InvertLabAndCMEk(int state);

		void OnClickUpdateReac();
		void OnClickGetKinematics();
		void OnClickWriteTable();
		void OnClickConvertSingle();
		void OnClickPlotGraph();

		void ResetAllFields(string mode);

		void FillReacInfoFields(string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamCMEkStr, double qValGsGs, bool aboveThr);

		void UpdateReacListBox(vector<string> reacList);

		void SetSingleConvertValues(map<string, string> resMap);
		void EnableSecondSolution(bool enable);
		void SwitchToSecondSolution();

		void ResetDefaultValue();

		void RedrawPlotWidget(string plotTitle);

		void AddGraph(string title, vector<double> x_, vector<double> y_, double xMin_, double xMax_, string xAxisLabel, string yAxisLabel, bool countGraph = true);

	signals:
		void KillApp();

		void RequestUpdateReac(string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamExStr, string targetExStr, string ejecExStr,
				string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn);
		void RequestGetKinematics(string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamExStr, string targetExStr, string ejecExStr,
				string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn);
		void RequestWriteTable(int reacID, string xMinStr, string xMaxStr, string stepWidthStr);
		void RequestConvertSingle(int reacID, QLineEdit* lineEdit);
		void RequestPlotGraph(vector<int> selectedEntries, string xAxisID, string yAxisID, string xMinStr, string xMaxStr, string stepWidthStr);

		void ForwardAddGraph(string title, vector<double> x_, vector<double> y_, double xMin_, double xMax_, string xAxisLabel, string yAxisLabel, bool countGraph = true);
};

class ForwardDoubleClick: public QObject
{
	Q_OBJECT

	private:

	protected:
		bool eventFilter(QObject* obj, QEvent* event);

	public:
		ForwardDoubleClick()
		{
		}
		~ForwardDoubleClick()
		{
		}
};

void HandleSingleConvertEnabled(QWidget* obj, QLineEdit* toEnable);

#endif // QZMAINFRAME_H

