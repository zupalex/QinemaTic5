#include "QZPlotwidget.h"
#include "ui_QZPlotwidget.h"
#include <algorithm>
#include <iostream>

QZPlotwidget::QZPlotwidget ( QWidget *parent ) :
    QWidget ( parent ),
    ui ( new Ui::QZPlotwidget )
{
    ui->setupUi ( this );

    MakeColorPalette();

    RegisterStandardSizes();

    QObject::connect ( findChild<QCheckBox*> ( "logScaleCB" ), SIGNAL ( stateChanged ( int ) ), this, SLOT ( ToogleLogScale ( int ) ) );

    QObject::connect ( findChild<QCheckBox*> ( "allowDragCB" ), SIGNAL ( stateChanged ( int ) ), this, SLOT ( ToogleAllowDrag ( int ) ) );
    QObject::connect ( findChild<QCheckBox*> ( "dragXCB" ), SIGNAL ( stateChanged ( int ) ), this, SLOT ( ToogleAllowDragX ( int ) ) );
    QObject::connect ( findChild<QCheckBox*> ( "dragYCB" ), SIGNAL ( stateChanged ( int ) ), this, SLOT ( ToogleAllowDragY ( int ) ) );

    QObject::connect ( findChild<QPushButton*> ( "recenterPB" ), SIGNAL ( released() ), this, SLOT ( RecenterAxis() ) );

    QObject::connect ( findChild<QLineEdit*> ( "xMinIF" ), SIGNAL ( editingFinished() ), this, SLOT ( RescaleXAxis() ) );
    QObject::connect ( findChild<QLineEdit*> ( "xMaxIF" ), SIGNAL ( editingFinished() ), this, SLOT ( RescaleXAxis() ) );
    QObject::connect ( findChild<QLineEdit*> ( "yMinIF" ), SIGNAL ( editingFinished() ), this, SLOT ( RescaleYAxis() ) );
    QObject::connect ( findChild<QLineEdit*> ( "yMaxIF" ), SIGNAL ( editingFinished() ), this, SLOT ( RescaleYAxis() ) );

    QObject::connect ( GetQCustomPlot()->xAxis, SIGNAL ( rangeChanged ( QCPRange ) ), this, SLOT ( UpdateXAxisRangeBoxes ( QCPRange ) ) );
    QObject::connect ( GetQCustomPlot()->yAxis, SIGNAL ( rangeChanged ( QCPRange ) ), this, SLOT ( UpdateYAxisRangeBoxes ( QCPRange ) ) );

    GetQCustomPlot()->setInteractions ( QCP::iSelectPlottables | QCP::iRangeDrag | QCP::iRangeZoom );

    GetQCustomPlot()->axisRect()->setRangeDrag ( 0 );
    GetQCustomPlot()->axisRect()->setRangeZoom ( 0 );

    fMt = new ForwardMouseTracking();

    findChild<QWidget*> ( "xAxisHitbox" )->installEventFilter ( fMt );
    findChild<QWidget*> ( "yAxisHitbox" )->installEventFilter ( fMt );
}

QZPlotwidget::~QZPlotwidget()
{
    delete ui;
}

void QZPlotwidget::MakeColorPalette()
{
    colorPalette[0] = QColor ( 0, 0, 255 );
    colorPalette[1] = QColor ( 255, 0, 0 );
    colorPalette[2] = QColor ( 0, 153, 0 );
    colorPalette[3] = QColor ( 0, 0, 0 );
    colorPalette[4] = QColor ( 255, 51, 153 );
    colorPalette[5] = QColor ( 0, 204, 204 );
    colorPalette[6] = QColor ( 255, 128, 0 );
    colorPalette[7] = QColor ( 255, 255, 51 );
    colorPalette[8] = QColor ( 153, 51, 255 );
    colorPalette[9] = QColor ( 255, 0, 255 );
}

void QZPlotwidget::RegisterStandardSizes()
{
    standardSizes[objectName()] = size();

    QList<QObject*> childrenList = findChildren<QObject*> ( QRegExp ( ".*" ) );

    for ( auto child = childrenList.begin(); child != childrenList.end(); child++ )
    {
        QWidget* widg = dynamic_cast<QWidget*> ( *child );

        if ( widg != nullptr )
        {
//             cout << "Found child " << widg->objectName().toUtf8().constData() << " with QWidget attributes:\n";
//             cout << "Size: " << widg->size().width() << " x " << widg->size().height() << endl;
            standardSizes[widg->objectName()] = widg->size();
        }
    }
}

