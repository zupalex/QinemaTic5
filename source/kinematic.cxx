#include "kinematic.h"

using std::string;

int zBeam_last, aBeam_last, zTarget_last, aTarget_last, zEjec_last, aEjec_last;
string xAxisID_last, yAxisID_last;
float beamEk_last, xMin_last, xMax_last;
float stepWidth_last;
int quietMode_last;
float exEjec_last, exRecoil_last;

std::map<string, RootKinCalc> kinResMap;

TGWindow* FindWindowByName ( std::string winName )
{
    std::string altName = winName;

    while ( altName.find ( " " ) != std::string::npos )
    {
        altName = altName.replace ( altName.find ( " " ), 1, "" );
    }

    auto winList = gClient->GetListOfWindows();

    for ( int i = 0; i < winList->GetSize(); i++ )
    {
        if ( winList->At ( i )->GetName() == winName || winList->At ( i )->GetName() == altName ) return ( TGWindow* ) winList->At ( i );
    }

    return nullptr;
}

TGFrame* FindFrameByName ( TGCompositeFrame* pFrame, std::string frameName )
{
//     std::cout << "Scanning Frame " << pFrame->GetName() << "\n";

    std::string altName = frameName;

    while ( altName.find ( " " ) != std::string::npos )
    {
        altName = altName.replace ( altName.find ( " " ), 1, "" );
    }

    TIter nFrame ( pFrame->GetList() );

    TGFrameElement* frameEl;

    while ( ( frameEl = ( TGFrameElement* ) nFrame() ) )
    {
        if ( frameEl->fFrame->GetName() == frameName || frameEl->fFrame->GetName() == altName ) return frameEl->fFrame;

        if ( frameEl->fFrame->InheritsFrom ( TGCompositeFrame::Class() ) )
        {
            TGFrame* findInSubFrame = FindFrameByName ( ( TGCompositeFrame* ) frameEl->fFrame, frameName );

            if ( findInSubFrame != NULL ) return findInSubFrame;
        }
    }

    return nullptr;
}

bool CharIsDigit ( char toCheck )
{
    if ( toCheck == '0' || toCheck == '1' || toCheck == '2' || toCheck == '3' || toCheck == '4' || toCheck == '5' || toCheck == '6' || toCheck == '7' || toCheck == '8' || toCheck == '9' )
        return true;
    else return false;
}

void DisplayListOfReactions()
{
    if ( kinResMap.size() > 0 )
    {
        int counter = 0;

        for ( auto itr = kinResMap.begin(); itr != kinResMap.end(); itr++ )
        {
            std::cout << Form ( "[%d] %s",counter, itr->first.c_str() ) << "\n";
            counter++;
        }
    }
    else
    {
        std::cout << "No calculations were performed yet!\n";
    }
}

float GetKinResIDValue ( KinCalcRes kcr, string ID )
{
    if ( ID == "Ejec. Lab Angle" ) return kcr.ejecLabAngle;
    else if ( ID == "Ejec. Lab Energy" ) return kcr.ejecLabEnergy;
    else if ( ID == "C.M. Angle" ) return kcr.ejecCMAngle;
    else if ( ID == "Recoil Lab Angle" ) return kcr.recoilLabAngle;
    else if ( ID == "Recoil Lab Energy" ) return kcr.recoilLabEnergy;
    else return -1;
}

string GetKinResIDString ( short ID )
{
    if ( ID == 0 ) return "Ejec. Lab Angle";
    else if ( ID == 1 ) return "Ejec. Lab Energy";
    else if ( ID == 2 ) return "C.M. Angle";
    else if ( ID == 3 ) return "Recoil Lab Angle";
    else if ( ID == 4 ) return "Recoil Lab Energy";
    else return "Invalid";
}

void UpdateReactionListBox ( TGListBox* lb )
{
    lb->RemoveAll();

    int ids = 0;

    for ( auto itr = kinResMap.begin(); itr != kinResMap.end(); itr++ )
    {
        lb->AddEntrySort ( itr->first.c_str(), ids );

        ids++;
    }

//     lb->Resize ( 400, 150 );
    lb->MapSubwindows();
    lb->Layout();
}

ReacInfo::ReacInfo()
{

}

