#include "QZPlotwidget.h"
#include "ui_QZPlotwidget.h"
#include <algorithm>
#include <iostream>

QZPlotwidget::QZPlotwidget(QWidget *parent)
		: QWidget(parent), ui(new Ui::QZPlotwidget)
{
	ui->setupUi(this);

	MakeColorPalette();

	RegisterStandardSizes();

	QObject::connect(findChild<QCheckBox*>("logScaleCB"), SIGNAL(stateChanged ( int )), this, SLOT(ToogleLogScale ( int )));

	QObject::connect(findChild<QCheckBox*>("allowDragCB"), SIGNAL(stateChanged ( int )), this, SLOT(ToogleAllowDrag ( int )));
	QObject::connect(findChild<QCheckBox*>("dragXCB"), SIGNAL(stateChanged ( int )), this, SLOT(ToogleAllowDragX ( int )));
	QObject::connect(findChild<QCheckBox*>("dragYCB"), SIGNAL(stateChanged ( int )), this, SLOT(ToogleAllowDragY ( int )));

	QObject::connect(findChild<QPushButton*>("recenterPB"), SIGNAL(released()), this, SLOT(RecenterAxis()));

	QObject::connect(findChild<QLineEdit*>("xMinIF"), SIGNAL(editingFinished()), this, SLOT(RescaleXAxis()));
	QObject::connect(findChild<QLineEdit*>("xMaxIF"), SIGNAL(editingFinished()), this, SLOT(RescaleXAxis()));
	QObject::connect(findChild<QLineEdit*>("yMinIF"), SIGNAL(editingFinished()), this, SLOT(RescaleYAxis()));
	QObject::connect(findChild<QLineEdit*>("yMaxIF"), SIGNAL(editingFinished()), this, SLOT(RescaleYAxis()));

	QObject::connect(GetQCustomPlot()->xAxis, SIGNAL(rangeChanged ( QCPRange )), this, SLOT(UpdateXAxisRangeBoxes ( QCPRange )));
	QObject::connect(GetQCustomPlot()->yAxis, SIGNAL(rangeChanged ( QCPRange )), this, SLOT(UpdateYAxisRangeBoxes ( QCPRange )));

	GetQCustomPlot()->setInteractions(QCP::iSelectPlottables | QCP::iRangeDrag | QCP::iRangeZoom);

	GetQCustomPlot()->axisRect()->setRangeDrag(0);
	GetQCustomPlot()->axisRect()->setRangeZoom(0);

	fMt = new ForwardMouseTracking();

	findChild<QWidget*>("xAxisHitbox")->installEventFilter(fMt);
	findChild<QWidget*>("yAxisHitbox")->installEventFilter(fMt);

	QDir resourcesFolder(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
	resourcesFolder.cd(QString("../"));
	resourcesFolder.cd(QString("./widgets/resources"));

//     cout << "Resources directory path: " << QDir::toNativeSeparators ( resourcesFolder.absolutePath() ).toUtf8().constData() << endl;;

	findChild<QLabel*>("xSliderHandleLeft")->setPixmap(QPixmap(QDir::toNativeSeparators(resourcesFolder.absolutePath() + QString("/slider.png"))));
	findChild<QLabel*>("xSliderHandleRight")->setPixmap(QPixmap(QDir::toNativeSeparators(resourcesFolder.absolutePath() + QString("/slider.png"))));
	findChild<QLabel*>("ySliderHandleTop")->setPixmap(QPixmap(QDir::toNativeSeparators(resourcesFolder.absolutePath() + QString("/slider_rot.png"))));
	findChild<QLabel*>("ySliderHandleBottom")->setPixmap(QPixmap(QDir::toNativeSeparators(resourcesFolder.absolutePath() + QString("/slider_rot.png"))));

	findChild<QLabel*>("xSliderHandleLeft")->installEventFilter(fMt);
	findChild<QLabel*>("xSliderHandleRight")->installEventFilter(fMt);
	findChild<QLabel*>("ySliderHandleTop")->installEventFilter(fMt);
	findChild<QLabel*>("ySliderHandleBottom")->installEventFilter(fMt);

	RegisterInitialSliderHandlesPos();

	findChild<QGroupBox*>("graphToolsGB")->hide();

	QObject::connect(GetQCustomPlot(), SIGNAL(selectionChangedByUser()), this, SLOT(ToogleGraphToolsGB()));

	QObject::connect(findChild<QSpinBox*>("lineSizeSB"), SIGNAL(valueChanged ( int )), this, SLOT(ChangeLineSize ( int )));

	refreshTime = new QTime();
	refreshTime->start();
}

QZPlotwidget::~QZPlotwidget()
{
	delete ui;
}

void QZPlotwidget::MakeColorPalette()
{
	colorPalette[0] = QColor(0, 0, 255);
	colorPalette[1] = QColor(255, 0, 0);
	colorPalette[2] = QColor(0, 153, 0);
	colorPalette[3] = QColor(0, 0, 0);
	colorPalette[4] = QColor(255, 51, 153);
	colorPalette[5] = QColor(0, 204, 204);
	colorPalette[6] = QColor(255, 128, 0);
	colorPalette[7] = QColor(255, 255, 51);
	colorPalette[8] = QColor(153, 51, 255);
	colorPalette[9] = QColor(255, 0, 255);
}

void QZPlotwidget::RegisterStandardSizes()
{
	standardSizes[objectName()] = size();

	QList<QObject*> childrenList = findChildren<QObject*>(QRegExp(".*"));

	for (auto child = childrenList.begin(); child != childrenList.end(); child++)
	{
		QWidget* widg = dynamic_cast<QWidget*>(*child);

		if (widg != nullptr)
		{
//             cout << "Found child " << widg->objectName().toUtf8().constData() << " with QWidget attributes:\n";
//             cout << "Size: " << widg->size().width() << " x " << widg->size().height() << endl;
			standardSizes[widg->objectName()] = widg->size();
		}
	}
}

void QZPlotwidget::RegisterInitialSliderHandlesPos()
{
	initialSliderHandlesPos[QString("xSliderHandleLeft")] = QPoint(findChild<QLabel*>("xSliderHandleLeft")->geometry().x(),
			findChild<QLabel*>("xSliderHandleLeft")->geometry().y());
	initialSliderHandlesPos[QString("xSliderHandleRight")] = QPoint(findChild<QLabel*>("xSliderHandleRight")->geometry().x(),
			findChild<QLabel*>("xSliderHandleRight")->geometry().y());

	xSliderRange = findChild<QLabel*>("xSliderHandleRight")->geometry().x() - findChild<QLabel*>("xSliderHandleLeft")->geometry().x();

	initialSliderHandlesPos[QString("ySliderHandleTop")] = QPoint(findChild<QLabel*>("ySliderHandleTop")->geometry().x(), findChild<QLabel*>("ySliderHandleTop")->geometry().y());
	initialSliderHandlesPos[QString("ySliderHandleBottom")] = QPoint(findChild<QLabel*>("ySliderHandleBottom")->geometry().x(),
			findChild<QLabel*>("ySliderHandleBottom")->geometry().y());

	ySliderRange = findChild<QLabel*>("ySliderHandleBottom")->geometry().y() - findChild<QLabel*>("ySliderHandleTop")->geometry().y();

//     cout << "X Left Slider: " << findChild<QLabel*> ( "xSliderHandleLeft" )->geometry().x() << " , " << findChild<QLabel*> ( "xSliderHandleLeft" )->geometry().y() << endl;
//     cout << "X Left Right: " << findChild<QLabel*> ( "xSliderHandleRight" )->geometry().x() << " , " << findChild<QLabel*> ( "xSliderHandleRight" )->geometry().y() << endl;
//     cout << "Y Top Slider: " << findChild<QLabel*> ( "ySliderHandleTop" )->geometry().x() << " , " << findChild<QLabel*> ( "ySliderHandleTop" )->geometry().y() << endl;
//     cout << "Y Botton Slider: " << findChild<QLabel*> ( "ySliderHandleBottom" )->geometry().x() << " , " << findChild<QLabel*> ( "ySliderHandleBottom" )->geometry().y() << endl;
}

void QZPlotwidget::RegisterMaxRanges(bool forceUpdate)
{
	double currXMin = findChild<QLineEdit*>("xMinIF")->text().toDouble();
	double currXMax = findChild<QLineEdit*>("xMaxIF")->text().toDouble();

	double currYMin = findChild<QLineEdit*>("yMinIF")->text().toDouble();
	double currYMax = findChild<QLineEdit*>("yMaxIF")->text().toDouble();

	if (currXMin < xMinBeforeDrag || forceUpdate) xMinBeforeDrag = currXMin;
	if (currXMax > xMaxBeforeDrag || forceUpdate) xMaxBeforeDrag = currXMax;

	xRangeBeforeDrag = xMaxBeforeDrag - xMinBeforeDrag;

	if (currYMin < yMinBeforeDrag || forceUpdate) yMinBeforeDrag = currYMin;
	if (currYMax > yMaxBeforeDrag || forceUpdate) yMaxBeforeDrag = currYMax;

	yRangeBeforeDrag = yMaxBeforeDrag - yMinBeforeDrag;

//     cout << "New Xmin: " << xMinBeforeDrag << endl;
//     cout << "New Xmax: " << xMaxBeforeDrag << endl;
//     cout << "New Ymin: " << yMinBeforeDrag << endl;
//     cout << "New Ymax: " << yMaxBeforeDrag << endl;
}

void QZPlotwidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	QList<QWidget*> childrenList = findChildren<QWidget*>(QRegExp(".*"));

	float scaleWidth = (float) size().width() / standardSizes[objectName()].width();
	float scaleHeigth = (float) size().height() / standardSizes[objectName()].height();

	for (auto child = childrenList.begin(); child != childrenList.end(); child++)
	{
		if (scaleWidth > 0 && scaleHeigth > 0)
		{
			(*child)->resize((int) (standardSizes[(*child)->objectName()].width() * scaleWidth), (int) (standardSizes[(*child)->objectName()].height() * scaleHeigth));
		}
	}
}