void QZPlotwidget::resizeEvent ( QResizeEvent* event )
{
    QWidget::resizeEvent ( event );

    QList<QWidget*> childrenList = findChildren<QWidget*> ( QRegExp ( ".*" ) );

    float scaleWidth = ( float ) size().width() / standardSizes[objectName()].width();
    float scaleHeigth = ( float ) size().height() / standardSizes[objectName()].height();

    for ( auto child = childrenList.begin(); child != childrenList.end(); child++ )
    {
        if ( scaleWidth > 0 && scaleHeigth > 0 )
        {
            ( *child )->resize ( ( int ) ( standardSizes[ ( *child )->objectName()].width() * scaleWidth ),
                                 ( int ) ( standardSizes[ ( *child )->objectName()].height() * scaleHeigth ) );
        }
    }
}

void QZPlotwidget::ToogleLogScale ( int isOn )
{
    if ( isOn ) GetQCustomPlot()->yAxis->setScaleType ( QCPAxis::stLogarithmic );
    else GetQCustomPlot()->yAxis->setScaleType ( QCPAxis::stLinear );

    GetQCustomPlot()->replot();
}

void QZPlotwidget::ToogleAllowDrag ( int isOn )
{
    if ( isOn )
    {
        findChild<QCheckBox*> ( "dragXCB" )->setChecked ( true );
        findChild<QCheckBox*> ( "dragYCB" )->setChecked ( true );

        GetQCustomPlot()->axisRect()->setRangeDrag ( Qt::Horizontal | Qt::Vertical );
    }
    else
    {
        findChild<QCheckBox*> ( "dragXCB" )->setChecked ( false );
        findChild<QCheckBox*> ( "dragYCB" )->setChecked ( false );

        GetQCustomPlot()->axisRect()->setRangeDrag ( 0 );
    }
}

void QZPlotwidget::ToogleAllowDragX ( int isOn )
{
    if ( isOn )
    {
        if ( !findChild<QCheckBox*> ( "allowDragCB" )->isChecked() )
        {
            findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( true );
            findChild<QCheckBox*> ( "allowDragCB" )->setChecked ( true );
            findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( false );
        }

        GetQCustomPlot()->axisRect()->setRangeDrag ( GetQCustomPlot()->axisRect()->rangeDrag() | Qt::Horizontal );
    }
    else if ( !findChild<QCheckBox*> ( "dragYCB" )->isChecked() )
    {
        findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( true );
        findChild<QCheckBox*> ( "allowDragCB" )->setChecked ( false );
        findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( false );

        GetQCustomPlot()->axisRect()->setRangeDrag ( 0 );
    }
    else
    {
        GetQCustomPlot()->axisRect()->setRangeDrag ( GetQCustomPlot()->axisRect()->rangeDrag() & ~Qt::Horizontal );
    }
}

void QZPlotwidget::ToogleAllowDragY ( int isOn )
{
    if ( isOn )
    {
        if ( !findChild<QCheckBox*> ( "allowDragCB" )->isChecked() )
        {
            findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( true );
            findChild<QCheckBox*> ( "allowDragCB" )->setChecked ( true );
            findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( false );
        }

        GetQCustomPlot()->axisRect()->setRangeDrag ( GetQCustomPlot()->axisRect()->rangeDrag() | Qt::Vertical );
    }
    else if ( !findChild<QCheckBox*> ( "dragXCB" )->isChecked() )
    {
        findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( true );
        findChild<QCheckBox*> ( "allowDragCB" )->setChecked ( false );
        findChild<QCheckBox*> ( "allowDragCB" )->blockSignals ( false );

        GetQCustomPlot()->axisRect()->setRangeDrag ( 0 );
    }
    else
    {
        GetQCustomPlot()->axisRect()->setRangeDrag ( GetQCustomPlot()->axisRect()->rangeDrag() & ~Qt::Vertical );
    }
}

void QZPlotwidget::RecenterAxis()
{
//     cout << "Requested a recenter with the following values: \n";
//     cout << "X Axis: " << origXMin << " , " << origXMax << endl;
//     cout << "Y Axis: " << origYMin << " , " << origYMax << endl;

    GetQCustomPlot()->xAxis->setRange ( origXMin, origXMax );
    GetQCustomPlot()->yAxis->setRange ( origYMin, origYMax );

    GetQCustomPlot()->replot();
}

