#include "kinematic.h"

using std::string;

int zBeam_last, aBeam_last, zTarget_last, aTarget_last, zEjec_last, aEjec_last;
string xAxisID_last, yAxisID_last;
float beamEk_last, xMin_last, xMax_last;
float stepWidth_last;
int quietMode_last;
float exEjec_last, exRecoil_last;

double EvalGraph(vector<double> x_, vector<double> y_, double toEval)
{
	if (x_.size() == 0 || y_.size() == 0)
	{
		cerr << "Something went wrong... One or more axis for EvalGraph is empty.\nPlease try again\n";
		return 0.0;
	}

	auto lowItr = std::find(x_.begin(), x_.end(), toEval);

	auto lstItr = x_.end();
	lstItr--;

	int low, up;

	low = std::distance(x_.begin(), lowItr);

	if (lowItr != x_.end()) return y_[low];
	else
	{
		lowItr = std::lower_bound(x_.begin(), x_.end(), toEval);

		if (lowItr == x_.end()) return -1e6;

		if (lowItr != x_.begin()) lowItr--;

		low = std::distance(x_.begin(), lowItr);

		auto upItr = lowItr;
		upItr++;

		up = std::distance(x_.begin(), upItr);

		return (y_[up] + (toEval - x_[up]) * (y_[low] - y_[up]) / (x_[low] - x_[up]));
	}
}

bool CharIsDigit(char toCheck)
{
	if (toCheck == '0' || toCheck == '1' || toCheck == '2' || toCheck == '3' || toCheck == '4' || toCheck == '5' || toCheck == '6' || toCheck == '7' || toCheck == '8'
			|| toCheck == '9') return true;
	else return false;
}

float GetKinResIDValue(KinCalcRes kcr, string ID)
{
	if (ID == "Ejec. Lab. Angle") return kcr.ejecLabAngle;
	else if (ID == "Ejec. Lab. Energy") return kcr.ejecLabEnergy;
	else if (ID == "C.M. Angle") return kcr.ejecCMAngle;
	else if (ID == "Recoil Lab. Angle") return kcr.recoilLabAngle;
	else if (ID == "Recoil Lab. Energy") return kcr.recoilLabEnergy;
	else if (ID == "Ejec. Lab. Angle 2") return kcr.ejecLabAngle2;
	else if (ID == "Ejec. Lab. Energy 2") return kcr.ejecLabEnergy2;
	else if (ID == "C.M. Angle 2") return kcr.ejecCMAngle2;
	else if (ID == "Recoil Lab. Angle 2") return kcr.recoilLabAngle2;
	else if (ID == "Recoil Lab. Energy 2") return kcr.recoilLabEnergy2;
	else return -1;
}

string GetKinResIDString(short ID)
{
	if (ID == 0) return "Ejec. Lab. Angle";
	else if (ID == 1) return "Ejec. Lab. Energy";
	else if (ID == 2) return "C.M. Angle";
	else if (ID == 3) return "Recoil Lab. Angle";
	else if (ID == 4) return "Recoil Lab. Energy";
	else if (ID == 5) return "Ejec. Lab. Angle 2";
	else if (ID == 6) return "Ejec. Lab. Energy 2";
	else if (ID == 7) return "C.M. Angle 2";
	else if (ID == 8) return "Recoil Lab. Angle 2";
	else if (ID == 9) return "Recoil Lab. Energy 2";
	else return "Invalid";
}

ReacInfo::ReacInfo()
{
	mapKeys[0] = "beam";
	mapKeys[1] = "target";
	mapKeys[2] = "ejectile";
	mapKeys[3] = "recoil";
}

void ReacInfo::ReinitMasses()
{
	string keys[4] = { "beam", "target", "ejectile", "recoil" };

	for (short i = 0; i < 4; i++)
	{
		A[keys[i]] = 0;
		Z[keys[i]] = 0;
		massExcess[keys[i]] = 0.0;
		deltaMassExcess[keys[i]] = 0.0;
		bindEnPerA[keys[i]] = 0.0;
		deltaBindEnPerA[keys[i]] = 0.0;
		betaDecayEn[keys[i]] = 0.0;
		deltaBetaDecayEn[keys[i]] = 0.0;
		atomicMassUnit[keys[i]] = 0.0;
		vOVERs[keys[i]] = 0.0;
		atomicElement[keys[i]] = "";
	}
}