void QZPlotwidget::ToogleLogScale(int isOn)
{
	if (isOn) GetQCustomPlot()->yAxis->setScaleType(QCPAxis::stLogarithmic);
	else GetQCustomPlot()->yAxis->setScaleType(QCPAxis::stLinear);

	GetQCustomPlot()->replot();
}

void QZPlotwidget::ToogleAllowDrag(int isOn)
{
	if (isOn)
	{
		findChild<QCheckBox*>("dragXCB")->setChecked(true);
		findChild<QCheckBox*>("dragYCB")->setChecked(true);

		GetQCustomPlot()->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
	}
	else
	{
		findChild<QCheckBox*>("dragXCB")->setChecked(false);
		findChild<QCheckBox*>("dragYCB")->setChecked(false);

		GetQCustomPlot()->axisRect()->setRangeDrag(0);
	}
}

void QZPlotwidget::ToogleAllowDragX(int isOn)
{
	if (isOn)
	{
		if (!findChild<QCheckBox*>("allowDragCB")->isChecked())
		{
			findChild<QCheckBox*>("allowDragCB")->blockSignals(true);
			findChild<QCheckBox*>("allowDragCB")->setChecked(true);
			findChild<QCheckBox*>("allowDragCB")->blockSignals(false);
		}

		GetQCustomPlot()->axisRect()->setRangeDrag(GetQCustomPlot()->axisRect()->rangeDrag() | Qt::Horizontal);
	}
	else if (!findChild<QCheckBox*>("dragYCB")->isChecked())
	{
		findChild<QCheckBox*>("allowDragCB")->blockSignals(true);
		findChild<QCheckBox*>("allowDragCB")->setChecked(false);
		findChild<QCheckBox*>("allowDragCB")->blockSignals(false);

		GetQCustomPlot()->axisRect()->setRangeDrag(0);
	}
	else
	{
		GetQCustomPlot()->axisRect()->setRangeDrag(GetQCustomPlot()->axisRect()->rangeDrag() & ~Qt::Horizontal);
	}
}

