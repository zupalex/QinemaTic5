#include "CalcMonitor.h"

CalcMonitor* CalcMonitor::s_instance = nullptr;

CalcMonitor* CalcMonitor::CreateCalcMonitor()
{
	if (s_instance == NULL)
	{
		s_instance = new CalcMonitor();

		s_instance->kinResMap.clear();

		s_instance->ResetState();

		s_instance->SetButtonNameToVar();
	}

	return s_instance;
}

void CalcMonitor::SetMainFrame(QZMainFrame* mainFrame_)
{
	mainFrame = mainFrame_;
}

QZMainFrame* CalcMonitor::GetMainFrame()
{
	return mainFrame;
}

void CalcMonitor::ResetState()
{
	errorState = false;
}

void CalcMonitor::SetButtonNameToVar()
{
	buttonNameToVar["ejecLabAngleIF"] = "Ejec. Lab. Angle";
	buttonNameToVar["recLabAngleIF"] = "Recoil Lab. Angle";
	buttonNameToVar["cMAngleIF"] = "C.M. Angle";
	buttonNameToVar["ejecLabEnIF"] = "Ejec. Lab. Energy";
	buttonNameToVar["recLabEnIF"] = "Recoil Lab. Energy";
}

vector<string> CalcMonitor::GetReacList()
{
	vector<string> reacList;
	reacList.clear();

	for (auto itr = kinResMap.begin(); itr != kinResMap.end(); itr++)
	{
		reacList.push_back(itr->first);
	}

	return reacList;
}

void CalcMonitor::UpdateReacInfo(string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamExStr, string targetExStr, string ejecExStr,
		string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn)
{
//     cout << "Clicked Update Reaction Button\n";
//
//     cout << "Beam: " << beamStr << " / Target: " << targetStr << " / Ejectile: " << ejecStr << " / Recoil: " << recoilStr << endl;

	if (beamStr.empty() || targetStr.empty()) return;
	if (ejecStr.empty() && recoilStr.empty()) return;
	if (beamEkStr.empty() && beamCMEkStr.empty()) return;

	ifstream mass_db("./mass_db.dat");

	if (!mass_db.is_open())
	{
		cerr << "No File Found for the Masses Database!\n";
		return;
	}

	RootKinCalc* kinCalc = new RootKinCalc();

	float beamEk = !beamEkStr.empty() ? stof(beamEkStr) : 0.0;

	float beamEx = !beamExStr.empty() ? stof(beamExStr) : 0.0;
	float targetEx = !targetExStr.empty() ? stof(targetExStr) : 0.0;
	float ejecEx = !ejecExStr.empty() ? stof(ejecExStr) : 0.0;
	float recEx = !recoilExStr.empty() ? stof(recoilExStr) : 0.0;

	float beamCMEk = !beamCMEkStr.empty() ? stof(beamCMEkStr) : 0.0;
	;

	kinCalc->GetBaseKinematicInfo(beamStr, targetStr, (!invertRecoilEjec ? ejecStr : recoilStr), (!invertLabCMEn ? beamEk : beamCMEk), ejecEx, recEx, invertRecoilEjec,
			invertLabCMEn);

	if (kinCalc->rInfo->A["ejectile"] < 1 || kinCalc->rInfo->A["recoil"] < 1 || kinCalc->rInfo->Z["ejectile"] < 0 || kinCalc->rInfo->Z["recoil"] < 0 || kinCalc->beamEkCM < 0
			|| kinCalc->beamEkLab < 0)
	{
		cerr << "Invalid reaction...\n";

		lastKinCalc = nullptr;

		emit RequestResetInputFields("reac info");

		return;
	}

	if (invertRecoilEjec) ejecStr = kinCalc->GetAtomicFormula(kinCalc->rInfo->A["ejectile"], kinCalc->rInfo->atomicElement["ejectile"]);
	else recoilStr = kinCalc->GetAtomicFormula(kinCalc->rInfo->A["recoil"], kinCalc->rInfo->atomicElement["recoil"]);

	if (invertLabCMEn)
	{
		char* buffer = new char[128];
		sprintf(buffer, "%f", kinCalc->beamEkLab);
		beamEkStr = buffer;
	}
	else
	{
		char* buffer = new char[128];
		sprintf(buffer, "%f", kinCalc->beamEkCM);
		beamCMEkStr = buffer;
	}

	lastKinCalc = kinCalc;

	emit RequestFillReacInfoFields(beamStr, targetStr, ejecStr, recoilStr, beamEkStr, beamCMEkStr, kinCalc->qValueGS, kinCalc->reacAboveThr);
}