string ReacInfo::GetKey(int keyNum)
{
	return mapKeys[keyNum];
}

RootKinCalc::RootKinCalc()
{
	rInfo = new ReacInfo();
}

bool RootKinCalc::AssignLastUsedValues(float zb_, float ab_, float zt_, float at_, float ze_, float ae_, string xAID_, string yAID_, float bek_, float xMin_, float xMax_,
		float stepWidth_, bool qm_, float exejec_, float exrec_)
{
	bool changed = false;

//     if ( zBeam_last != zb_ || aBeam_last != ab_ || zTarget_last != zt_ || aTarget_last != at_ || zEjec_last != ze_ || aEjec_last != ae_
//             || beamEk_last != bek_ || xMin_last != ejecMin_ || xMax_last != ejecMax_ || stepWidth_last != nstep_
//             || quietMode_last != qm_ || exEjec_last != exejec_ || exRecoil_last != exrec_ )
	if (zBeam_last != zb_ || aBeam_last != ab_ || zTarget_last != zt_ || aTarget_last != at_ || zEjec_last != ze_ || aEjec_last != ae_) changed = true;

	zBeam_last = zb_;
	aBeam_last = ab_;
	zTarget_last = zt_;
	aTarget_last = at_;
	zEjec_last = ze_;
	aEjec_last = ae_;

	xAxisID_last = xAID_;
	yAxisID_last = yAID_;

	beamEk_last = bek_;
	xMin_last = xMin_;
	xMax_last = xMax_;
	stepWidth_last = stepWidth_;
	quietMode_last = qm_;
	exEjec_last = exejec_;
	exRecoil_last = exrec_;

	return changed;
}

int RootKinCalc::InitReadMassesForKinematic(std::ifstream& mass_db)
{
	if (!mass_db.is_open())
	{
		std::cerr << "No File Found for the Masses Database!\n";
		return -1;
	}

	std::stringstream readMassDB;
	string readLine;

	int massTableLineBeg = -1;

	while (std::getline(mass_db, readLine))
	{
		if (readLine.length() == 0) continue;

		string readWord[17];

		readMassDB.clear();
		readMassDB.str(readLine);

		if (readLine.find("MASS") != string::npos && readLine.find("LIST") != string::npos)
		{
			massTableLineBeg += 5;
			break;
		}
	}

	return massTableLineBeg;
}

void RootKinCalc::GetRelevantInfoPositions(string* readWord, short& posMassExcess, short& posBindingEnergy, short& posBetaDecay, short& posAMU, short& posElement)
{
	std::stringstream checkNumConversion;
	float dummyf;

	checkNumConversion.str(readWord[posElement]);
	checkNumConversion >> dummyf;

	if (!checkNumConversion.fail()) // Our element is a number and not a string!
	{
//         std::cout << Form ( "Did not fail to parse Word#%d as a float ** ", posElement );
		posMassExcess++;
		posElement++;
	}

	checkNumConversion.clear();
	checkNumConversion.str(readWord[posMassExcess]);
	checkNumConversion >> dummyf;

	if (checkNumConversion.fail()) // Our Mass Excess number is not a number!
	{
//         std::cout << Form ( "Failed to parse Word#%d as an float ** ", posMassExcess );
		posMassExcess++;
	}

	posBindingEnergy = posMassExcess + 2;

	posBetaDecay = posBindingEnergy + 3;

	if (readWord[posMassExcess][readWord[posMassExcess].length() - 1] == '#') posMassExcess = -1;

	if (readWord[posBindingEnergy][readWord[posBindingEnergy].length() - 1] == '#') posBindingEnergy = -1;

	if (readWord[posBetaDecay] == "*")
	{
		posAMU = posBetaDecay + 1;
		posBetaDecay = -1;
	}
	else
	{
		posAMU = posBetaDecay + 2;
	}

	if (readWord[posAMU + 1][readWord[posAMU + 1].length() - 1] == '#') posAMU = -1;

	return;
}

