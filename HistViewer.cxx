#include "HistViewer.h"
#include "HistFileReader.h"

#include <iostream>
#include <exception>

#include "TApplication.h"
#include "TGClient.h"
#include "TGWindow.h"
#include "TGFrame.h"
#include "TGMenu.h"
#include "TGLayout.h"
#include "TGLabel.h"
#include "TRootEmbeddedCanvas.h"
#include "TGSplitter.h"
#include "TGListBox.h"
#include "TGFileDialog.h"

#include "TColor.h"
#include "TCanvas.h"
#include "TH1.h"

using namespace std;

enum ETestCommandIdentifiers {
  MENU_OPEN,
  MENU_CLOSE,
  MENU_SAVE,
  MENU_EXIT,

  M_HELP_CONTENTS,
  M_HELP_SEARCH,
  M_HELP_ABOUT
};

const char *opentypes[] = { "ROOT files",    "*.root",
                            "All files",     "*",
                             0,               0 };
const char *savetypes[] = { "All files",     "*",
                            "PDF files",     "*.pdf",
                            "PNG files",     "*.png",
                            "ROOT files",    "*.root",
                             0,               0 };

class FileInfo: public TGFileInfo {
public:
  FileInfo(const char** types, const char* dir=".") {
    fFileTypes = types;
    fIniDir    = StrDup(dir);
  }
  virtual ~FileInfo() { }
};

HistViewer::HistViewer(Int_t w, Int_t h, Int_t wmin, Int_t hmin)
: fReader(NULL)
{
  // Create test main frame. A TGMainFrame is a top level window.
  fMain = new TGMainFrame(gClient->GetRoot());

  fMain->SetWindowName(gApplication->ApplicationName());
  fMain->SetWMSize(wmin, hmin); // Minimum window size

  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);

  // Hints **********************************************************
  // hints used several times

  TGLayoutHints *HintExpand = new TGLayoutHints(
    kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0
  );
  TGLayoutHints *HintLeftY = new TGLayoutHints(
    kLHintsLeft | kLHintsExpandY
  );

  // Menu ***********************************************************
  fMenuFrame = new TGHorizontalFrame(fMain);
  fMain->AddFrame(fMenuFrame, new TGLayoutHints(kLHintsExpandX, 0, 0, 1, 1));

  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
  fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

  fMenuFile = new TGPopupMenu(gClient->GetRoot());
  fMenuFile->AddEntry("&Open file", MENU_OPEN);
  fMenuFile->AddEntry("&Close file", MENU_CLOSE);
  fMenuFile->AddEntry("&Save canvas", MENU_SAVE);
  fMenuFile->AddEntry("&Quit", MENU_EXIT);

  fMenuFile->DisableEntry(MENU_CLOSE);
  fMenuFile->DisableEntry(MENU_SAVE);

  fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("&Contents", M_HELP_CONTENTS);
  fMenuHelp->AddEntry("&Search", M_HELP_SEARCH);
  fMenuHelp->AddSeparator();
  fMenuHelp->AddEntry("&About", M_HELP_ABOUT);

  fMenuBar = new TGMenuBar(fMenuFrame, 1, 1, kHorizontalFrame);
  fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

  fMenuFrame->AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

  // Canvas *********************************************************
  fCanvasFrame = new TGHorizontalFrame(fMain);

  fVleft  = new TGVerticalFrame(fCanvasFrame, 10, 10, kFixedWidth);
  fVright = new TGVerticalFrame(fCanvasFrame, 10, 10);

  fVleft->AddFrame( fHistListBox = new TGListBox(fVleft), HintExpand );

  fECanvas = new TRootEmbeddedCanvas("canvas", fVright);
  fCanvas = fECanvas->GetCanvas(); // Get pointer to the TCanvas
  fCanvas->SetBorderMode(0);

  fVright->AddFrame(fECanvas, HintExpand);

  fVleft ->Resize(100, fVleft->GetDefaultHeight());
  fVright->Resize(fECanvas->GetDefaultWidth(), fVleft->GetDefaultHeight());
  fCanvasFrame->AddFrame(fVleft, HintLeftY);

  TGVSplitter *splitter = new TGVSplitter(fCanvasFrame,3,2);
  splitter->SetFrame(fVleft, kTRUE);
  fCanvasFrame->AddFrame(splitter, HintLeftY);

  fCanvasFrame->AddFrame(fVright, HintExpand);
  fMain->AddFrame(fCanvasFrame, HintExpand);

  // Status *********************************************************
  fStatusFrame = new TGHorizontalFrame(fMain, 60, 20, kSunkenFrame);
  fMain->AddFrame(fStatusFrame, new TGLayoutHints(kLHintsBottom |
                  kLHintsExpandX, 0, 0, 2, 2));

  fFileName = new TGLabel(fStatusFrame);
  fStatusFrame->AddFrame(fFileName, new TGLayoutHints(kLHintsTop |
                         kLHintsLeft, 2, 2, 2, 2));

  // Signals ********************************************************
  fMain->Connect("CloseWindow()", "HistViewer", this, "CloseWindow()");

  fMenuFile->Connect("Activated(Int_t)", "HistViewer", this, "HandleMenu(Int_t)");

  fHistListBox->Connect("Selected(Int_t)", "HistViewer", this, "Plot(Int_t)");

  // Open file from the terminal argument ***************************
  if (gApplication->Argc()>1) OpenFile(gApplication->Argv(1));

  // Misc ***********************************************************
  fMain->Resize(w, h); // Initial window size

  fMain->MapSubwindows();
  fMain->Resize();
  fMain->MapWindow();

  //fMain->Print();

  // struct for open file menu
  //fFileInfo->fFileTypes = filetypes;
  //fFileInfo->fIniDir = init_dir;
}

