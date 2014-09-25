#ifndef HistFileReader_h
#define HistFileReader_h

#include <string>
#include <vector>
#include <map>

class TFile;
class TH1;

class HistFileReader {

private:
  TFile* fFile;

  typedef std::map<std::string, std::vector<TH1*> > h_map;
  typedef h_map::iterator h_it;

  h_map hists;
  h_it* its;

public:
  HistFileReader(const char* fname);
  virtual ~HistFileReader();

  const char* GetFileName() const;

  size_t GetNum() const;
  const std::string& GetHistName(size_t i) const;
  const std::vector<TH1*>& GetHists(const std::string& name);
  const std::vector<TH1*>& GetHists(size_t i);
};

#endif
