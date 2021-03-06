#ifndef ROOTKINCALC_H
#define ROOTKINCALC_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <streambuf>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <type_traits>
#include <cxxabi.h>
#include <pthread.h>
#include <iomanip>
#include <cmath>
#include "math.h"
#include <cstdint>

#include <sys/types.h>

using namespace std;

class ReacInfo
{
	private:
		string mapKeys[4];

	public:
		ReacInfo();

		~ReacInfo()
		{
		}

		string GetKey(int keyNum);

		//These maps are designed to hold 4 keys later on: "beam", "target", "ejectile" and "recoil"
		map<string, int> A;
		map<string, int> Z;
		map<string, float> massExcess; // keV
		map<string, float> deltaMassExcess; // keV
		map<string, float> bindEnPerA; // keV
		map<string, float> deltaBindEnPerA; // keV
		map<string, float> betaDecayEn; // keV
		map<string, float> deltaBetaDecayEn; // keV
		map<string, float> atomicMassUnit; // micro-u
		map<string, float> vOVERs;
		map<string, string> atomicElement;

		void ReinitMasses();
};

struct KinCalcRes
{
		float ejecLabAngle;
		float ejecLabAngle2;

		float cosagl;
		float b;
		float a;
		float c;
		float d__2;
		float b3L1;
		float b3L2;

		float ejecLabEnergy;
		float ejecLabEnergy2;

		float ejecCMAngle;
		float ejecCMAngle2;

		float recoilLabEnergy;
		float recoilLabEnergy2;

		float recoilLabAngle;
		float recoilLabAngle2;
};

class RootKinCalc
{
	private:

	public:
		RootKinCalc();
		~RootKinCalc()
		{
		}

		ReacInfo* rInfo;

		string mapKey;

		float beamEkLab;
		float exEjec;
		float exRecoil;

		float massBeam;
		float massTarget;
		float massEjec;
		float massRecoil;

		float totMassInput;

		float beamEkCM;

		bool reacAboveThr;

		float betaC;

		float qValueGS;
		float qValueFinal;

		float eCMi;
		float eCMf;

		float e3CM;

		float beta3C;

		float yNew;

		std::map<float, KinCalcRes> kinRes;

		static RootKinCalc* OnClickUpdateInfo();
		static void OnClickCalcKin();
		static void OnClickWriteTable();
		static void OnClickProcessSC();

		static bool AssignLastUsedValues(float zb_, float ab_, float zt_, float at_, float ze_, float ae_, string xAID_, string yAID_, float bek_, float xMin_, float xMax_,
				float stepWidth_, bool qm_, float exejec_, float exrec_);

		int InitReadMassesForKinematic(std::ifstream& mass_db);

		void DecodeAtomicFormula(std::ifstream& mass_db, string toDecode, int& mass, int& charge, short memberID);
		void GetAtomicFormula(std::ifstream& mass_db, int mass, int charge, string& toReconstruct, short memberID);
		static string GetAtomicFormula(int mass, string element);

		void GetRelevantInfoPositions(string* readWord, short& posMassExcess, short& posBindingEnergy, short& posBetaDecay, short& posAMU, short& posElement);

		template<typename T2> inline int CheckForMatch(string* readWord, short posMassExcess, short posBindingEnergy, short posBetaDecay, short posAMU, short posElement,
				short massCheck, T2 chargeCheck, short memberID);

		void GetMassesForKinematic(int charge, int mass, short memberID);

		std::tuple<int, int> GetMassesForKinematic(string particle, short memberID);

		void GetBaseKinematicInfo(int zBeam, int aBeam, int zTarget, int aTarget, int zEjec, int aEjec, float beamEk_, float exEjec_ = 0.0, float exRecoil_ = 0.0,
				bool invertEjecRec = false, bool invertLabCMEn = false);
		void GetBaseKinematicInfo(string beam, string target, string ejectile, float beamEk_, float exEjec_ = 0.0, float exRecoil_ = 0.0, bool invertEjecRec = false,
				bool invertLabCMEn = false);

		void CalcKinematic(float ejecLabAngle_);

		pair<vector<double>, vector<double>> PlotKinematicGraph(string xAxisID, string yAxisID, float xMin, float xMax, float stepWidth);

		float ConvertSingleValue(string fromQuantity, string toQuantity, float val);