void ReacInfo::ReinitMasses()
{
    for ( short i = 0; i < 4; i++ )
    {
        A[i] = 0;
        Z[i] = 0;
        massExcess[i] = 0.0;
        deltaMassExcess[i] = 0.0;
        bindEnPerA[i] = 0.0;
        deltaBindEnPerA[i] = 0.0;
        betaDecayEn[i] = 0.0;
        deltaBetaDecayEn[i] = 0.0;
        atomicMassUnit[i] = 0.0;
        vOVERs[i] = 0.0;
        atomicElement[i] = "";
    }
}

RootKinCalc::RootKinCalc()
{
    rInfo = new ReacInfo();
}

void RootKinCalc::OnClickPlotGraphs()
{
    TList* selectedEntries = new TList();

    TGWindow* mw = FindWindowByName ( "ROOT Kinematic Calculator" );

    if ( mw == NULL )
    {
        std::cerr << "Main Window not found!\n";

        return;
    }

    TGMainFrame* mf = ( TGMainFrame* ) mw->GetMainFrame();

    if ( mf == NULL )
    {
        std::cerr << "Main Frame not found!\n";

        return;
    }

    TGButtonGroup* xBG = dynamic_cast<TGButtonGroup*> ( FindFrameByName ( mf, "X Axis BG" ) );
    TGButtonGroup* yBG = dynamic_cast<TGButtonGroup*> ( FindFrameByName ( mf, "Y Axis BG" ) );

    if ( xBG == NULL )
    {
        std::cerr << "X Axis Button Group not found!\n";

        return;
    }

    if ( yBG == NULL )
    {
        std::cerr << "Y Axis Button Group not found!\n";

        return;
    }

    string xAxisID;
    TIter xBGItr ( xBG->GetList() );

    while ( xBGItr.Next() )
    {
        TGRadioButton* rb = ( ( TGRadioButton* ) ( ( TGFrameElement* ) *xBGItr )->fFrame );

        if ( rb->GetState() )
            xAxisID = ( ( TGRadioButton* ) ( ( TGFrameElement* ) *xBGItr )->fFrame )->GetString();
    }

    string yAxisID;
    TIter yBGItr ( yBG->GetList() );

    while ( yBGItr.Next() )
    {
        TGRadioButton* rb = ( ( TGRadioButton* ) ( ( TGFrameElement* ) *yBGItr )->fFrame );

        if ( rb->GetState() )
            yAxisID = ( ( TGRadioButton* ) ( ( TGFrameElement* ) *yBGItr )->fFrame )->GetString();
    }

    if ( xAxisID.empty() || yAxisID.empty() )
    {
        std::cerr << "Failed to read the axis IDs\n";

        return;
    }

    TGNumberEntryField* xMinIF_ = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "X Min IF" ) );
    TGNumberEntryField* xMaxIF_ = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "X Max IF" ) );
    TGNumberEntryField* stepIF_ = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "Step Width IF" ) );

    if ( xMinIF_ == NULL || xMaxIF_ == NULL || stepIF_ == NULL )
    {
        std::cerr << "Unabled to read the Axis range and step width\n";

        return;
    }

    float xMin_, xMax_, stepWidth_;

    xMin_ = xMinIF_->GetNumber();
    xMax_ = xMaxIF_->GetNumber();
    stepWidth_ = stepIF_->GetNumber();

    if ( xMin_ >= xMax_ )
    {
        std::cerr << "X Min has to be bigger than X Max!\n";

        return;
    }

    if ( xMax_ > 180 )
    {
        std::cerr << "X Max cannot be bigger than 180\n";

        return;
    }

    if ( stepWidth_ <= 0 )
    {
        std::cerr << "The Step Width cannot be 0\n";

        return;
    }

    TGListBox* reacFrame = dynamic_cast<TGListBox*> ( FindFrameByName ( mf, "Reactions ListBox" ) );

    if ( reacFrame == NULL )
    {
        std::cerr << "Reaction ListBox not found!\n";

        return;
    }

    reacFrame->GetSelectedEntries ( selectedEntries );

    TIter selectedEntryItr ( selectedEntries );

    while ( selectedEntryItr.Next() )
    {
        string reacTitle = ( ( TGLBEntry* ) *selectedEntryItr )->GetTitle();

        auto found = kinResMap.find ( reacTitle );

        int counter = -1;

        for ( auto itr = kinResMap.begin(); itr != kinResMap.end(); itr++ )
        {
            counter++;

            if ( itr == found ) break;
        }

        if ( counter == kinResMap.size() )
        {
            std::cerr << "ERROR: selected reaction somehow not anymore in the reactions map !?\n";

            return;
        }

        short reacID = counter;

// 	std::cout << reacID << "\n";

        PlotKinematicGraph ( reacID, xAxisID, yAxisID, xMin_, xMax_, stepWidth_ );
    }
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

    rInfo->A[0] = aBeam;
    rInfo->A[1] = aTarget;
    rInfo->A[2] = aEjec;
    rInfo->A[3] = aRecoil;
    rInfo->Z[0] = zBeam;
    rInfo->Z[1] = zTarget;
    rInfo->Z[2] = zEjec;
    rInfo->Z[3] = zRecoil;

    GetMassesForKinematic ( zBeam, aBeam, 0 );
    GetMassesForKinematic ( zTarget, aTarget, 1 );
    GetMassesForKinematic ( zEjec, aEjec, 2 );
    GetMassesForKinematic ( zRecoil, aRecoil, 3 );

    string beamElement = ( zBeam == 0 ? "n" : ( ( zBeam == 1 && aBeam == 1 ) ? "p" : rInfo->atomicElement[0] ) );
    string targetElement = ( zTarget == 0 ? "n" : ( ( zTarget == 1 && aTarget == 1 ) ? "p" : rInfo->atomicElement[1] ) );
    string ejecElement = ( zEjec == 0 ? "n" : ( ( zEjec == 1 && aEjec == 1 ) ? "p" : rInfo->atomicElement[2] ) );
    string recoilElement = ( zRecoil == 0 ? "n" : ( ( zRecoil == 1 && aRecoil == 1 ) ? "p" : rInfo->atomicElement[3] ) );

    massBeam = rInfo->atomicMassUnit[0] * 1e-6 * amu; // MeV
    massTarget = rInfo->atomicMassUnit[1] * 1e-6 * amu; // MeV
    massEjec = rInfo->atomicMassUnit[2] * 1e-6 * amu + exEjec; // MeV
    massRecoil = rInfo->atomicMassUnit[3] * 1e-6 * amu + exRecoil; // MeV

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

    qValueGS = ( rInfo->massExcess[0] + rInfo->massExcess[1] - rInfo->massExcess[2] - rInfo->massExcess[3] ) / 1000.;
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

    if ( 1 - kcr->b3L1 * kcr->b3L1  <= 0 || kcr->b3L1 < 0) kcr->ejecLabEnergy = -1000;
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

