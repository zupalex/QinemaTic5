#include <TRint.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TRootEmbeddedCanvas.h>

#include "kinematic.h"

using std::string;

TGTextEntry* beamElLabel;
TGTextEntry* targetElLabel;
TGTextEntry* ejecElLabel;
TGTextEntry* recoilElLabel;

TGNumberEntryField* beamKeLabel;
TGNumberEntryField* targetKeLabel;
TGNumberEntryField* ejecKeLabel;
TGNumberEntryField* recoilKeLabel;

TGNumberEntryField* beamExLabel;
TGNumberEntryField* targetExLabel;
TGNumberEntryField* ejecExLabel;
TGNumberEntryField* recoilExLabel;

CalcMonitor* CalcMonitor::s_instance = nullptr;

CalcMonitor* CalcMonitor::CreateCalcMonitor()
{
    if ( s_instance == NULL )
    {
        s_instance = new CalcMonitor();
    }

    return s_instance;
}

void CalcMonitor::OnClickSingleCalcRB()
{
    std::cout << "A button has been Clicked!\n";

    TGRadioButton* clickedButton = ( TGRadioButton* ) gTQSender; // gTQSender gives the current "sender".

    std::cout<<clickedButton->GetName()<<std::endl;

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

    TGRadioButton* ejecLabAngleRB = dynamic_cast<TGRadioButton*> ( FindFrameByName ( mf, "Ejec. Lab Angle RB SC" ) ); // Check from line 601 for the names of the other elements we want to interact with
    TGRadioButton* ejecLabEnergyRB = dynamic_cast<TGRadioButton*> ( FindFrameByName ( mf, "Ejec. Lab Energy RB SC" ) );
    TGRadioButton* ejecCMAngleRB = dynamic_cast<TGRadioButton*> ( FindFrameByName ( mf, "Ejec. C.M. Angle RB SC" ) );
    TGRadioButton* recoilLabAngleRB = dynamic_cast<TGRadioButton*> ( FindFrameByName ( mf, "Recoil Lab Angle RB SC" ) ); // Check from line 601 for the names of the other elements we want to interact with
    TGRadioButton* recoilLabEnergyRB = dynamic_cast<TGRadioButton*> ( FindFrameByName ( mf, "Recoil Lab Energy RB SC" ) );
    TGRadioButton* recoilCMAngleRB = dynamic_cast<TGRadioButton*> ( FindFrameByName ( mf, "Recoil C.M. Angle RB SC" ) );
    
    if (clickedButton != NULL)
    {
      if (clickedButton != ejecLabAngleRB)
      {
	ejecLabAngleRB->SetState(kButtonUp, false);
      }
      
      if (clickedButton != ejecLabEnergyRB)
      {
	ejecLabEnergyRB->SetState(kButtonUp, false);
      }
      
     if (clickedButton != ejecCMAngleRB)
      {
	ejecCMAngleRB->SetState(kButtonUp, false);
      }
      if (clickedButton != recoilLabAngleRB)
      {
	recoilLabAngleRB->SetState(kButtonUp, false);
      }
      
      if (clickedButton != recoilLabEnergyRB)
      {
	recoilLabEnergyRB->SetState(kButtonUp, false);
      }
      
     if (clickedButton != recoilCMAngleRB)
      {
	recoilCMAngleRB->SetState(kButtonUp, false);
      }
    }
  
}

ClassImp ( CalcMonitor );

RootKinCalc* RootKinCalc::OnClickUpdateInfo()
{
    std::ifstream mass_db ( "./mass_db.dat" );

    if ( !mass_db.is_open() )
    {
        std::cerr << "No File Found for the Masses Database!\n";
        return nullptr;
    }

    RootKinCalc* kinCalc = new RootKinCalc();

    string beamStr, targetStr, ejecStr;

    beamStr = beamElLabel->GetText();
    targetStr = targetElLabel->GetText();
    ejecStr = ejecElLabel->GetText();

    if ( beamStr.empty() || targetStr.empty() || ejecStr.empty() ) return nullptr;

    float beamEk;
    float ejecEx, recEx;

    beamEk = beamKeLabel->GetNumber();
    ejecEx = ejecExLabel->GetNumber();
    recEx = recoilExLabel->GetNumber();

    kinCalc->GetBaseKinematicInfo ( beamStr, targetStr, ejecStr, beamEk, ejecEx, recEx );

    TGWindow* mw = FindWindowByName ( "ROOT Kinematic Calculator" );

    if ( mw == NULL )
    {
        std::cerr << "Main Window not found!\n";

        return nullptr;
    }

    TGMainFrame* mf = ( TGMainFrame* ) mw->GetMainFrame();

    if ( mf == NULL )
    {
        std::cerr << "Main Frame not found!\n";

        return nullptr;
    }

    TGTextEntry* recTE = dynamic_cast<TGTextEntry*> ( FindFrameByName ( mf, "Recoil Element IF" ) );

    recTE->SetText ( Form ( "%i%s", kinCalc->rInfo->A[3], kinCalc->rInfo->atomicElement[3].c_str() ) );

    TGNumberEntryField* bCMEk = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "Beam C.M. Energy IF" ) );

    bCMEk->SetNumber ( kinCalc->beamEkCM );

    TGNumberEntryField* thrIF = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "Reaction Threshold IF" ) );

    if ( thrIF == NULL )
    {
        std::cerr << "Reaction Threshold Display Box not found!\n";

        return nullptr;
    }

    thrIF->SetNumber ( kinCalc->qValueGS );

    if ( kinCalc->reacAboveThr ) thrIF->SetTextColor ( 0x009933 );
    else thrIF->SetTextColor ( 0xff0000 );

    return kinCalc;
}

