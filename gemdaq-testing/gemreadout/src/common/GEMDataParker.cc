#include "gem/readout/GEMDataParker.h"
#include "gem/readout/GEMDataAMCformat.h"
#include "gem/hw/glib/HwGLIB.h"

#include <boost/utility/binary.hpp>
#include <bitset>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "gem/utils/GEMLogging.h"

int OHv=2;
int counterVFATs_=0, event_=0;
bool dumpGEMevent_ = false;
uint64_t ZSFlag=0;

// Main constructor
gem::readout::GEMDataParker::GEMDataParker(gem::hw::glib::HwGLIB& glibDevice,
					   std::string const& outFileName,
					   std::string const& outputType) :
  gemLogger_(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("gem:readout:GEMDataParker")))
{
  //gemLogger_   = log4cplus::Logger::getInstance("gem:readout:GEMDataParker");
  glibDevice_  = &glibDevice;
  outFileName_ = outFileName;
  outputType_  = outputType;
  counter_ = 0;
}

int gem::readout::GEMDataParker::dumpDataToDisk()
{
  // Book GEM Data format
  gem::readout::GEMData  gem;
  gem::readout::GEBData  geb;
  gem::readout::VFATData vfat;
  
  // get GLIB data from one VFAT chip, as it's (update that part for MP7 when it'll be)
  dumpGEMevent_ = false;
  counter_ = gem::readout::GEMDataParker::getGLIBData(gem, geb, vfat);
  
  // Write GEM Data to Disk, when GEM event is off
  if( dumpGEMevent_ ){
    event_++;
    DEBUG(" dumpDataToDisk:: dumpGEMevent: event_ " << event_ 
	  << " counter_ " << counter_
	  << " counterVFATs " << counterVFATs_);
    gem::readout::GEMDataParker::writeGEMevent(gem, geb, vfat);
  }
  
  return counter_;
}

