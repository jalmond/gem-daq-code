#ifndef DEBUG
#define DEBUG 1
#endif
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
#include <memory>
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

/**
* GEM Tree Reader example (reader) application 
*/

/*! 
  \brief GEM Tree reader reads file with the GEM Event Tree and fills the histogram with number of vfats per event
*/

using namespace std;

//uint16_t gem::readout::GEMslotContents::slot[NVFAT] = {
//      0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
//      0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
//      0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,
//    };
//bool gem::readout::GEMslotContents::isFileRead = false;

class gemTreeReader {
  public:
    gemTreeReader(const std::string &ifilename, const bool &print_hist_)
    {
      std::string tmp = ifilename.substr(ifilename.size()-9, ifilename.size());
      if (tmp != ".raw.root") throw std::runtime_error("Wrong input filename (should end with '.raw.root'): "+ifilename);
      ifile = new TFile(ifilename.c_str(), "READ");
      ofilename = ifilename.substr(0,ifilename.size()-9);
      ofilename += ".analyzed.root";
      ofile = new TFile(ofilename.c_str(), "RECREATE");
      if (DEBUG) std::cout << std::dec << "[gemTreeReader]: File for histograms created, start to book histograms " << std::endl;   
      print_hist = print_hist_;
      this->bookHistograms();
    }
    ~gemTreeReader(){}
    void createHistograms()
    {
      std::string path = std::getenv("BUILD_HOME");
      for (int im = 0; im < NVFAT; im++){
        if (im < 2) {
          path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips0-1.csv";
        } else if (im < 16) {
          path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips2-15.csv";
        } else if (im < 18) {
          path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips16-17.csv";
        } else {
          path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips18-23.csv";
        }
        if (DEBUG) std::cout << "[gemTreeReader]: path to maps : " << path << std::endl; 
        maps[im] = path;
        path = std::getenv("BUILD_HOME");
      }
      for (int ivm = 0; ivm < NVFAT; ivm++) this->readMap(ivm);
      //this->printMaps();
      this->fillHistograms();
    }
  private:
    TFile *ifile;
    TFile *ofile;
    std::string ofilename;
    bool print_hist;

    //TString occupancy_plots = {"BeamProfile", "", "1100", "1110", "CRC1_vs_CRC2", "DiffCRC", "_VFAT"};

  //std::vector<std::pair<int,int>> strip_maps[NVFAT];
    std::vector<int> tmp_strips;

    std::map<int,int> strip_maps[NVFAT];
    std::string maps[NVFAT];
    std::map<int, GEMStripCollection> allstrips;

    TH1F* hiVFAT                     [3]; // Number of VFATs in event
    TH1F* hiVFATsn                   [3]; // VFAT slot number distribution
    TH1I* hiChip                     [3]; // VFAT ChipID distribution
    TH1I* hi1010                     [3]; // Control bit 1010
    TH1I* hi1100                     [3]; // Control bit 1100
    TH1I* hi1110                     [3]; // Control bit 1110
    TH1I* hiFlag                     [3]; // VFAT Flag
    TH1I* hiCRC                      [3]; // VFAT CRC
    TH1I* hiDiffCRC                  [3]; // CRC difference between the one supplied by VFAT and recomputed one
    TH1I* hiFake                     [3]; // Number of bad VFAT blocks in event 
    TH1F* hiCh128                    [3];     
    TH2I* hi2DCRC                    [3];     
    TH1I* hiDiffBXandBC              [3];
    TH1I* hiRatioBXandBC             [3];
    TH1I* hiSignal                   [3]; // Number of good VFAT blocks in event
    TH1I* hichfired                  [3];
    TH1I* hichnotfired               [3];
    TH1F* hiCh_notfired              [3];
    TH1F* hiVFATfired_perevent[3][NVFAT];
    TH1F* hiCh128chipFired    [3][NVFAT];
    TH1F* hiStripsFired       [3][NVFAT];
    TH2I* hi2DCRCperVFAT      [3][NVFAT];
    TH1I* hiClusterMult              [3];
    TH1I* hiClusterSize              [3];
    TH1I* hiClusterMultEta     [3][NETA];
    TH1I* hiClusterSizeEta     [3][NETA];
    TH2I* hiBeamProfile              [3];