void RootKinCalc::GetReactionKinematic ( )
{
    float stepWidth = 0.1;

    if ( !reacAboveThr )
    {
        std::cout << "The reaction in below the threshold...\n";

        return;
    }

    float labAngle = 0.0;

    while ( labAngle <= 180.0 )
    {
        CalcKinematic ( labAngle );

//         std::cout << kinResMap[mapKey][kinResMap[mapKey].size()-1].ejecLabAngle << " <-> " << kinResMap[mapKey][kinResMap[mapKey].size()-1].ejecLabEnergy << "\n";

        labAngle += stepWidth;
    }

    kinResMap[mapKey] = *this;

    TGWindow* mw = FindWindowByName ( "ROOT Kinematic Calculator" );

    if ( mw == NULL )
    {
        std::cerr << "Main Window not found!\n";

        return;
    }

    TGMainFrame* mf = ( TGMainFrame* ) mw->GetMainFrame();

    if ( mf == NULL )
    {
        std::cerr << "Main Frame not found!\n";

        return;
    }

    TGListBox* reacFrame = dynamic_cast<TGListBox*> ( FindFrameByName ( mf, "Reactions ListBox" ) );

    if ( reacFrame == NULL )
    {
        std::cerr << "Reaction ListBox not found!\n";

        return;
    }

    UpdateReactionListBox ( reacFrame );

    return;
}

