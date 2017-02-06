#include "kinematic.h"

using std::string;

int zBeam_last, aBeam_last, zTarget_last, aTarget_last, zEjec_last, aEjec_last;
string xAxisID_last, yAxisID_last;
float beamEk_last, xMin_last, xMax_last;
float stepWidth_last;
int quietMode_last;
float exEjec_last, exRecoil_last;

bool CharIsDigit ( char toCheck )
{
    if ( toCheck == '0' || toCheck == '1' || toCheck == '2' || toCheck == '3' || toCheck == '4' || toCheck == '5' || toCheck == '6' || toCheck == '7' || toCheck == '8' || toCheck == '9' )
        return true;
    else return false;
}

float GetKinResIDValue ( KinCalcRes kcr, string ID )
{
    if ( ID == "Ejec. Lab. Angle" ) return kcr.ejecLabAngle;
    else if ( ID == "Ejec. Lab. Energy" ) return kcr.ejecLabEnergy;
    else if ( ID == "C.M. Angle" ) return kcr.ejecCMAngle;
    else if ( ID == "Recoil Lab. Angle" ) return kcr.recoilLabAngle;
    else if ( ID == "Recoil Lab. Energy" ) return kcr.recoilLabEnergy;
    else return -1;
}

string GetKinResIDString ( short ID )
{
    if ( ID == 0 ) return "Ejec. Lab. Angle";
    else if ( ID == 1 ) return "Ejec. Lab. Energy";
    else if ( ID == 2 ) return "C.M. Angle";
    else if ( ID == 3 ) return "Recoil Lab. Angle";
    else if ( ID == 4 ) return "Recoil Lab. Energy";
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
    string keys[4] = {"beam", "target", "ejectile", "recoil"};

    for ( short i = 0; i < 4; i++ )
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

string ReacInfo::GetKey ( int keyNum )
{
    return mapKeys[keyNum];
}

RootKinCalc::RootKinCalc()
{
    rInfo = new ReacInfo();
}

bool RootKinCalc::AssignLastUsedValues ( float zb_, float ab_, float zt_, float at_, float ze_, float ae_,
        string xAID_, string yAID_, float bek_,
        float xMin_, float xMax_, float stepWidth_, bool qm_, float exejec_, float exrec_ )
{
    bool changed = false;

//     if ( zBeam_last != zb_ || aBeam_last != ab_ || zTarget_last != zt_ || aTarget_last != at_ || zEjec_last != ze_ || aEjec_last != ae_
//             || beamEk_last != bek_ || xMin_last != ejecMin_ || xMax_last != ejecMax_ || stepWidth_last != nstep_
//             || quietMode_last != qm_ || exEjec_last != exejec_ || exRecoil_last != exrec_ )
    if ( zBeam_last != zb_ || aBeam_last != ab_ || zTarget_last != zt_ || aTarget_last != at_ || zEjec_last != ze_ || aEjec_last != ae_ )
        changed = true;

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

int RootKinCalc::InitReadMassesForKinematic ( std::ifstream& mass_db )
{
    if ( !mass_db.is_open() )
    {
        std::cerr << "No File Found for the Masses Database!\n";
        return -1;
    }

    std::stringstream readMassDB;
    string readLine;

    int massTableLineBeg = -1;

    while ( std::getline ( mass_db, readLine ) )
    {
        if ( readLine.length() == 0 ) continue;

        string readWord[17];

        readMassDB.clear();
        readMassDB.str ( readLine );

        if ( readLine.find ( "MASS" ) != string::npos &&  readLine.find ( "LIST" ) != string::npos )
        {
            massTableLineBeg += 5;
            break;
        }
    }

    return massTableLineBeg;
}

void RootKinCalc::GetRelevantInfoPositions ( string* readWord, short& posMassExcess, short& posBindingEnergy, short& posBetaDecay, short& posAMU, short& posElement )
{
    std::stringstream checkNumConversion;
    float dummyf;

    checkNumConversion.str ( readWord[posElement] );
    checkNumConversion >> dummyf;

    if ( !checkNumConversion.fail() ) // Our element is a number and not a string!
    {
//         std::cout << Form ( "Did not fail to parse Word#%d as a float ** ", posElement );
        posMassExcess++;
        posElement++;
    }

    checkNumConversion.clear();
    checkNumConversion.str ( readWord[posMassExcess] );
    checkNumConversion >> dummyf;

    if ( checkNumConversion.fail() ) // Our Mass Excess number is not a number!
    {
//         std::cout << Form ( "Failed to parse Word#%d as an float ** ", posMassExcess );
        posMassExcess++;
    }

    posBindingEnergy = posMassExcess+2;

    posBetaDecay = posBindingEnergy+3;

    if ( readWord[posMassExcess][readWord[posMassExcess].length()-1] == '#' ) posMassExcess = -1;

    if ( readWord[posBindingEnergy][readWord[posBindingEnergy].length()-1] == '#' ) posBindingEnergy = -1;

    if ( readWord[posBetaDecay] == "*" )
    {
        posAMU = posBetaDecay+1;
        posBetaDecay = -1;
    }
    else
    {
        posAMU = posBetaDecay+2;
    }

    if ( readWord[posAMU+1][readWord[posAMU+1].length()-1] == '#' ) posAMU = -1;

    return;
}

void RootKinCalc::DecodeAtomicFormula ( std::ifstream& mass_db, string toDecode, int& mass, int& charge, short memberID )
{
    std::vector<int> massDigits;

    string element = "";
    mass = 0;
    charge = -1;

    massDigits.clear();

    if ( toDecode == "p" )
    {
        mass = 1;
        element = "H";
    }
    else if ( toDecode == "d" )
    {
        mass = 2;
        element = "H";
    }
    else if ( toDecode == "n" )
    {
        mass = 1;
        element = "n";
    }
    else if ( toDecode == "t" )
    {
        mass = 3;
        element = "H";
    }
    else
    {
        for ( int i = 0; i < toDecode.length(); i++ )
        {
            if ( CharIsDigit ( toDecode[i] ) )
            {
                string digit;
                digit = toDecode[i];

                massDigits.push_back ( std::stoi ( digit ) );
            }
            else
            {
                element += toDecode[i];
            }
        }

        for ( unsigned short i = 0; i < massDigits.size(); i++ )
        {
//         std::cout << "Retreived mass digit: " << massDigits[i] << "\n";
            mass += massDigits[i] * pow ( 10, massDigits.size() - 1 - i );
        }
    }

//     std::cout << "Retreiving the charge of element: " << element << " ...\n";
//     std::cout << "Decoded Mass: " << mass << "\n";

    std::stringstream readMassDB;
    string readLine;

    int massTableLineBeg = InitReadMassesForKinematic ( mass_db );

    for ( int i = 0; i < massTableLineBeg; i++ )
    {
        std::getline ( mass_db, readLine );
    }

    while ( std::getline ( mass_db, readLine ) )
    {
        readMassDB.clear();
        readMassDB.str ( readLine );

        short posMassExcess = 5, posBindingEnergy = -1, posBetaDecay = -1, posAMU = -1, posElement = 4;

        string readWord[17];

//         std::cout << "Read Line:\n";

        for ( int i = 0; i < 17; i++ )
        {
            readMassDB >> readWord[i];
//             std::cout << readWord[i] << "  ";
        }

//         std::cout << "\n";

        GetRelevantInfoPositions ( readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement );

        charge = CheckForMatch<string> ( readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement, mass, element, memberID );

        if ( charge >= 0 ) break;
    }

    return;
}

void RootKinCalc::GetAtomicFormula ( std::ifstream& mass_db, int mass, int charge, string& toReconstruct, short int memberID )
{
    string element = "";

    if ( mass == 1 && charge == 1 )
    {
        toReconstruct = "p";
    }
    else if ( mass == 2 && charge == 1 )
    {
        toReconstruct = "d";
    }
    else if ( mass == 1 && charge == 0 )
    {
        toReconstruct = "n";
    }
    else if ( mass == 3 && charge == 1 )
    {
        toReconstruct = "t";
    }
    else
    {
        std::stringstream readMassDB;
        string readLine;

        int massTableLineBeg = InitReadMassesForKinematic ( mass_db );

        for ( int i = 0; i < massTableLineBeg; i++ )
        {
            std::getline ( mass_db, readLine );
        }

        while ( std::getline ( mass_db, readLine ) )
        {
            readMassDB.clear();
            readMassDB.str ( readLine );

            short posMassExcess = 5, posBindingEnergy = -1, posBetaDecay = -1, posAMU = -1, posElement = 4;

            string readWord[17];

//         std::cout << "Read Line:\n";

            for ( int i = 0; i < 17; i++ )
            {
                readMassDB >> readWord[i];
//             std::cout << readWord[i] << "  ";
            }

//         std::cout << "\n";

            GetRelevantInfoPositions ( readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement );

            if ( charge == std::stoi ( readWord[posElement-2] ) )
            {
                toReconstruct = Form ( "%i", mass );
                toReconstruct += readWord[posElement];

                break;
            }
        }

        return;
    }
}

string RootKinCalc::GetAtomicFormula ( int mass, string element )
{
    string atomicFormula = "";

    if ( element == "n" ) return "n";
    else if ( element == "H" && mass == 1 ) return "p";
    else if ( element == "H" && mass == 2 ) return "d";
    else if ( element == "H" && mass == 3 ) return "t";
    else atomicFormula = Form ( "%i%s", mass, element.c_str() );

    return atomicFormula;
}

void RootKinCalc::GetMassesForKinematic ( int charge, int mass, short memberID )
{
    std::ifstream mass_db ( "./mass_db.dat" );

    if ( !mass_db.is_open() )
    {
        std::cerr << "No File Found for the Masses Database!\n";
        return;
    }

    std::stringstream readMassDB;
    string readLine;

    int massTableLineBeg = InitReadMassesForKinematic ( mass_db );

    for ( int i = 0; i < massTableLineBeg; i++ )
    {
        std::getline ( mass_db, readLine );
    }

    while ( std::getline ( mass_db, readLine ) )
    {
        readMassDB.clear();
        readMassDB.str ( readLine );

        short posMassExcess = 5, posBindingEnergy = -1, posBetaDecay = -1, posAMU = -1, posElement = 4;

        string readWord[17];

        for ( int i = 0; i < 17; i++ )
        {
            readMassDB >> readWord[i];
        }

        GetRelevantInfoPositions ( readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement );

        CheckForMatch<int> ( readWord, posMassExcess, posBindingEnergy, posBetaDecay, posAMU, posElement, mass, charge, memberID );
    }

    return;
}

std::tuple<int, int> RootKinCalc::GetMassesForKinematic ( string particle, short memberID )
{
    std::tuple<int, int> output;

    std::ifstream mass_db ( "./mass_db.dat" );

    if ( !mass_db.is_open() )
    {
        std::cerr << "No File Found for the Masses Database!\n";
        return output;
    }

    int charge, mass;

    DecodeAtomicFormula ( mass_db, particle, mass, charge, memberID );
//     std::cout << "Decoded Charge: " << charge << "\n";

    mass_db.seekg ( 0 );

    std::get<0> ( output ) = charge;
    std::get<1> ( output ) = mass;

    return output;
}

void RootKinCalc::GetBaseKinematicInfo ( int zBeam, int aBeam, int zTarget, int aTarget, int zEjec, int aEjec, float beamEk_, float exEjec_, float exRecoil_, bool invertEjecRec, bool invertLabCMEn )
{
    if ( zBeam < 0 || aBeam <= 0 || zTarget < 0 || aTarget <= 0 || zEjec < 0 || aEjec <= 0 || beamEk_ < 0 || exEjec_ < 0 || exRecoil_ < 0 )
    {
        cerr << "Invalid Input...\n";
        return;
    }

    float dtr = TMath::DegToRad();
    float rtd = TMath::RadToDeg();

    float amu = 931.502; // MeV

    int zRecoil, aRecoil;

    if ( !invertEjecRec )
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

    GetMassesForKinematic ( zBeam, aBeam, 0 );
    GetMassesForKinematic ( zTarget, aTarget, 1 );
    GetMassesForKinematic ( zEjec, aEjec, 2 );
    GetMassesForKinematic ( zRecoil, aRecoil, 3 );

    string beamElement = ( zBeam == 0 ? "n" : ( ( zBeam == 1 && aBeam == 1 ) ? "p" : rInfo->atomicElement["beam"] ) );
    string targetElement = ( zTarget == 0 ? "n" : ( ( zTarget == 1 && aTarget == 1 ) ? "p" : rInfo->atomicElement["target"] ) );
    string ejecElement = ( zEjec == 0 ? "n" : ( ( zEjec == 1 && aEjec == 1 ) ? "p" : rInfo->atomicElement["ejectile"] ) );
    string recoilElement = ( zRecoil == 0 ? "n" : ( ( zRecoil == 1 && aRecoil == 1 ) ? "p" : rInfo->atomicElement["recoil"] ) );

    massBeam = rInfo->atomicMassUnit["beam"] * 1e-6 * amu; // MeV
    massTarget = rInfo->atomicMassUnit["target"] * 1e-6 * amu; // MeV
    massEjec = rInfo->atomicMassUnit["ejectile"] * 1e-6 * amu + exEjec; // MeV
    massRecoil = rInfo->atomicMassUnit["recoil"] * 1e-6 * amu + exRecoil; // MeV

    totMassInput = massBeam + massTarget;

    if ( !invertLabCMEn )
    {
        beamEkLab = beamEk_;
        beamEkCM = beamEkLab*massTarget / ( massBeam+massTarget );
    }
    else
    {
        beamEkCM = beamEk_;
        beamEkLab = beamEkCM * ( massBeam+massTarget ) / massTarget;
    }

    exEjec = exEjec_;
    exRecoil = exRecoil_;

    mapKey = Form ( "%d%s(%d%s,%d%s)%d%s @%4.3f MeV (E*ejec = %4.3f / E*rec = %4.3f)", aTarget, targetElement.c_str(), aBeam, beamElement.c_str(),
                    aEjec, ejecElement.c_str(), aRecoil, recoilElement.c_str(), beamEkLab, exEjec, exRecoil );

    betaC = TMath::Sqrt ( beamEkLab * ( beamEkLab + 2*massBeam ) ) / ( totMassInput + beamEkLab );

    qValueGS = ( rInfo->massExcess["beam"] + rInfo->massExcess["target"] - rInfo->massExcess["ejectile"] - rInfo->massExcess["recoil"] ) / 1000.;
    qValueFinal = qValueGS - exEjec - exRecoil;

    reacAboveThr = ( beamEkCM + qValueGS ) > 0;

    if ( !reacAboveThr ) return;

    eCMi = TMath::Sqrt ( totMassInput * totMassInput + ( 2*beamEkLab*massTarget ) );
    eCMf = eCMi + qValueFinal - totMassInput + massEjec + massRecoil;

    e3CM = ( eCMf * eCMf + ( massEjec + massRecoil ) * ( massEjec - massRecoil ) ) / ( 2*eCMf );

    beta3C = TMath::Sqrt ( 1 - ( ( massEjec * massEjec ) / ( e3CM * e3CM ) ) );

    yNew = pow ( e3CM / massEjec, 2 ) * ( 1 - betaC * betaC );
}

void RootKinCalc::GetBaseKinematicInfo ( string beam, string target, string ejectile, float beamEk_, float exEjec_, float exRecoil_, bool invertEjecRec, bool invertLabCMEn )
{
    std::ifstream mass_db ( "./mass_db.dat" );

    if ( !mass_db.is_open() )
    {
        std::cerr << "No File Found for the Masses Database!\n";
        return;
    }

    int zBeam, aBeam, zTarget, aTarget, zEjec, aEjec;

    DecodeAtomicFormula ( mass_db, beam, aBeam, zBeam, 0 );
    mass_db.seekg ( 0 );
    DecodeAtomicFormula ( mass_db, target, aTarget, zTarget,1 );
    mass_db.seekg ( 0 );
    DecodeAtomicFormula ( mass_db, ejectile, aEjec, zEjec, 2 );
    mass_db.seekg ( 0 );

    GetBaseKinematicInfo ( zBeam, aBeam, zTarget, aTarget, zEjec, aEjec, beamEk_, exEjec_, exRecoil_, invertEjecRec, invertLabCMEn );
}

void RootKinCalc::CalcKinematic ( float ejecLabAngle_ )
{
    float dtr = TMath::DegToRad();
    float rtd = TMath::RadToDeg();

    float amu = 931.502; // MeV

    KinCalcRes* kcr;
    kcr = &kinRes[ejecLabAngle_];

    kcr->ejecLabAngle = ejecLabAngle_;

    kcr->cosagl = TMath::Cos ( ejecLabAngle_ * dtr );

    kcr->b = -betaC * kcr->cosagl;
    kcr->a = yNew + kcr->b*kcr->b;
    kcr->c = 1 - yNew;

    kcr->d__2 = kcr->b * kcr->b - kcr->a * kcr->c;

    if ( kcr->d__2 < 0 || kcr->a == 0 ) kcr->b3L1 = 1;
    else kcr->b3L1 = ( -kcr->b + TMath::Sqrt ( kcr->d__2 ) ) / kcr->a;

    if ( 1 - kcr->b3L1 * kcr->b3L1  <= 0 || kcr->b3L1 < 0 ) kcr->ejecLabEnergy = -1000;
    else kcr->ejecLabEnergy = massEjec * ( 1 / TMath::Sqrt ( 1 - kcr->b3L1 * kcr->b3L1 ) - 1 );

    if ( kcr->ejecLabEnergy > 0 )
    {
        kcr->ejecCMAngle = TMath::ACos ( ( kcr->b3L1 * kcr->cosagl - betaC ) / ( ( 1 - betaC * kcr->b3L1 * kcr->cosagl ) * beta3C ) ) * rtd;

        kcr->recoilLabEnergy = beamEkLab + qValueFinal - kcr->ejecLabEnergy;

        kcr->recoilLabAngle = ( TMath::ASin ( TMath::Sqrt ( ( kcr->ejecLabEnergy * ( kcr->ejecLabEnergy + 2*massEjec ) ) / ( kcr->recoilLabEnergy * ( kcr->recoilLabEnergy + 2*massRecoil ) ) )   *
                                              TMath::Sin ( ejecLabAngle_ * dtr ) ) ) * rtd;
    }
    else
    {
        kcr->ejecCMAngle = -1000;
        kcr->recoilLabEnergy = -1000;
        kcr->recoilLabAngle = -1000;
    }

    return;
}

TGraph* RootKinCalc::PlotKinematicGraph ( TCanvas* canvas, string xAxisID, string yAxisID, float xMin, float xMax, float stepWidth, bool doDraw )
{
    if(doDraw) canvas->cd();

    string grTitle = Form ( "%s vs. %s for %s", xAxisID.c_str(), yAxisID.c_str(), mapKey.c_str() );

    if ( doDraw ) std::cout << "Plotting " << grTitle << " with steps of " << stepWidth << "...\n";

    std::vector<TGraph*> listOfGraphs;

    listOfGraphs.clear();

    if ( doDraw && canvas != NULL )
    {
        auto lOK = canvas->GetListOfPrimitives();

        TMultiGraph* currentMG = new TMultiGraph();

        for ( int i = 0; i < lOK->GetSize(); i++ )
        {
            TGraph* gPadGraph = dynamic_cast<TGraph*> ( lOK->At ( i ) );
            TMultiGraph* testMG = dynamic_cast<TMultiGraph*> ( lOK->At ( i ) );

            if ( gPadGraph != NULL )
            {
                listOfGraphs.push_back ( gPadGraph );
            }
            else if ( testMG != NULL ) currentMG = testMG;
        }

        if ( currentMG != NULL && ( ( string ) currentMG->GetTitle() ).length() > 0 && currentMG->GetListOfGraphs()->GetSize() > 0 )
        {
            listOfGraphs.clear();

            for ( int i = 0; i < currentMG->GetListOfGraphs()->GetSize(); i++ )
            {
                listOfGraphs.push_back ( ( TGraph* ) currentMG->GetListOfGraphs()->At ( i ) );
            }

            string mgType = currentMG->GetTitle();
            mgType = mgType.substr ( 0, mgType.find ( " for" ) );
        }
        else if ( listOfGraphs.size() > 0 )
        {
            string grType = listOfGraphs[0]->GetTitle();
            grType = grType.substr ( 0, grType.find ( " for" ) );
        }
    }

    float x_ = xMin;

    TGraph* tempGr = new TGraph ( kinRes.size() );

    int counter = 0;

    for ( auto itr = kinRes.begin(); itr != kinRes.end(); itr++ )
    {
//         std::cout << GetKinResIDValue ( &reacItr->second[i], xAxisID ) << " <-> " << GetKinResIDValue ( &reacItr->second[i], yAxisID ) << "\n";
        if ( GetKinResIDValue ( itr->second, xAxisID ) > 0 && GetKinResIDValue ( itr->second, yAxisID ) > 0 )
        {
            tempGr->SetPoint ( counter, GetKinResIDValue ( itr->second, xAxisID ), GetKinResIDValue ( itr->second, yAxisID ) );

            counter++;
        }
    }

    TGraph* gr = new TGraph ( ( int ) ( xMin - xMax ) / stepWidth );

    int pointNum = 0;

    while ( x_ <= xMax )
    {
//         std::cout << x_ << " <-> " << tempGr->Eval ( x_ ) << "\n";

        if ( x_ > 0 && tempGr->Eval ( x_ ) > 0 )
        {
            gr->SetPoint ( pointNum, x_, tempGr->Eval ( x_ ) );
            pointNum++;
        }

        x_ += stepWidth;
    }

    gr->SetTitle ( grTitle.c_str() );

    listOfGraphs.push_back ( gr );

    if ( doDraw )
    {
        TMultiGraph* mg = new TMultiGraph();

        for ( int i = 0; i < listOfGraphs.size(); i++ )
        {
            listOfGraphs[i]->SetLineColor ( i + 1 );
            mg->Add ( listOfGraphs[i] );
        }

        string mgTitle = listOfGraphs[0]->GetTitle();
        mgTitle = mgTitle.substr ( 0, mgTitle.find ( " for" ) );

        mg->Draw ( "ALP" );
        mg->SetTitle ( mgTitle.c_str() );

        TLegend* legend = new TLegend ( 0.55,0.7,0.9,0.9 );

        for ( unsigned short i = 0; i < listOfGraphs.size(); i++ )
        {
            string legStr = listOfGraphs[i]->GetTitle();

            std::size_t headerEnd = legStr.find ( "for" ) + 4;

            legStr.replace ( 0, headerEnd, "" );

            legend->AddEntry ( listOfGraphs[i], legStr.c_str(), "l" );
        }

        legend->Draw();

        canvas->Modified();
        canvas->Update();
    }

    return gr;
}

float RootKinCalc::ConvertSingleValue ( string fromQuantity, string toQuantity, float val )
{
    TGraph* tempGr = new TGraph ( kinRes.size() );

    int counter = 0;

    for ( auto itr = kinRes.begin(); itr != kinRes.end(); itr++ )
    {
        tempGr->SetPoint ( counter, GetKinResIDValue ( itr->second, fromQuantity ), GetKinResIDValue ( itr->second, toQuantity ) );

        counter++;
    }

//     std::cout << fromQuantity << ": " << val << "  <--->  " << tempGr->Eval ( val ) << " :" << toQuantity << "\n";

    return tempGr->Eval ( val );
}

