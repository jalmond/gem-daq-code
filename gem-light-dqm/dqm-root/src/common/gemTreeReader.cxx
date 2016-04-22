#define DEBUG 1
#define NVFAT 24
#define NETA 8
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <array>
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
#include <TError.h>
#include <TBufferJSON.h>
#include <memory>
#include <unordered_map>

#include "gem/datachecker/GEMDataChecker.h"
#include "gem/readout/GEMslotContents.h"
#include "GEMClusterization/GEMStrip.h"
#include "GEMClusterization/GEMStripCollection.h"
#include "GEMClusterization/GEMClusterContainer.h"
#include "GEMClusterization/GEMClusterizer.h"
#include "plotter.cxx"
#include "logger.cxx"
#include "integrity_checker.cxx"
#include "GEMDQMerrors.cxx"
#include "AMC13_histogram.cxx"

using namespace std;

class treeReader
{
public:
  treeReader(const std::string &ifilename)
  {
    std::string tmp = ifilename.substr(ifilename.size()-9, ifilename.size());
    if (tmp != ".raw.root") throw std::runtime_error("Wrong input filename (should end with '.raw.root'): "+ifilename);
    ifile = new TFile(ifilename.c_str(), "READ");
    ofilename = ifilename.substr(0,ifilename.size()-9);
    ofilename += ".analyzed.root";
    ofile = new TFile(ofilename.c_str(), "RECREATE");
    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: File for histograms created" << std::endl;   

    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fetching hardware" << std::endl;   
    this->fetchHardware();

    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Booking histograms" << std::endl;   
    this->bookAllHistograms();
    this->fillAllHistograms();
  }
  ~treeReader(){}

private:
  TFile *ifile;
  TFile *ofile;
  std::string ofilename;

  std::vector<TDirectory*> AMC13dir;
  std::vector<TDirectory*> AMCdir;
  std::vector<TDirectory*> GEBdir;
  std::vector<TDirectory*> VFATdir;

  vector<AMC13Event> v_amc13;
  vector<AMCdata> v_amc;
  vector<GEBdata> v_geb;
  vector<VFATdata> v_vfat;


  vector<AMC13_histogram> v_amc13H;
  vector<AMC_histogram> v_amcH;
  vector<GEB_histogram> v_gebH;
  vector<VFAT_histogram> v_vfatH;

  unordered_map<std::string, int> vfat_map;
  unordered_map<std::string, int> geb_map;

  AMC13_histogram * m_amc13H;
  AMC_histogram * m_amcH;
  GEB_histogram * m_gebH;
  VFAT_histogram * m_vfatH;

  int m_RunType;
  int m_deltaV;
  int m_Latency;