    TDirectory *dir[3];
    TDirectory *subdir[24];

    int counters_[4]; // [0] - total events
                      // [1] - good events
                      // [2] - bad events 
                      // [3] - good events failed CRC check

    void bookHistograms()
    {
      std::string dirname[3] = {"AllEvents", "GoodEvents", "BadEvents"};
      std::string eta_partitions[8] = {"eta_1", "eta_2", "eta_3", "eta_4", "eta_5", "eta_6", "eta_7", "eta_8"};
      std::string tempname = "OtherData";
      char name[4][128], title[4][500];
      std::string type[NVFAT] = {"Slot0" , "Slot1" , "Slot2" , "Slot3" , "Slot4" , "Slot5" , "Slot6" , "Slot7", 
                              "Slot8" , "Slot9" , "Slot10", "Slot11", "Slot12", "Slot13", "Slot14", "Slot15", 
                              "Slot16", "Slot17", "Slot18", "Slot19", "Slot20", "Slot21", "Slot22", "Slot23"};
      if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Make directories and create histograms inside" << std::endl;   
      for (int i = 0; i < 3; i++) {
        dir[i] = ofile->mkdir(dirname[i].c_str());
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Directory " << i+1 << " created" << std::endl;   
        dir[i]->cd();
      	gDirectory->mkdir(tempname.c_str());
	      gDirectory->cd(tempname.c_str());
        hiVFAT         [i] = new TH1F((dirname[i]+"_VFAT").c_str(), "Number VFAT blocks per event", 24,  0., 24. );
        hiVFATsn       [i] = new TH1F((dirname[i]+"_VFATsn").c_str(), "VFAT slot number", 24,  0., 24. );
        hiDiffBXandBC  [i] = new TH1I((dirname[i]+"_DiffBXandBC").c_str(), "Difference of BX and BC", 100000, 0x0, 0x1869F );
        hiRatioBXandBC [i] = new TH1I((dirname[i]+"_RatioBXandBC").c_str(), "Ratio of BX and BC", 1000, 0x0, 0xa );
        hiChip         [i] = new TH1I((dirname[i]+"_ChipID").c_str(), "ChipID",         4096, 0x0, 0xfff );
        hi1010         [i] = new TH1I((dirname[i]+"_1010").c_str(), "Control Bits 1010", 15, 0x0, 0xf );
        hi1100         [i] = new TH1I((dirname[i]+"_1100").c_str(), "Control Bits 1100", 15, 0x0, 0xf );
        hi1110         [i] = new TH1I((dirname[i]+"_1110").c_str(), "Control Bits 1110", 15, 0x0, 0xf );
        hiFlag         [i] = new TH1I((dirname[i]+"_Flag").c_str()  , "Flag",            15, 0x0, 0xf );
        hiCRC          [i] = new TH1I((dirname[i]+"_CRC").c_str(),     "CRC",             100, 0x0, 0xffff );
        hiDiffCRC      [i] = new TH1I((dirname[i]+"_DiffCRC").c_str(), "CRC_Diff",    100, -32767, 32767 );
        hiFake         [i] = new TH1I((dirname[i]+"_Fake").c_str(), "Number of bad VFAT blocks in event",    24, 0x0, 0x18 );
        hiSignal       [i] = new TH1I((dirname[i]+"_Signal").c_str(), "Number of good VFAT blocks in event",    24, 0x0, 0x18 );
        hichfired      [i] = new TH1I((dirname[i]+"_chfired").c_str(), "Channels fired per event",      500, 0., 500. );
        hichnotfired   [i] = new TH1I((dirname[i]+"_chnotfired").c_str(), "Channels not fired per event",      500, 0., 500. );
        hiCh_notfired  [i] = new TH1F((dirname[i]+"_Ch_notfired").c_str(), "Strips",          128, 0., 128. );
        hiCh128        [i] = new TH1F((dirname[i]+"_Ch128").c_str(), "Strips",          128, 0., 128. );
        hi2DCRC        [i] = new TH2I((dirname[i]+"_CRC1_vs_CRC2").c_str(), "CRC_{calc} vs CRC_{VFAT}", 100, 0x0000, 0xffff, 100, 0x0000, 0xffff);
        hiClusterMult  [i] = new TH1I((dirname[i]+"_ClusterMult").c_str(), "Cluster multiplicity", 384,  0, 384 );
        hiClusterSize  [i] = new TH1I((dirname[i]+"_ClusterSize").c_str(), "Cluster size", 384,  0, 384 );
        hiBeamProfile  [i] = new TH2I((dirname[i]+"_BeamProfile").c_str(), "Beam Profile", 8, 0, 8, 384, 0, 384);
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Main histograms ["<<i<<"] created" << std::endl;   
        for(int ie=0; ie < NETA; ie++){
          hiClusterMultEta  [i][ie] = new TH1I((dirname[i]+"_ClusterMult"+eta_partitions[ie]).c_str(), "Cluster multiplicity", 384,  0, 384 );
          hiClusterSizeEta  [i][ie] = new TH1I((dirname[i]+"_ClusterSize"+eta_partitions[ie]).c_str(), "Cluster size", 384,  0, 384 );
        }
        for(int j=0; j < NVFAT; j++){
	  dir[i]->cd();
          gDirectory->mkdir(type[j].c_str());
          gDirectory->cd(type[j].c_str());
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Start 2d array of histograms ["<<i<<"]["<<j<<"] creation" << std::endl;   
          sprintf (name[0]  , (dirname[i]+"_hiVFATfired_perevent_%s").c_str(), type[j].c_str());
          sprintf (title[0] , "VFAT chip %s fired per event", type[j].c_str());
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Get name and title for hiVFATfired_perevent["<<i<<"]["<<j<<"]: name : " << name << " title : " << title << std::endl;   
          sprintf (name[1] , (dirname[i]+"_hiCh128chipFired_%s").c_str(), type[j].c_str());
          sprintf (title[1], "Channels fired for VFAT chip %s", type[j].c_str());
          sprintf (name[3] , (dirname[i]+"_hiStripsFired_%s").c_str(), type[j].c_str());
          sprintf (title[3], "Strips fired for VFAT chip %s", type[j].c_str());
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Get name and title for hiCh128chipFired["<<i<<"]["<<j<<"]" << std::endl;   
          hiVFATfired_perevent[i][j] = new TH1F(name[0], title[0], 20, 0., 20.);
          hiCh128chipFired    [i][j] = new TH1F(name[1], title[1], 128, 0., 128.);
          hiStripsFired    [i][j] = new TH1F(name[3], title[3], 129, 0., 129.);
          if (DEBUG) std::cout << std::dec << "[gemTreeReader]: 2d array of histograms ["<<i<<"]["<<j<<"] created" << std::endl;   
          sprintf (name[2] , (dirname[i]+"_2DCRCperVFAT_%s").c_str(), type[j].c_str());
          sprintf (title[2], "2D CRC for VFAT chip %s", type[j].c_str());
          hi2DCRCperVFAT      [i][j] = new TH2I(name[2], title[2], 100, 0x0000, 0xffff, 100, 0x0000, 0xffff);
        }
      }
    }
    void fillHistograms()
    {
      if (DEBUG) std::cout << "[gemTreeReader]: Starting filling the histograms" << std::endl;
      TTree *tree = (TTree*)ifile->Get("GEMtree");
      Event *event = new Event();
      TBranch *branch = tree->GetBranch("GEMEvents");
      branch->SetAddress(&event);
      Int_t nentries = tree->GetEntries();
      if (DEBUG) std::cout << "[gemTreeReader]: Number of entries in the TTree : " << nentries << std::endl;
      // init counters
      for (int c_=0; c_<4; c_++) counters_[c_] = 0;
      //initialize logger
      std::string tmp_ = ofilename.substr(0, ofilename.size()-14);
      tmp_ += ".log";
      std::string run_ = ofilename.substr(16,16);
      //std::string run_ = tmp_.substr(16, tmp_.size()-4);
      logger* logger_ = new logger(tmp_,run_);

      //create errors container

      GEMDQMerrors * errors_ = new GEMDQMerrors();

      // loop over tree entries
      //
          for (unsigned ia = 0; ia < 128; ia++) {
            tmp_strips.push_back(0);
          }
      
      //for (Int_t i = 0; i < nentries; i++)
      for (int i = 0; i < nentries; i++)
      {
        //if ((DEBUG) && (i>10)) break;
        if (DEBUG) std::cout << "[gemTreeReader]: Processing event " << i << std::endl;
        // clear number of VFATs
        int nVFAT[3] = {0,0,0};
        int nBadVFAT[3] = {0,0,0};
        int nGoodVFAT[3] = {0,0,0};
        int firedchannels[3] = {0,0,0};
        int notfiredchannels[3] = {0,0,0};
        int vfatId[3][NVFAT];
        for (int g=0; g < 3; g++){
          for (int l = 0; l<NVFAT; l++){
            vfatId[g][l] = 0;
            //if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fired chip counter when initializing event"  <<  vfatId[g][l]  << std::endl;   
          }
        }
        // Retrieve next entry
        branch->GetEntry(i);
        //bool eventIsOK = event->isEventGood();
        bool eventIsOK = true;
        vector<AMC13Event> v_amc13 = event->amc13s();
        vector<AMCdata> v_amc = v_amc13[0].amcs();
        // retrieve bunch crossing from evet
        uint32_t BX = v_amc[0].BX();
        uint16_t BC;
        // create vector of GEBdata. For data format details look at Event.h
        vector<GEBdata> v_geb;
        v_geb = v_amc[0].gebs();
        // loop over gebs
        for (Int_t j = 0; j < v_geb.size(); j++)
        {
          // create vector of VFATdata. For data format details look at Event.h
          vector<VFATdata> v_vfat;
          v_vfat = v_geb.at(j).vfats();
          // Increment number of VFATs in the given event
          nVFAT[0] += v_vfat.size();
          if (eventIsOK) { nVFAT[1] += v_vfat.size();} else { nVFAT[2] += v_vfat.size();}
          // loop over vfats
          for (Int_t k = 0; k < v_vfat.size(); k++)
          {
            //GEMDQMIntegrityChecker *intCheck = new GEMDQMIntegrityChecker(v_vfat.at(k));
            //if (!intCheck->check()) errors_->addError(i,"AMC0","C0",v_vfat.at(k).ChipID(), v_vfat.at(k).SlotNumber(), intCheck->getErrorCode());
            //delete intCheck;
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: VFAT # "  <<  k << std::endl;   
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: EC of the vfat inside loop===> "  <<  static_cast<uint32_t>(v_vfat.at(k).EC()) << std::hex << std::endl;   
            if (DEBUG) std::cout << std::dec << "[gemTreeReader]: BC of the vfat inside loop===> "  <<  v_vfat.at(k).BC() << std::hex << std::endl;   
            //if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Slot number of responded chip "  <<  v_vfat.at(k).SlotNumber()  << std::endl;   
            //if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fired chip counter before incrementing"  <<  vfatId[v_vfat.at(k).SlotNumber()]  << std::endl;   
            //if (v_vfat.at(k).SlotNumber()>(-1)){ 
            //  vfatId[0][v_vfat.at(k).SlotNumber()]++;
            //  if (eventIsOK) {vfatId[1][v_vfat.at(k).SlotNumber()]++;} else {vfatId[2][v_vfat.at(k).SlotNumber()]++;}
            //}
            //if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fired chip counter "  <<  vfatId[v_vfat.at(k).SlotNumber()]  << std::endl;   
            // fill histograms for all events
            dir[0]->cd();
            this->fillVFATHistograms(&v_vfat.at(k), hiVFATsn[0], hiCh128[0], hiCh_notfired[0], hiChip[0], hi1010[0], hi1100[0], hi1110[0], hiFlag[0], hiCRC[0], hiDiffCRC[0], hi2DCRC[0], hi2DCRCperVFAT[0], hiCh128chipFired[0], hiStripsFired[0], hiBeamProfile[0], firedchannels[0], notfiredchannels[0]);
            //if (v_vfat.at(k).isBlockGood()){
            //  nGoodVFAT[0]++;
            //}else {
            //  nBadVFAT[0]++;
            //}// end if isBlockGood
            //// fill histograms for good and bad events
            //if (eventIsOK){
            //  if (v_vfat.at(k).isBlockGood()){
            //    nGoodVFAT[1]++;
            //  }else {
            //    nBadVFAT[1]++;
            //  }
            //  dir[1]->cd();
            //  this->fillVFATHistograms(&v_vfat.at(k), hiVFATsn[1], hiCh128[1], hiCh_notfired[1], hiChip[1], hi1010[1], hi1100[1], hi1110[1], hiFlag[1], hiCRC[1], hiDiffCRC[1], hi2DCRC[1], hi2DCRCperVFAT[1], hiCh128chipFired[1], hiStripsFired[1], hiBeamProfile[1], firedchannels[1], notfiredchannels[1]);
            //} else {
            //  if (v_vfat.at(k).isBlockGood()){
            //    nGoodVFAT[2]++;
            //  }else {
            //    nBadVFAT[2]++;
            //  }
            //  dir[2]->cd();
            //  this->fillVFATHistograms(&v_vfat.at(k), hiVFATsn[2], hiCh128[2], hiCh_notfired[2], hiChip[2], hi1010[2], hi1100[2], hi1110[2], hiFlag[2], hiCRC[2], hiDiffCRC[2], hi2DCRC[2], hi2DCRCperVFAT[2], hiCh128chipFired[2], hiStripsFired[2], hiBeamProfile[2], firedchannels[2], notfiredchannels[2]);
            //}// end if eventIsOK
            BC = v_vfat.at(k).BC();
          }// end of loop over VFATs
        }// end of loop over GEBs
        dir[0]->cd();
        this->fillEventHistograms(BX, BC, nVFAT[0], nBadVFAT[0], nGoodVFAT[0], firedchannels[0], notfiredchannels[0], hiDiffBXandBC[0], hiRatioBXandBC[0], hiVFAT[0], hiFake[0], hiSignal[0], hichfired[0], hichnotfired[0], hiVFATfired_perevent[0], vfatId[0], hiClusterMult[0], hiClusterSize[0], hiClusterMultEta[0], hiClusterSizeEta[0]);
        //if (eventIsOK){
        //  dir[1]->cd();
        //  this->fillEventHistograms(BX, BC, nVFAT[1], nBadVFAT[1], nGoodVFAT[1], firedchannels[1], notfiredchannels[1], hiDiffBXandBC[1], hiRatioBXandBC[1], hiVFAT[1], hiFake[1], hiSignal[1], hichfired[1], hichnotfired[1], hiVFATfired_perevent[1], vfatId[1], hiClusterMult[1], hiClusterSize[1], hiClusterMultEta[1], hiClusterSizeEta[1]);
        //} else {
        //  dir[2]->cd();
        //  this->fillEventHistograms(BX, BC, nVFAT[2], nBadVFAT[2], nGoodVFAT[2], firedchannels[2], notfiredchannels[2], hiDiffBXandBC[2], hiRatioBXandBC[2], hiVFAT[2], hiFake[2], hiSignal[2], hichfired[2], hichnotfired[2], hiVFATfired_perevent[2], vfatId[2], hiClusterMult[2], hiClusterSize[2], hiClusterMultEta[2], hiClusterSizeEta[2]);
        //}
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Event histograms filled for event  " << i << std::endl;   
        logger_->addEvent(i,eventIsOK,nVFAT[0],nGoodVFAT[0],nBadVFAT[0]);
        allstrips.clear();
      }// end of loop over events

      errors_->saveErrors();
      delete errors_;

      logger_->addResponseEfficiency(hiClusterMult[1]->GetEntries()-hiClusterMult[1]->GetBinContent(1), hiClusterMult[1]->GetBinContent(1));
      logger_->writeLog();
      //logger_->printLog();
      delete logger_;
      for (int st = 0; st < 3; st++){
        dir[st]->cd();
        setTitles(hiVFATsn[st], "Slot Number", "Number of responses");   
        setTitles(hiVFAT[st], "Number of VFAT blocks per Event", "Number of Events");   
        setTitles(hiChip[st], "ChipID value, max 0xfff", "Number of VFAT blocks");
        setTitles(hi1010[st], "1010 marker, max 0xf", "Number of VFAT blocks");   
        setTitles(hi1100[st], "1100 marker, max 0xf", "Number of VFAT blocks");   
        setTitles(hi1110[st], "1110 marker, max 0xf", "Number of VFAT blocks");   
        setTitles(hiFlag[st], "Flag marker value, max 0xf", "Number of VFAT blocks");   
        setTitles(hiCRC[st], "CRC value, max 0xffff", "Number of VFAT blocks");
        setTitles(hiDiffCRC[st], "CRC_{VFAT}-CRC_{calc}", "Number of VFAT blocks");
        setTitles(hiFake[st], "Fake events", "Number of Events");
        setTitles(hiCh128[st], "Strips, max 128", "Number of VFAT blocks"); 
        setTitles(hi2DCRC[st], "CRC_{VFAT}", "CRC_{calc}");  
      }
      ofile->Write();
      if (print_hist){
        gErrorIgnoreLevel = kWarning; // Supress the Info outputs from ROOT
        gROOT->SetBatch(kTRUE);// don't draw all the canvases
        drawStack(dir[1], dir[2], 4, 2, "png", ofilename.substr(0, ofilename.size()-14)+"_hist/stacks/");
        TString prefix[3] = {"/tmp/dqm_hists/","/tmp/dqm_hists/","/tmp/dqm_hists/"};

        //TString prefix[3] = {ofilename.substr(0, ofilename.size()-14)+"_hist/all_events/", ofilename.substr(0, ofilename.size()-14)+"_hist/good_events/", ofilename.substr(0, ofilename.size()-14)+"_hist/bad_events/"};
        std::string tempname = "OtherData";
	      std::string type[NVFAT] = {"0" , "1" , "2" , "3" , "4" , "5" , "6" , "7",
                              "8" , "9" , "10", "11", "12", "13", "14", "15",
                              "16", "17", "18", "19", "20", "21", "22", "23"};
	      for (int id = 0; id < 1; id++){// temporary print only all_events
	        for (int j = 0; j < NVFAT; j++){
		        dir[id]->cd();
		        gDirectory->cd(("Slot"+ type[j]).c_str());
		        printHistograms(gDirectory->GetDirectory(""),"png",prefix[id]+type[j]+"/",true);
	        }
	        dir[id]->cd();
	        gDirectory->cd(tempname.c_str());
	        printHistograms(gDirectory->GetDirectory(""),"png",prefix[id]+tempname+"/",true);
        }
        gROOT->SetBatch(kFALSE);
        printDQMCanvases();
      }
    }