void RootKinCalc::DecodeAtomicFormula(std::ifstream& mass_db, string toDecode, int& mass, int& charge, short memberID)
{
	std::vector<int> massDigits;

	string element = "";
	mass = 0;
	charge = -1;

	massDigits.clear();

	if (toDecode == "p")
	{
		mass = 1;
		element = "H";
	}
	else if (toDecode == "d")
	{
		mass = 2;
		element = "H";
	}
	else if (toDecode == "n")
	{
		mass = 1;
		element = "n";
	}
	else if (toDecode == "t")
	{
		mass = 3;
		element = "H";
	}
	else
	{
		for (char c : toDecode)
		{
			if (CharIsDigit(c))
			{
				string digit;
				digit = c;

				massDigits.push_back(std::stoi(digit));
			}
			else
			{
				element += c;
			}
		}

		for (unsigned short i = 0; i < massDigits.size(); i++)
		{
//         std::cout << "Retreived mass digit: " << massDigits[i] << "\n";
			mass += massDigits[i] * pow(10, massDigits.size() - 1 - i);
		}
	}

//     std::cout << "Retreiving the charge of element: " << element << " ...\n";
//     std::cout << "Decoded Mass: " << mass << "\n";

	std::stringstream readMassDB;
	string readLine;

	int massTableLineBeg = InitReadMassesForKinematic(mass_db);

	for (int i = 0; i < massTableLineBeg; i++)
	{
		std::getline(mass_db, readLine);
	}

	while (std::getline(mass_db, readLine))
	{
		readMassDB.clear();
		readMassDB.str(readLine);

		short posMassExcess = 5, posBindingEnergy = -1, posBetaDecay = -1, posAMU = -1, posElement = 4;

		string readWord[17];

//         std::cout << "Read Line:\n";

		for (int i = 0; i < 17; i++)
		{
			readMassDB >> readWord[i];
//             std::cout << readWord[i] << "  ";
		}

//         std::cout << "\n";

		GetRelevantInfoPositions(readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement);

		charge = CheckForMatch<string>(readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement, mass, element, memberID);

		if (charge >= 0) break;
	}

	return;
}

void RootKinCalc::GetAtomicFormula(std::ifstream& mass_db, int mass, int charge, string& toReconstruct, short int memberID)
{
	string element = "";

	if (mass == 1 && charge == 1)
	{
		toReconstruct = "p";
	}
	else if (mass == 2 && charge == 1)
	{
		toReconstruct = "d";
	}
	else if (mass == 1 && charge == 0)
	{
		toReconstruct = "n";
	}
	else if (mass == 3 && charge == 1)
	{
		toReconstruct = "t";
	}
	else
	{
		std::stringstream readMassDB;
		string readLine;

		int massTableLineBeg = InitReadMassesForKinematic(mass_db);

		for (int i = 0; i < massTableLineBeg; i++)
		{
			std::getline(mass_db, readLine);
		}

		while (std::getline(mass_db, readLine))
		{
			readMassDB.clear();
			readMassDB.str(readLine);

			short posMassExcess = 5, posBindingEnergy = -1, posBetaDecay = -1, posAMU = -1, posElement = 4;

			string readWord[17];

//         std::cout << "Read Line:\n";

			for (int i = 0; i < 17; i++)
			{
				readMassDB >> readWord[i];
//             std::cout << readWord[i] << "  ";
			}

//         std::cout << "\n";

			GetRelevantInfoPositions(readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement);

			if (charge == std::stoi(readWord[posElement - 2]))
			{
				char* massChar = new char[4];
				sprintf(massChar, "%i", mass);

				toReconstruct = massChar;
				toReconstruct += readWord[posElement];

				break;
			}
		}

		return;
	}
}

string RootKinCalc::GetAtomicFormula(int mass, string element)
{
	char* atomicFormulaChar = new char[12];

	if (element == "n") return "n";
	else if (element == "H" && mass == 1) return "p";
	else if (element == "H" && mass == 2) return "d";
	else if (element == "H" && mass == 3) return "t";
	else sprintf(atomicFormulaChar, "%i%s", mass, element.c_str());

	string atomicFormula = atomicFormulaChar;

	return atomicFormula;
}

void RootKinCalc::GetMassesForKinematic(int charge, int mass, short memberID)
{
	std::ifstream mass_db("./mass_db.dat");

	if (!mass_db.is_open())
	{
		std::cerr << "No File Found for the Masses Database!\n";
		return;
	}

	std::stringstream readMassDB;
	string readLine;

	int massTableLineBeg = InitReadMassesForKinematic(mass_db);

	for (int i = 0; i < massTableLineBeg; i++)
	{
		std::getline(mass_db, readLine);
	}

	while (std::getline(mass_db, readLine))
	{
		readMassDB.clear();
		readMassDB.str(readLine);

		short posMassExcess = 5, posBindingEnergy = -1, posBetaDecay = -1, posAMU = -1, posElement = 4;

		string readWord[17];

		for (int i = 0; i < 17; i++)
		{
			readMassDB >> readWord[i];
		}

		GetRelevantInfoPositions(readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement);

		CheckForMatch<int>(readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement, mass, charge, memberID);
	}

	return;
}

