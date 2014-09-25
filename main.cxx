#include <iostream>

#include "TApplication.h"
#include "TGClient.h"

#include "HistViewer.h"

int main(int argc, char** argv)
{

  // Create application
  TApplication* RootApp = new TApplication("Hist ntuple viewer", &argc, argv);

  HistViewer* hv = new HistViewer(600,400,300,200);

  RootApp->Run();

  // Cleanup
  delete hv;
  delete RootApp;

  return 0;
}
