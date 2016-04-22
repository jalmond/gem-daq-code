#include <vector>
class VFATdata 
{
  private:
    uint8_t fb1010;                    // 1010:4 Control bits, shoud be 1010
    uint16_t fBC;                      // Bunch Crossing number, 12 bits
    uint8_t fb1100;                    // 1100:4, Control bits, shoud be 1100
    uint16_t fEC;                      // Event Counter, 8 bits
    uint8_t fFlag;                     // Control Flags: 4 bits, Hamming Error/AFULL/SEUlogic/SUEI2C
    uint8_t fb1110;                    // 1110:4 Control bits, shoud be 1110
    uint16_t fChipID;                  // Chip ID, 12 bits
    uint64_t flsData;                  // channels from 1to64 
    uint64_t fmsData;                  // channels from 65to128
    uint16_t fcrc;                     // Check Sum value, 16 bits
    uint16_t fcrc_calc;                // Check Sum value recalculated, 16 bits
    int fSlotNumber;                   // Calculated chip position
    bool fisBlockGood;                 // Shows if block is good (control bits, chip ID and CRC checks)
  
  public:
    VFATdata(){}
    VFATdata(const uint8_t &b1010_, 
            const uint16_t &BC_,
            const uint8_t &b1100_, 
            const uint8_t &EC_,
            const uint8_t &Flag_, 
            const uint8_t &b1110_, 
            const uint16_t &ChipID_, 
            const uint64_t &lsData_, 
            const uint64_t &msData_, 
            const uint16_t &crc_,
            const uint16_t &crc_calc_,
            const int &SlotNumber_,
            const bool &isBlockGood_) : 
        fb1010(b1010_),
        fBC(BC_),
        fb1100(b1100_),
        fEC(EC_),
        fFlag(Flag_),
        fb1110(b1110_),
        fChipID(ChipID_),
        flsData(lsData_),
        fmsData(msData_),
        fcrc(crc_),
        fcrc_calc(crc_calc_),
        fSlotNumber(SlotNumber_),
        fisBlockGood(isBlockGood_){}
    ~VFATdata(){}
    
    uint8_t   b1010      (){ return fb1010;      }
    uint16_t  BC         (){ return fBC;         }
    uint8_t   b1100      (){ return fb1100;      }
    uint8_t   EC         (){ return fEC;         }
    uint8_t   Flag       (){ return fFlag;       }
    uint8_t   b1110      (){ return fb1110;      }
    uint16_t  ChipID     (){ return fChipID;     }
    uint64_t  lsData     (){ return flsData;     }
    uint64_t  msData     (){ return fmsData;     }
    uint16_t  crc        (){ return fcrc;        }
    uint16_t  crc_calc   (){ return fcrc_calc;   }
    int       SlotNumber (){ return fSlotNumber; }
    bool      isBlockGood(){ return fisBlockGood;}

};

class GEBdata
{
  private:
    std::vector<VFATdata> vfatd;

    //GEM chamber header

      uint32_t m_ZeroSup;  //Zero Suppression Flags:24  (8 zeroes):8
                           //Bitmask indicating if certain VFAT blocks have been zero suppressed
      uint8_t m_InputID;   //Input ID:5                 000:3
                           //GLIB input ID (starting at 0)
      uint16_t m_Vwh;      //VFAT word count:12         0000:4
                           //Size of VFAT payload in 64 bit words
      uint16_t m_ErrorC;
                           //EvtFIFO full:1       InFIFO full:1      L1AFIFO full:1    Even size overflow:1  EvtFIFO near full:1  InFIFO near full:1 
                           //L1AFIFO near full:1  Event size warn:1  No VFAT marker:1  OOS GLIB VFAT:1       OOS GLIB OH:1        
                           //BX mismatch GLIB VFAT:1                 BX mismatch GLIB OH:1                   
                           //000:3

    //GEM chamber trailer
		