void QZPlotwidget::RescaleXAxis()
{
    xAxisMin = findChild<QLineEdit*> ( "xMinIF" )->text().toDouble();
    xAxisMax = findChild<QLineEdit*> ( "xMaxIF" )->text().toDouble();

    GetQCustomPlot()->xAxis->blockSignals ( true );
    GetQCustomPlot()->xAxis->setRange ( xAxisMin, xAxisMax );

    GetQCustomPlot()->replot();
    GetQCustomPlot()->xAxis->blockSignals ( false );
}

void QZPlotwidget::RescaleYAxis()
{
    yAxisMin = findChild<QLineEdit*> ( "yMinIF" )->text().toDouble();
    yAxisMax = findChild<QLineEdit*> ( "yMaxIF" )->text().toDouble();

    GetQCustomPlot()->yAxis->blockSignals ( true );
    GetQCustomPlot()->yAxis->setRange ( yAxisMin, yAxisMax );

    GetQCustomPlot()->replot();
    GetQCustomPlot()->yAxis->blockSignals ( false );
}

void QZPlotwidget::UpdateXAxisRangeBoxes ( const QCPRange& newRange )
{
    char* newRangeStr = new char[16];

    double newXMin = newRange.lower;
    sprintf ( newRangeStr, "%2.3f", newXMin );

//     cout << "New XMin value : " << newRangeStr << endl;

    findChild<QLineEdit*> ( "xMinIF" )->blockSignals ( true );
    findChild<QLineEdit*> ( "xMinIF" )->setText ( QString ( newRangeStr ) );
    findChild<QLineEdit*> ( "xMinIF" )->blockSignals ( false );

    double newXMax = newRange.upper;

    sprintf ( newRangeStr, "%2.3f", newXMax );
    findChild<QLineEdit*> ( "xMaxIF" )->blockSignals ( true );
    findChild<QLineEdit*> ( "xMaxIF" )->setText ( QString ( newRangeStr ) );
    findChild<QLineEdit*> ( "xMaxIF" )->blockSignals ( false );
}

void QZPlotwidget::UpdateYAxisRangeBoxes ( const QCPRange& newRange )
{
    char* newRangeStr = new char[16];

    double newXMin = newRange.lower;
    sprintf ( newRangeStr, "%2.3f", newXMin );

//     cout << "New XMin value : " << newRangeStr << endl;

    findChild<QLineEdit*> ( "yMinIF" )->blockSignals ( true );
    findChild<QLineEdit*> ( "yMinIF" )->setText ( QString ( newRangeStr ) );
    findChild<QLineEdit*> ( "yMinIF" )->blockSignals ( false );

    double newXMax = newRange.upper;

    sprintf ( newRangeStr, "%2.3f", newXMax );
    findChild<QLineEdit*> ( "yMaxIF" )->blockSignals ( true );
    findChild<QLineEdit*> ( "yMaxIF" )->setText ( QString ( newRangeStr ) );
    findChild<QLineEdit*> ( "yMaxIF" )->blockSignals ( false );
}

