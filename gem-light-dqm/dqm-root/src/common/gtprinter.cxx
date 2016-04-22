#define DEBUG 0
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <cstdint>
#include <TFile.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TNtuple.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TRandom3.h>
#include <TBenchmark.h>
#include <TInterpreter.h>
#include <TApplication.h>
#include <TString.h>
#include <Event.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>
#include <TRef.h>
#include <TH1.h>
#include <TBits.h>
#include <TMath.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TTree.h>
#include <TBranch.h>
#include <time.h>
#include <math.h>

#include "plotter.cxx"

using namespace std;

int main(int argc, char** argv)
{
  gErrorIgnoreLevel = kWarning;
  cout << endl;
  if (argc!=2) 
    {
      cout << "Please provide ONE root file." << endl;
      cout << "Usage: <path>/printer input.root" << endl;
      return 0;
    }
  
  TString ifilename = argv[1];
  string ifilestring = argv[1];
  TFile *ifile;

  if(ifilestring.substr(ifilestring.length()-5,5)!=".root")
    {		
      cout << "Input File must be in .root format." << endl;
      cout << endl;
      return 0;
    }

  ifile = new TFile(ifilename, "READ");
  if(DEBUG) std::cout<<"[gtprinter]" << "ifile: " << ifile->GetName() << std::endl;
  TDirectory *idir = gDirectory->GetDirectory("");
  if(DEBUG) std::cout<<"[gtprinter]" << "idir: " << idir->GetName() << std::endl;


  TString iname; //name of file without .analyzed.root extension

  TString name = ifilename;
  int loc = name.Last('/')+1;
  int len = name.Length()-loc-14;
  TSubString subname = name(loc,len);
  iname = subname;
  if(DEBUG) std::cout<<"[gtprinter]"<< "iname: " << iname << std::endl;

  //Create date path to hold prints based on time printer was run
  // time_t t = time(NULL);
  // tm* timePtr = localtime(&t);
  //char oPath [64];
  // strftime (oPath, 64, "./output/%m_%d-%H/%b%d_%G-%H_%M_%S/", timePtr);

  TString dPath = "/tmp/"+iname+"/"; 
  if(DEBUG) std::cout<<"[gtprinter]" << "dPath: " << dPath << std::endl;

  gROOT->ProcessLine(".!mkdir -p "+dPath);
  
  // TFile *ofile = new TFile(dPath+iname+".root", "RECREATE");
  // cout<<"Output File: "<<dPath<<iname<<".root"<<endl;
    

  gemTreePrint(idir,dPath,true);

  if(DEBUG) std::cout<<"[gtprinter]"<< "Printing complete. " << iname << std::endl;

  
  return 0;
}