void QZPlotwidget::ToogleAllowDragY(int isOn)
{
	if (isOn)
	{
		if (!findChild<QCheckBox*>("allowDragCB")->isChecked())
		{
			findChild<QCheckBox*>("allowDragCB")->blockSignals(true);
			findChild<QCheckBox*>("allowDragCB")->setChecked(true);
			findChild<QCheckBox*>("allowDragCB")->blockSignals(false);
		}

		GetQCustomPlot()->axisRect()->setRangeDrag(GetQCustomPlot()->axisRect()->rangeDrag() | Qt::Vertical);
	}
	else if (!findChild<QCheckBox*>("dragXCB")->isChecked())
	{
		findChild<QCheckBox*>("allowDragCB")->blockSignals(true);
		findChild<QCheckBox*>("allowDragCB")->setChecked(false);
		findChild<QCheckBox*>("allowDragCB")->blockSignals(false);

		GetQCustomPlot()->axisRect()->setRangeDrag(0);
	}
	else
	{
		GetQCustomPlot()->axisRect()->setRangeDrag(GetQCustomPlot()->axisRect()->rangeDrag() & ~Qt::Vertical);
	}
}

void QZPlotwidget::ToogleGraphToolsGB()
{
	if (GetQCustomPlot()->selectedPlottables().count() > 0)
	{
		findChild<QGroupBox*>("graphToolsGB")->show();
		findChild<QSpinBox*>("lineSizeSB")->blockSignals(true);
		findChild<QSpinBox*>("lineSizeSB")->setValue(GetQCustomPlot()->selectedPlottables().at(0)->pen().width());
		findChild<QSpinBox*>("lineSizeSB")->blockSignals(false);
	}
	else findChild<QGroupBox*>("graphToolsGB")->hide();
}

