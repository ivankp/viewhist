#include "HistFileReader.h"

#include <iostream>
#include <stdexcept>

#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"
#include "TCollection.h"
#include "TKey.h"
#include "TList.h"

#include "TCanvas.h"
#include "TH1.h"

using namespace std;

HistFileReader::HistFileReader(const char* fname)
: fFile(NULL)
{
  if (fFile) delete fFile;
  fFile = new TFile(fname,"read");

  // if there is a problem opening file
  if (fFile->IsZombie()) {
    throw runtime_error(string("Cannot open file: ")+fname);
  }

  //cout << "Opened root file: " << fFile->GetName() << endl;

  // Read the file
  TKey *key1;
  TIter nextkey1(fFile->GetListOfKeys());
  while ((key1 = (TKey*)nextkey1())) {
    TObject *obj = key1->ReadObj();

    if (obj->IsFolder()) {
      TDirectory* dir = (TDirectory*)obj;
      //cout << dir->GetName() << endl << endl;

      TKey *key2;
      TIter nextkey2(dir->GetListOfKeys());
      while ((key2 = (TKey*)nextkey2())) {
        obj = key2->ReadObj();

        string histname(obj->GetName());
        histname = histname.substr(0,histname.find_last_of('_'));
        //cout << histname << endl;
        hists[histname].push_back((TH1*)obj);
      }

    } // end if folder
  } // end while key

  const size_t size = hists.size();
  its = new h_it[size];
  for (size_t i=0;i<size;i++) {
    static h_it it = hists.begin();
    its[i] = it;
    ++it;
  }

}

HistFileReader::~HistFileReader() {
  fFile->Close();
  delete fFile;
  delete[] its;
}

const char* HistFileReader::GetFileName() const {
  return fFile->GetName();
}

size_t HistFileReader::GetNum() const {
  return hists.size();
}

const string& HistFileReader::GetHistName(size_t i) const {
  return its[i]->first;
}

const vector<TH1*>& HistFileReader::GetHists(const string& name) {
  return hists[name];
}
const vector<TH1*>& HistFileReader::GetHists(size_t i) {
  return its[i]->second;
}
