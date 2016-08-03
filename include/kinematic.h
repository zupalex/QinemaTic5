#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <type_traits>
#include <cxxabi.h>
#include <pthread.h>
#include <iomanip>

using std::string;

#include "TROOT.h"
#include "TClass.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "THashList.h"
#include "TAxis.h"
#include "TLegend.h"
#include "TFrame.h"
#include "TMultiGraph.h"
#include "TGListBox.h"
#include "TGWindow.h"
#include "TGButton.h"
#include "TGFrame.h"
#include "TGClient.h"
#include "TGButtonGroup.h"
#include <TGLabel.h>
#include <TGLayout.h>
#include <TGDimension.h>
#include <TGText.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include "RQ_OBJECT.h"

#ifndef ROOTKINCALC_H
#define ROOTKINCALC_H

class ReacInfo
{
private:

public:
    ReacInfo();

    ~ReacInfo() {}

    //[0] == Beam, [1] == Target, [2] == Ejectile, [3] == Recoil
    float massExcess[4]; // keV
    float deltaMassExcess[4]; // keV
    float bindEnPerA[4]; // keV
    float deltaBindEnPerA[4]; // keV
    float betaDecayEn[4]; // keV
    float deltaBetaDecayEn[4]; // keV
    float atomicMassUnit[4]; // micro-u
    float vOVERs[4];
    string atomicElement[4];

    void ReinitMasses();
};

class RootKinCalc
{
private:

public:
    RootKinCalc();
    ~RootKinCalc() {}

    ReacInfo* rInfo;

    float massBeam;
    float massTarget;
    float massEjec;
    float massRecoil;

    float totMassInput;

    float betaC;

    float cosagl;

    float qValueGS;
    float qValueFinal;

    float eCMi;
    float eCMf;

    float e3CM;

    float beta3C;

    float yNew;

    float b;
    float a;
    float c;
    float d__2;
    float b3L1;

    float ejecLabAngle;
    float ejecLabEnergy;
    float ejecCMAngle;
    float recoilLabEnergy;
    float recoilLabAngle;
    float recoilCMAngle;

    static void OnClickCalcKin();
    static void OnClickPlotGraphs();

    static bool AssignLastUsedValues ( float zb_, float ab_, float zt_, float at_, float ze_, float ae_,
                                string xAID_, string yAID_, float bek_,
                                float xMin_, float xMax_, float stepWidth_, bool qm_, float exejec_, float exrec_ );

    int InitReadMassesForKinematic ( std::ifstream& mass_db );

    void DecodeAtomicFormula ( std::ifstream& mass_db, string toDecode, int& mass, int& charge, short memberID );

    void GetRelevantInfoPositions ( string* readWord, short& posMassExcess, short& posBindingEnergy, short& posBetaDecay, short& posAMU, short& posElement );

    template<typename T2> inline int CheckForMatch ( string* readWord, short posMassExcess, short posBindingEnergy, short posBetaDecay, short posAMU, short posElement,
            short massCheck, T2 chargeCheck, short memberID );

    void GetMassesForKinematic ( int charge, int mass, short memberID );

    std::tuple<int, int> GetMassesForKinematic ( string particle, short memberID );

    void CalcKinematic ( float beamEk, float ejecLabAngle, float exEjec = 0.0, float exRecoil = 0.0 );

    void GetReactionKinematic ( int zBeam, int aBeam, int zTarget, int aTarget, int zEjec, int aEjec, float beamEk, float exEjec, float exRecoil );
    void GetReactionKinematic ( string beam, string target, string ejectile, float beamEk, float exEjec, float exRecoil );
    void GetReactionKinematic ( float beamEk = -1 );

    static TGraph* PlotKinematicGraph ( short reactionID, string xAxisID, string yAxisID, float xMin, float xMax, float stepWidth, bool doDraw = true );

    static TGraph* PlotKinematicGraph ( string opt = "" );

    float ConvertSingleValue ( short reactionID, string fromQuantity, string toQuantity, float val );
    float ConvertSingleValue ( float val = -1 );

    void Dump ( short reactionID, short entry );

    void WriteTableToFile ( short reactionID, float xMin, float xMax, float precision );

    ClassDef ( RootKinCalc, 1 );
};