  void fillVFATHistograms(VFATdata *m_vfat, TH1F* m_hiVFATsn, TH1F* m_hiCh128, TH1F* m_hiCh_notfired, TH1I* m_hiChip, TH1I* m_hi1010, TH1I* m_hi1100, 
			  TH1I* m_hi1110, TH1I* m_hiFlag, TH1I* m_hiCRC, TH1I* m_hiDiffCRC, TH2I* m_hi2DCRC, TH2I* m_hi2DCRCperVFAT[], 
	                  TH1F* m_hiCh128chipFired[], TH1F* m_hiStripsFired[], TH2I* m_hiBeamProfile, int & firedchannels, 
			  int & notfiredchannels)
    {
      // fill the control bits histograms
      m_hi1010->Fill(m_vfat->b1010());
      m_hi1100->Fill(m_vfat->b1100());
      m_hi1110->Fill(m_vfat->b1110());
      // fill Flag and chip id histograms
      m_hiFlag->Fill(m_vfat->Flag());
      m_hiChip->Fill(m_vfat->ChipID());
      // calculate and fill VFAT slot number
      //int sn_ = m_vfat->SlotNumber();
      //int sn_ = -1;
      //read slot
      std::unique_ptr<gem::readout::GEMslotContents> slotInfo_ = std::unique_ptr<gem::readout::GEMslotContents> (new gem::readout::GEMslotContents("slot_table.csv"));
      int sn_ = slotInfo_->GEBslotIndex(m_vfat->ChipID());
      std::cout << "slot N " << sn_ << std::endl;
      m_hiVFATsn->Fill(sn_);
      // calculate and fill the crc and crc_diff
      m_hiCRC->Fill(m_vfat->crc());
      // CRC check
      uint16_t b1010 = (0x000f & m_vfat->b1010());
      uint16_t b1100 = (0x000f & m_vfat->b1100());
      uint16_t b1110 = (0x000f & m_vfat->b1110());
      uint16_t flag  = (0x000f & m_vfat->Flag());
      uint16_t ec    = (0x00ff & m_vfat->EC());
      //BC             = m_vfat->BC();
      if (DEBUG) std::cout << "[gemTreeReader]: CRC read from vfat : " << std::hex << m_vfat->crc() << std::endl;
      if (DEBUG) std::cout << "[gemTreeReader]: CRC recalculated   : " << std::hex << m_vfat->crc_calc() << std::endl;
      m_hiDiffCRC->Fill(m_vfat->crc()-m_vfat->crc_calc());
      m_hi2DCRC->Fill(m_vfat->crc(), m_vfat->crc_calc());
      //I think it would be nice to time this...
      uint16_t chan0xf = 0;
      for (int chan = 0; chan < 128; ++chan) {
        if (chan < 64){
          chan0xf = ((m_vfat->lsData() >> chan) & 0x1);
          if(chan0xf) {
             m_hiCh128->Fill(chan);
             firedchannels++;
          } else {
            m_hiCh_notfired->Fill(chan);
            notfiredchannels++;
          }
        } else {
          chan0xf = ((m_vfat->msData() >> (chan-64)) & 0x1);
          if(chan0xf) {
             m_hiCh128->Fill(chan);
             firedchannels++;
          } else {
            m_hiCh_notfired->Fill(chan);
            notfiredchannels++;
          }
        }
      }// end of loop over channels 
      // strip container mapped for eta partitions...

      for(int m=0; m < NVFAT; m++){
        if(sn_ == m){
          if (DEBUG) std::cout << "[gemTreeReader]: Starting to fill hiCh128chipFired for slot : " << m << std::dec << std::endl;
          if (DEBUG) std::cout << "[gemTreeReader]: LS data           " << std::bitset<64>(m_vfat->lsData()) <<  std::endl;
          if (DEBUG) std::cout << "[gemTreeReader]: MS data           " << std::bitset<64>(m_vfat->msData()) <<  std::endl;
          m_hi2DCRCperVFAT[m]->Fill(m_vfat->crc(), m_vfat->crc_calc());
          uint16_t chan0xfFiredchip = 0;
          for (int chan = 0; chan < 128; ++chan) {
            if (chan < 64){
              chan0xfFiredchip = ((m_vfat->lsData() >> chan) & 0x1);
              if(chan0xfFiredchip) {
                tmp_strips[chan] += 1;
                m_hiCh128chipFired[m]->Fill(chan);
                m_hiStripsFired[m]->Fill(strip_maps[m].find(chan+1)->second);
                int m_i = (int) m%8;
                int m_j = 127 - strip_maps[m].find(chan+1)->second + ((int) m/8)*128;
		            if (allstrips.find(m_i) == allstrips.end()){
		              GEMStripCollection strips;
		              allstrips[m_i]=strips;
		            }
		            // bx set to 0...
		            GEMStrip s(m_j,0);
		            allstrips[m_i].insert(s);
                if (DEBUG) std::cout << "[gemTreeReader]: Beam profile x : " << m_i << " Beam profile y : " << m_j <<  std::endl;
                m_hiBeamProfile->Fill(m_i,m_j);
              }
            } else {
              chan0xfFiredchip = ((m_vfat->msData() >> (chan-64)) & 0x1);
              if(chan0xfFiredchip) {
                tmp_strips[chan] += 1;
                m_hiCh128chipFired[m]->Fill(chan);
                m_hiStripsFired[m]->Fill(strip_maps[m].find(chan+1)->second);
                int m_i = (int) m%8;
                int m_j = 127 - strip_maps[m].find(chan+1)->second + ((int) m/8)*128;
		            if (allstrips.find(m_i) == allstrips.end()){
		              GEMStripCollection strips;
		              allstrips[m_i]=strips;
		            }
		            // bx set to 0...
		            GEMStrip s(m_j,0);
		            allstrips[m_i].insert(s);
                if (DEBUG) std::cout << "[gemTreeReader]: Beam profile x : " << m_i << " Beam profile y : " << m_j <<  std::endl;
                m_hiBeamProfile->Fill(m_i,m_j);
              }
            }
          }
        } 
      } // end of VFAT loop
    }
  void fillEventHistograms(const int& m_BX, const int& m_BC, const int & m_nVFAT, const int & m_nBadVFAT, const int & m_nGoodVFAT, const int & m_firedchannels, const int& m_notfiredchannels, TH1I* m_hiDiffBXandBC, TH1I* m_hiRatioBXandBC, TH1F* m_hiVFAT, TH1I* m_hiFake, TH1I* m_hiSignal, TH1I* m_hichfired, TH1I* m_hichnotfired, TH1F* m_hiVFATfired_perevent[], int vfatId[], TH1I* m_hiClusterMult, TH1I* m_hiClusterSize, TH1I* m_hiClusterMultEta[], TH1I* m_hiClusterSizeEta[] )
    {
      int diffBXandBC =  fabs(m_BX - m_BC);  
      double ratioBXandBC = (double) m_BX / m_BC;
      m_hiDiffBXandBC->Fill(diffBXandBC); 
      m_hiRatioBXandBC->Fill(ratioBXandBC);
      m_hiVFAT->Fill(m_nVFAT);
      m_hiFake->Fill(m_nBadVFAT);
      m_hiSignal->Fill(m_nGoodVFAT);
      for(Int_t x=0; x<NVFAT; x++) {
        m_hiVFATfired_perevent[x]->Fill(vfatId[x]);
        if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fired chip counter when filling event "  <<  vfatId[x] << " VFAT N " << x << std::endl;   
      }
      m_hichfired->Fill(m_firedchannels);
      m_hichnotfired->Fill(m_notfiredchannels);
      int ncl=0;
      int ncleta=0;
      for (std::map<int, GEMStripCollection>::iterator ieta=allstrips.begin(); ieta!= allstrips.end(); ieta++){
        ncleta=0;
        GEMClusterizer clizer;
        GEMClusterContainer cls = clizer.doAction(ieta->second);
        ncl+=cls.size();
        ncleta+=cls.size();
        for (GEMClusterContainer::iterator icl=cls.begin();icl!=cls.end();icl++){
          m_hiClusterSize->Fill(icl->clusterSize());	  
          m_hiClusterSizeEta[NETA-1-ieta->first]->Fill(icl->clusterSize());	  
        }
        m_hiClusterMultEta[NETA-1-ieta->first]->Fill(ncleta);	  
      }
      m_hiClusterMult->Fill(ncl);	  
      //allstrips.clear();
    }
    void readMap(int slot)
    {
      std::string ifpath_ = maps[slot];
      std::ifstream icsvfile_;
      icsvfile_.open(ifpath_);
      if(!icsvfile_.is_open()) {
        std::cout << "\nThe file: " << icsvfile_ << " is missing.\n" << std::endl;
        return;
      }  
      for (int il = 0; il < 128; il++) {
        std::string line;
        std::getline(icsvfile_, line);
        //if (DEBUG) std::cout << "[gemTreeReader]: Read line : " << line << std::endl; 
        std::istringstream iss(line);
        std::pair<int,int> map_;
        std::string val;
        std::getline(iss,val,',');
        //if (DEBUG) std::cout << "[gemTreeReader]: First val : " << val << std::endl; 
        std::stringstream convertor(val);
        convertor >> std::dec >> map_.second;
        //if (DEBUG) std::cout << "[gemTreeReader]: First val recorded : " << map_.second << std::endl; 
        std::getline(iss,val,',');
        //if (DEBUG) std::cout << "[gemTreeReader]: Second val : " << val << std::endl; 
        convertor.str("");
        convertor.clear();
        convertor << val;
        convertor >> std::dec >> map_.first;
        //if (DEBUG) std::cout << "[gemTreeReader]: Second val recorded : " << map_.first << std::endl; 
        strip_maps[slot].insert(map_);
      }
    }
    void printMaps()
    {
      std::map<int,int>::iterator it;
      for (int ism = 0; ism < NVFAT; ism++){
        std::cout << "Map for chip " << ism << std::endl;
        for (it = strip_maps[ism].begin(); it != strip_maps[ism].end(); ++it){
          std::cout << "Channel : " << it->first << " Strip : " << it->second << std::endl;
        }
      }
    }
    #include "dqmCanvases.cxx"
};