void QZPlotwidget::AddGraph ( string title, vector<double> x_, vector<double> y_, double xMin_, double xMax_, string xAxisLabel, string yAxisLabel )
{
    QPen pen;
    pen.setColor ( colorPalette[GetQCustomPlot()->graphCount()] );
    pen.setWidth ( 2 );

    QVector<double> xQVect = QVector<double>::fromStdVector ( x_ );
    QVector<double> yQVect = QVector<double>::fromStdVector ( y_ );
    // create graph and assign data to it:
    GetQCustomPlot()->addGraph();

    string legendLabel = title.substr ( title.find ( "for " ) + 4 );

    GetQCustomPlot()->graph()->setName ( legendLabel.c_str() );
    GetQCustomPlot()->graph ()->setData ( xQVect, yQVect );
    // give the axes some labels:
    GetQCustomPlot()->xAxis->setLabel ( QString ( xAxisLabel.c_str() ) );
    GetQCustomPlot()->yAxis->setLabel ( QString ( yAxisLabel.c_str() ) );
    // set axes ranges, so we see all data:
    GetQCustomPlot()->xAxis->setRange ( xMin_, xMax_ );

    origXMin = xMin_;
    origXMax = xMax_;

    xAxisMin = xMin_;
    xAxisMax = xMax_;

    auto yMinItr = min_element ( y_.begin(), y_.end() );
    if ( *yMinItr < yAxisMin || yAxisMin <= -666 ) yAxisMin = *yMinItr;

    auto yMaxItr = max_element ( y_.begin(), y_.end() );
    if ( *yMaxItr > yAxisMax || yAxisMax <= -666 ) yAxisMax = *yMaxItr;

    GetQCustomPlot()->yAxis->setRange ( std::max ( 0., yAxisMin - ( yAxisMax-yAxisMin ) *0.1 ), yAxisMax + ( yAxisMax-yAxisMin ) *0.1 );

    GetQCustomPlot()->graph()->setPen ( pen );

    GetQCustomPlot()->legend->setVisible ( true );
    GetQCustomPlot()->legend->setFont ( QFont ( "Helvetica",9 ) );

    char* valStr = new char[16];

    sprintf ( valStr, "%2.3f", xMin_ );
    findChild<QLineEdit*> ( "xMinIF" )->setText ( QString ( valStr ) );

    sprintf ( valStr, "%2.3f", xMax_ );
    findChild<QLineEdit*> ( "xMaxIF" )->setText ( QString ( valStr ) );

    sprintf ( valStr, "%2.3f", yAxisMin );
    findChild<QLineEdit*> ( "yMinIF" )->setText ( QString ( valStr ) );

    sprintf ( valStr, "%2.3f", yAxisMax );
    findChild<QLineEdit*> ( "yMaxIF" )->setText ( QString ( valStr ) );

    origYMin = std::max ( 0., yAxisMin - ( yAxisMax-yAxisMin ) *0.1 );
    origYMax = yAxisMax + ( yAxisMax-yAxisMin ) *0.1;

    GetQCustomPlot()->replot();
}

QWidget* GetFirstParent ( QWidget* widget )
{
    QWidget* parent = widget->parentWidget();

    if ( parent == nullptr ) return widget;

    QWidget* nextParent = parent;

    while ( nextParent != nullptr )
    {
        nextParent = parent->parentWidget();

        if ( nextParent != nullptr ) parent = nextParent;
    }

    return parent;
}

bool ForwardMouseTracking::eventFilter ( QObject* obj, QEvent* event )
{
    QWidget* widg = dynamic_cast<QWidget*> ( obj );
    if ( widg == nullptr ) return true;

    QZPlotwidget* plotWidg = dynamic_cast<QZPlotwidget*> ( GetFirstParent ( widg ) );
    if ( plotWidg == nullptr ) return true;

    if ( event->type() == QEvent::Enter )
    {
//         cout << "Forwarded a mouse enter event\n";

        if ( widg != nullptr )
        {
//             cout << "Forwarded a mouse enter event to a QWidget Object\n";

            if ( plotWidg != nullptr )
            {
                if ( widg->objectName() == QString ( "xAxisHitbox" ) )
                {
//                     cout << "Hovering on the x axis...\n";
                    plotWidg->GetQCustomPlot()->axisRect()->setRangeZoom ( Qt::Horizontal );
                }
                else if ( widg->objectName() == QString ( "yAxisHitbox" ) )
                {
                    plotWidg->GetQCustomPlot()->axisRect()->setRangeZoom ( Qt::Vertical );
                }
            }
        }

        return true;
    }
    else if ( event->type() == QEvent::Leave )
    {
//         cout << "Forwarded a mouse leave event\n";

        if ( widg != nullptr )
        {
//             cout << "Forwarded a mouse enter leave to a QWidget Object\n";

            if ( plotWidg != nullptr )
            {
//                 cout << "Found the first parent: " << plotWidg->objectName().toUtf8().constData() << endl;

                plotWidg->GetQCustomPlot()->axisRect()->setRangeZoom ( 0 );
            }
        }

        return true;
    }
    else if ( plotWidg->GetQCustomPlot()->axisRect()->rangeZoom() != 0 )
    {
//         cout << "rangeZoom is not equal to 0\n";

        if ( event->type() == QEvent::Wheel )
        {
//             cout << "Forwarded a wheel event to the QCustomWidget\n";

            QCoreApplication::sendEvent ( plotWidg->GetQCustomPlot(), event );
        }
    }
    else
    {
        // standard event processing
        return QObject::eventFilter ( obj, event );
    }
}

#include "../include/moc_QZPlotwidget.cpp"