  void fetchHardware()
  {
    try{
      TTree *tree = (TTree*)ifile->Get("GEMtree");
      Event *event = new Event();
      TBranch *branch = tree->GetBranch("GEMEvents");
      branch->SetAddress(&event);
      Int_t nentries = tree->GetEntries();
      branch->GetEntry(0);
      v_amc13 = event->amc13s();
      for(auto a13 = v_amc13.begin(); a13!= v_amc13.end(); a13++){
	v_amc = a13->amcs();
	for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
	  v_geb = a->gebs();
	  for(auto g=v_geb.begin(); g!=v_geb.end();g++){
	    v_vfat=g->vfats();
	  }
	}
      }
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of TTree entries: " << nentries << "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of AMC13s: " << v_amc13.size()<< "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of AMCs: " << v_amc.size()<< "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of GEBs: " << v_geb.size()<< "\n";
      if (DEBUG) std::cout<< "[gemTreeReader]: " << "Number of VFATs: " << v_vfat.size()<< "\n";
    }
    catch(...){
      std::cout<< "[gemTreeReader]: " << "No GEMtree in input raw file!" << std::endl;
      return;
    }
  }

  void bookAllHistograms()
  {
    int a13_c=0;    //counter through AMC13s
    int a_c=0;      //counter through AMCs
    int g_c=0;      //counter through GEBs
    int v_c=0;      //counter through VFATs

    /* LOOP THROUGH AMC13s */
    for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
      v_amc = a13->amcs();

      char diramc13[30];        //filename for AMC13 directory
      diramc13[0]='\0';         
      char serial_ch[20];       //char used to put serial number into directory name
      serial_ch[0] = '\0';
      int serial = v_amc13[a13_c].nAMC();  //obtains the serial number from the AMC13 Event
      sprintf(serial_ch, "%d", serial);
      strcat(diramc13,"AMC13-");
      strcat(diramc13,serial_ch);
      if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC13 Directory " << diramc13 << " created" << std::endl;
      //AMC13 HISTOGRAMS HERE
      m_amc13H = new AMC13_histogram(ofilename, gDirectory->mkdir(diramc13), serial_ch);
      m_amc13H->bookHistograms();

      a_c=0;

      /* LOOP THROUGH AMCs */
      for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
        v_geb = a->gebs();
        char diramc[30];        //filename for AMC directory  
        diramc[0]='\0';
        char aslot_ch[2];       //char used to put AMC slot number inot directory name
        aslot_ch[0] = '\0';
        int aslot = v_amc[a_c].AMCnum();  //obtains the slot number from the AMCdata
        sprintf(aslot_ch, "%d", aslot);
        strcat(diramc,"AMC-");
        strcat(diramc, aslot_ch);
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC Directory " << diramc << " created" << std::endl;
        m_amcH = new AMC_histogram(ofilename, gDirectory->mkdir(diramc), aslot_ch);
        m_amcH->bookHistograms();
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC13 AMCs size " << m_amc13H->amcsH().size() << std::endl;
        m_RunType = v_amc[a_c].Rtype();  //obtain the run type 
        g_c=0;

	      /* LOOP THROUGH GEBs */
        for(auto g=v_geb.begin(); g!=v_geb.end();g++){
          v_vfat=g->vfats();
          char dirgeb[30];    //filename for GEB directory
          dirgeb[0]='\0';    
          char g_ch[2];       //char used to put GEB number into directory name
          g_ch[0]='\0';
          int g_inputID = g->InputID();
          sprintf(g_ch, "%d", g_inputID);
          strcat(dirgeb,"GTX-");
          strcat(dirgeb,g_ch);
          //char buff[10];
          //buff[0] = '\0';
          //strcpy(buff, aslot_ch);
          //strcat(buff,g_ch);
          //strcpy(g_ch, buff);
          geb_map.insert(std::make_pair(g_ch, g_c));
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB Directory " << dirgeb << " created" << std::endl;
          //GEB HISTOGRAMS HERE
          m_gebH = new GEB_histogram(ofilename, gDirectory->mkdir(dirgeb), g_ch);
          m_gebH->bookHistograms();
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: AMC GEBs size " << m_amcH->gebsH().size() << std::endl;

          v_c=0;

	        /* LOOP THROUGH VFATs */
          for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
            char dirvfat[30];   //filename for VFAT directory
            dirvfat[0]='\0';    
            char vslot_ch[2];   //char used to put VFAT number into directory name
            vslot_ch[0] = '\0';
            std::unique_ptr<gem::readout::GEMslotContents> slotInfo_ = std::unique_ptr<gem::readout::GEMslotContents> (new gem::readout::GEMslotContents("slot_table.csv"));     
            int vslot = slotInfo_->GEBslotIndex(v->ChipID());  //converts Chip ID into VFAT slot number
            sprintf(vslot_ch, "%d", vslot);
            strcat(dirvfat,"VFAT-");
            strcat(dirvfat, vslot_ch);
            int vID = v->ChipID();
            char vID_ch[10];
            vID_ch[0] = '\0';
            sprintf(vID_ch, "%d", vID);
            char buff[10];
            buff[0] = '\0';
            strcpy(buff,g_ch);
            strcat(buff,vID_ch);
            strcpy(vID_ch,buff);
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: VFAT Directory " << dirvfat << " created" << std::endl;
            //VFAT HISTOGRAMS HERE
            m_vfatH = new VFAT_histogram(ofilename, gDirectory->mkdir(dirvfat), vslot_ch);
            m_vfatH->bookHistograms();
            std::cout << "VFAT ID " << vID_ch << std::endl;
            vfat_map.insert(std::make_pair(vID_ch, v_c));
            m_gebH->addVFATH(*m_vfatH);
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: GEB VFATs size " << m_gebH->vfatsH().size() << std::endl;

            gDirectory->cd("..");   //moves back to previous directory
            v_c++;
          } /* END VFAT LOOP */
          gDirectory->cd("..");     //moves back to previous directory
          g_c++;
          m_amcH->addGEBH(*m_gebH);
        } /* END GEB LOOP */
        gDirectory->cd("..");       //moves back to previous directory
       	a_c++;
        m_amc13H->addAMCH(*m_amcH);
      } /* END AMC LOOP */
      a13_c++;
    } /* END AMC13 LOOP */
  }
  void fillAllHistograms()
  {
    int a13_c=0;    //counter through AMC13s
    int a_c=0;      //counter through AMCs
    int g_c=0;      //counter through GEBs
    int v_c=0;      //counter through VFATs

    TTree *tree = (TTree*)ifile->Get("GEMtree");
    Event *event = new Event();
    TBranch *branch = tree->GetBranch("GEMEvents");
    branch->SetAddress(&event);
    Int_t nentries = tree->GetEntries();
    /* LOOP THROUGH Events */
    for (int i = 0; i < nentries; i++){
      branch->GetEntry(i);
      v_amc13 = event->amc13s();
      /* LOOP THROUGH AMC13s */
      for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
        v_amc = a13->amcs();
        v_amcH = m_amc13H->amcsH();
        m_amc13H->fillHistograms(&*a13);
        a_c=0;
        /* LOOP THROUGH AMCs */
        for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
          v_geb = a->gebs();
          v_gebH = v_amcH[a_c].gebsH();
          //AMC_histogram * t_amcH = &(m_amc13H->amcsH().at(a_c));
          v_amcH[a_c].fillHistograms(&*a);
          if (m_RunType){
            m_deltaV = a->Param2() - a->Param3();
            m_Latency = a->Param1();
          }
          g_c=0;
	        /* LOOP THROUGH GEBs */
          for(auto g=v_geb.begin(); g!=v_geb.end();g++){
            v_vfat = g->vfats();
            int gID = g->InputID();
            char gID_ch[10];
            gID_ch[0] = '\0';
            sprintf(gID_ch, "%d", gID);
            auto gebH_ = geb_map.find(gID_ch);
            if(gebH_ != geb_map.end()) {
              v_gebH[gebH_->second].fillHistograms(&*g);
              v_vfatH = v_gebH[gebH_->second].vfatsH();
            }
            else {
                std::cout << "GEB Not found\n";
                continue;
            }
	          /* LOOP THROUGH VFATs */
            for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
              int vID = v->ChipID();
              char vID_ch[10];
              vID_ch[0] = '\0';
              sprintf(vID_ch, "%d", vID);
              char buff[10];
              buff[0] = '\0';
              strcpy(buff,gID_ch);
              strcat(buff,vID_ch);
              strcpy(vID_ch,buff);
              auto vfatH_ = vfat_map.find(vID_ch);
              if(vfatH_ != vfat_map.end()) {
                v_vfatH[vfatH_->second].fillHistograms(&*v);
                if (m_RunType){
                  v_vfatH[vfatH_->second].fillScanHistograms(&*v, m_RunType, m_deltaV, m_Latency);
                }
              }
              else {
                  std::cout << "VFAT Not found\n";
              }
            } /* END VFAT LOOP */
          } /* END GEB LOOP */
         	a_c++;
        } /* END AMC LOOP */
        a13_c++;
      } /* END AMC13 LOOP */
    } /* END EVENTS LOOP */
    ofile->Write();
  }
};
