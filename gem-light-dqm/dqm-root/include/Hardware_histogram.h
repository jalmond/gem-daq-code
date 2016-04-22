#include "TDirectory.h"
#include "TFile.h"
#include <string>

class Hardware_histogram
{
  public:
    Hardware_histogram(const std::string & filename, TDirectory * dir, const std::string & hwid){
      //m_file = new TFile(filename.c_str(), "UPDATE");
      m_dir = dir;
      m_HWID = hwid;
    }
    virtual ~Hardware_histogram(){}//m_file->Close();}
    void bookHistograms(){}
    TDirectory * getTDir(){return m_dir;}
    std::string getHWID(){return m_HWID;}
  protected:
    TDirectory *m_dir;
    //TFile *m_file;
    std::string m_HWID;
};
