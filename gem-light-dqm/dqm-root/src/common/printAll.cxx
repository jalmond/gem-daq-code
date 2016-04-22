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

    if (argc!=2) 
    {
        cout << "Please provide ONE root file." << endl;
        cout << "Usage: <path>/printer input.root" << endl;
        return 0;
    }

    vector<TH1*> hs;
  
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

    TString iname; //name of file without extension

    TString name = ifilename;
    int loc = name.Last('/')+1;
    int len = name.Length()-loc-5;
    TSubString subname = name(loc,len);
    iname = subname;
    
    //Create date path to hold prints based on time printer was run
    time_t t = time(NULL);
    tm* timePtr = localtime(&t);
    char oPath [64];
    strftime (oPath, 64, "./output/%m_%d-%H/%b%d_%G-%H_%M_%S/", timePtr);
    
    TString dPath = oPath;    

    gROOT->ProcessLine(".!mkdir -p "+dPath);
    TFile *ofile = new TFile(dPath+iname+".root", "RECREATE");
    cout<<"Output File: "<<dPath<<".root"<<endl<<endl;
    
    //Retrieve histograms from file
    TList* keylist = new TList;
    keylist = ifile->GetListOfKeys();
    TIter nextkey(keylist);
    TKey *key = new TKey;
    while (key = (TKey*)nextkey())
    {
	TClass *cl = gROOT->GetClass(key->GetClassName());
	if (!cl->InheritsFrom("TH1")) 
	    continue;

	TH1 *h = (TH1*)key->ReadObj();
	hs.push_back(h);
    }
  
    int numH = hs.size();
    std::cout << "Number of Histograms located: " << numH << std::endl;
    //Which types will be stored
    vector<TString> types;
    types.push_back("pdf");
    types.push_back("png");
    types.push_back("jpg");
    types.push_back("root");

    //Print Canvases of each histogram individually
    for (int j=0;j<numH;j++)
    {
	TCanvas *cv = newCanvas(hs[j]->GetName());
	printPictures(hs[j],iname,dPath,cv,types);
    }

    //Plot all histograms for each file on individual canvases
    vector<vector<TH1*>> temph;
    temph.push_back(hs);
    vector<TString> tempn;
    tempn.push_back(iname);

    plotEvery(temph, tempn, dPath, types);

    return 0;
}
