#include "VFAT_histogram.cxx"
#include "TH1.h"

class GEB_histogram: public Hardware_histogram
{
  public:
    GEB_histogram(const std::string & filename, TDirectory * dir, const std::string & hwid):Hardware_histogram(filename, dir, hwid){}//call base constructor
    void bookHistograms()
    {
      m_dir->cd();
      ZeroSup  = new TH1F("ZeroSup", "Zero Suppression", 0xffffff,  0x0 , 0xffffff);
      InputID  = new TH1F("InputID", "GLIB input ID", 31,  0x0 , 0b11111);      
      Vwh      = new TH1F("Vwh", "VFAT word count", 4095,  0x0 , 0xfff);
      // Assing custom bin labels
      const char *error_flags[5] = {"Event Size Overflow", "L1AFIFO Full", "InFIFO Full", "Evt FIFO Full","InFIFO Underflow"};
      Errors   = new TH1I("Errors", "Critical errors", 5,  0, 5);
      for (int i = 1; i<6; i++) Errors->GetXaxis()->SetBinLabel(i, error_flags[i-1]);
      const char *warning_flags[10] = {"BX AMC-OH Mismatch", "BX AMC-VFAT Mismatch", "OOS AMC OH", "OOS AMC VFAT","No VFAT Marker","Event Size Warn", "L1AFIFO Near Full", "InFIFO Near Full", "EvtFIFO Near Full", "Stuck Data"};
      Warnings = new TH1I("Warnings", "Warnings", 10,  0, 10);
      for (int i = 1; i<11; i++) Warnings->GetXaxis()->SetBinLabel(i, warning_flags[i-1]);
      OHCRC    = new TH1F("OHCRC", "OH CRC", 0xffff,  0x0 , 0xffff);
      Vwt      = new TH1F("Vwt", "VFAT word count", 4095,  0x0 , 0xfff);
    }
    void fillHistograms(GEBdata * geb){
      ZeroSup->Fill(geb->ZeroSup());
      InputID->Fill(geb->InputID());
      Vwh->Fill(geb->Vwh());
      //ErrorC->Fill(geb->ErrorC());
      OHCRC->Fill(geb->OHCRC());
      Vwt->Fill(geb->Vwt());
      //InFu->Fill(geb->InFu());
      //Stuckd->Fill(geb->Stuckd());
      uint8_t binFired = 0;
      // Fill Warnings histogram
      for (int bin = 0; bin < 9; bin++){
        binFired = ((geb->ErrorC() >> bin) & 0x1);
        if (binFired) Warnings->Fill(bin);
      }
      binFired = (geb->Stuckd() & 0x1);
      if (binFired) Warnings->Fill(9);
      // Fill Errors histogram
      for (int bin = 9; bin < 13; bin++){
        binFired = ((geb->ErrorC() >> bin) & 0x1);
        if (binFired) Errors->Fill(bin-9);
      }
      binFired = (geb->InFu() & 0x1);
      if (binFired) Warnings->Fill(4);
    }
    void addVFATH(VFAT_histogram vfatH){m_vfatsH.push_back(vfatH);}
    std::vector<VFAT_histogram> vfatsH(){return m_vfatsH;}
  private:
    std::vector<VFAT_histogram> m_vfatsH;
    TH1F* ZeroSup;
    TH1F* InputID;
    TH1F* Vwh;
    TH1I* Errors;
    TH1I* Warnings;
    TH1F* OHCRC;
    TH1F* Vwt;
};