		static void WriteTableToFile(short reactionID, float xMin, float xMax, float precision);
};

template<typename T> bool IsSameValue(T a_, T b_)
{
	return a_ == b_;
}

template<typename T1, typename T2> bool IsSameValue(T1 a_, T2 b_)
{
	if (!std::is_same<T1, T2>::value) return false;
	else return IsSameValue<T1>(a_, b_);
}

template<typename T2> inline int RootKinCalc::CheckForMatch(string* readWord, short posMassExcess, short posBindingEnergy, short posBetaDecay, short posAMU, short posElement,
		short massCheck, T2 chargeCheck, short memberID)
{
//     std::cout << "Performing the CheckForMatch function with: " << posMassExcess << " / " << posBindingEnergy << " / " << posBetaDecay << " / " << posAMU;
//     std::cout << " / " << posElement << " / " << massCheck << " / " << chargeCheck << " / " << memberID << " / "<< "\n";

	int charge = -1;

	int foundMatch = -1;

	if (std::stoi(readWord[posElement - 1]) == massCheck)
	{
		if (std::is_same<int, decltype ( chargeCheck )>::value)
		{
			if (IsSameValue(std::stoi(readWord[posElement - 2]), chargeCheck)) foundMatch = memberID;
		}
		else if (std::is_same<string, decltype ( chargeCheck )>::value)
		{
//             std::cout << "Searching Element by Atomic Symbol " << chargeCheck << " ...\n";

			if (IsSameValue(readWord[posElement], chargeCheck))
			{
				foundMatch = memberID;
			}
		}
	}

	if (foundMatch >= 0)
	{
//         std::cout << "Found a matching pattern: " << foundMatch << " ...\n";

		charge = std::stoi(readWord[posElement - 2]);

//         std::cout << "Decoded charge: " << charge << " ...\n";

		string rMapKey = rInfo->GetKey(foundMatch);

		if (posMassExcess >= 5)
		{
			rInfo->massExcess[rMapKey] = std::stof(readWord[posMassExcess]);
			rInfo->deltaMassExcess[rMapKey] = std::stof(readWord[posMassExcess + 1]);
// 	    std::cout << "Decoded Mass Excess: " << rInfo->massExcess[rMapKey] << " +/- " << rInfo->deltaMassExcess[rMapKey] << " ...\n";
		}
		if (posBindingEnergy >= 7)
		{
//             std::cout << "Decoded Binding Energy: " << std::stof ( readWord[posBindingEnergy] ) << " +/- " << std::stof ( readWord[posBindingEnergy+1] ) << " ...\n";
			rInfo->bindEnPerA[rMapKey] = std::stof(readWord[posBindingEnergy]);
			rInfo->deltaBindEnPerA[rMapKey] = std::stof(readWord[posBindingEnergy + 1]);
		}
		if (posBetaDecay >= 10)
		{
//             std::cout << "Decoded Beta Decay: " << std::stof ( readWord[posBetaDecay] ) << " +/- " << std::stof ( readWord[posBetaDecay+1] ) << " ...\n";
			rInfo->betaDecayEn[rMapKey] = std::stof(readWord[posBetaDecay]);
			rInfo->deltaBetaDecayEn[rMapKey] = std::stof(readWord[posBetaDecay + 1]);
		}
		if (posAMU >= 11)
		{
//             std::cout << "Decoded AMU: " << std::stof ( readWord[posAMU] ) << " +/- " << std::stof ( readWord[posAMU+1] ) << " ...\n";

			float mainVal, decVal;

			mainVal = std::stof(readWord[posAMU]);
			decVal = std::stof(readWord[posAMU + 1]);

			rInfo->atomicMassUnit[rMapKey] = mainVal * 1e6 + decVal;
			rInfo->vOVERs[rMapKey] = std::stof(readWord[posAMU + 2]);
		}

//         std::cout << "Decoded Element: " << readWord[posElement] << " ...\n";

		rInfo->atomicElement[rMapKey] = readWord[posElement];
	}

	return charge;
}

double EvalGraph(vector<double> x_, vector<double> y_, double toEval);
bool CharIsDigit(char toCheck);
float GetKinResIDValue(KinCalcRes kcr, string ID);
string GetKinResIDString(short ID);

#endif

