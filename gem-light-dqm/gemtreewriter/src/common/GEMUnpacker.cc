#include <cstdio>
#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <bitset>
//#include "GEMAMC13EventFormat.h"
#if defined(__CINT__) && !defined(__MAKECINT__)
    #include "libEvent.so"
#else
    #include "Event.h"
#endif
#include <TFile.h>
#include <TTree.h>

class GEMUnpacker
{
  public:
    GEMUnpacker(const std::string & ifilename, const std::string & isFedKit)
    {
      try {
        m_file = std::fopen(ifilename.c_str(), "rb");
      }
      catch (int e)
      {
        std::cout << "An exception occured. Exception code " << e << std::endl;
      }
      ofilename = ifilename.substr(0, ifilename.size()-4);
      ofilename += ".raw.root";
      m_isFedKit = isFedKit;
    }
    ~GEMUnpacker()
    {
      if (m_file != NULL) std::fclose(m_file);
    }
    void unpack()
    {

     TFile *hfile = new TFile(ofilename.c_str(),"RECREATE","GEM Raw ROOT");
     TTree GEMtree("GEMtree","A Tree with GEM Events");
     Event *ev = new Event(); 
     GEMtree.Branch("GEMEvents", &ev);

     while (true){
        // read and print FEROL headers
        if (m_isFedKit == "ferol") {
          std::size_t sz = std::fread(&m_word, sizeof(uint64_t), 1, m_file);
          if (sz == 0 ) break;
          printf("%016llX\n", m_word);
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
          printf("%016llX\n", m_word);
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
          printf("%016llX\n", m_word);
          // ferol headers read and printed, now read CDF header
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        } else {
          std::size_t sz = std::fread(&m_word, sizeof(uint64_t), 1, m_file);
          if (sz == 0 ) break;
        }
        // read and print "BADC0FFEEBADCAFE" and another artificial header
        //printf("%016llX\n", m_word);
        //std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        //printf("%016llX\n", m_word);
        m_AMC13Event = new AMC13Event();
        //std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("%016llX\n", m_word);
        m_AMC13Event->setCDFHeader(m_word);
        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("%016llX\n", m_word);
        m_AMC13Event->setAMC13header(m_word);
        //printf("%016llX\n", m_word);
        std::cout << "n_AMC = " << m_AMC13Event->nAMC() << std::endl;
        // Readout out AMC headers
        for (unsigned short i = 0; i < m_AMC13Event->nAMC(); i++){
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("%016llX\n", m_word);
          m_AMC13Event->addAMCheader(m_word);
        }
        // Readout out AMC payloads
        for (unsigned short i = 0; i < m_AMC13Event->nAMC(); i++){
          AMCdata * m_amcdata = new AMCdata();
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("AMC HEADER1\n");
        printf("%016llX\n", m_word);
          m_amcdata->setAMCheader1(m_word);
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("AMC HEADER2\n");
        printf("%016llX\n", m_word);
          m_amcdata->setAMCheader2(m_word);
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
          m_amcdata->setGEMeventHeader(m_word);
        printf("GEM EVENT HEADER\n");
        printf("%016llX\n", m_word);
          // fill the geb data here
          std::cout << "GDcount = " << m_amcdata->GDcount() << std::endl;
          for (unsigned short j = 0; j < m_amcdata->GDcount(); j++){
            GEBdata * m_gebdata = new GEBdata();
            std::fread(&m_word, sizeof(uint64_t), 1, m_file);
            m_gebdata->setChamberHeader(m_word);
        printf("GEM CHAMBER HEADER\n");
        printf("%016llX\n", m_word);
            // fill the vfat data here
            std::cout << "Number of VFAT words " << m_gebdata->Vwh() << std::endl;
            int m_nvb = m_gebdata->Vwh() / 3; // number of VFAT2 blocks. Eventually add here sanity check
            std::cout << "Number of VFAT blocks " << m_nvb << std::endl;
            for (unsigned short k = 0; k < m_nvb; k++){
              VFATdata * m_vfatdata = new VFATdata();
              // read 3 vfat block words, totaly 192 bits
              std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("VFAT WORD 1\n");
        printf("%016llX\n", m_word);
              m_vfatdata->read_fw(m_word);
              std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("VFAT WORD 2\n");
        printf("%016llX\n", m_word);
              m_vfatdata->read_sw(m_word);
              std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("VFAT WORD 3\n");
        printf("%016llX\n", m_word);
              m_vfatdata->read_tw(m_word);
              //
        printf("VFAT MS Data 3\n");
        printf("%016llX\n", m_vfatdata->msData());
        printf("VFAT LS Data 3\n");
        printf("%016llX\n", m_vfatdata->lsData());
              //
              m_gebdata->v_add(*m_vfatdata);
              delete m_vfatdata;
            }
            std::fread(&m_word, sizeof(uint64_t), 1, m_file);
            m_gebdata->setChamberTrailer(m_word);
            m_amcdata->g_add(*m_gebdata);
            delete m_gebdata;
          }
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
          m_amcdata->setGEMeventTrailer(m_word);
          std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        printf("AMC TRALIER\n");
        printf("%016llX\n", m_word);
          m_amcdata->setAMCTrailer(m_word);
          m_AMC13Event->addAMCpayload(*m_amcdata);
          delete m_amcdata;
        }
        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        m_AMC13Event->setAMC13trailer(m_word);
        std::fread(&m_word, sizeof(uint64_t), 1, m_file);
        m_AMC13Event->setCDFTrailer(m_word);
        ev->Build(true);
        ev->SetHeader(m_AMC13Event->LV1_id(), 0, 0);
        ev->addAMC13Event(*m_AMC13Event);
        GEMtree.Fill();
        ev->Clear();
      }
      hfile->Write();// Save file with tree
    }
  private:
    std::FILE *m_file;
    uint64_t m_word;
    uint32_t m_word32;
    bool type;
    AMC13Event * m_AMC13Event;
    std::string ofilename;
    std::string m_isFedKit;
};
 
int main (int argc, char** argv)
{
  std::cout << "[GEMUnpacker]: ---> Main()" << std::endl;
  if (argc<3) 
  {
    std::cout << "Please provide input filename and source type" << std::endl;
    std::cout << "Usage: <path>/unpacker ifile ferol(sdram)" << std::endl;
    return 0;
  }
  std::string ifile   = argv[1];
  std::string isFedKit = argv[2];

  GEMUnpacker * m_unpacker = new GEMUnpacker(ifile, isFedKit);
  m_unpacker->unpack();
  delete m_unpacker;
}