TGraph* RootKinCalc::PlotKinematicGraph ( short reactionID, string xAxisID, string yAxisID, float xMin, float xMax, float stepWidth, bool doDraw )
{
    if ( reactionID >= kinResMap.size() ) return nullptr;

    AssignLastUsedValues ( zBeam_last, aBeam_last, zTarget_last, aTarget_last, zEjec_last, aEjec_last, xAxisID, yAxisID, beamEk_last, xMin, xMax, stepWidth, quietMode_last, exEjec_last, exRecoil_last );

    auto reacItr = kinResMap.begin();

    std::advance ( reacItr, reactionID );

    string grTitle = Form ( "%s vs. %s for %s", xAxisID.c_str(), yAxisID.c_str(), reacItr->first.c_str() );

    if ( doDraw ) std::cout << "Plotting " << grTitle << " with steps of " << stepWidth << "...\n";

    bool drawSame = false;

    std::vector<TGraph*> listOfGraphs;

    listOfGraphs.clear();

    if ( doDraw && gPad != NULL )
    {
        auto lOK = gPad->GetCanvas()->GetListOfPrimitives();

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

//             if ( grTitle.substr ( 0, grTitle.find ( " for" ) ) == mgType )
//             {
//                 std::cout << "Do you want to draw the new graph on the same TCanvas ([1] yes / [0] no) ?";
//                 std::cin >> drawSame;
//             }
//             else
//             {
//                 std::cout << "The current Canvas already contains different data. Replace it ([1] yes / [0] no) ?";
//                 std::cin >> drawSame;
//
//                 if ( !drawSame ) return nullptr;
//                 else drawSame = false;
//             }
        }
        else if ( listOfGraphs.size() > 0 )
//             if ( listOfGraphs.size() > 0 )
        {
            string grType = listOfGraphs[0]->GetTitle();
            grType = grType.substr ( 0, grType.find ( " for" ) );

//             if ( grTitle.substr ( 0, grTitle.find ( " for" ) ) == grType )
//             {
//                 std::cout << "Do you want to draw the new graph on the same TCanvas ([1] yes / [0] no) ?";
//                 std::cin >> drawSame;
//             }
//             else
//             {
//                 std::cout << "The current Canvas already contains different data. Replace it ([1] yes / [0] no) ?";
//                 std::cin >> drawSame;
//
//                 if ( !drawSame ) return nullptr;
//                 else drawSame = false;
//             }
        }
    }

    drawSame = true;

    float x_ = xMin;

    TGraph* tempGr = new TGraph ( reacItr->second.kinRes.size() );

    int counter = 0;

    for ( auto itr = reacItr->second.kinRes.begin(); itr != reacItr->second.kinRes.end(); itr++ )
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

    if ( doDraw && !drawSame )
    {
        TCanvas* c = new TCanvas();

        gr->Draw ( "ALP" );

        c->Update();
        c->ForceUpdate();
    }
    else if ( doDraw )
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

        gPad->GetCanvas()->Update();
        gPad->GetCanvas()->ForceUpdate();
    }

    return gr;
}

TGraph* RootKinCalc::PlotKinematicGraph ( string opt )
{
    short reactionID;
    short xAxisIDNum, yAxisIDNum;
    string xAxisID, yAxisID;
    float xMin, xMax, stepWidth;

    std::cout << "Which reaction do you want to use?\n";
    DisplayListOfReactions();
    std::cin >> reactionID;

    if ( opt == "same" )
    {
        return PlotKinematicGraph ( reactionID, xAxisID_last, yAxisID_last, xMin_last, xMax_last, stepWidth_last );
    }

    std::cout << "\nWhat do you want to plot?\n";
    std::cout << "-------------------------\n";
    std::cout << "[0] Ejectile Lab Angle\n";
    std::cout << "[1] Ejectile Lab Energy\n";
    std::cout << "[2] Ejectile C.M. Angle\n";
    std::cout << "[3] Recoil Lab Angle\n";
    std::cout << "[4] Recoil C.M. Angle\n";
    std::cout << "[5] Recoil Lab energy\n";
    std::cout << "-------------------------\n";
    std::cout << "On the X Axis? ";
    std::cin >> xAxisIDNum;
    std::cout << "On the Y Axis? ";
    std::cin >> yAxisIDNum;
    std::cout << "\nLower boundary for the X Axis? ";
    std::cin >> xMin;
    std::cout << "Upper boundary for the X Axis? ";
    std::cin >> xMax;
    std::cout << "Precision of the X Axis? ";
    std::cin >> stepWidth;

    xAxisID = GetKinResIDString ( xAxisIDNum );
    yAxisID = GetKinResIDString ( yAxisIDNum );

    return PlotKinematicGraph ( reactionID, xAxisID, yAxisID, xMin, xMax, stepWidth );
}