int gem::readout::GEMDataParker::getGLIBData(gem::readout::GEMData& gem, gem::readout::GEBData& geb, gem::readout::VFATData& vfat)
{
  // Book VFAT variables
  bool     isFirst = true;
  uint8_t  SBit, flags;
  uint16_t bcn, evn, chipid, crc;
  uint32_t bxNum, bxExp, TrigReg, bxNumTr;
  uint64_t msData, lsData;

  // GLIB data buffer validation
  boost::format linkForm("LINK%d");
  uint32_t fifoDepth[3];
  fifoDepth[0] = glibDevice_->getFIFOOccupancy(0x0);
  fifoDepth[1] = glibDevice_->getFIFOOccupancy(0x1);
  fifoDepth[2] = glibDevice_->getFIFOOccupancy(0x2);

  if(fifoDepth[0])
    INFO(glibDevice_->getDeviceBaseNode() << "." << boost::str(linkForm%(0))+".TRK_FIFO.DEPTH -- " <<
	 "bufferDepth[0] = " << std::hex << fifoDepth[0] << std::dec);
  if(fifoDepth[1])
    INFO(glibDevice_->getDeviceBaseNode() << "." << boost::str(linkForm%(1))+".TRK_FIFO.DEPTH -- " <<
	 "bufferDepth[1] = " << std::hex << fifoDepth[1] << std::dec);
  if(fifoDepth[2])
    INFO(glibDevice_->getDeviceBaseNode() << "." << boost::str(linkForm%(2))+".TRK_FIFO.DEPTH -- " <<
	 "bufferDepth[2] = " << std::hex << fifoDepth[2] << std::dec);

  /** the FIFO depth is not reliable */
  int bufferDepth = 0;
  /*
  if ( fifoDepth[0] != fifoDepth[1] || fifoDepth[0] != fifoDepth[2] || fifoDepth[1] != fifoDepth[2] ) {
    if (OHv == 1){
      bufferDepth = std::min(fifoDepth[0],std::min(fifoDepth[1],fifoDepth[2]));
    } else if (OHv == 2){
      bufferDepth = std::min(fifoDepth[0],fifoDepth[2]);
    }
    }*/

  // LINK1
  bufferDepth = fifoDepth[1];
  INFO("OHv " << OHv << " bufferDepth = " << std::hex << bufferDepth << std::dec);

  // For each event in GLIB data buffer
  // should probably switch this while with the next if, to ensure that there is actually a value in the vector
  while (bufferDepth) {
    std::vector<uint32_t> data;

    if (glibDevice_->hasTrackingData(0x1)) {

      data = glibDevice_->getTrackingData(0x1);
      //for (int word = 0; word < 7; ++word) {
      //	std::stringstream ss9;
      //	ss9 << "DATA." << word;
      //	data.push_back(glibDevice_->readReg(glibDevice_->getDeviceBaseNode(),ss9.str()));
      //}
    }

    // read trigger data
    TrigReg = glibDevice_->readTriggerFIFO(0x1);
    bxNumTr = TrigReg >> 6;
    SBit = TrigReg & 0x0000003F;

    uint16_t b1010, b1100, b1110;
    b1010 = ((data.at(5) & 0xF0000000)>>28);
    b1100 = ((data.at(5) & 0x0000F000)>>12);
    b1110 = ((data.at(4) & 0xF0000000)>>28);
	
    if (!(((b1010 == 0xa) && (b1100==0xc) && (b1110==0xe)))){
      WARN("VFAT headers do not match expectation");
      bufferDepth = glibDevice_->getFIFOOccupancy(0x1);
      continue;
    }

    bxNum = data.at(6);

    if (isFirst){
      bxExp = bxNum;
      if (counterVFATs_ != 0){
	dumpGEMevent_ = true;
	DEBUG(" getGLIBData:: End Event: counter_ " << counter_ << " counterVFATs " << counterVFATs_);
	ZSFlag = 0;
      }
      counterVFATs_ = 0;
    }

    counter_++;
    counterVFATs_++;

    if (bxNum == bxExp){
      isFirst = false;
    } else { 
      isFirst = true;
    }

    // bxExp:28
    // bxNum  = (bxNum << 8 ) | (SBit); // bxNum:8  | SBit:8

    bcn    = (0x0fff0000 & data.at(5)) >> 16;
    evn    = (0x00000ff0 & data.at(5)) >> 4;
    chipid = (0x0fff0000 & data.at(4)) >> 16;
    //cout<< " " << hex << b1110 << " ChipID " << chipid << dec << endl;
    flags  = (0x0000000f & data.at(5));
    crc    = (0x0000ffff & data.at(0));

    uint64_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
    uint64_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
    uint64_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
    uint64_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);

    lsData = (data3 << 32) | (data4);
    msData = (data1 << 32) | (data2);

    vfat.BC     = ( b1010 << 12 ) | (bcn);                // 1010     | bcn:12
    vfat.EC     = ( b1100 << 12 ) | (evn << 4) | (flags); // 1100     | EC:8      | Flag:4
    vfat.ChipID = ( b1110 << 12 ) | (chipid);             // 1110     | ChipID:12
    vfat.lsData = lsData;                                 // lsData:64
    vfat.msData = msData;                                 // msData:64
    vfat.crc    = crc;                                    // crc:16

   /*
    * dump VFAT data
    gem::readout::printVFATdataBits(counter_, vfat);
    */

    bufferDepth = glibDevice_->getFIFOOccupancy(0x1);

    /*
     * dump VFAT data */
    gem::readout::printVFATdataBits(counter_, vfat);
      
    // GEM data filling
    gem::readout::GEMDataParker::fillGEMevent(gem, geb, vfat);

    //sergey//glibDevice_->setDeviceBaseNode("OptoHybrid.GEB.TRK_DATA.COL1");

  }//closes check on DATA_RDY
  //}//closes while loop
  
  return counter_;
}

