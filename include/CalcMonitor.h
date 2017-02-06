#ifndef CALCMONITOR_H
#define CALCMONITOR_H

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

#include <QObject>
#include <QtWidgets/QLineEdit>
#include <vector>
#include "kinematic.h"

class QZMainFrame;

class CalcMonitor : public QObject
{
    Q_OBJECT

private:
    CalcMonitor() {};

    static CalcMonitor* s_instance;
    QZMainFrame* mainFrame;

    bool errorState;

    void SetButtonNameToVar();

public:
    ~CalcMonitor() {};

    static CalcMonitor* CreateCalcMonitor ();

    static CalcMonitor* sinstance()
    {
        return s_instance;
    }

    void SetMainFrame ( QZMainFrame* mainFrame_ );
    void ResetState();

    QZMainFrame* GetMainFrame();

    map<string, string> buttonNameToVar;

    vector<string> GetReacList();

    std::map<string, RootKinCalc> kinResMap;
    RootKinCalc* lastKinCalc;

public slots:
    void UpdateReacInfo ( string beamStr, string targetStr, string ejecStr, string recoilStr,
                          string beamEkStr, string beamExStr, string targetExStr, string ejecExStr, string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn );
    void GetReacKinematics ( string beamStr, string targetStr, string ejecStr, string recoilStr,
                             string beamEkStr, string beamExStr, string targetExStr, string ejecExStr, string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn );
    void WriteOutputTable ( int reacID, string xMinStr, string xMaxStr, string stepWidthStr );
    void ConvertSingleValue ( int reacID, QLineEdit* lineEdit );
    void PlotKinematicsGraph ( TCanvas* canvas, vector<int> selectedEntries, string xAxisID, string yAxisID, string xMinStr, string xMaxStr, string stepWidthStr );

signals:
    void RequestResetInputFields ( string mode );
    void RequestFillReacInfoFields ( string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamCMEkStr, double qValGsGs, bool aboveThr );

    void RequestUpdateReacList ( vector<string> reacList );

    void RequestSetSingleConvertValues ( map<string, string> resMap );
};

#endif // CALCMONITOR_H

