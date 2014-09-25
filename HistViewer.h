#ifndef HistViewer_h
#define HistViewer_h

#include "RQ_OBJECT.h"

class TGMainFrame;
class TGHorizontalFrame;
class TGVerticalFrame;
class TGMenuBar;
class TGPopupMenu;
class TGLabel;
class TGLayoutHints;
class TRootEmbeddedCanvas;
class TGListBox;
//class TGFileInfo;

class TCanvas;

class HistFileReader;

class HistViewer {

RQ_OBJECT("HistViewer")

private:

  // GUI members
  TGMainFrame         *fMain;
  TGHorizontalFrame   *fMenuFrame, *fCanvasFrame, *fStatusFrame;
  TGVerticalFrame     *fVleft, *fVright;

  TGMenuBar           *fMenuBar;
  TGPopupMenu         *fMenuFile, *fMenuHelp;
  TGLayoutHints       *fMenuBarItemLayout, *fMenuBarHelpLayout;

  TGLabel             *fFileName;

  TGListBox           *fHistListBox;
  TRootEmbeddedCanvas *fECanvas;

//  TGFileInfo          *fFileInfo;

  // Plotting members
  TCanvas *fCanvas;

  // Data members
  HistFileReader *fReader;

public:
  HistViewer(Int_t w, Int_t h, Int_t wmin=0, Int_t hmin=0);
  virtual ~HistViewer();

  // slots
  void HandleMenu(Int_t id);
  void CloseWindow();

  void OpenFile(const char* fname);
  void CloseFile();

  void SaveCanvas(const char* fname);

  void Plot(Int_t id);
};

#endif