void gem::readout::GEMDataParker::fillGEMevent(gem::readout::GEMData& gem, gem::readout::GEBData& geb, gem::readout::VFATData& vfat)
{
  /*
   *  GEM, All Chamber Data
   */

  // GEM Event Headers [1]
  uint64_t AmcNo       = BOOST_BINARY( 1 );    // :4 
  uint64_t ZeroFlag    = BOOST_BINARY( 0000 ); // :4
  uint64_t LV1ID       = BOOST_BINARY( 1 );    // :24
  uint64_t BXID        = BOOST_BINARY( 1 );    // :12
  uint64_t DataLgth    = BOOST_BINARY( 1 );    // :20

  gem.header1 = (AmcNo <<60)|(ZeroFlag << 56)|(LV1ID <<32)|(BXID << 20)|(DataLgth);

  AmcNo    =  (0xf000000000000000 & gem.header1) >> 60;
  ZeroFlag =  (0x0f00000000000000 & gem.header1) >> 56; 
  LV1ID    =  (0x00ffffff00000000 & gem.header1) >> 32; 
  BXID     =  (0x00000000fff00000 & gem.header1) >> 20;
  DataLgth =  (0x00000000000fffff & gem.header1);

  // GEM Event Headers [2]
  uint64_t User        = BOOST_BINARY( 1 );    // :32
  uint64_t OrN         = BOOST_BINARY( 1 );    // :16
  uint64_t BoardID     = BOOST_BINARY( 1 );    // :16

  gem.header2 = (User << 32)|(OrN << 16)|(BoardID);

  User     =  (0xffffffff00000000 & gem.header2) >> 32; 
  OrN      =  (0x00000000ffff0000 & gem.header2) >> 16;
  BoardID  =  (0x000000000000ffff & gem.header2);

  // GEM Event Headers [3]
  uint64_t DAVList     = BOOST_BINARY( 1 );    // :24
  uint64_t BufStat     = BOOST_BINARY( 1 );    // :24
  uint64_t DAVCount    = BOOST_BINARY( 1 );    // :5
  uint64_t FormatVer   = BOOST_BINARY( 1 );    // :3
  uint64_t MP7BordStat = BOOST_BINARY( 1 );    // :8

  gem.header3 = (BufStat << 40)|(DAVCount << 16)|(DAVCount << 11)|(FormatVer << 8)|(MP7BordStat);

  DAVList     = (0xffffff0000000000 & gem.header3) >> 40; 
  BufStat     = (0x000000ffffff0000 & gem.header3) >> 16;
  DAVCount    = (0x000000000000ff00 & gem.header3) >> 11;
  FormatVer   = (0x0000000000000f00 & gem.header3) >> 8;
  MP7BordStat = (0x00000000000000ff & gem.header3);

  // GEM Event Treailer [2]
  uint64_t EventStat  = BOOST_BINARY( 1 );    // :32
  uint64_t GEBerrFlag = BOOST_BINARY( 1 );    // :24

  gem.trailer2 = ( EventStat << 40)|(GEBerrFlag);

  FormatVer   = (0xffffffffff000000 & gem.trailer2) >> 40;
  MP7BordStat = (0x0000000000ffffff & gem.trailer2);

  // GEM Event Treailer [1]
  uint64_t crc      = BOOST_BINARY( 1 );    // :32
  uint64_t LV1IDT   = BOOST_BINARY( 1 );    // :8
  ZeroFlag = BOOST_BINARY( 0000 ); // :4
  DataLgth = BOOST_BINARY( 1 );    // :20

  gem.trailer1 = (crc<<32)|(LV1IDT << 24)|(ZeroFlag <<20)|(DataLgth);

  crc      = (0xffffffff00000000 & gem.trailer1) >> 32;
  LV1IDT   = (0x00000000ff000000 & gem.trailer1) >> 24;
  ZeroFlag = (0x0000000000f00000 & gem.trailer1) >> 20;
  DataLgth = (0x00000000000fffff & gem.trailer1);

  /*
    int nGEBs = 1;
    for (int nume = 0; nume < nGEBs; nume++){
    gem.gebs.push_back(geb);
    }
    DEBUG(" gem.gebs.size " << int(gem.gebs.size())); */

  /*
   * One GEM bord loop, 24 VFAT chips maximum
   */

  /*
   * GEB, One Chamber Data
   */
  int IndexVFATChipOnGEB = -99;
  // VFAT position definition on the board, very temporary
  if ((0x0fff & vfat.ChipID)        == 0x838 ){
    IndexVFATChipOnGEB = 0;
  } else if ((0x0fff & vfat.ChipID) == 0xe7b ){ 
    IndexVFATChipOnGEB = 4;
  } else if ((0x0fff & vfat.ChipID) == 0xe21 ){ 
    IndexVFATChipOnGEB = 8;
  } else if ((0x0fff & vfat.ChipID) == 0xe74 ){ 
    IndexVFATChipOnGEB = 12;
  } else if ((0x0fff & vfat.ChipID) == 0x840 ){ 
    IndexVFATChipOnGEB = 16;
  } else if ((0x0fff & vfat.ChipID) == 0xa64 ){ 
    IndexVFATChipOnGEB = 20;
  } else { 
  };

  geb.vfats.push_back(vfat);
  DEBUG(" geb.vfats.size " << int(geb.vfats.size()));
    
  // Chamber Header, Zero Suppression flags, Chamber ID
  ZSFlag           = (ZSFlag | (1 << (23-IndexVFATChipOnGEB))); // :24
  uint64_t ChamID  = 0xdea;                                     // :12
  uint64_t sumVFAT = int(geb.vfats.size());                     // :28, geb.vfats.size was placed a very temporary here!!!

  geb.header  = (ZSFlag << 40)|(ChamID << 28)|(sumVFAT);

  //show24bits(ZSFlag); 
  DEBUG(" ChipID 0x" << hex << (0x0fff & vfat.ChipID) << dec << " IndexVFATChipOnGEB " << IndexVFATChipOnGEB);

  ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; 
  ChamID =  (0x000000fff0000000 & geb.header) >> 28; 

  DEBUG(" ZSFlag " << hex << ZSFlag << " ChamID " << ChamID << dec << " sumVFAT " << sumVFAT);

  // Chamber Trailer, OptoHybrid: crc, wordcount, Chamber status
  uint64_t OHcrc       = BOOST_BINARY( 1 ); // :16
  uint64_t OHwCount    = BOOST_BINARY( 1 ); // :16
  uint64_t ChamStatus  = BOOST_BINARY( 1 ); // :16
  geb.trailer = ((OHcrc << 48)|(OHwCount << 32 )|(ChamStatus << 16));

  OHcrc      = (0xffff000000000000 & geb.trailer) >> 48; 
  OHwCount   = (0x0000ffff00000000 & geb.trailer) >> 32; 
  ChamStatus = (0x00000000ffff0000 & geb.trailer) >> 16;

  DEBUG(" OHcrc " << hex << OHcrc << " OHwCount " << OHwCount << " ChamStatus " << ChamStatus << dec);

}