void CalcMonitor::ConvertSingleValue(int reacID, QLineEdit* lineEdit)
{
//     std::cout << "Clicked Convert Single Value Button\n";

	if (reacID >= kinResMap.size() || reacID == -1) return;

//     cout << "Reaction ID: " << reacID << endl;

	auto reacItr = kinResMap.begin();

	std::advance(reacItr, reacID);

	bool haveSecondSolution = false;

	if (reacItr->second.kinRes.begin()->second.ejecLabEnergy2 > 0)
	{
		emit RequestEnableSecondSolution(true);
		haveSecondSolution = true;
	}

	string fromQuantity = buttonNameToVar[lineEdit->objectName().toUtf8().constData()];

	float val = stof(lineEdit->text().toUtf8().constData());

	map<string, string> resMap;

	for (auto varItr = buttonNameToVar.begin(); varItr != buttonNameToVar.end(); varItr++)
	{
		if (varItr->second != fromQuantity)
		{
			float res = reacItr->second.ConvertSingleValue(fromQuantity, varItr->second, val);

			char* buffer = new char[128];
			sprintf(buffer, "%2.3f", res);

			resMap[varItr->first] = (res != -1e6 ? buffer : "none");

			if (haveSecondSolution)
			{
				res = reacItr->second.ConvertSingleValue(fromQuantity + " 2", varItr->second + " 2", val);

				buffer = new char[128];
				sprintf(buffer, "%2.3f", res);

				resMap[varItr->first + " 2"] = (res != -1e6 ? buffer : "none");
			}
		}
	}

	emit RequestSetSingleConvertValues(resMap);
}

void CalcMonitor::GetReacKinematics(string beamStr, string targetStr, string ejecStr, string recoilStr, string beamEkStr, string beamExStr, string targetExStr, string ejecExStr,
		string recoilExStr, string beamCMEkStr, bool invertRecoilEjec, bool invertLabCMEn)
{
//     std::cout << "Clicked Get Kinematics Button\n";

	UpdateReacInfo(beamStr, targetStr, ejecStr, recoilStr, beamEkStr, beamExStr, targetExStr, ejecExStr, recoilExStr, beamCMEkStr, invertRecoilEjec, invertLabCMEn);

	if (lastKinCalc == nullptr) return;

	float stepWidth = 0.1;

	if (!lastKinCalc->reacAboveThr)
	{
		std::cout << "The reaction in below the threshold...\n";

		return;
	}

	float labAngle = 0.0;

	while (labAngle <= 180.0)
	{
		lastKinCalc->CalcKinematic(labAngle);

//         std::cout << kinResMap[mapKey][kinResMap[mapKey].size()-1].ejecLabAngle << " <-> " << kinResMap[mapKey][kinResMap[mapKey].size()-1].ejecLabEnergy << "\n";

		labAngle += stepWidth;
	}

	kinResMap[lastKinCalc->mapKey] = *lastKinCalc;

	emit RequestUpdateReacList(GetReacList());
}

void CalcMonitor::PlotKinematicsGraph(vector<int> selectedEntries, string xAxisID, string yAxisID, string xMinStr, string xMaxStr, string stepWidthStr)
{
//     std::cout << "Clicked Plot Graph Button\n";

	float xMin_, xMax_, stepWidth_;

	xMin_ = stoi(xMinStr);
	xMax_ = stoi(xMaxStr);
	stepWidth_ = stof(stepWidthStr);

	char* plotTitle = new char[512];
	sprintf(plotTitle, "%s vs. %s", xAxisID.c_str(), yAxisID.c_str());

	emit RedrawPlotWidget((string) plotTitle);

	for (unsigned int i = 0; i < selectedEntries.size(); i++)
	{
		auto reacItr = kinResMap.begin();
		std::advance(reacItr, selectedEntries[i]);

		char* grTitle = new char[512];
		sprintf(grTitle, "%s vs. %s for %s", xAxisID.c_str(), yAxisID.c_str(), reacItr->first.c_str());

		std::cout << "Plotting " << grTitle << " with steps of " << stepWidth_ << "...\n";

		pair<vector<double>, vector<double>> result = reacItr->second.PlotKinematicGraph(xAxisID, yAxisID, xMin_, xMax_, stepWidth_);

		if (result.first.size() == 0 || result.second.size() == 0) return;

		emit AddGraph((string) grTitle, result.first, result.second, xMin_, xMax_, xAxisID, yAxisID);

		if (reacItr->second.kinRes.begin()->second.ejecLabEnergy2 > 0)
		{
			result = reacItr->second.PlotKinematicGraph(xAxisID + " 2", yAxisID + " 2", xMin_, xMax_, stepWidth_);

			emit AddGraph((string) grTitle, result.first, result.second, xMin_, xMax_, xAxisID, yAxisID, false);
		}
	}
}

