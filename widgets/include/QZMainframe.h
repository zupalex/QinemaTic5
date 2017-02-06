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


class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QPushButton;
class QTimer;
class TCanvas;

class QRootCanvas : public QWidget
{
    Q_OBJECT

public:
    QRootCanvas ( QWidget *parent = 0 );
    virtual ~QRootCanvas() {}
    TCanvas* GetTCanvas()
    {
        return fCanvas;
    }

protected:
    TCanvas        *fCanvas;

    virtual void mouseMoveEvent ( QMouseEvent *e );
    virtual void mousePressEvent ( QMouseEvent *e );
    virtual void mouseReleaseEvent ( QMouseEvent *e );
    virtual void paintEvent ( QPaintEvent *e );
    virtual void resizeEvent ( QResizeEvent *e );
};

class QMainCanvas : public QWidget
{
    Q_OBJECT

protected:
    QRootCanvas    *canvas;
    QTimer         *fRootTimer;

public:
    QMainCanvas ( QWidget *parent = 0 );
    virtual ~QMainCanvas() {}
    virtual void changeEvent ( QEvent * e );

public slots:
    void handle_root_events();

    QRootCanvas* GetQRootCanvas()
    {
        return canvas;
    }

    QTimer* GetRootTimer()
    {
        return fRootTimer;
    }

signals:
    void RootProcessingDone();
};


namespace Ui
{
class QZMainFrame;
}

class QZMainFrame : public QMainWindow
{
    Q_OBJECT

private:
    Ui::QZMainFrame *ui;

    void RegisterChildrenDefaultValues();
    bool IsValueOK ( QLineEdit* lineEdit );

    bool isRootProcessDone;

public:
    explicit QZMainFrame ( QWidget *parent = 0 );
    ~QZMainFrame();

    QMainCanvas* displayWidget;
    void CreateDisplayWidget ( QString title, int x, int y , int w, int h );

    QLineEdit* activeSingleConvert;

    map<QLineEdit*, QString> defaultChildrenValues;

    bool invertEjecRecoil;
    bool invertLabCMEn;

protected:
    virtual void closeEvent ( QCloseEvent * event );

public slots:
    void InvertRecoilAndEjectile ( int state );
    void InvertLabAndCMEk ( int state );

    void OnClickUpdateReac();
    void OnClickGetKinematics();
    void OnClickWriteTable();
    void OnClickConvertSingle();
    void OnClickPlotGraph();

    void ResetAllFields ( string mode );

    void FillReacInfoFields ( string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamCMEkStr, double qValGsGs, bool aboveThr );

    void UpdateReacListBox ( vector<string> reacList );

    void SetSingleConvertValues ( map<string, string> resMap );

    void ResetDefaultValue();

    void RootProccessingDone();

signals:
    void KillApp();

    void RequestUpdateReac ( string beamStr, string targetStr, string ejecStr, string recoilStr,
                             string beamEkStr, string beamExStr, string targetExStr, string ejecExStr, string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn );
    void RequestGetKinematics ( string beamStr, string targetStr, string ejecStr, string recoilStr,
                                string beamEkStr, string beamExStr, string targetExStr, string ejecExStr, string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn );
    void RequestWriteTable ( int reacID, string xMinStr, string xMaxStr, string stepWidthStr );
    void RequestConvertSingle ( int reacID, QLineEdit* lineEdit );
    void RequestPlotGraph ( TCanvas* canvas, vector<int> selectedEntries, string xAxisID, string yAxisID, string xMinStr, string xMaxStr, string stepWidthStr );
};

class ForwardDoubleClick : public QObject
{
    Q_OBJECT

private:

protected:
    bool eventFilter ( QObject* obj, QEvent* event );

public:
    ForwardDoubleClick() {}
    ~ForwardDoubleClick() {}
};

void HandleSingleConvertEnabled ( QWidget* obj, QLineEdit* toEnable );
QWidget* GetFirstParent ( QWidget* widget );

#endif // QZMAINFRAME_H
