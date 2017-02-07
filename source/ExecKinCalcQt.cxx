#include "ExecKinCalcQt.h"
#include <QApplication>

int main ( int argc, char *argv[] )
{
    QApplication theApp ( argc, argv );
    QZMainFrame mainWindow;
    mainWindow.show();

    CalcMonitor* calcM = CalcMonitor::CreateCalcMonitor ();
    calcM->SetMainFrame ( &mainWindow );

    QObject::connect ( &mainWindow, SIGNAL ( RequestUpdateReac ( string, string, string, string, string, string, string, string, string, string, bool, bool ) ),
                       calcM, SLOT ( UpdateReacInfo ( string, string, string, string, string, string, string, string, string, string, bool, bool ) ) );

    QObject::connect ( &mainWindow, SIGNAL ( RequestConvertSingle ( int,QLineEdit* ) ), calcM, SLOT ( ConvertSingleValue ( int,QLineEdit* ) ) );

    QObject::connect ( &mainWindow, SIGNAL ( RequestGetKinematics ( string, string, string, string, string, string, string, string, string, string, bool, bool ) ),

                       calcM, SLOT ( GetReacKinematics ( string, string, string, string, string, string, string, string, string, string, bool, bool ) ) );

    QObject::connect ( &mainWindow, SIGNAL ( RequestPlotGraph ( vector<int>,string,string,string,string,string ) ),
                       calcM, SLOT ( PlotKinematicsGraph ( vector<int>,string,string,string,string,string ) ) );

    QObject::connect ( &mainWindow, SIGNAL ( RequestWriteTable ( int,string,string,string ) ), calcM, SLOT ( WriteOutputTable ( int,string,string,string ) ) );

    QObject::connect ( calcM, SIGNAL ( RequestResetInputFields ( string ) ), &mainWindow, SLOT ( ResetAllFields ( string ) ) );

    QObject::connect ( calcM, SIGNAL ( RequestFillReacInfoFields ( string,string,string,string,string,string,double,bool ) ),
                       &mainWindow, SLOT ( FillReacInfoFields ( string,string,string,string,string,string,double,bool ) ) );

    QObject::connect ( calcM, SIGNAL ( RequestUpdateReacList ( vector<string> ) ), &mainWindow, SLOT ( UpdateReacListBox ( vector<string> ) ) );

    QObject::connect ( calcM, SIGNAL ( RequestSetSingleConvertValues ( map<string,string> ) ), &mainWindow, SLOT ( SetSingleConvertValues ( map<string,string> ) ) );

    QObject::connect ( calcM, SIGNAL ( RedrawPlotWidget ( string ) ), &mainWindow, SLOT ( RedrawPlotWidget ( string ) ) );

    QObject::connect ( calcM, SIGNAL ( AddGraph ( string,vector<double>,vector<double>,double,double,string,string ) ),
                       &mainWindow, SLOT ( AddGraph ( string,vector<double>,vector<double>,double,double,string,string ) ) );

    QObject::connect ( &mainWindow, SIGNAL ( KillApp() ), &theApp, SLOT ( quit() ) );

    theApp.exec();

    mainWindow.disconnect();
    calcM->disconnect();
    delete calcM;

    return 0;
}