void QZPlotwidget::ChangeLineSize(int newSize)
{
	if (GetQCustomPlot()->selectedPlottables().count() > 0)
	{
		QPen newPen = GetQCustomPlot()->selectedPlottables().at(0)->pen();
		newPen.setWidth(newSize);

		GetQCustomPlot()->selectedPlottables().at(0)->setPen(newPen);

		QCPSelectionDecorator* oldSD = GetQCustomPlot()->selectedPlottables().at(0)->selectionDecorator();
		QPen newSDPen = oldSD->pen();
		newSDPen.setWidth(newSize);
		newSDPen.setColor(newPen.color());

		oldSD->setPen(newSDPen);

		oldSD->pen().setWidth(newSize);

		GetQCustomPlot()->replot();
	}
}

void QZPlotwidget::RecenterAxis()
{
//     cout << "Requested a recenter with the following values: \n";
//     cout << "X Axis: " << origXMin << " , " << origXMax << endl;
//     cout << "Y Axis: " << origYMin << " , " << origYMax << endl;

	GetQCustomPlot()->xAxis->setRange(origXMin, origXMax);
	GetQCustomPlot()->yAxis->setRange(origYMin, origYMax);

	xAxisMin = origXMin;
	xAxisMax = origXMax;
	yAxisMin = origYMin;
	yAxisMax = origYMax;

	findChild<QWidget*>("xSliderHandleLeft")->move(initialSliderHandlesPos["xSliderHandleLeft"].x(), initialSliderHandlesPos["xSliderHandleLeft"].y());
	findChild<QWidget*>("xSliderHandleRight")->move(initialSliderHandlesPos["xSliderHandleRight"].x(), initialSliderHandlesPos["xSliderHandleRight"].y());
	findChild<QWidget*>("ySliderHandleTop")->move(initialSliderHandlesPos["ySliderHandleTop"].x(), initialSliderHandlesPos["ySliderHandleTop"].y());
	findChild<QWidget*>("ySliderHandleBottom")->move(initialSliderHandlesPos["ySliderHandleBottom"].x(), initialSliderHandlesPos["ySliderHandleBottom"].y());

	RegisterMaxRanges(true);

	GetQCustomPlot()->replot();
}