float RootKinCalc::ConvertSingleValue ( short reactionID, string fromQuantity, string toQuantity, float val )
{
    if ( reactionID >= kinResMap.size() ) return -1;

    auto reacItr = kinResMap.begin();

    std::advance ( reacItr, reactionID );

    AssignLastUsedValues ( zBeam_last, aBeam_last, zTarget_last, aTarget_last, zEjec_last, aEjec_last, fromQuantity, toQuantity, beamEk_last, xMin_last, xMax_last, stepWidth_last, quietMode_last, exEjec_last, exRecoil_last );

    TGraph* tempGr = new TGraph ( reacItr->second.kinRes.size() );

    int counter = 0;

    for ( auto itr = reacItr->second.kinRes.begin(); itr != reacItr->second.kinRes.end(); itr++ )
    {
        tempGr->SetPoint ( counter, GetKinResIDValue ( itr->second, fromQuantity ), GetKinResIDValue ( itr->second, toQuantity ) );

        counter++;
    }

//     std::cout << fromQuantity << ": " << val << "  <--->  " << tempGr->Eval ( val ) << " :" << toQuantity << "\n";

    return tempGr->Eval ( val );
}

float RootKinCalc::ConvertSingleValue ( float val )
{
    short reactionID;
    short fQIDNum, tQIDNum;
    string fQID, tQID;

    std::cout << "Which reaction do you want to use?\n";
    DisplayListOfReactions();
    std::cin >> reactionID;

    if ( val >= 0 ) return ConvertSingleValue ( reactionID, xAxisID_last, yAxisID_last, val );

    std::cout << "\nWhat do you want to convert:\n";
    std::cout << "-------------------------\n";
    std::cout << "[0] Ejectile Lab Angle\n";
    std::cout << "[1] Ejectile Lab Energy\n";
    std::cout << "[2] Ejectile C.M. Angle\n";
    std::cout << "[3] Recoil Lab Angle\n";
    std::cout << "[4] Recoil C.M. Angle\n";
    std::cout << "[5] Recoil Lab energy\n";
    std::cout << "-------------------------\n";
    std::cout << "From? ";
    std::cin >> fQIDNum;
    std::cout << "To? ";
    std::cin >> tQIDNum;
    std::cout << "Value to convert? ";
    std::cin >> val;

    fQID = GetKinResIDString ( fQIDNum );
    tQID = GetKinResIDString ( tQIDNum );

    return ConvertSingleValue ( reactionID, fQID, tQID, val );
}