std::tuple<int, int> RootKinCalc::GetMassesForKinematic(string particle, short memberID)
{
	std::tuple<int, int> output;

	std::ifstream mass_db("./mass_db.dat");

	if (!mass_db.is_open())
	{
		std::cerr << "No File Found for the Masses Database!\n";
		return output;
	}

	int charge, mass;

	DecodeAtomicFormula(mass_db, particle, mass, charge, memberID);
//     std::cout << "Decoded Charge: " << charge << "\n";

	mass_db.seekg(0);

	std::get<0>(output) = charge;
	std::get<1>(output) = mass;

	return output;
}

void RootKinCalc::GetBaseKinematicInfo(int zBeam, int aBeam, int zTarget, int aTarget, int zEjec, int aEjec, float beamEk_, float exEjec_, float exRecoil_, bool invertEjecRec,
		bool invertLabCMEn)
{
	if (zBeam < 0 || aBeam <= 0 || zTarget < 0 || aTarget <= 0 || zEjec < 0 || aEjec <= 0 || beamEk_ < 0 || exEjec_ < 0 || exRecoil_ < 0)
	{
		cerr << "Invalid Input...\n";
		return;
	}

	float amu = 931.502; // MeV

	int zRecoil, aRecoil;

	if (!invertEjecRec)
	{
		zRecoil = zBeam + zTarget - zEjec;
		aRecoil = aBeam + aTarget - aEjec;
	}
	else
	{
		zRecoil = zEjec;
		aRecoil = aEjec;

		zEjec = zBeam + zTarget - zRecoil;
		aEjec = aBeam + aTarget - aRecoil;
	}

	rInfo->ReinitMasses();

	rInfo->A["beam"] = aBeam;
	rInfo->A["target"] = aTarget;
	rInfo->A["ejectile"] = aEjec;
	rInfo->A["recoil"] = aRecoil;
	rInfo->Z["beam"] = zBeam;
	rInfo->Z["target"] = zTarget;
	rInfo->Z["ejectile"] = zEjec;
	rInfo->Z["recoil"] = zRecoil;

	GetMassesForKinematic(zBeam, aBeam, 0);
	GetMassesForKinematic(zTarget, aTarget, 1);
	GetMassesForKinematic(zEjec, aEjec, 2);
	GetMassesForKinematic(zRecoil, aRecoil, 3);

	string beamElement = (zBeam == 0 ? "n" : ((zBeam == 1 && aBeam == 1) ? "p" : rInfo->atomicElement["beam"]));
	string targetElement = (zTarget == 0 ? "n" : ((zTarget == 1 && aTarget == 1) ? "p" : rInfo->atomicElement["target"]));
	string ejecElement = (zEjec == 0 ? "n" : ((zEjec == 1 && aEjec == 1) ? "p" : rInfo->atomicElement["ejectile"]));
	string recoilElement = (zRecoil == 0 ? "n" : ((zRecoil == 1 && aRecoil == 1) ? "p" : rInfo->atomicElement["recoil"]));

	massBeam = rInfo->atomicMassUnit["beam"] * 1e-6 * amu; // MeV
	massTarget = rInfo->atomicMassUnit["target"] * 1e-6 * amu; // MeV
	massEjec = rInfo->atomicMassUnit["ejectile"] * 1e-6 * amu + exEjec; // MeV
	massRecoil = rInfo->atomicMassUnit["recoil"] * 1e-6 * amu + exRecoil; // MeV

	totMassInput = massBeam + massTarget;

	if (!invertLabCMEn)
	{
		beamEkLab = beamEk_;
		beamEkCM = beamEkLab * massTarget / (massBeam + massTarget);
	}
	else
	{
		beamEkCM = beamEk_;
		beamEkLab = beamEkCM * (massBeam + massTarget) / massTarget;
	}

	exEjec = exEjec_;
	exRecoil = exRecoil_;

	char* mapKeyChar = new char[1024];
	sprintf(mapKeyChar, "%d%s(%d%s,%d%s)%d%s @%4.3f MeV (E*ejec = %4.3f / E*rec = %4.3f)", aTarget, targetElement.c_str(), aBeam, beamElement.c_str(), aEjec, ejecElement.c_str(),
			aRecoil, recoilElement.c_str(), beamEkLab, exEjec, exRecoil);
	mapKey = mapKeyChar;

	betaC = sqrt(beamEkLab * (beamEkLab + 2 * massBeam)) / (totMassInput + beamEkLab);

	qValueGS = (rInfo->massExcess["beam"] + rInfo->massExcess["target"] - rInfo->massExcess["ejectile"] - rInfo->massExcess["recoil"]) / 1000.;
	qValueFinal = qValueGS - exEjec - exRecoil;

	reacAboveThr = (beamEkCM + qValueGS) > 0;

	if (!reacAboveThr) return;

	eCMi = sqrt(totMassInput * totMassInput + (2 * beamEkLab * massTarget));
	eCMf = eCMi + qValueFinal - totMassInput + massEjec + massRecoil;

	e3CM = (eCMf * eCMf + (massEjec + massRecoil) * (massEjec - massRecoil)) / (2 * eCMf);

	beta3C = sqrt(1 - ((massEjec * massEjec) / (e3CM * e3CM)));

	yNew = pow(e3CM / massEjec, 2) * (1 - betaC * betaC);
}

