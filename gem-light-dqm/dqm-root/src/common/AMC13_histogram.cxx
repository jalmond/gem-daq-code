#include "AMC_histogram.cxx"
#include "TH1.h"

class AMC13_histogram: public Hardware_histogram
{
  public:
    AMC13_histogram(const std::string & filename, TDirectory * dir, const std::string & hwid):Hardware_histogram(filename, dir, hwid){}//call base constructor
    void bookHistograms()
    {
      m_dir->cd();
      control_bit5 = new TH1F("Control_Bit5", "Control Bit 5", 15,  0x0 , 0xf);
      control_bitA = new TH1F("Control_BitA", "Control Bit A", 15,  0x0 , 0xf);
      Evt_ty       = new TH1F("Evt_ty", "Evt_ty", 15, 0x0, 0xf);
      LV1_id       = new TH1F("LV1_id", "LV1_id", 0xffffff, 0x0, 0xffffff);
      BX_id        = new TH1F("Bx_id", "Bx_id", 4095, 0x0, 0xfff);
      Source_id    = new TH1F("Source_id", "Source_id", 4095, 0x0, 0xfff);
      CalTyp       = new TH1F("CalTyp", "CalTyp", 15, 0x0, 0xf);
      nAMC         = new TH1F("nAMC", "nAMC", 12, 0, 12);
      OrN          = new TH1F("OrN", "OrN", 0xffffffff, 0x0, 0xffffffff);// N bins overflow!!
      CRC_amc13    = new TH1F("CRC_amc13", "CRC_amc13", 0xffffffff, 0x0, 0xffffffff);// N bins overflow!!
      Blk_Not      = new TH1F("Blk_Not", "Blk_Not", 255, 0x0, 0xff);
      LV1_idT      = new TH1F("LV1_idT", "LV1_idT", 255, 0x0, 0xff);
      BX_idT       = new TH1F("BX_idT", "BX_idT", 4095, 0x0, 0xfff);
      EvtLength    = new TH1F("EvtLength", "EvtLength", 0xffffff, 0x0, 0xffffff);
      CRC_cdf      = new TH1F("CRC_cdf", "CRC_cdf", 0xffff, 0x0, 0xffff);
    }
    void fillHistograms(AMC13Event *amc13){
      nAMC->Fill(amc13->nAMC());
      LV1_id->Fill(amc13->LV1_id());
    }
    void addAMCH(AMC_histogram amcH){m_amcsH.push_back(amcH);}
    std::vector<AMC_histogram> amcsH(){return m_amcsH;}
  private:
    std::vector<AMC_histogram> m_amcsH;
    TH1F* control_bit5;
    TH1F* control_bitA;
    TH1F* Evt_ty;
    TH1F* LV1_id;
    TH1F* BX_id;
    TH1F* Source_id;
    TH1F* CalTyp;
    TH1F* nAMC;
    TH1F* OrN;
    TH1F* CRC_amc13;
    TH1F* Blk_Not;
    TH1F* LV1_idT;
    TH1F* BX_idT;
    TH1F* EvtLength;
    TH1F* CRC_cdf;
};