void RootKinCalc::Dump ( short reactionID, short entry )
{
    if ( reactionID >= kinResMap.size() ) return;

    auto reacItr = kinResMap.begin();

    std::advance ( reacItr, reactionID );

    if ( entry >= reacItr->second.kinRes.size() ) return;

    RootKinCalc toDump = reacItr->second;

    std::cout << "********************************************\n";
    std::cout << "********************************************\n";
//     std::cout << "Requested Beam Element: " << aBeam << atomicElement[0] << "\n";
//     std::cout << "Mass Excess: " << massExcess[0] << " +/- " << deltaMassExcess[0] << " keV\n";
//     std::cout << "Binding Energy per nucleon: " << bindEnPerA[0] << " +/- "  << deltaBetaDecayEn[0] << " keV\n";
//     std::cout << "Beta Decay Energy: " << betaDecayEn[0] << " +/- "  << deltaBetaDecayEn[0] << " keV\n";
//     std::cout << "Atomic Mass Unit: " << atomicMassUnit[0] << " micro-u\n";
//     std::cout << "V/S: " << vOVERs[0] << "\n";
//     std::cout << "-------------------------------------\n";
//     std::cout << "Requested Target Element: " << aTarget << atomicElement[1] << "\n";
//     std::cout << "Mass Excess: " << massExcess[1] << " +/- " << deltaMassExcess[1] << " keV\n";
//     std::cout << "Binding Energy per nucleon: " << bindEnPerA[1] << " +/- "  << deltaBetaDecayEn[1] << " keV\n";
//     std::cout << "Beta Decay Energy: " << betaDecayEn[1] << " +/- "  << deltaBetaDecayEn[1] << " keV\n";
//     std::cout << "Atomic Mass Unit: " << atomicMassUnit[1] << " micro-u\n";
//     std::cout << "V/S: " << vOVERs[1] << "\n";
//     std::cout << "-------------------------------------\n";
//     std::cout << "Requested Ejectile Element: " << aEjec << atomicElement[2] << "\n";
//     std::cout << "Mass Excess: " << massExcess[2] << " +/- " << deltaMassExcess[2] << " keV\n";
//     std::cout << "Binding Energy per nucleon: " << bindEnPerA[2] << " +/- "  << deltaBetaDecayEn[2] << " keV\n";
//     std::cout << "Beta Decay Energy: " << betaDecayEn[2] << " +/- "  << deltaBetaDecayEn[2] << " keV\n";
//     std::cout << "Atomic Mass Unit: " << atomicMassUnit[2] << " micro-u\n";
//     std::cout << "V/S: " << vOVERs[2] << "\n";
//     std::cout << "-------------------------------------\n";
//     std::cout << "Computed Recoil Element: " << aRecoil << atomicElement[3] << "\n";
//     std::cout << "Mass Excess: " << massExcess[3] << " +/- " << deltaMassExcess[3] << " keV\n";
//     std::cout << "Binding Energy per nucleon: " << bindEnPerA[3] << " +/- "  << deltaBetaDecayEn[3] << " keV\n";
//     std::cout << "Beta Decay Energy: " << betaDecayEn[3] << " +/- "  << deltaBetaDecayEn[3] << " keV\n";
//     std::cout << "Atomic Mass Unit: " << atomicMassUnit[3] << " micro-u\n";
//     std::cout << "V/S: " << vOVERs[3] << "\n";
//     std::cout << "============================================\n";

//     std::cout << "Dumping Calculations........\n";
//     std::cout << "Mass of the beam: " << toDump.massBeam << " MeV\n";
//     std::cout << "Mass of the target: " << toDump.massTarget << " MeV\n";
//     std::cout << "Mass of the ejectile: " << toDump.massEjec << " MeV\n";
//     std::cout << "Mass of the recoil: " << toDump.massRecoil << " MeV\n";
//     std::cout << "Total mass: " << toDump.totMassInput << " MeV\n";
//     std::cout << "Qvalue G.S: " << toDump.qValueGS << " MeV\n";
//     std::cout << "Qvalue Final State: " << toDump.qValueFinal << " MeV\n";
//     std::cout << "..........................\n";
//     std::cout << "betaC: " << toDump.betaC << "\n";
//     std::cout << "cosagl: " << toDump.cosagl << "\n";
//     std::cout << "eCMi: " << toDump.eCMi << "\n";
//     std::cout << "eCMf: " << toDump.eCMf << "\n";
//     std::cout << "e3CM: " << toDump.e3CM << "\n";
//     std::cout << "beta3C: " << toDump.beta3C << "\n";
//     std::cout << "yNew: " << toDump.yNew << "\n";
//     std::cout << "a: " << toDump.a << "\n";
//     std::cout << "b: " << toDump.b << "\n";
//     std::cout << "c: " << toDump.c << "\n";
//     std::cout << "d**2: " << toDump.d__2 << "\n";
//     std::cout << "b3L1: " << toDump.b3L1 << "\n";
//     std::cout << "..........................\n";
//     std::cout << "Ejectile Lab Energy @" << toDump.ejecLabAngle << " degrees: " << toDump.ejecLabEnergy << " MeV\n";
//     std::cout << "C.M Angle: " << toDump.ejecCMAngle << " degrees\n";
//     std::cout << "********************************************\n";
//     std::cout << "********************************************\n";
}