      uint16_t m_OHCRC;    //OH CRC:16
                           //CRC of OH data (currently not available)
      uint16_t m_Vwt;      //VFAT word count:12   0000:4
                           //Same as in header. This one actually counts the number of valid words that were sent to AMC13; the one in header is 
                           //what we expected to send to AMC13
      uint8_t m_InFu;      //InFIFO underflow:1   (7 0's):7
                           //Input status (critical): Input FIFO underflow occured while sending this event
      uint8_t m_Stuckd;    //Stuck data:1    (7 0's):7
                           //Input status (warning): data in InFIFO or EvtFIFO when L1A FIFO was empty. Only resets with resync or reset
		
  public:
    GEBdata(){};
    GEBdata(const uint32_t &ZeroSup_, 
              const uint8_t &InputID_, 
              const uint16_t &Vwh_, 
              const uint16_t &ErrorC_, 
              const uint16_t &OHCRC_, 
              const uint16_t &Vwt_,
              const uint8_t &InFu_,
              const uint8_t &Stuckd_) : 
          m_ZeroSup(ZeroSup_),                                
          m_InputID(InputID_),
          m_Vwh(Vwh_),                                  
          m_ErrorC(ErrorC_),
          m_OHCRC(OHCRC_),                                    
          m_Vwt(Vwt_),                             
          m_InFu(InFu_),                                   
          m_Stuckd(Stuckd_){}         
    ~GEBdata(){}


    uint32_t ZeroSup()  {return m_ZeroSup;}
    uint8_t  InputID()  {return m_InputID;}
    uint16_t Vwh()      {return m_Vwh;}
    uint16_t ErrorC()   {return m_ErrorC;}

    uint16_t OHCRC()    {return m_OHCRC;}
    uint16_t Vwt()      {return m_Vwt;}
    uint8_t  InFu()     {return m_InFu;}
    uint8_t  Stuckd()   {return m_Stuckd;}


    void v_add(VFATdata v){vfatd.push_back(v);}
};

class AMCdata
{
  private:
    std::vector<GEBdata> gebd;

    //AMC header #1	

      uint8_t  m_AMCnum;      //0000:4   AMC#:4
                              //Slot number of AMC(GLIB/MP7/EC7, etc.)
      uint32_t m_L1A;         //L1A ID:24        (8 0's):8
                              //basically like event number, but reset by resync
      uint16_t m_BX;          //BX ID:12         0000:4
                              //Bunch crossing ID
      uint32_t m_Dlength;     //Data length:20   (12 0's):12
                              //Overall size of this FED event fragment in 64bit words (including headers and trailers)

    //AMC header #2
      uint8_t m_FV;           //Format Version:4    0000:4
      uint8_t m_Rtype;        //Run Type:4          0000:4
                              //current version = 0x0;  Could be used to encode run types like physics, cosmics, threshold scan, etc.
      uint8_t m_Param1;       //Run param1:8 
      uint8_t m_Param2;       //Run param2:8
      uint8_t m_Param3;       //Run param3:8 
      uint16_t m_Onum;        //Orbit number:16 
      uint16_t m_BID;         //Board ID:16
                              //This is currently filled with 8bit long GLIB serial number

    //GEM event header
      uint32_t m_GEMDAV;   //GEM DAV list:24    (8 zeroes):8
                           //Bitmask indicating which inputs/chambers have data
      uint64_t m_Bstatus;  // Buffer Status:34  (30 zeroes):30
                           //Bitmask indicating buffer error in given inputs
      uint8_t  m_GDcount;  //GEM DAV count:5    000:3
                           //Number of chamber blocks
      uint8_t  m_Tstate;   //TTS state:4        0000:4
                           //Debug: GLIB TTS state at the moment when this event was built


    //GEM event trailer
      uint32_t m_ChamT;    //Chamber timeout:24   (8 0's):8
                           //Bitmask indicating if GLIB did not recieve data from particular input for this L1A in X amount of GTX clock cycles
      uint8_t  m_OOSG;     //OOS GLIB:1    (7 0's):7
                           //GLIB is out-of-sync (critical): L1A ID is different for different chambers in this event.

    //AMC_trailer
      uint32_t m_CRC;
      uint8_t m_L1AT;
      uint32_t m_DlengthT;
	