void QZPlotwidget::RescaleXAxis()
{
	xAxisMin = findChild<QLineEdit*>("xMinIF")->text().toDouble();
	xAxisMax = findChild<QLineEdit*>("xMaxIF")->text().toDouble();

	GetQCustomPlot()->xAxis->blockSignals(true);
	GetQCustomPlot()->xAxis->setRange(xAxisMin, xAxisMax);

	GetQCustomPlot()->replot();
	GetQCustomPlot()->xAxis->blockSignals(false);

	RegisterMaxRanges(false);
}

void QZPlotwidget::RescaleYAxis()
{
	yAxisMin = findChild<QLineEdit*>("yMinIF")->text().toDouble();
	yAxisMax = findChild<QLineEdit*>("yMaxIF")->text().toDouble();

	GetQCustomPlot()->yAxis->blockSignals(true);
	GetQCustomPlot()->yAxis->setRange(yAxisMin, yAxisMax);

	GetQCustomPlot()->replot();
	GetQCustomPlot()->yAxis->blockSignals(false);

	RegisterMaxRanges(false);
}

void QZPlotwidget::UpdateXAxisRangeBoxes(const QCPRange& newRange)
{
	char* newRangeStr = new char[16];

	xAxisMin = newRange.lower;
	sprintf(newRangeStr, "%2.3f", xAxisMin);

//     cout << "New XMin value : " << newRangeStr << endl;

	findChild<QLineEdit*>("xMinIF")->blockSignals(true);
	findChild<QLineEdit*>("xMinIF")->setText(QString(newRangeStr));
	findChild<QLineEdit*>("xMinIF")->blockSignals(false);

	xAxisMax = newRange.upper;

	sprintf(newRangeStr, "%2.3f", xAxisMax);
	findChild<QLineEdit*>("xMaxIF")->blockSignals(true);
	findChild<QLineEdit*>("xMaxIF")->setText(QString(newRangeStr));
	findChild<QLineEdit*>("xMaxIF")->blockSignals(false);
}

void QZPlotwidget::UpdateYAxisRangeBoxes(const QCPRange& newRange)
{
	char* newRangeStr = new char[16];

	yAxisMin = newRange.lower;
	sprintf(newRangeStr, "%2.3f", yAxisMin);

//     cout << "New XMin value : " << newRangeStr << endl;

	findChild<QLineEdit*>("yMinIF")->blockSignals(true);
	findChild<QLineEdit*>("yMinIF")->setText(QString(newRangeStr));
	findChild<QLineEdit*>("yMinIF")->blockSignals(false);

	yAxisMax = newRange.upper;

	sprintf(newRangeStr, "%2.3f", yAxisMax);
	findChild<QLineEdit*>("yMaxIF")->blockSignals(true);
	findChild<QLineEdit*>("yMaxIF")->setText(QString(newRangeStr));
	findChild<QLineEdit*>("yMaxIF")->blockSignals(false);
}

