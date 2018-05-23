#include <QPushButton>
#include <QLayout>
#include <QTimer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMessageBox>

#include "QZMainframe.h"
#include "ui_QZMainframe.h"

#include <QTime>

void ResetDefaultValue(QLineEdit* lineEdit, QString defaultStr)
{
	if (lineEdit->text().length() == 0) lineEdit->setText(defaultStr);
}

QZMainFrame::QZMainFrame(QWidget *parent)
		: QMainWindow(parent), ui(new Ui::QZMainFrame)
{
	ui->setupUi(this);

	ForwardDoubleClick* fDc = new ForwardDoubleClick();

	QObject::connect(findChild<QToolButton*>("updateReacInfoBut"), SIGNAL(released()), this, SLOT(OnClickUpdateReac()));
	QObject::connect(findChild<QToolButton*>("getReacKinBut"), SIGNAL(released()), this, SLOT(OnClickGetKinematics()));
	QObject::connect(findChild<QToolButton*>("writeTableBut"), SIGNAL(released()), this, SLOT(OnClickWriteTable()));
	QObject::connect(findChild<QToolButton*>("processBut"), SIGNAL(released()), this, SLOT(OnClickConvertSingle()));
	QObject::connect(findChild<QToolButton*>("plotGraphBut"), SIGNAL(released()), this, SLOT(OnClickPlotGraph()));

	QObject::connect(findChild<QCheckBox*>("beamCMCB"), SIGNAL(stateChanged ( int )), this, SLOT(InvertLabAndCMEk ( int )));
	QObject::connect(findChild<QCheckBox*>("recoilCB"), SIGNAL(stateChanged ( int )), this, SLOT(InvertRecoilAndEjectile ( int )));

	findChild<QLineEdit*>("ejecLabAngleIF")->installEventFilter(fDc);
	findChild<QLineEdit*>("recLabAngleIF")->installEventFilter(fDc);
	findChild<QLineEdit*>("cMAngleIF")->installEventFilter(fDc);
	findChild<QLineEdit*>("ejecLabEnIF")->installEventFilter(fDc);
	findChild<QLineEdit*>("recLabEnIF")->installEventFilter(fDc);

	QRegExp elementsRegExp("\\d{0,3}([a-z]{1,1}|[A-Z]{1,1})[a-z]{0,1}");

	QRegExp realOnly("0{0,1}|[1-9]{1,}[0-9]{0,}\\.{0,1}[0-9]{0,}|0{1,1}\\.{1,1}[0-9]{0,}");
	QRegExp integerOnly("0{0,1}|[1-9]{1,}[0-9]{0,}");

	findChild<QLineEdit*>("beamElementIF")->setValidator(new QRegExpValidator(elementsRegExp, 0));
	findChild<QLineEdit*>("targetElementIF")->setValidator(new QRegExpValidator(elementsRegExp, 0));
	findChild<QLineEdit*>("ejecElementIF")->setValidator(new QRegExpValidator(elementsRegExp, 0));
	findChild<QLineEdit*>("recoilElementIF")->setValidator(new QRegExpValidator(elementsRegExp, 0));

	findChild<QLineEdit*>("beamEkIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("beamExIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("targetExIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("ejecExIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("recoilExIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("beamCMEnIF")->setValidator(new QRegExpValidator(realOnly, 0));

	findChild<QLineEdit*>("xMinTableIF")->setValidator(new QRegExpValidator(integerOnly, 0));
	findChild<QLineEdit*>("xMaxTableIF")->setValidator(new QRegExpValidator(integerOnly, 0));
	findChild<QLineEdit*>("stepWidthTableIF")->setValidator(new QRegExpValidator(realOnly, 0));

	findChild<QLineEdit*>("xMinGraphIF")->setValidator(new QRegExpValidator(integerOnly, 0));
	findChild<QLineEdit*>("xMaxGraphIF")->setValidator(new QRegExpValidator(integerOnly, 0));
	findChild<QLineEdit*>("stepWidthGraphIF")->setValidator(new QRegExpValidator(realOnly, 0));

	findChild<QLineEdit*>("ejecLabAngleIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("recLabAngleIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("cMAngleIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("ejecLabEnIF")->setValidator(new QRegExpValidator(realOnly, 0));
	findChild<QLineEdit*>("recLabEnIF")->setValidator(new QRegExpValidator(realOnly, 0));

	QObject::connect(findChild<QCheckBox*>("sSolCB"), SIGNAL(stateChanged ( int )), this, SLOT(SwitchToSecondSolution ()));
	findChild<QCheckBox*>("sSolCB")->setVisible(false);

	RegisterChildrenDefaultValues();

	CreateDisplayWidget();

	invertEjecRecoil = false;
	invertLabCMEn = false;
}

QZMainFrame::~QZMainFrame()
{
	delete ui;
}

void QZMainFrame::RedrawPlotWidget(string plotTitle)
{
	if (displayWidget == nullptr) CreateDisplayWidget();

	displayWidget->GetQCustomPlot()->clearGraphs();

	displayWidget->yAxisMin = -666.666;
	displayWidget->yAxisMax = -666.666;

	displayWidget->findChild<QLabel*>("plotTitle")->setText(QString(plotTitle.c_str()));

	displayWidget->show();
}

void QZMainFrame::AddGraph(string title, vector<double> x_, vector<double> y_, double xMin_, double xMax_, string xAxisLabel, string yAxisLabel, bool countGraph)
{
	emit ForwardAddGraph(title, x_, y_, xMin_, xMax_, xAxisLabel, yAxisLabel, countGraph);
}

void QZMainFrame::closeEvent(QCloseEvent* event)
{
	QWidget::closeEvent(event);

	displayWidget->disconnect();
	delete displayWidget;

	emit KillApp();
}

void QZMainFrame::CreateDisplayWidget()
{
	displayWidget = new QZPlotwidget(0);

	displayWidget->yAxisMin = -666.666;
	displayWidget->yAxisMax = -666.666;

	QObject::connect(this, SIGNAL(ForwardAddGraph ( string,vector<double>,vector<double>,double,double,string,string,bool )), displayWidget,
			SLOT(AddGraph ( string,vector<double>,vector<double>,double,double,string,string,bool )));

//     displayWidget->show();
}

void QZMainFrame::RegisterChildrenDefaultValues()
{
	QList<QWidget*> childrenList = findChildren<QWidget*>(QRegExp(".*"));

	for (auto child = childrenList.begin(); child != childrenList.end(); child++)
	{
		QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(*child);

//         cout << "Scanning " << ( *child )->objectName().toUtf8().constData() <<endl;

		if (lineEdit != nullptr)
		{
//             cout << "Connecting " << lineEdit->objectName().toUtf8().constData() << " to the ResetDefaultValue() slot\n";
			QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(ResetDefaultValue()));

			if (lineEdit->text().length() != 0)
			{
				defaultChildrenValues[lineEdit] = lineEdit->text();
//                 cout << "Register the default value: " << lineEdit->text().toUtf8().constData() << " for inputfield: " << lineEdit->objectName().toUtf8().constData() << endl;;
			}
		}
	}
}

void QZMainFrame::InvertLabAndCMEk(int state)
{
	QLineEdit* beamEkIF = findChild<QLineEdit*>("beamEkIF");
	QLineEdit* beamCMEnIF = findChild<QLineEdit*>("beamCMEnIF");

	if (state)
	{
		beamEkIF->setEnabled(false);
		beamCMEnIF->setEnabled(true);
		invertLabCMEn = true;
	}
	else
	{
		beamEkIF->setEnabled(true);
		beamCMEnIF->setEnabled(false);
		invertLabCMEn = false;
	}
}

void QZMainFrame::InvertRecoilAndEjectile(int state)
{
	QLineEdit* ejecElementIF = findChild<QLineEdit*>("ejecElementIF");
	QLineEdit* recoilElementIF = findChild<QLineEdit*>("recoilElementIF");

	if (state)
	{
		ejecElementIF->setEnabled(false);
		recoilElementIF->setEnabled(true);
		invertEjecRecoil = true;
	}
	else
	{
		ejecElementIF->setEnabled(true);
		recoilElementIF->setEnabled(false);
		invertEjecRecoil = false;
	}
}

void QZMainFrame::ResetAllFields(string mode)
{
	bool all = (mode.find("all") != string::npos);
	bool reacInfo = (mode.find("reac info") != string::npos);
	bool tableIF = (mode.find("table") != string::npos);
	bool graphIF = (mode.find("graph") != string::npos);
	bool singleConvertIF = (mode.find("single convert") != string::npos);

	if (all || reacInfo)
	{
		findChild<QLineEdit*>("beamElementIF")->setText("");
		findChild<QLineEdit*>("targetElementIF")->setText("");
		findChild<QLineEdit*>("ejecElementIF")->setText("");
		findChild<QLineEdit*>("recoilElementIF")->setText("");

		findChild<QLineEdit*>("beamEkIF")->setText("0.0");
		findChild<QLineEdit*>("beamExIF")->setText("0.0");
		findChild<QLineEdit*>("targetExIF")->setText("0.0");
		findChild<QLineEdit*>("ejecExIF")->setText("0.0");
		findChild<QLineEdit*>("recoilExIF")->setText("0.0");
		findChild<QLineEdit*>("beamCMEnIF")->setText("0.0");
		findChild<QLCDNumber*>("gsQvalDisplay")->display(0);
	}

	if (all || tableIF)
	{
		findChild<QLineEdit*>("xMinTableIF")->setText("0");
		findChild<QLineEdit*>("xMaxTableIF")->setText("180");
		findChild<QLineEdit*>("stepWidthTableIF")->setText("1");
	}

	if (all || graphIF)
	{
		findChild<QLineEdit*>("xMinGraphIF")->setText("0");
		findChild<QLineEdit*>("xMaxGraphIF")->setText("180");
		findChild<QLineEdit*>("stepWidthGraphIF")->setText("1");
	}

	if (all || singleConvertIF)
	{
		findChild<QLineEdit*>("ejecLabAngleIF")->setText("");
		findChild<QLineEdit*>("recLabAngleIF")->setText("");
		findChild<QLineEdit*>("cMAngleIF")->setText("");
		findChild<QLineEdit*>("ejecLabEnIF")->setText("");
		findChild<QLineEdit*>("recLabEnIF")->setText("");
	}
}

bool QZMainFrame::IsValueOK(QLineEdit* lineEdit)
{
//     cout << "Checking value validity: " << lineEdit->objectName().toUtf8().constData() << endl;

	QLineEdit* beamElementIF = findChild<QLineEdit*>("beamElementIF");
	QLineEdit* targetElementIF = findChild<QLineEdit*>("targetElementIF");
	QLineEdit* ejecElementIF = findChild<QLineEdit*>("ejecElementIF");
	QLineEdit* recoilElementIF = findChild<QLineEdit*>("recoilElementIF");

	QLineEdit* beamEkIF = findChild<QLineEdit*>("beamEkIF");
	QLineEdit* beamExIF = findChild<QLineEdit*>("beamExIF");
	QLineEdit* targetExIF = findChild<QLineEdit*>("targetExIF");
	QLineEdit* ejecExIF = findChild<QLineEdit*>("ejecExIF");
	QLineEdit* recoilExIF = findChild<QLineEdit*>("recoilExIF");
	QLineEdit* beamCMEnIF = findChild<QLineEdit*>("beamCMEnIF");

	QLineEdit* xMinTableIF = findChild<QLineEdit*>("xMinTableIF");
	QLineEdit* xMaxTableIF = findChild<QLineEdit*>("xMaxTableIF");
	QLineEdit* stepWidthTableIF = findChild<QLineEdit*>("stepWidthTableIF");

	QLineEdit* xMinGraphIF = findChild<QLineEdit*>("xMinGraphIF");
	QLineEdit* xMaxGraphIF = findChild<QLineEdit*>("xMaxGraphIF");
	QLineEdit* stepWidthGraphIF = findChild<QLineEdit*>("stepWidthGraphIF");

	QLineEdit* ejecLabAngleIF = findChild<QLineEdit*>("ejecLabAngleIF");
	QLineEdit* recLabAngleIF = findChild<QLineEdit*>("recLabAngleIF");
	QLineEdit* cMAngleIF = findChild<QLineEdit*>("cMAngleIF");
	QLineEdit* ejecLabEnIF = findChild<QLineEdit*>("ejecLabEnIF");
	QLineEdit* recLabEnIF = findChild<QLineEdit*>("recLabEnIF");

	if (lineEdit == beamEkIF || lineEdit == beamExIF || lineEdit == targetExIF || lineEdit == ejecExIF || lineEdit == recoilExIF || lineEdit == beamCMEnIF)
	{
		if (lineEdit->text().toFloat() < 0) return false;
	}
	else if (lineEdit == xMinTableIF)
	{
		if (lineEdit->text().toFloat() < 0 || lineEdit->text().toFloat() > xMaxTableIF->text().toFloat()) return false;
	}
	else if (lineEdit == xMaxTableIF)
	{
		if (lineEdit->text().toFloat() < 0 || lineEdit->text().toFloat() < xMinTableIF->text().toFloat()) return false;
	}
	else if (lineEdit == stepWidthTableIF || lineEdit == stepWidthGraphIF)
	{
		if (lineEdit->text().toFloat() <= 0) return false;
	}
	else if (lineEdit == xMinGraphIF)
	{
		if (lineEdit->text().toFloat() < 0 || lineEdit->text().toFloat() >= xMaxGraphIF->text().toFloat()) return false;
	}
	else if (lineEdit == xMaxGraphIF)
	{
		if (lineEdit->text().toFloat() < 0 || lineEdit->text().toFloat() <= xMinGraphIF->text().toFloat()) return false;
	}
	else if (lineEdit == ejecLabAngleIF || lineEdit == recLabAngleIF || lineEdit == cMAngleIF || lineEdit == ejecLabEnIF || lineEdit == recLabEnIF)
	{
		if (lineEdit->text().toFloat() < 0) return false;
	}

	return true;
}

void QZMainFrame::ResetDefaultValue()
{
	QObject* objSender = sender();

	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(objSender);

//     cout << "Checking length after finishing editing: " << lineEdit->text().length() << endl;

	if (lineEdit->text().length() == 0 || !IsValueOK(lineEdit))
	{
//         cout << "Request a reset to the default value: " << defaultChildrenValues[lineEdit].toUtf8().constData() << endl;

		if (defaultChildrenValues.find(lineEdit) != defaultChildrenValues.end()) lineEdit->setText(defaultChildrenValues[lineEdit]);
		else lineEdit->setText(QString(""));
	}
}

void QZMainFrame::OnClickUpdateReac()
{
	string beamStr = findChild<QLineEdit*>("beamElementIF")->text().toUtf8().constData();
	string targetStr = findChild<QLineEdit*>("targetElementIF")->text().toUtf8().constData();
	string ejecStr = findChild<QLineEdit*>("ejecElementIF")->text().toUtf8().constData();
	string recoilStr = findChild<QLineEdit*>("recoilElementIF")->text().toUtf8().constData();

	string beamEkStr = findChild<QLineEdit*>("beamEkIF")->text().toUtf8().constData();
	string beamExStr = findChild<QLineEdit*>("beamExIF")->text().toUtf8().constData();
	string targetExStr = findChild<QLineEdit*>("targetExIF")->text().toUtf8().constData();
	string ejecExStr = findChild<QLineEdit*>("ejecExIF")->text().toUtf8().constData();
	string recoilExStr = findChild<QLineEdit*>("recoilExIF")->text().toUtf8().constData();
	string beamCMEkStr = findChild<QLineEdit*>("beamCMEnIF")->text().toUtf8().constData();

	emit RequestUpdateReac(beamStr, targetStr, ejecStr, recoilStr, beamEkStr, beamExStr, targetExStr, ejecExStr, recoilExStr, beamCMEkStr, invertEjecRecoil, invertLabCMEn);
}

void QZMainFrame::OnClickConvertSingle()
{
	EnableSecondSolution(false);

	QListWidget* reacListBox = findChild<QListWidget*>("reactionListWidget");

	int reacID = -1;

	QList<QListWidgetItem*> selectedItems = reacListBox->selectedItems();

	if (selectedItems.count() != 1)
	{
		cerr << "ERROR: Please select EXACTLY one reaction from the list to perform the calculation\n";
		return;
	}

	QListWidgetItem* selection = selectedItems.at(0);

	reacID = reacListBox->row(selection);

	emit RequestConvertSingle(reacID, activeSingleConvert);
}

void QZMainFrame::OnClickGetKinematics()
{
	string beamStr = findChild<QLineEdit*>("beamElementIF")->text().toUtf8().constData();
	string targetStr = findChild<QLineEdit*>("targetElementIF")->text().toUtf8().constData();
	string ejecStr = findChild<QLineEdit*>("ejecElementIF")->text().toUtf8().constData();
	string recoilStr = findChild<QLineEdit*>("recoilElementIF")->text().toUtf8().constData();

	string beamEkStr = findChild<QLineEdit*>("beamEkIF")->text().toUtf8().constData();
	string beamExStr = findChild<QLineEdit*>("beamExIF")->text().toUtf8().constData();
	string targetExStr = findChild<QLineEdit*>("targetExIF")->text().toUtf8().constData();
	string ejecExStr = findChild<QLineEdit*>("ejecExIF")->text().toUtf8().constData();
	string recoilExStr = findChild<QLineEdit*>("recoilExIF")->text().toUtf8().constData();
	string beamCMEkStr = findChild<QLineEdit*>("beamCMEnIF")->text().toUtf8().constData();

	emit RequestGetKinematics(beamStr, targetStr, ejecStr, recoilStr, beamEkStr, beamExStr, targetExStr, ejecExStr, recoilExStr, beamCMEkStr, invertEjecRecoil, invertLabCMEn);
}

void QZMainFrame::OnClickPlotGraph()
{
	QListWidget* reacListBox = findChild<QListWidget*>("reactionListWidget");

	vector<int> reacIDs;

	QList<QListWidgetItem*> selectedItems = reacListBox->selectedItems();

	for (auto itr = selectedItems.begin(); itr != selectedItems.end(); itr++)
	{
		reacIDs.push_back(reacListBox->row(*itr));
	}

	string xMinStr = findChild<QLineEdit*>("xMinGraphIF")->text().toUtf8().constData();
	string xMaxStr = findChild<QLineEdit*>("xMaxGraphIF")->text().toUtf8().constData();
	string stepWidthStr = findChild<QLineEdit*>("stepWidthGraphIF")->text().toUtf8().constData();

	QGroupBox* xAxisGB = findChild<QGroupBox*>("xAxisGroupBox");
	QGroupBox* yAxisGB = findChild<QGroupBox*>("yAxisGroupBox");

	string xAxisID, yAxisID;

	for (auto itr = xAxisGB->children().begin(); itr != xAxisGB->children().end(); itr++)
	{
		QRadioButton* rB = dynamic_cast<QRadioButton*>(*itr);

		if (rB != nullptr && rB->isChecked())
		{
			xAxisID = rB->text().toUtf8().constData();
			break;
		}
	}

	for (auto itr = yAxisGB->children().begin(); itr != yAxisGB->children().end(); itr++)
	{
		QRadioButton* rB = dynamic_cast<QRadioButton*>(*itr);

		if (rB != nullptr && rB->isChecked())
		{
			yAxisID = rB->text().toUtf8().constData();
			break;
		}
	}

	if (displayWidget == nullptr)
	{
		CreateDisplayWidget();
	}

	displayWidget->graphCount = 0;

	displayWidget->show();

	emit RequestPlotGraph(reacIDs, xAxisID, yAxisID, xMinStr, xMaxStr, stepWidthStr);
}

void QZMainFrame::OnClickWriteTable()
{
	QListWidget* reacListBox = findChild<QListWidget*>("reactionListWidget");

	int reacID = -1;

	QList<QListWidgetItem*> selectedItems = reacListBox->selectedItems();

	if (selectedItems.count() != 1)
	{
		cerr << "ERROR: Please select EXACTLY one reaction from the list to produce the output table\n";
		return;
	}

	QListWidgetItem* selection = selectedItems.at(0);

	reacID = reacListBox->row(selection);

	string xMinStr = findChild<QLineEdit*>("xMinTableIF")->text().toUtf8().constData();
	string xMaxStr = findChild<QLineEdit*>("xMaxTableIF")->text().toUtf8().constData();
	string stepWidthStr = findChild<QLineEdit*>("stepWidthTableIF")->text().toUtf8().constData();

	if (xMinStr.empty())
	{
		findChild<QLineEdit*>("xMinTableIF")->setText("0");
		xMinStr = "0";
	}

	if (xMaxStr.empty())
	{
		findChild<QLineEdit*>("xMinTableIF")->setText("180");
		xMaxStr = "180";
	}

	if (stepWidthStr.empty())
	{
		findChild<QLineEdit*>("xMinTableIF")->setText("1.0");
		stepWidthStr = "1.0";
	}

	emit RequestWriteTable(reacID, xMinStr, xMaxStr, stepWidthStr);
}

void QZMainFrame::FillReacInfoFields(string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamCMEkStr, double qValGsGs, bool aboveThr)
{
	findChild<QLineEdit*>("beamElementIF")->setText(QString(beamStr.c_str()));
	findChild<QLineEdit*>("targetElementIF")->setText(QString(targetStr.c_str()));
	findChild<QLineEdit*>("ejecElementIF")->setText(QString(ejecStr.c_str()));
	findChild<QLineEdit*>("recoilElementIF")->setText(QString(recoilStr.c_str()));

	findChild<QLineEdit*>("beamEkIF")->setText(QString(beamEkStr.c_str()));
	findChild<QLineEdit*>("beamCMEnIF")->setText(QString(beamCMEkStr.c_str()));
	findChild<QLCDNumber*>("gsQvalDisplay")->display(qValGsGs);

	if (aboveThr) findChild<QLCDNumber*>("gsQvalDisplay")->setStyleSheet("color: green");
	else findChild<QLCDNumber*>("gsQvalDisplay")->setStyleSheet("color: red");
}

void QZMainFrame::UpdateReacListBox(vector<string> reacList)
{
	QListWidget* reacListBox = findChild<QListWidget*>("reactionListWidget");

	reacListBox->clear();

	for (unsigned int i = 0; i < reacList.size(); i++)
	{
		reacListBox->addItem(QString(reacList[i].c_str()));
	}
}

void QZMainFrame::SetSingleConvertValues(map<string, string> resMap)
{
	bool doSecSol = findChild<QCheckBox*>("sSolCB")->isVisible() && findChild<QCheckBox*>("sSolCB")->isChecked();

	lastProcessedSingleConvert = resMap;

	for (auto itr = resMap.begin(); itr != resMap.end(); itr++)
	{
		if (doSecSol && itr->first.find(" 2") != string::npos)
		{
			QLineEdit* lineEdit = findChild<QLineEdit*>(QString((itr->first.substr(0, itr->first.length() - 2)).c_str()));

			lineEdit->setText(QString(itr->second.c_str()));
		}
		else if (!doSecSol && itr->first.find(" 2") == string::npos)
		{
			QLineEdit* lineEdit = findChild<QLineEdit*>(QString(itr->first.c_str()));

			lineEdit->setText(QString(itr->second.c_str()));
		}
	}
}

void QZMainFrame::SwitchToSecondSolution()
{
	SetSingleConvertValues(lastProcessedSingleConvert);
}

void QZMainFrame::EnableSecondSolution(bool enable)
{
	if (enable && !findChild<QCheckBox*>("sSolCB")->isVisible()) findChild<QCheckBox*>("sSolCB")->setVisible(true);
	else if (!enable && findChild<QCheckBox*>("sSolCB")->isVisible()) findChild<QCheckBox*>("sSolCB")->setVisible(false);
}

void HandleSingleConvertEnabled(QWidget* obj, QLineEdit* toEnable)
{
	QGroupBox* gB = static_cast<QGroupBox*>(obj);

	if (gB != nullptr)
	{
		QObjectList childrenList = gB->children();

		for (auto child = childrenList.begin(); child != childrenList.end(); child++)
		{
			QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(*child);

			if (lineEdit != nullptr)
			{
				if (lineEdit == toEnable)
				{
					if (!lineEdit->isEnabled())
					{
						lineEdit->setEnabled(true);
						lineEdit->setFocus();

						QZMainFrame* mainFrame_ = dynamic_cast<QZMainFrame*>(GetFirstParent(gB));

						if (mainFrame_ != nullptr)
						{
							mainFrame_->activeSingleConvert = lineEdit;
						}
					}
					else lineEdit->selectAll();
				}
				else lineEdit->setEnabled(false);
			}
		}
	}
}

bool ForwardDoubleClick::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonDblClick)
	{
//         cout << "Forwarded a double click\n";

		QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(obj);

		if (lineEdit != nullptr)
		{
//             cout << "Forwarded a double click to a QLineEdit Object\n";

			HandleSingleConvertEnabled(lineEdit->parentWidget(), lineEdit);
		}

		return true;
	}
	else
	{
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}

#include "../include/moc_QZMainframe.cpp"