  public:
    AMCdata(){};
    AMCdata(const uint8_t &AMCnum_, 
              const uint32_t &L1A_,
              const uint16_t &BX_, 
              const uint32_t &Dlength_,
              const uint8_t &FV_,
              const uint8_t &Rtype_, 
              const uint8_t &Param1_, 
              const uint8_t &Param2_, 
              const uint8_t &Param3_, 
              const uint16_t &Onum_, 
              const uint16_t &BID_,
              const uint32_t &GEMDAV_, 
              const uint64_t &Bstatus_,
              const uint8_t &GDcount_, 
              const uint8_t &Tstate_,
              const uint32_t &ChamT_,
              const uint8_t OOSG_) :
          m_AMCnum(AMCnum_), 
          m_L1A(L1A_),
          m_BX(BX_),                                 
          m_Dlength(Dlength_),
          m_FV(FV_),
          m_Rtype(Rtype_),                                
          m_Param1(Param1_),
          m_Param2(Param2_),                                  
          m_Param3(Param3_),
          m_Onum(Onum_),                                    
          m_BID(BID_),
          m_GEMDAV(GEMDAV_), 
          m_Bstatus(Bstatus_),
          m_GDcount(GDcount_),                                 
          m_Tstate(Tstate_),                             
          m_ChamT(ChamT_),                                   
          m_OOSG(OOSG_){}
    ~AMCdata(){}

    uint8_t  AMCnum()  {return m_AMCnum;}
    uint32_t L1A()     {return m_L1A;}
    uint16_t BX()      {return m_BX;}
    uint32_t Dlength() {return m_Dlength;}

    uint8_t  FV()      {return m_FV;}
    uint8_t  Rtype()   {return m_Rtype;}
    uint8_t  Param1()  {return m_Param1;}
    uint8_t  Param2()  {return m_Param2;}
    uint8_t  Param3()  {return m_Param3;}
    uint16_t Onum()    {return m_Onum;}
    uint16_t BID()     {return m_BID;}

    uint32_t GEMDAV ()  {return m_GEMDAV;}
    uint64_t Bstatus()  {return m_Bstatus;}
    uint8_t  GDcount()  {return m_GDcount;}
    uint8_t  Tstate()   {return m_Tstate;}

    uint32_t ChamT()    {return m_ChamT;}
    uint8_t  OOSG()     {return m_OOSG;}    

    uint32_t CRC()    {return m_CRC;}
    uint8_t L1AT()    {return m_L1AT;}
    uint32_t DlengthT()    {return m_DlengthT;}

    void g_add(GEBdata g){gebd.push_back(g);}
};

class AMC13Event
{
  private:
    // CDF Header
    uint8_t m_cb5; // control bit, should be 0x5 bits 60-63
    uint8_t m_Evt_ty;
    uint32_t m_LV1_id;
    uint16_t m_BX_id;
    uint16_t m_Source_id;
    // AMC headers
    std::vector<uint32_t> m_AMC_size;
    std::vector<uint8_t> m_Blk_No;
    std::vector<uint8_t> m_AMC_No;
    std::vector<uint16_t> m_BoardID;
    // AMCs payload
    std::vector<AMCdata> m_amcs;
    //AMC13 trailer
    uint32_t m_CRC_amc13;
    uint8_t m_Blk_NoT;
    uint8_t m_LV1_idT;
    uint16_t m_BX_idT;
    //CDF trailer
    uint8_t m_cbA; // control bit, should be 0xA bits 60-63
    uint32_t m_EvtLength;
    uint16_t m_CRC_cdf;

  public:

    AMC13Event(){}
    ~AMC13Event(){}

    void addAMCheader(const uint32_t & AMC_size_, const uint8_t & Blk_No_, const uint8_t & AMC_No_, const uint16_t & BoardID_)
    {
      m_AMC_size.push_back(AMC_size_);
      m_Blk_No.push_back(Blk_No_);
      m_AMC_No.push_back(AMC_No_);
      m_BoardID.push_back(BoardID_);
    }
    void addAMCpayload(AMCdata a){m_amcs.push_back(a);}

};