void QZPlotwidget::AddGraph(string title, vector<double> x_, vector<double> y_, double xMin_, double xMax_, string xAxisLabel, string yAxisLabel, bool countGraph)
{
	if (countGraph) graphCount++;
	QPen pen;
	pen.setColor(colorPalette[graphCount]);
	pen.setWidth(2);

	QVector<double> xQVect = QVector<double>::fromStdVector(x_);
	QVector<double> yQVect = QVector<double>::fromStdVector(y_);
	// create graph and assign data to it:
	GetQCustomPlot()->addGraph();

	string legendLabel = title.substr(title.find("for ") + 4);

	if (countGraph) GetQCustomPlot()->graph()->setName(legendLabel.c_str());
	GetQCustomPlot()->graph()->setData(xQVect, yQVect);

	// give the axes some labels:
	GetQCustomPlot()->xAxis->setLabel(QString(xAxisLabel.c_str()));
	GetQCustomPlot()->yAxis->setLabel(QString(yAxisLabel.c_str()));
	// set axes ranges, so we see all data:

	GetQCustomPlot()->xAxis->setRange(xMin_, xMax_);

	origXMin = xMin_;
	origXMax = xMax_;

	xAxisMin = xMin_;
	xAxisMax = xMax_;

	auto yMinItr = min_element(y_.begin(), y_.end());
	if (*yMinItr < yAxisMin || yAxisMin <= -666) yAxisMin = *yMinItr;

	auto yMaxItr = max_element(y_.begin(), y_.end());
	if (*yMaxItr > yAxisMax || yAxisMax <= -666) yAxisMax = *yMaxItr;

	GetQCustomPlot()->yAxis->setRange(std::max(0., yAxisMin - (yAxisMax - yAxisMin) * 0.1), yAxisMax + (yAxisMax - yAxisMin) * 0.1);

	GetQCustomPlot()->graph()->setPen(pen);

	QPen newSDPen = GetQCustomPlot()->graph()->selectionDecorator()->pen();
	newSDPen.setColor(pen.color());
	GetQCustomPlot()->graph()->selectionDecorator()->setPen(newSDPen);

	GetQCustomPlot()->legend->setVisible(true);
	GetQCustomPlot()->legend->setFont(QFont("Helvetica", 9));

	char* valStr = new char[16];

	sprintf(valStr, "%2.3f", xMin_);
	findChild<QLineEdit*>("xMinIF")->setText(QString(valStr));

	sprintf(valStr, "%2.3f", xMax_);
	findChild<QLineEdit*>("xMaxIF")->setText(QString(valStr));

	sprintf(valStr, "%2.3f", yAxisMin);
	findChild<QLineEdit*>("yMinIF")->setText(QString(valStr));

	sprintf(valStr, "%2.3f", yAxisMax);
	findChild<QLineEdit*>("yMaxIF")->setText(QString(valStr));

	origYMin = std::max(0., yAxisMin - (yAxisMax - yAxisMin) * 0.1);
	origYMax = yAxisMax + (yAxisMax - yAxisMin) * 0.1;

	if (!countGraph) GetQCustomPlot()->legend->removeItem(GetQCustomPlot()->legend->itemCount() - 1);

	GetQCustomPlot()->replot();

	RegisterMaxRanges(true);
}

QWidget* GetFirstParent(QWidget* widget)
{
	QWidget* parent = widget->parentWidget();

	if (parent == nullptr) return widget;

	QWidget* nextParent = parent;

	while (nextParent != nullptr)
	{
		nextParent = parent->parentWidget();

		if (nextParent != nullptr) parent = nextParent;
	}

	return parent;
}