HistViewer::~HistViewer()
{
  delete fMain;
  if (fReader) delete fReader;
  //delete fFileInfo;

  //cout << "Deleted HistViewer" << endl;
}

void HistViewer::CloseWindow()
{
  // Got close message for this MainFrame. Terminates the application.
  delete this;
  gApplication->Terminate();
}

void HistViewer::OpenFile(const char* fname)
{
  fMenuFile->EnableEntry(MENU_CLOSE);
  fMenuFile->EnableEntry(MENU_SAVE);

  try {

    fReader = new HistFileReader(fname);
    fFileName->SetText( fReader->GetFileName() );

    // red text
    Pixel_t black;
    gClient->GetColorByName("black", black);
    fFileName->SetTextColor(black);

    for (size_t i=0;i<fReader->GetNum();i++)
      fHistListBox->AddEntry(fReader->GetHistName(i).c_str(),i);

    fVleft->Resize(
      fHistListBox->GetDefaultWidth()+30,
      fVleft->GetDefaultHeight()
    );
    fMain->Resize();

    // !!! Opening a second file causes
    // !!! segmentation violation
    fMenuFile->DisableEntry(MENU_OPEN);

  } catch (exception& e) {
    fFileName->SetText(e.what());

    // red text
    Pixel_t red;
    gClient->GetColorByName("red", red);
    fFileName->SetTextColor(red);
  }
}

void HistViewer::CloseFile()
{
  if (fReader) {
    cout << "Closed file " << fReader->GetFileName() << endl;
    delete fReader;
    fCanvas->Clear();
    fCanvas->Flush();
    fHistListBox->RemoveAll();
    fFileName->SetText("");
    fReader = NULL;

    fMenuFile->DisableEntry(MENU_CLOSE);
    fMenuFile->DisableEntry(MENU_SAVE);
    // !!! Opening a second file causes
    // !!! segmentation violation
    //fMenuFile->EnableEntry(MENU_OPEN);
  }
}

void HistViewer::HandleMenu(Int_t id)
{
  switch (id) {

    case MENU_OPEN:
    {
      static FileInfo fi(opentypes);
      new TGFileDialog(gClient->GetRoot(), fMain, kFDOpen, &fi);
      // dialog doesn't need to be deleted

      if (fi.fFilename) {
        CloseFile(); // <-- will be useful when can open second file
        OpenFile(fi.fFilename);
      }
    }
    break;

    case MENU_CLOSE: CloseFile(); break;

    case MENU_SAVE:
    {
      static FileInfo fi(savetypes);
      new TGFileDialog(gClient->GetRoot(), fMain, kFDSave, &fi);
      // dialog doesn't need to be deleted

      if (fi.fFilename) SaveCanvas(fi.fFilename);
    }
    break;

    case MENU_EXIT: CloseWindow(); break;

    case M_HELP_CONTENTS:

    break;

    case M_HELP_SEARCH:

    break;

    case M_HELP_ABOUT:

    break;
  }
}

void HistViewer::SaveCanvas(const char* fname)
{
  fCanvas->SaveAs(fname);
}

void HistViewer::Plot(Int_t id)
{
  //cout << "Hist " << id << ": " << fReader->GetHistName(id) << endl;

  fCanvas->Clear();

  const vector<TH1*>& hists = fReader->GetHists(id);
  for (vector<TH1*>::const_iterator it=hists.begin();it!=hists.end();++it) {
    it==hists.begin() ? (*it)->Draw() : (*it)->Draw("same");
    //cout << "Drew " << (*it)->GetName() << endl;
  }
  fCanvas->Update();
}