void RootKinCalc::GetBaseKinematicInfo(string beam, string target, string ejectile, float beamEk_, float exEjec_, float exRecoil_, bool invertEjecRec, bool invertLabCMEn)
{
	std::ifstream mass_db("./mass_db.dat");

	if (!mass_db.is_open())
	{
		std::cerr << "No File Found for the Masses Database!\n";
		return;
	}

	int zBeam, aBeam, zTarget, aTarget, zEjec, aEjec;

	DecodeAtomicFormula(mass_db, beam, aBeam, zBeam, 0);
	mass_db.seekg(0);
	DecodeAtomicFormula(mass_db, target, aTarget, zTarget, 1);
	mass_db.seekg(0);
	DecodeAtomicFormula(mass_db, ejectile, aEjec, zEjec, 2);
	mass_db.seekg(0);

	GetBaseKinematicInfo(zBeam, aBeam, zTarget, aTarget, zEjec, aEjec, beamEk_, exEjec_, exRecoil_, invertEjecRec, invertLabCMEn);
}

void RootKinCalc::CalcKinematic(float ejecLabAngle_)
{
	float dtr = M_PI / 180.;
	float rtd = 180. / M_PI;

	KinCalcRes* kcr = new KinCalcRes();

	kcr->ejecLabAngle = ejecLabAngle_;
	kcr->ejecLabAngle2 = ejecLabAngle_;

	kcr->cosagl = cos(ejecLabAngle_ * dtr);

	kcr->b = -betaC * kcr->cosagl;
	kcr->a = yNew + kcr->b * kcr->b;
	kcr->c = 1 - yNew;

	kcr->d__2 = kcr->b * kcr->b - kcr->a * kcr->c;

	if (kcr->d__2 < 0 || kcr->a == 0)
	{
		kcr->b3L1 = -100;
		kcr->b3L2 = -100;
	}
	else
	{
		kcr->b3L1 = (-kcr->b + sqrt(kcr->d__2)) / kcr->a;
		kcr->b3L2 = (-kcr->b - sqrt(kcr->d__2)) / kcr->a;
	}

	if (1 - kcr->b3L1 * kcr->b3L1 <= 0 || kcr->b3L1 <= 0) kcr->ejecLabEnergy = -1000;
	else kcr->ejecLabEnergy = massEjec * (1 / sqrt(1 - kcr->b3L1 * kcr->b3L1) - 1);

	if (1 - kcr->b3L2 * kcr->b3L2 <= 0 || kcr->b3L2 <= 0) kcr->ejecLabEnergy2 = -1000;
	else kcr->ejecLabEnergy2 = massEjec * (1 / sqrt(1 - kcr->b3L2 * kcr->b3L2) - 1);

	if (kcr->ejecLabEnergy > 0)
	{
		kcr->ejecCMAngle = acos((kcr->b3L1 * kcr->cosagl - betaC) / ((1 - betaC * kcr->b3L1 * kcr->cosagl) * beta3C)) * rtd;

		kcr->recoilLabEnergy = beamEkLab + qValueFinal - kcr->ejecLabEnergy;

		kcr->recoilLabAngle = (asin(
				sqrt((kcr->ejecLabEnergy * (kcr->ejecLabEnergy + 2 * massEjec)) / (kcr->recoilLabEnergy * (kcr->recoilLabEnergy + 2 * massRecoil))) * sin(ejecLabAngle_ * dtr)))
				* rtd;

		kinRes[ejecLabAngle_] = *kcr;
	}

	if (kcr->ejecLabEnergy2 > 0)
	{
		kcr->ejecCMAngle2 = acos((kcr->b3L2 * kcr->cosagl - betaC) / ((1 - betaC * kcr->b3L2 * kcr->cosagl) * beta3C)) * rtd;

		kcr->recoilLabEnergy2 = beamEkLab + qValueFinal - kcr->ejecLabEnergy2;

		kcr->recoilLabAngle2 = (asin(
				sqrt((kcr->ejecLabEnergy2 * (kcr->ejecLabEnergy2 + 2 * massEjec)) / (kcr->recoilLabEnergy2 * (kcr->recoilLabEnergy2 + 2 * massRecoil))) * sin(ejecLabAngle_ * dtr)))
				* rtd;

		kinRes[ejecLabAngle_] = *kcr;
	}

	delete (kcr);

	return;
}