bool ForwardMouseTracking::eventFilter(QObject* obj, QEvent* event)
{
	QWidget* widg = dynamic_cast<QWidget*>(obj);
	if (widg == nullptr) return true;

	QZPlotwidget* plotWidg = dynamic_cast<QZPlotwidget*>(GetFirstParent(widg));
	if (plotWidg == nullptr) return true;

	int minRefresh = 50;

	if (widg->objectName() == QString("xSliderHandleLeft"))
	{
		if (event->type() == QEvent::MouseButtonPress) widg->grabMouse();
		if (event->type() == QEvent::MouseButtonRelease) widg->releaseMouse();

		if (plotWidg->refreshTime->elapsed() > minRefresh)
		{
			plotWidg->refreshTime->restart();

			if (event->type() == QEvent::MouseMove)
			{
				QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

				QPoint localMousePos = plotWidg->mapFromGlobal(QPoint(mouseEvent->globalPos()));

				if (localMousePos.x() > plotWidg->initialSliderHandlesPos["xSliderHandleLeft"].x()
						&& localMousePos.x()
								< plotWidg->findChild<QLabel*>("xSliderHandleRight")->geometry().x() - plotWidg->findChild<QLabel*>("xSliderHandleRight")->geometry().width())
				{
					widg->move(localMousePos.x(), widg->geometry().y());

					double slideMove = localMousePos.x() - plotWidg->initialSliderHandlesPos["xSliderHandleLeft"].x();
					double rangeMultiplier = slideMove / plotWidg->xSliderRange;

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->xMinBeforeDrag + plotWidg->xRangeBeforeDrag * rangeMultiplier);

					plotWidg->findChild<QLineEdit*>("xMinIF")->setText(QString(newVal));
					emit plotWidg->RescaleXAxis();
				}
				else if (localMousePos.x() <= plotWidg->initialSliderHandlesPos["xSliderHandleLeft"].x())
				{
					widg->move(plotWidg->initialSliderHandlesPos["xSliderHandleLeft"].x(), widg->geometry().y());

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->xMinBeforeDrag);
					plotWidg->findChild<QLineEdit*>("xMinIF")->setText(QString(newVal));
					emit plotWidg->RescaleXAxis();
				}
				else return true;
			}
		}

		return QObject::eventFilter(obj, event);
	}
	else if (widg->objectName() == QString("xSliderHandleRight"))
	{
		if (event->type() == QEvent::MouseButtonPress) widg->grabMouse();
		if (event->type() == QEvent::MouseButtonRelease) widg->releaseMouse();

		if (plotWidg->refreshTime->elapsed() > minRefresh)
		{
			plotWidg->refreshTime->restart();
			if (event->type() == QEvent::MouseMove)
			{
				QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

				QPoint localMousePos = plotWidg->mapFromGlobal(QPoint(mouseEvent->globalPos()));

				if (localMousePos.x() < plotWidg->initialSliderHandlesPos["xSliderHandleRight"].x()
						&& localMousePos.x()
								> plotWidg->findChild<QLabel*>("xSliderHandleLeft")->geometry().x() + plotWidg->findChild<QLabel*>("xSliderHandleLeft")->geometry().width() / 2.)
				{
					widg->move(localMousePos.x(), widg->geometry().y());

					double slideMove = localMousePos.x() - plotWidg->initialSliderHandlesPos["xSliderHandleRight"].x();
					double rangeMultiplier = slideMove / plotWidg->xSliderRange;

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->xMaxBeforeDrag + plotWidg->xRangeBeforeDrag * rangeMultiplier);

					plotWidg->findChild<QLineEdit*>("xMaxIF")->setText(QString(newVal));
					emit plotWidg->RescaleXAxis();
				}
				else if (localMousePos.x() >= plotWidg->initialSliderHandlesPos["xSliderHandleRight"].x())
				{
					widg->move(plotWidg->initialSliderHandlesPos["xSliderHandleRight"].x(), widg->geometry().y());

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->xMaxBeforeDrag);
					plotWidg->findChild<QLineEdit*>("xMaxIF")->setText(QString(newVal));
					emit plotWidg->RescaleXAxis();
				}
				else return true;
			}
		}

		return QObject::eventFilter(obj, event);
	}
	else if (widg->objectName() == QString("ySliderHandleTop"))
	{
		if (event->type() == QEvent::MouseButtonPress) widg->grabMouse();
		if (event->type() == QEvent::MouseButtonRelease) widg->releaseMouse();

		if (plotWidg->refreshTime->elapsed() > minRefresh)
		{
			plotWidg->refreshTime->restart();
			if (event->type() == QEvent::MouseMove)
			{
				QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

				QPoint localMousePos = plotWidg->mapFromGlobal(QPoint(mouseEvent->globalPos()));

				if (localMousePos.y() > plotWidg->initialSliderHandlesPos["ySliderHandleTop"].y()
						&& localMousePos.y()
								< plotWidg->findChild<QLabel*>("ySliderHandleBottom")->geometry().y()
										- plotWidg->findChild<QLabel*>("ySliderHandleBottom")->geometry().height() / 2.)
				{
					widg->move(widg->geometry().x(), localMousePos.y());

					double slideMove = localMousePos.y() - plotWidg->initialSliderHandlesPos["ySliderHandleTop"].y();
					double rangeMultiplier = slideMove / plotWidg->ySliderRange;

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->yMaxBeforeDrag - plotWidg->yRangeBeforeDrag * rangeMultiplier);

					plotWidg->findChild<QLineEdit*>("yMaxIF")->setText(QString(newVal));
					emit plotWidg->RescaleYAxis();
				}
				else if (localMousePos.y() <= plotWidg->initialSliderHandlesPos["ySliderHandleTop"].y())
				{
					widg->move(widg->geometry().x(), plotWidg->initialSliderHandlesPos["ySliderHandleTop"].y());

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->yMaxBeforeDrag);
					plotWidg->findChild<QLineEdit*>("yMaxIF")->setText(QString(newVal));
					emit plotWidg->RescaleYAxis();
				}
				else return true;
			}
		}

		return QObject::eventFilter(obj, event);
	}
	else if (widg->objectName() == QString("ySliderHandleBottom"))
	{
		if (event->type() == QEvent::MouseButtonPress) widg->grabMouse();
		if (event->type() == QEvent::MouseButtonRelease) widg->releaseMouse();

		if (plotWidg->refreshTime->elapsed() > minRefresh)
		{
			plotWidg->refreshTime->restart();
			if (event->type() == QEvent::MouseMove)
			{
				QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

				QPoint localMousePos = plotWidg->mapFromGlobal(QPoint(mouseEvent->globalPos()));

				if (localMousePos.y() < plotWidg->initialSliderHandlesPos["ySliderHandleBottom"].y()
						&& localMousePos.y()
								> plotWidg->findChild<QLabel*>("ySliderHandleTop")->geometry().y() + plotWidg->findChild<QLabel*>("ySliderHandleTop")->geometry().height() / 2.)
				{
					widg->move(widg->geometry().x(), localMousePos.y());

					double slideMove = localMousePos.y() - plotWidg->initialSliderHandlesPos["ySliderHandleBottom"].y();
					double rangeMultiplier = slideMove / plotWidg->ySliderRange;

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->yMinBeforeDrag - plotWidg->yRangeBeforeDrag * rangeMultiplier);

					plotWidg->findChild<QLineEdit*>("yMinIF")->setText(QString(newVal));
					emit plotWidg->RescaleYAxis();
				}
				else if (localMousePos.y() >= plotWidg->initialSliderHandlesPos["ySliderHandleBottom"].y())
				{
					widg->move(widg->geometry().x(), plotWidg->initialSliderHandlesPos["ySliderHandleBottom"].y());

					char* newVal = new char[24];
					sprintf(newVal, "%2.3f", plotWidg->yMinBeforeDrag);
					plotWidg->findChild<QLineEdit*>("yMinIF")->setText(QString(newVal));
					emit plotWidg->RescaleYAxis();
				}
				else return true;
			}
		}

		return QObject::eventFilter(obj, event);
	}
	else if (event->type() == QEvent::Enter)
	{
		if (widg != nullptr)
		{
			if (plotWidg != nullptr)
			{
				if (widg->objectName() == QString("xAxisHitbox")) plotWidg->GetQCustomPlot()->axisRect()->setRangeZoom(Qt::Horizontal);
				else if (widg->objectName() == QString("yAxisHitbox")) plotWidg->GetQCustomPlot()->axisRect()->setRangeZoom(Qt::Vertical);
			}
		}

		return true;
	}
	else if (event->type() == QEvent::Leave)
	{
		if (widg != nullptr)
		{
			if (plotWidg != nullptr) plotWidg->GetQCustomPlot()->axisRect()->setRangeZoom(0);
		}

		return true;
	}
	else if (plotWidg->GetQCustomPlot()->axisRect()->rangeZoom() != 0)
	{
		if (event->type() == QEvent::Wheel) return QCoreApplication::sendEvent(plotWidg->GetQCustomPlot(), event);

		return false;
	}
	else return QObject::eventFilter(obj, event);
}

#include "../include/moc_QZPlotwidget.cpp"