void RootKinCalc::WriteTableToFile ( short reactionID, float xMin, float xMax, float precision )
{
    if ( reactionID >= kinResMap.size() )
    {
        std::cerr << "Graph Number specified is incorrect!\n";
        return;
    }

    auto grItr = kinResMap.begin();

    std::advance ( grItr, reactionID );

    std::stringstream grTitle;

    grTitle.clear();

    grTitle.str ( grItr->first );

    string dummy, reaction, energy, unit;

    grTitle >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;
    grTitle >> reaction >> energy >> unit;

    string outputName = grItr->first;

    std::size_t foundPar1 = outputName.find ( "(" );
    outputName.replace ( foundPar1, 1, "_" );

    std::size_t foundPar2 = outputName.find ( ")" );
    outputName.replace ( foundPar2, 1, "_" );

    std::size_t foundComa = outputName.find ( "," );
    outputName.replace ( foundComa, 1, "_" );

    std::size_t foundUnit = outputName.find ( "eV" ) - 2;
    outputName.replace ( foundUnit, 1, "" );

    std::size_t foundAt = outputName.find ( "@" );
    outputName.replace ( foundAt-1, 1, "" );

    std::size_t foundPar3 = outputName.find ( "(" );
    outputName.replace ( foundPar3-1, 2, "_" );

    std::size_t foundPar4 = outputName.find ( ")" );
    outputName.replace ( foundPar4, 1, "" );

    std::size_t foundSlash = outputName.find ( "/" );
    outputName.replace ( foundSlash-1, 3, "_" );

    std::size_t foundEqual1 = outputName.find ( "=" );
    outputName.replace ( foundEqual1-1, 3, "=" );

    std::size_t foundEqual2 = outputName.find ( "=" );
    foundEqual2 = outputName.find ( "=", foundEqual2+1 );
    outputName.replace ( foundEqual2-1, 3, "=" );

    std::size_t foundStar1 = outputName.find ( "*" );
    outputName.replace ( foundStar1, 2, "xE" );

    std::size_t foundStar2 = outputName.find ( "*" );
    outputName.replace ( foundStar2, 2, "xR" );

    struct stat checkDir;

    stat ( "./tables/", &checkDir );

    if ( ! ( checkDir.st_mode & S_IFDIR ) )
    {
        mkdir ( "./tables/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }

    outputName = "./tables/" + outputName + ".dat";

    std::cout << "Writing table to file: " << outputName << "\n";

//     std::cout << "Preparing temp graphs...\n";

    TGraph* aLabEjecVSCMEjec = PlotKinematicGraph ( reactionID, GetKinResIDString ( 0 ), GetKinResIDString ( 2 ), xMin, xMax, precision, false );
    TGraph* aLabejecVSenLabEjec = PlotKinematicGraph ( reactionID, GetKinResIDString ( 0 ), GetKinResIDString ( 1 ), xMin, xMax, precision, false );
    TGraph* aLabejecVSaLabRec = PlotKinematicGraph ( reactionID, GetKinResIDString ( 0 ), GetKinResIDString ( 3 ), xMin, xMax, precision, false );
    TGraph* aLabejecVSenLabRec = PlotKinematicGraph ( reactionID, GetKinResIDString ( 0 ), GetKinResIDString ( 4 ), xMin, xMax, precision, false );

//     std::cout << "Finished creating temp graphs...\n";

    std::ofstream outTable ( outputName.c_str() );

    if ( !outTable.is_open() )
    {
        std::cerr << "Failed to open file " << outputName << "\n";
        return;
    }

    outTable << grItr->first;
    outTable << "\n";
    outTable << "Ejectile Lab Angle (deg.)    C.M. Angle (deg.)    Ejectile Lab Energy (" << unit << ")    Recoil Lab Angle (deg.)    Recoil Lab Energy (" << unit << ")\n";

    float x_ = xMin;

    while ( x_ <= xMax )
    {
        outTable << x_ << "                         ";
        outTable << aLabejecVSenLabEjec->Eval ( x_ ) << "                         ";
        outTable << aLabEjecVSCMEjec->Eval ( x_ ) << "                         ";
        outTable << aLabejecVSaLabRec->Eval ( x_ ) << "                         ";
        outTable << aLabejecVSenLabRec->Eval ( x_ );
        outTable << "\n";

        x_ += precision;
    }

    return;
}