void gem::readout::GEMDataParker::writeGEMevent(gem::readout::GEMData& gem, gem::readout::GEBData& geb, gem::readout::VFATData& vfat)
{
  INFO("\nwriteGEMevent:: event_ " << event_ << " counter= " << counter_ << " counterVFATs " << counterVFATs_  
       << " sumVFAT " << (0x000000000fffffff & geb.header));

  // GEM Chamber's data level
  /*
    int nGEB=0;
    for (vector<GEBData>::iterator iGEB=gem.gebs.begin(); iGEB != gem.gebs.end(); ++iGEB){
    nGEB++;
    uint64_t ZSFlag =  (0xffffff0000000000 & geb.header) >> 40; show24bits(ZSFlag);
  */

  // GEB data level
  if(outputType_ == "Hex"){
    writeGEBheader (outFileName_, event_, geb);
  } else {
    writeGEBheaderBinary (outFileName_, event_, geb);
  } 
  printGEBheader (event_, geb);
    
  int nChip=0;
  for (vector<VFATData>::iterator iVFAT=geb.vfats.begin(); iVFAT != geb.vfats.end(); ++iVFAT){
    nChip++;
    vfat.BC     = (*iVFAT).BC;
    vfat.EC     = (*iVFAT).EC;
    vfat.ChipID = (*iVFAT).ChipID;
    vfat.lsData = (*iVFAT).lsData;
    vfat.msData = (*iVFAT).msData;
    vfat.crc    = (*iVFAT).crc;
      
    if(outputType_ == "Hex"){
      gem::readout::writeVFATdata (outFileName_, nChip, vfat); 
    } else {
      gem::readout::writeVFATdataBinary (outFileName_, nChip, vfat);
    } 
    gem::readout::printVFATdataBits(nChip, vfat);
  } //end of VFAT

  if(outputType_ == "Hex"){
    writeGEBtrailer (outFileName_, event_, geb);
  } else {
    writeGEBtrailerBinary (outFileName_, event_, geb);
  } 

  /* } // end of GEB */
}