void CalcMonitor::WriteOutputTable(int reacID, string xMinStr, string xMaxStr, string stepWidthStr)
{
//     std::cout << "Clicked Write Output Table Button\n";
//     cout << "Reaction ID: " << reacID << endl;

	if (reacID >= kinResMap.size() || reacID == -1) return;

	int xMin = stoi(xMinStr);
	int xMax = stoi(xMaxStr);
	float precision = stof(stepWidthStr);

	auto grItr = kinResMap.begin();

	std::advance(grItr, reacID);

	std::stringstream grTitle;

	grTitle.clear();

	grTitle.str(grItr->first);

	string dummy, reaction, energy, unit;

	grTitle >> reaction >> energy >> unit;

	string outputName = grItr->first;

	std::size_t foundPar1 = outputName.find("(");
	outputName.replace(foundPar1, 1, "_");

	std::size_t foundPar2 = outputName.find(")");
	outputName.replace(foundPar2, 1, "_");

	std::size_t foundComa = outputName.find(",");
	outputName.replace(foundComa, 1, "_");

	std::size_t foundUnit = outputName.find("eV") - 2;
	outputName.replace(foundUnit, 1, "");

	std::size_t foundAt = outputName.find("@");
	outputName.replace(foundAt - 1, 1, "");

	std::size_t foundPar3 = outputName.find("(");
	outputName.replace(foundPar3 - 1, 2, "_");

	std::size_t foundPar4 = outputName.find(")");
	outputName.replace(foundPar4, 1, "");

	std::size_t foundSlash = outputName.find("/");
	outputName.replace(foundSlash - 1, 3, "_");

	std::size_t foundEqual1 = outputName.find("=");
	outputName.replace(foundEqual1 - 1, 3, "=");

	std::size_t foundEqual2 = outputName.find("=");
	foundEqual2 = outputName.find("=", foundEqual2 + 1);
	outputName.replace(foundEqual2 - 1, 3, "=");

	std::size_t foundStar1 = outputName.find("*");
	outputName.replace(foundStar1, 2, "xE");

	std::size_t foundStar2 = outputName.find("*");
	outputName.replace(foundStar2, 2, "xR");

	struct stat checkDir;

	stat("./tables/", &checkDir);

	if (!(checkDir.st_mode & S_IFDIR))
	{
		mkdir("./tables/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	outputName = "./tables/" + outputName + ".dat";

	std::cout << "Writing table to file: " << outputName << "\n";

//     std::cout << "Preparing temp graphs...\n";

	pair<vector<double>, vector<double>> aLabEjecVSCMEjec = grItr->second.PlotKinematicGraph(GetKinResIDString(0), GetKinResIDString(2), xMin, xMax, precision);
	pair<vector<double>, vector<double>> aLabejecVSenLabEjec = grItr->second.PlotKinematicGraph(GetKinResIDString(0), GetKinResIDString(1), xMin, xMax, precision);
	pair<vector<double>, vector<double>> aLabejecVSaLabRec = grItr->second.PlotKinematicGraph(GetKinResIDString(0), GetKinResIDString(3), xMin, xMax, precision);
	pair<vector<double>, vector<double>> aLabejecVSenLabRec = grItr->second.PlotKinematicGraph(GetKinResIDString(0), GetKinResIDString(4), xMin, xMax, precision);

//     std::cout << "Finished creating temp graphs...\n";

	std::ofstream outTable(outputName.c_str());

	if (!outTable.is_open())
	{
		std::cerr << "Failed to open file " << outputName << "\n";
		return;
	}

	outTable << grItr->first;
	outTable << "\n";
	outTable << left << setw(29) << "Ejectile Lab Angle (deg.)";
	outTable << left << setw(21) << "Ejectile Lab Energy (" << unit << std::setw(5) << ")";
	outTable << left << setw(21) << "C.M. Angle (deg.)";
	outTable << left << setw(27) << "Recoil Lab Angle (deg.)";
	outTable << left << setw(20) << "Recoil Lab Energy (" << unit << ")\n";

	float x_ = xMin;

	while (x_ <= xMax)
	{
		outTable << setw(29) << left << x_;
		outTable << setw(29) << left << EvalGraph(aLabejecVSenLabEjec.first, aLabejecVSenLabEjec.second, x_);
		outTable << setw(21) << left << EvalGraph(aLabEjecVSCMEjec.first, aLabEjecVSCMEjec.second, x_);
		outTable << setw(27) << left << EvalGraph(aLabejecVSaLabRec.first, aLabejecVSaLabRec.second, x_);
		outTable << setw(24) << left << EvalGraph(aLabejecVSenLabRec.first, aLabejecVSenLabRec.second, x_);
		outTable << "\n";

		x_ += precision;
	}
}

#include "../include/moc_CalcMonitor.cpp"