pair<vector<double>, vector<double>> RootKinCalc::PlotKinematicGraph(string xAxisID, string yAxisID, float xMin, float xMax, float stepWidth)
{
	double x_ = xMin;

	vector<double> x_temp, y_temp;
	x_temp.clear();
	y_temp.clear();

	for (auto itr = kinRes.begin(); itr != kinRes.end(); itr++)
	{
//         std::cout << GetKinResIDValue ( &reacItr->second[i], xAxisID ) << " <-> " << GetKinResIDValue ( &reacItr->second[i], yAxisID ) << "\n";
		if (GetKinResIDValue(itr->second, xAxisID) > 0 && GetKinResIDValue(itr->second, yAxisID) > 0)
		{
			x_temp.push_back(GetKinResIDValue(itr->second, xAxisID));
			y_temp.push_back(GetKinResIDValue(itr->second, yAxisID));
//             cout << "Pushed back: " << GetKinResIDValue ( itr->second, xAxisID ) << " , " << GetKinResIDValue ( itr->second, yAxisID ) << "\n";
		}
	}

//     cout << "***********************************************\n";

	vector<double> xVect, yVect;
	xVect.clear();
	yVect.clear();

	if (x_temp.size() == 0 || y_temp.size() == 0)
	{
		cerr << "Something went wrong... Unabled to find matching reaction in processed reaction list." << endl;
		cerr << "Please try to \"Get Reac. Info\" or \"Get Reac. Kinematics\" again and retry." << endl;
		return make_pair(xVect, yVect);
	}

	while (x_ <= xMax)
	{
//         cout << x_ << " <-> " << EvalGraph ( x_temp, y_temp, x_ ) << "\n";

		double evalX = EvalGraph(x_temp, y_temp, x_);

		if (x_ > 0 && evalX > 0)
		{
			xVect.push_back(x_);
			yVect.push_back(evalX);
		}

		x_ += stepWidth;
	}

	return make_pair(xVect, yVect);
}

float RootKinCalc::ConvertSingleValue(string fromQuantity, string toQuantity, float val)
{
	vector<double> x_temp, y_temp;
	x_temp.clear();
	y_temp.clear();

	for (auto itr = kinRes.begin(); itr != kinRes.end(); itr++)
	{
		x_temp.push_back(GetKinResIDValue(itr->second, fromQuantity));
		y_temp.push_back(GetKinResIDValue(itr->second, toQuantity));
	}

//     std::cout << fromQuantity << ": " << val << "  <--->  " << tempGr->Eval ( val ) << " :" << toQuantity << "\n";

	if (x_temp.size() == 0 || y_temp.size() == 0)
	{
		cerr << "Something went wrong... Unabled to find matching reaction in processed reaction list." << endl;
		cerr << "Please try to \"Get Reac. Info\" or \"Get Reac. Kinematics\" again and retry." << endl;
		return 0.0;
	}

	return EvalGraph(x_temp, y_temp, (double) val);
}