template<typename T2> inline int RootKinCalc::CheckForMatch ( string* readWord, short posMassExcess, short posBindingEnergy, short posBetaDecay, short posAMU, short posElement,
        short massCheck, T2 chargeCheck, short memberID )
{
//     std::cout << "Performing the CheckForMatch function with: " << posMassExcess << " / " << posBindingEnergy << " / " << posBetaDecay << " / " << posAMU;
//     std::cout << " / " << posElement << " / " << massCheck << " / " << chargeCheck << " / " << memberID << " / "<< "\n";

    int charge = -1;

    int foundMatch = -1;

    if ( std::stoi ( readWord[posElement-1] ) == massCheck )
    {
        if ( std::is_same<int, decltype ( chargeCheck ) >::value )
        {
            if ( std::stoi ( readWord[posElement-2] ) == chargeCheck ) foundMatch = memberID;
        }
        else if ( std::is_same<string, decltype ( chargeCheck ) >::value )
        {
//             std::cout << "Searching Element by Atomic Symbol " << chargeCheck << " ...\n";

            if ( readWord[posElement] == chargeCheck )
            {
                foundMatch = memberID;
            }
        }
    }

    if ( foundMatch >= 0 )
    {
//         std::cout << "Found a matching pattern: " << foundMatch << " ...\n";

        charge = std::stoi ( readWord[posElement-2] );

//         std::cout << "Decoded charge: " << charge << " ...\n";

        if ( posMassExcess >= 5 )
        {
            rInfo->massExcess[foundMatch] = std::stof ( readWord[posMassExcess] );
            rInfo->deltaMassExcess[foundMatch] = std::stof ( readWord[posMassExcess+1] );
// 	    std::cout << "Decoded Mass Excess: " << rInfo->massExcess[foundMatch] << " +/- " << rInfo->deltaMassExcess[foundMatch] << " ...\n";
        }
        if ( posBindingEnergy >= 7 )
        {
//             std::cout << "Decoded Binding Energy: " << std::stof ( readWord[posBindingEnergy] ) << " +/- " << std::stof ( readWord[posBindingEnergy+1] ) << " ...\n";
            rInfo->bindEnPerA[foundMatch] = std::stof ( readWord[posBindingEnergy] );
            rInfo->deltaBindEnPerA[foundMatch] = std::stof ( readWord[posBindingEnergy+1] );
        }
        if ( posBetaDecay >= 10 )
        {
//             std::cout << "Decoded Beta Decay: " << std::stof ( readWord[posBetaDecay] ) << " +/- " << std::stof ( readWord[posBetaDecay+1] ) << " ...\n";
            rInfo->betaDecayEn[foundMatch] = std::stof ( readWord[posBetaDecay] );
            rInfo->deltaBetaDecayEn[foundMatch] = std::stof ( readWord[posBetaDecay+1] );
        }
        if ( posAMU >= 11 )
        {
//             std::cout << "Decoded AMU: " << std::stof ( readWord[posAMU] ) << " +/- " << std::stof ( readWord[posAMU+1] ) << " ...\n";

            float mainVal, decVal;

            mainVal = std::stof ( readWord[posAMU] );
            decVal = std::stof ( readWord[posAMU+1] );

            rInfo->atomicMassUnit[foundMatch] = mainVal*1e6 + decVal;
            rInfo->vOVERs[foundMatch] = std::stof ( readWord[posAMU+2] );
        }

//         std::cout << "Decoded Element: " << readWord[posElement] << " ...\n";

        rInfo->atomicElement[foundMatch] = readWord[posElement];
    }

    return charge;
}

extern std::map<string, std::vector<RootKinCalc>> kinResMap;

TGWindow* FindWindowByName ( std::string winName );
TGFrame* FindFrameByName ( TGCompositeFrame* pFrame, std::string frameName );

bool CharIsDigit ( char toCheck );
void DisplayListOfReactions();
void DecodeAtomicFormula ( std::ifstream& mass_db, string toDecode, int& mass, int& charge, short memberID );
float GetKinResIDValue ( RootKinCalc* kinRes, string ID );
string GetKinResIDString ( short ID );

void UpdateReactionListBox ( TGListBox* lb );

#endif