void RootKinCalc::OnClickCalcKin()
{
    RootKinCalc* kinCalc = OnClickUpdateInfo();

    kinCalc->GetReactionKinematic();
}

void RootKinCalc::OnClickWriteTable()
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

    TGNumberEntryField* xMinIF_ = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "Table XMin IF" ) );
    TGNumberEntryField* xMaxIF_ = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "Table XMax IF" ) );
    TGNumberEntryField* stepIF_ = dynamic_cast<TGNumberEntryField*> ( FindFrameByName ( mf, "Table Step Width IF" ) );

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

        WriteTableToFile ( reacID, xMin_, xMax_, stepWidth_ );
    }
}

int main ( int argc, char *argv[] )
{
    // Create an interactive ROOT application
    TRint *theApp = new TRint ( "Rint", &argc, argv );

    CalcMonitor* calcM = CalcMonitor::CreateCalcMonitor();

    TGMainFrame* controlFrame = new TGMainFrame ( gClient->GetRoot(), 2000, 2000 );
    controlFrame->SetWindowName ( "ROOT Kinematic Calculator" );
    controlFrame->SetName ( "ROOT Kinematic Calculator" );
//     controlFrame->SetBackgroundColor ( 0x4d004d );

//     controlFrame->Connect ( "Destroyed()", "GoddessCalib", sinstance(), "DeleteSInstance()" );

    // ------ Creating Main IF Frame ------ //

    TGCompositeFrame* mainIFFrame = new TGCompositeFrame ( controlFrame, 2000, 2000 );
    mainIFFrame->SetName ( "Main IF Frame" );
    mainIFFrame->SetLayoutManager ( new TGRowLayout ( mainIFFrame, 20 ) );

    // ------ Creating Reaction List Frame ------ //

    TGCompositeFrame* reacFrame = new TGCompositeFrame ( mainIFFrame, 2000, 2000 );
    reacFrame->SetName ( "Reaction List Frame" );
    reacFrame->SetLayoutManager ( new TGColumnLayout ( reacFrame, 20 ) );

    // ------ Creating Reaction List Selectable Menu ------ //

    TGLabel* reacListLabel = new TGLabel ( reacFrame, "Reactions List" );

    TGListBox *reacListBox = new TGListBox ( reacFrame, 90 );
    reacListBox->SetName ( "Reactions ListBox" );
    reacListBox->Resize ( 420, 150 );
    reacListBox->SetMultipleSelections ( kTRUE );

    reacFrame->AddFrame ( reacListLabel );
    reacFrame->AddFrame ( reacListBox );

    // ------ Creating Axis Choice Buttons ------ //

    TGCompositeFrame* axisCBFrame = new TGCompositeFrame ( reacFrame, 2000, 2000 );
    axisCBFrame->SetName ( "Axis Check Buttons Frame" );
    axisCBFrame->SetLayoutManager ( new TGRowLayout ( axisCBFrame, 60 ) );

    TGButtonGroup* xAxisBG = new TGButtonGroup ( axisCBFrame, "X Axis", kVerticalFrame );
    xAxisBG->SetName ( "X Axis BG" );
    TGRadioButton* ejecLabAngleCBX = new TGRadioButton ( xAxisBG, "Ejec. Lab Angle" );
    ejecLabAngleCBX->SetName ( "Ejec. Lab Angle CB X" );
    ejecLabAngleCBX->SetState ( kButtonDown );
    TGRadioButton* ejecLabEnergyCBX = new TGRadioButton ( xAxisBG, "Ejec. Lab Energy" );
    ejecLabEnergyCBX->SetName ( "Ejec. Lab Energy CB X" );
    TGRadioButton* ejecCMAngleCBX = new TGRadioButton ( xAxisBG, "Ejec. C.M. Angle" );
    ejecCMAngleCBX->SetName ( "Ejec. C.M. Angle CB X" );
    TGRadioButton* recLabAngleCBX = new TGRadioButton ( xAxisBG, "Recoil Lab Angle" );
    recLabAngleCBX->SetName ( "Recoil Lab Angle CB X" );
    TGRadioButton* recLabEnergyCBX = new TGRadioButton ( xAxisBG, "Recoil Lab Energy" );
    recLabEnergyCBX->SetName ( "Recoil Lab Energy CB X" );
    TGRadioButton* recCMAngleCBX = new TGRadioButton ( xAxisBG, "Recoil C.M. Angle" );
    recCMAngleCBX->SetName ( "Recoil C.M. Angle CB X" );
    xAxisBG->Show();

    TGButtonGroup* yAxisBG = new TGButtonGroup ( axisCBFrame, "Y Axis", kVerticalFrame );
    yAxisBG->SetName ( "Y Axis BG" );
    TGRadioButton* ejecLabAngleCBY = new TGRadioButton ( yAxisBG, "Ejec. Lab Angle" );
    ejecLabAngleCBY->SetName ( "Ejec. Lab Angle CB X" );
    TGRadioButton* ejecLabEnergyCBY = new TGRadioButton ( yAxisBG, "Ejec. Lab Energy" );
    ejecLabEnergyCBY->SetName ( "Ejec. Lab Energy CB X" );
    ejecLabEnergyCBY->SetState ( kButtonDown );
    TGRadioButton* ejecCMAngleCBY = new TGRadioButton ( yAxisBG, "Ejec. C.M. Angle" );
    ejecCMAngleCBY->SetName ( "Ejec. C.M. Angle CB X" );
    TGRadioButton* recLabAngleCBY = new TGRadioButton ( yAxisBG, "Recoil Lab Angle" );
    recLabAngleCBY->SetName ( "Recoil Lab Angle CB X" );
    TGRadioButton* recLabEnergyCBY = new TGRadioButton ( yAxisBG, "Recoil Lab Energy" );
    recLabEnergyCBY->SetName ( "Recoil Lab Energy CB X" );
    TGRadioButton* recCMAngleCBY = new TGRadioButton ( yAxisBG, "Recoil C.M. Angle" );
    recCMAngleCBY->SetName ( "Recoil C.M. Angle CB X" );
    yAxisBG->Show();

    axisCBFrame->AddFrame ( xAxisBG );
    axisCBFrame->AddFrame ( yAxisBG );

    reacFrame->AddFrame ( axisCBFrame );

    // ------ Creating Axis Range IF and Plot Button ------ //

    TGCompositeFrame* plotOptionsFrame = new TGCompositeFrame ( reacFrame, 2000, 2000 );
    plotOptionsFrame->SetName ( "Plot Options Frame" );
    plotOptionsFrame->SetLayoutManager ( new TGRowLayout ( plotOptionsFrame, 25 ) );

    TGCompositeFrame* axisLabelsFrame = new TGCompositeFrame ( plotOptionsFrame, 2000, 2000 );
    axisLabelsFrame->SetName ( "Axis Labels Frame" );
    axisLabelsFrame->SetLayoutManager ( new TGColumnLayout ( axisLabelsFrame, 26 ) );

    TGLabel* xMinLabel = new TGLabel ( axisLabelsFrame, "X Min:" );
    TGLabel* xMaxLabel = new TGLabel ( axisLabelsFrame, "X Max:" );
    TGLabel* xStepLabel = new TGLabel ( axisLabelsFrame, "Step Width:" );

    axisLabelsFrame->AddFrame ( xMinLabel );
    axisLabelsFrame->AddFrame ( xMaxLabel );
    axisLabelsFrame->AddFrame ( xStepLabel );

    plotOptionsFrame->AddFrame ( axisLabelsFrame );

    TGCompositeFrame* axisIFFrame = new TGCompositeFrame ( plotOptionsFrame, 2000, 2000 );
    axisIFFrame->SetName ( "Axis IF Frame" );
    axisIFFrame->SetLayoutManager ( new TGColumnLayout ( axisIFFrame, 20 ) );

    TGNumberEntryField* xMinIF = new TGNumberEntryField ( axisIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    xMinIF->SetName ( "X Min IF" );
    xMinIF->SetNumber ( 0.0 );
    TGNumberEntryField* xMaxIF = new TGNumberEntryField ( axisIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    xMaxIF->SetName ( "X Max IF" );
    xMaxIF->SetNumber ( 180.0 );
    TGNumberEntryField* xStepIF = new TGNumberEntryField ( axisIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    xStepIF->SetName ( "Step Width IF" );
    xStepIF->SetNumber ( 1.0 );

    axisIFFrame->AddFrame ( xMinIF );
    axisIFFrame->AddFrame ( xMaxIF );
    axisIFFrame->AddFrame ( xStepIF );

    plotOptionsFrame->AddFrame ( axisIFFrame );

    TGCompositeFrame* plotButtonFrame = new TGCompositeFrame ( plotOptionsFrame, 2000, 2000 );
    plotButtonFrame->SetName ( "Plot Button Frame" );
    plotButtonFrame->SetLayoutManager ( new TGColumnLayout ( plotButtonFrame, 20 ) );

    TGLabel* emptySpace1 = new TGLabel ( plotButtonFrame, "" );
    TGTextButton* plotGraphs = new TGTextButton ( plotButtonFrame, "Plot Graphs", "RootKinCalc::OnClickPlotGraphs()" );
    plotGraphs->SetFont ( "-*-helvetica-medium-r-*-*-16-*-*-*-*-*-iso8859-1" );
    plotGraphs->Resize ( plotGraphs->GetDefaultSize() );

    plotButtonFrame->AddFrame ( emptySpace1 );
    plotButtonFrame->AddFrame ( plotGraphs );

    plotOptionsFrame->AddFrame ( plotButtonFrame );

    reacFrame->AddFrame ( plotOptionsFrame );

    mainIFFrame->AddFrame ( reacFrame );

    // ------ Creating Main Reaction IF Frame ------ //

    TGCompositeFrame* reacInfoFrame = new TGCompositeFrame ( mainIFFrame, 2000, 2000 );
    reacInfoFrame->SetName ( "Reaction Info Frame" );
    reacInfoFrame->SetLayoutManager ( new TGColumnLayout ( reacInfoFrame, 26 ) );

    TGCompositeFrame* reacIFsFrame = new TGCompositeFrame ( reacInfoFrame, 2000, 2000 );
    reacIFsFrame->SetName ( "Reaction Input Fields Frame" );
    reacIFsFrame->SetLayoutManager ( new TGRowLayout ( reacIFsFrame, 26 ) );

    // ------ Creating Reaction Labels Frame ------ //

    TGCompositeFrame* reacLabelsFrame = new TGCompositeFrame ( reacIFsFrame, 2000, 2000 );
    reacLabelsFrame->SetName ( "Reaction Labels Frame" );
    reacLabelsFrame->SetLayoutManager ( new TGColumnLayout ( reacLabelsFrame, 26 ) );

    TGLabel* emptyCell1 = new TGLabel ( reacLabelsFrame, "" );
    TGLabel* beamLabel = new TGLabel ( reacLabelsFrame, "Beam:" );
    TGLabel* targetLabel = new TGLabel ( reacLabelsFrame, "Target:" );
    TGLabel* ejecLabel = new TGLabel ( reacLabelsFrame, "Ejectile:" );
    TGLabel* recoilLabel = new TGLabel ( reacLabelsFrame, "Recoil:" );

    reacLabelsFrame->AddFrame ( emptyCell1 );
    reacLabelsFrame->AddFrame ( beamLabel );
    reacLabelsFrame->AddFrame ( targetLabel );
    reacLabelsFrame->AddFrame ( ejecLabel );
    reacLabelsFrame->AddFrame ( recoilLabel );

    reacIFsFrame->AddFrame ( reacLabelsFrame );

    // ------ Creating Elements IF Labels Frame ------ //

    TGCompositeFrame* elIFFrame = new TGCompositeFrame ( reacIFsFrame, 2000, 2000 );
    elIFFrame->SetName ( "Elements IF Frame" );
    elIFFrame->SetLayoutManager ( new TGColumnLayout ( elIFFrame, 20 ) );

    TGLabel* elLabel = new TGLabel ( elIFFrame, "               Element" );
    beamElLabel = new TGTextEntry ( elIFFrame );
    beamElLabel->SetName ( "Beam Element IF" );
    targetElLabel = new TGTextEntry ( elIFFrame );
    targetElLabel->SetName ( "Target Element IF" );
    ejecElLabel = new TGTextEntry ( elIFFrame );
    ejecElLabel->SetName ( "Ejectile Element IF" );
    recoilElLabel = new TGTextEntry ( elIFFrame );
    recoilElLabel->SetName ( "Recoil Element IF" );
    recoilElLabel->SetState ( kFALSE );

    elIFFrame->AddFrame ( elLabel );
    elIFFrame->AddFrame ( beamElLabel );
    elIFFrame->AddFrame ( targetElLabel );
    elIFFrame->AddFrame ( ejecElLabel );
    elIFFrame->AddFrame ( recoilElLabel );

    reacIFsFrame->AddFrame ( elIFFrame );

    // ------ Creating Kinetic Energy IF Labels Frame ------ //

    TGCompositeFrame* keIFFrame = new TGCompositeFrame ( reacIFsFrame, 2000, 2000 );
    keIFFrame->SetName ( "Kinetic Energy IF Frame" );
    keIFFrame->SetLayoutManager ( new TGColumnLayout ( keIFFrame, 20 ) );

    TGLabel* keLabel = new TGLabel ( keIFFrame, "    Kinetic Energy (MeV)" );
    beamKeLabel = new TGNumberEntryField ( keIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    beamKeLabel->SetNumber ( 0 );
//     targetKeLabel = new TGNumberEntryField ( keIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
//     ejecKeLabel = new TGNumberEntryField ( keIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
//     recoilKeLabel = new TGNumberEntryField ( keIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
//     recoilElLabel->SetState ( kFALSE );

    keIFFrame->AddFrame ( keLabel );
    keIFFrame->AddFrame ( beamKeLabel );
//     keIFFrame->AddFrame ( targetKeLabel );
//     keIFFrame->AddFrame ( ejecKeLabel );
//     keIFFrame->AddFrame ( recoilKeLabel );

    reacIFsFrame->AddFrame ( keIFFrame );

    // ------ Creating Excitation Energy IF Labels Frame ------ //

    TGCompositeFrame* exIFFrame = new TGCompositeFrame ( reacIFsFrame, 2000, 2000 );
    exIFFrame->SetName ( "Excitation Energy IF Frame" );
    exIFFrame->SetLayoutManager ( new TGColumnLayout ( exIFFrame, 20 ) );

    TGLabel* exLabel = new TGLabel ( exIFFrame, "  Excitation Energy (MeV)" );
    beamExLabel = new TGNumberEntryField ( exIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    beamExLabel->SetNumber ( 0 );
    targetExLabel = new TGNumberEntryField ( exIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    targetExLabel->SetNumber ( 0 );
    ejecExLabel = new TGNumberEntryField ( exIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    ejecExLabel->SetNumber ( 0 );
    recoilExLabel = new TGNumberEntryField ( exIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    recoilExLabel->SetNumber ( 0 );
    recoilElLabel->SetState ( kFALSE );

    exIFFrame->AddFrame ( exLabel );
    exIFFrame->AddFrame ( beamExLabel );
    exIFFrame->AddFrame ( targetExLabel );
    exIFFrame->AddFrame ( ejecExLabel );
    exIFFrame->AddFrame ( recoilExLabel );

    reacIFsFrame->AddFrame ( exIFFrame );

    reacInfoFrame->AddFrame ( reacIFsFrame );

    // ------ Adding the Reaction Threshold Info ------ //

    TGCompositeFrame* threshInfoFrame = new TGCompositeFrame ( reacInfoFrame, 2000, 2000 );
    threshInfoFrame->SetName ( "Reaction Threshold Info Frame" );
    threshInfoFrame->SetLayoutManager ( new TGRowLayout ( threshInfoFrame, 20 ) );

    TGLabel* beamCMEnergyLabel = new TGLabel ( threshInfoFrame, "Beam C.M. Energy" );
    TGNumberEntryField* beamCMEnIF = new TGNumberEntryField ( threshInfoFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    beamCMEnIF->SetName ( "Beam C.M. Energy IF" );
    beamCMEnIF->Resize ( beamCMEnIF->GetDefaultSize() );
    beamCMEnIF->SetState ( kFALSE );

    TGLabel* threshInfoLabel = new TGLabel ( threshInfoFrame, "G.S. to G.S.\nReaction Q value" );
    TGNumberEntryField* threshInfoIF = new TGNumberEntryField ( threshInfoFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    threshInfoIF->SetName ( "Reaction Threshold IF" );
    threshInfoIF->SetFont ( "-*-helvetica-medium-r-*-*-16-*-*-*-*-*-iso8859-1" );
    threshInfoIF->Resize ( threshInfoIF->GetDefaultSize() );
    threshInfoIF->SetState ( kFALSE );

    threshInfoFrame->AddFrame ( beamCMEnergyLabel );
    threshInfoFrame->AddFrame ( beamCMEnIF );

    threshInfoFrame->AddFrame ( threshInfoLabel );
    threshInfoFrame->AddFrame ( threshInfoIF );

    reacInfoFrame->AddFrame ( threshInfoFrame );

    // ------ Adding the Get Reaction Kinematic Button ------ //

    TGCompositeFrame* kinCalcButtonsFrame = new TGCompositeFrame ( reacInfoFrame, 2000, 2000 );
    kinCalcButtonsFrame->SetName ( "Kinematic Calculation Buttons Frame" );
    kinCalcButtonsFrame->SetLayoutManager ( new TGRowLayout ( kinCalcButtonsFrame, 20 ) );

    TGTextButton* updateReacKinInfo = new TGTextButton ( kinCalcButtonsFrame, "Update Info", "RootKinCalc::OnClickUpdateInfo()" );
    updateReacKinInfo->SetFont ( "-*-helvetica-medium-r-*-*-16-*-*-*-*-*-iso8859-1" );
    updateReacKinInfo->Resize ( updateReacKinInfo->GetDefaultSize() );

    TGTextButton* getReacKin = new TGTextButton ( kinCalcButtonsFrame, "Get Reaction Kinematic", "RootKinCalc::OnClickCalcKin()" );
    getReacKin->SetFont ( "-*-helvetica-medium-r-*-*-16-*-*-*-*-*-iso8859-1" );
    getReacKin->Resize ( getReacKin->GetDefaultSize() );

    kinCalcButtonsFrame->AddFrame ( updateReacKinInfo );
    kinCalcButtonsFrame->AddFrame ( getReacKin );

    reacInfoFrame->AddFrame ( kinCalcButtonsFrame );

    // ------ Adding the Table Output Menu ------ //

    TGCompositeFrame* tableOutputMainFrame = new TGCompositeFrame ( reacInfoFrame, 2000, 2000 );
    tableOutputMainFrame->SetName ( "Table Output Main Frame" );
    tableOutputMainFrame->SetLayoutManager ( new TGRowLayout ( tableOutputMainFrame, 20 ) );

    TGCompositeFrame* tableLablesFrame = new TGCompositeFrame ( tableOutputMainFrame, 2000, 2000 );
    tableLablesFrame->SetName ( "Table Labels Frame" );
    tableLablesFrame->SetLayoutManager ( new TGColumnLayout ( tableLablesFrame, 26 ) );

    TGLabel* tableXMinLabel = new TGLabel ( tableLablesFrame, "X Min:" );
    TGLabel* tableXMaxLabel = new TGLabel ( tableLablesFrame, "X Max:" );
    TGLabel* tableStepWidthLabel = new TGLabel ( tableLablesFrame, "Step Width:" );

    tableLablesFrame->AddFrame ( tableXMinLabel );
    tableLablesFrame->AddFrame ( tableXMaxLabel );
    tableLablesFrame->AddFrame ( tableStepWidthLabel );

    tableOutputMainFrame->AddFrame ( tableLablesFrame );

    TGCompositeFrame* tableIFFrame = new TGCompositeFrame ( tableOutputMainFrame, 2000, 2000 );
    tableIFFrame->SetName ( "Table IF Frame" );
    tableIFFrame->SetLayoutManager ( new TGColumnLayout ( tableIFFrame, 20 ) );

    TGNumberEntryField* tableXMinIF = new TGNumberEntryField ( tableIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    tableXMinIF->SetName ( "Table XMin IF" );
    tableXMinIF->Resize ( beamCMEnIF->GetDefaultSize() );
    tableXMinIF->SetNumber ( 0 );
    TGNumberEntryField* tableXMaxIF = new TGNumberEntryField ( tableIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    tableXMaxIF->SetName ( "Table XMax IF" );
    tableXMaxIF->Resize ( beamCMEnIF->GetDefaultSize() );
    tableXMaxIF->SetNumber ( 180 );
    TGNumberEntryField* tableStepWidthIF = new TGNumberEntryField ( tableIFFrame, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    tableStepWidthIF->SetName ( "Table Step Width IF" );
    tableStepWidthIF->Resize ( beamCMEnIF->GetDefaultSize() );
    tableStepWidthIF->SetNumber ( 1 );

    tableIFFrame->AddFrame ( tableXMinIF );
    tableIFFrame->AddFrame ( tableXMaxIF );
    tableIFFrame->AddFrame ( tableStepWidthIF );

    tableOutputMainFrame->AddFrame ( tableIFFrame );

    TGCompositeFrame* tableButtonsFrame = new TGCompositeFrame ( tableOutputMainFrame, 2000, 2000 );
    tableButtonsFrame->SetName ( "Table Buttons Frame" );
    tableButtonsFrame->SetLayoutManager ( new TGColumnLayout ( tableButtonsFrame, 20 ) );

    TGTextButton* writeTableButton = new TGTextButton ( tableButtonsFrame, "Write Table", "RootKinCalc::OnClickWriteTable()" );
    writeTableButton->SetFont ( "-*-helvetica-medium-r-*-*-16-*-*-*-*-*-iso8859-1" );
    writeTableButton->Resize ( writeTableButton->GetDefaultSize() );

    tableButtonsFrame->AddFrame ( writeTableButton );

    tableOutputMainFrame->AddFrame ( tableButtonsFrame );

    reacInfoFrame->AddFrame ( tableOutputMainFrame );

    // ------ Adding the Single Calculation Menu ------ //

    TGCompositeFrame* singleCalcMainFrame = new TGCompositeFrame ( reacInfoFrame, 2000, 2000 );
    singleCalcMainFrame->SetName ( "Single Calculation Main Frame" );
    singleCalcMainFrame->SetLayoutManager ( new TGRowLayout ( singleCalcMainFrame, 20 ) );

    TGCompositeFrame* singleCalcLabelsSubFrame1 = new TGCompositeFrame ( singleCalcMainFrame, 2000, 2000 );
    singleCalcLabelsSubFrame1->SetName ( "Single Calc Labels Sub Frame 1" );
    singleCalcLabelsSubFrame1->SetLayoutManager ( new TGColumnLayout ( singleCalcLabelsSubFrame1, 26 ) );

    TGRadioButton* sCEjecLabAngleRB = new TGRadioButton ( singleCalcLabelsSubFrame1, "Ejec. Lab Angle" );
    sCEjecLabAngleRB->SetName ( "Ejec. Lab Angle RB SC" );
    sCEjecLabAngleRB->SetState ( kButtonDown );
    TQObject::Connect ( sCEjecLabAngleRB, "Clicked()", "CalcMonitor", CalcMonitor::sinstance(), "OnClickSingleCalcRB()" );
    TGRadioButton* esCEecLabEnergyRB = new TGRadioButton ( singleCalcLabelsSubFrame1, "Ejec. Lab Energy" );
    esCEecLabEnergyRB->SetName ( "Ejec. Lab Energy RB SC" );
    TQObject::Connect ( esCEecLabEnergyRB, "Clicked()", "CalcMonitor", CalcMonitor::sinstance(), "OnClickSingleCalcRB()" );
    TGRadioButton* sCEjecCMAngleRB = new TGRadioButton ( singleCalcLabelsSubFrame1, "Ejec. C.M. Angle" );
    sCEjecCMAngleRB->SetName ( "Ejec. C.M. Angle RB SC" );
    TQObject::Connect ( sCEjecCMAngleRB, "Clicked()", "CalcMonitor", CalcMonitor::sinstance(), "OnClickSingleCalcRB()" );

    singleCalcLabelsSubFrame1->AddFrame ( sCEjecLabAngleRB );
    singleCalcLabelsSubFrame1->AddFrame ( esCEecLabEnergyRB );
    singleCalcLabelsSubFrame1->AddFrame ( sCEjecCMAngleRB );

    singleCalcMainFrame->AddFrame ( singleCalcLabelsSubFrame1 );

    TGCompositeFrame* singleCalcIFSubFrame1 = new TGCompositeFrame ( singleCalcMainFrame, 2000, 2000 );
    singleCalcIFSubFrame1->SetName ( "Single Calculation IF Sub Frame 1" );
    singleCalcIFSubFrame1->SetLayoutManager ( new TGColumnLayout ( singleCalcIFSubFrame1, 20 ) );

    TGNumberEntryField* sCEjecLabAngleIF = new TGNumberEntryField ( singleCalcIFSubFrame1, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    sCEjecLabAngleIF->SetName ( "Ejec. Lab Angle IF SC" );
    sCEjecLabAngleIF->Resize ( sCEjecLabAngleIF->GetDefaultSize() );
    sCEjecLabAngleIF->SetNumber ( 0 );
    TGNumberEntryField* esCEecLabEnergyIF = new TGNumberEntryField ( singleCalcIFSubFrame1, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    esCEecLabEnergyIF->SetName ( "Ejec. Lab Energy IF SC" );
    esCEecLabEnergyIF->Resize ( esCEecLabEnergyIF->GetDefaultSize() );
    esCEecLabEnergyIF->SetState ( kFALSE );
    TGNumberEntryField* sCEjecCMAngleIF = new TGNumberEntryField ( singleCalcIFSubFrame1, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    sCEjecCMAngleIF->SetName ( "Ejec. C.M. Angle IF SC" );
    sCEjecCMAngleIF->Resize ( sCEjecCMAngleIF->GetDefaultSize() );
    sCEjecCMAngleIF->SetState ( kFALSE );

    singleCalcIFSubFrame1->AddFrame ( sCEjecLabAngleIF );
    singleCalcIFSubFrame1->AddFrame ( esCEecLabEnergyIF );
    singleCalcIFSubFrame1->AddFrame ( sCEjecCMAngleIF );

    singleCalcMainFrame->AddFrame ( singleCalcIFSubFrame1 );

    TGCompositeFrame* singleCalcLabelsSubFrame2 = new TGCompositeFrame ( singleCalcMainFrame, 2000, 2000 );
    singleCalcLabelsSubFrame2->SetName ( "Single Calc Labels Sub Frame 2" );
    singleCalcLabelsSubFrame2->SetLayoutManager ( new TGColumnLayout ( singleCalcLabelsSubFrame2, 26 ) );

    TGRadioButton* sCRecLabAngleRB = new TGRadioButton ( singleCalcLabelsSubFrame2, "Recoil Lab Angle" );
    sCRecLabAngleRB->SetName ( "Recoil Lab Angle RB SC" );
    TQObject::Connect ( sCRecLabAngleRB, "Clicked()", "CalcMonitor", CalcMonitor::sinstance(), "OnClickSingleCalcRB()" );
    TGRadioButton* sCRecLabEnergyRB = new TGRadioButton ( singleCalcLabelsSubFrame2, "Recoil Lab Energy" );
    sCRecLabEnergyRB->SetName ( "Recoil Lab Energy RB SC" );
    TQObject::Connect ( sCRecLabEnergyRB, "Clicked()", "CalcMonitor", CalcMonitor::sinstance(), "OnClickSingleCalcRB()" );
    TGRadioButton* sCRecCMAngleRB = new TGRadioButton ( singleCalcLabelsSubFrame2, "Recoil C.M. Angle" );
    sCRecCMAngleRB->SetName ( "Recoil C.M. Angle RB SC" );
    TQObject::Connect ( sCRecCMAngleRB, "Clicked()", "CalcMonitor", CalcMonitor::sinstance(), "OnClickSingleCalcRB()" );

    singleCalcLabelsSubFrame2->AddFrame ( sCRecLabAngleRB );
    singleCalcLabelsSubFrame2->AddFrame ( sCRecLabEnergyRB );
    singleCalcLabelsSubFrame2->AddFrame ( sCRecCMAngleRB );

    singleCalcMainFrame->AddFrame ( singleCalcLabelsSubFrame2 );

    TGCompositeFrame* singleCalcIFSubFrame2 = new TGCompositeFrame ( singleCalcMainFrame, 2000, 2000 );
    singleCalcIFSubFrame2->SetName ( "Single Calculation IF Sub Frame 2" );
    singleCalcIFSubFrame2->SetLayoutManager ( new TGColumnLayout ( singleCalcIFSubFrame2, 20 ) );

    TGNumberEntryField* sCRecLabAngleIF = new TGNumberEntryField ( singleCalcIFSubFrame2, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    sCRecLabAngleIF->SetName ( "Ejec. C.M. Angle IF SC" );
    sCRecLabAngleIF->Resize ( sCRecLabAngleIF->GetDefaultSize() );
    sCRecLabAngleIF->SetState ( kFALSE );
    TGNumberEntryField* sCRecLabEnergyIF = new TGNumberEntryField ( singleCalcIFSubFrame2, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    sCRecLabEnergyIF->SetName ( "Recoil Lab Energy IF SC" );
    sCRecLabEnergyIF->Resize ( sCRecLabEnergyIF->GetDefaultSize() );
    sCRecLabEnergyIF->SetState ( kFALSE );
    TGNumberEntryField* sCRecCMAngleIF = new TGNumberEntryField ( singleCalcIFSubFrame2, -1, 0, TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive );
    sCRecCMAngleIF->SetName ( "Recoil C.M. Angle IF SC" );
    sCRecCMAngleIF->Resize ( sCRecCMAngleIF->GetDefaultSize() );
    sCRecCMAngleIF->SetState ( kFALSE );

    singleCalcIFSubFrame2->AddFrame ( sCRecLabAngleIF );
    singleCalcIFSubFrame2->AddFrame ( sCRecLabEnergyIF );
    singleCalcIFSubFrame2->AddFrame ( sCRecCMAngleIF );

    singleCalcMainFrame->AddFrame ( singleCalcIFSubFrame2 );

    reacInfoFrame->AddFrame ( singleCalcMainFrame );

    // ------ Wraping everything in the main frame ------ //

    mainIFFrame->AddFrame ( reacInfoFrame );

    controlFrame->AddFrame ( mainIFFrame, new TGLayoutHints ( kLHintsLeft, 20, 20, 20, 10 ) );

//     controlFrame->ChangeSubframesBackground ( 0x4d004d );

    controlFrame->MapSubwindows();
    controlFrame->Resize ( controlFrame->GetDefaultSize() );
//     vertFrame->Resize ( vertFrame->GetDefaultSize() );
//     firstFrame->Resize ( firstFrame->GetDefaultSize() );
    controlFrame->MapWindow();

    theApp->Run();
}










