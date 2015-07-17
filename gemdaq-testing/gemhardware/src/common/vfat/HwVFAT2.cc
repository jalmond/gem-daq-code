//General structure taken blatantly from tcds::utils::HwDeviceTCA/HwGLIB

#include "gem/hw/vfat/HwVFAT2.h"

/* removing HW initialization with an application in favour of just a log4cplus::Logger
gem::hw::vfat::HwVFAT2::HwVFAT2(xdaq::Application* vfatApp,
				std::string const& vfatDevice):
  gem::hw::GEMHwDevice::GEMHwDevice(vfatApp)
//logVFAT2_(vfatApp->getApplicationLogger()),
//hwVFAT2_(0)
//monVFAT2_(0)
{
  //this->gem::hw::GEMHwDevice::GEMHwDevice();
  //gem::hw::vfat::HwVFAT2::initDevice();
  //can use a different address table for the VFAT access
  setAddressTableFileName("geb_vfat_address_table.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceID("VFAT2Hw");
  setDeviceBaseNode("VFATS."+vfatDevice);

  //what's the difference between connect, init, enable for VFAT?
  //check that register values are hardware default values, if not, something may be amiss
  
  //set register values to sw default values

  //hardware is enabled!

  //set register values to desired values

  //hardware is configured!

  //set run bit

  //hardware is running
}
*/

gem::hw::vfat::HwVFAT2::HwVFAT2(std::string const& vfatDevice):
  gem::hw::GEMHwDevice::GEMHwDevice(vfatDevice)
//logVFAT2_(vfatApp->getApplicationLogger()),
//hwVFAT2_(0)
//monVFAT2_(0)
{
  //this->gem::hw::GEMHwDevice::GEMHwDevice();
  //gem::hw::vfat::HwVFAT2::initDevice();
  //can use a different address table for the VFAT access
  setAddressTableFileName("geb_vfat_address_table.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceID("VFAT2Hw");
  setDeviceBaseNode("VFATS."+vfatDevice);
  is_connected_ = false;
  //what's the difference between connect, init, enable for VFAT?
  //check that register values are hardware default values, if not, something may be amiss
  
  //set register values to sw default values

  //hardware is enabled!

  //set register values to desired values

  //hardware is configured!

  //set run bit

  //hardware is running
}

gem::hw::vfat::HwVFAT2::~HwVFAT2()
{
  //what does release device do to the hardware?
  //does it turn it off?  it probably shouldn't, in case code crashes but
  //hardware should still take data
  releaseDevice();
}

void gem::hw::vfat::HwVFAT2::loadDefaults()
{
  //here load the default settings
  setTriggerMode(    0x3); //set to S1 to S8
  setCalibrationMode(0x0); //set to normal
  setMSPolarity(     0x1); //negative
  setCalPolarity(    0x1); //negative
  
  setProbeMode(        0x0);
  setLVDSMode(         0x0);
  setDACMode(          0x0);
  setHitCountCycleTime(0x0); //maximum number of bits
  
  setHitCountMode( 0x0);
  setMSPulseLength(0x3);
  setInputPadMode( 0x0);
  setTrimDACRange( 0x0);
  setBandgapPad(   0x0);
  sendTestPattern( 0x0);
  
  
  setIPreampIn(  168);
  setIPreampFeed(150);
  setIPreampOut(  80);
  setIShaper(    150);
  setIShaperFeed(100);
  setIComp(       75);
  
  setLatency(15);
  
  setVThreshold1(25);
  setVThreshold2(0);
}

void gem::hw::vfat::HwVFAT2::configureDevice(std::string const& xmlSettings)
{
  //here load the xml file settings onto the chip
  
}

void gem::hw::vfat::HwVFAT2::configureDevice()
{
  //determine the manner in which to configure the device (XML or DB parameters)
  
}

bool gem::hw::vfat::HwVFAT2::isHwConnected() 
{
  if ( is_connected_ )
    return true;
  
  else if (gem::hw::GEMHwDevice::isHwConnected()) {
    DEBUG("Checking hardware connection");
    try {
      uint32_t chipTest = readVFATReg("ChipID0");
      DEBUG("1) read chipID0 0x" << std::hex << chipTest << std::dec << std::endl);
      is_connected_ = true;
      return true;      
    } catch (gem::hw::vfat::exception::TransactionError const& e) {
      is_connected_ = false;
      return false;      
    } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
      is_connected_ = false;
      return false;      
    } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
      is_connected_ = false;
      return false;      
    }
    
  } else {
    is_connected_ = false;
    return false;
  }
}

//
uint8_t gem::hw::vfat::HwVFAT2::readVFATReg( std::string const& regName) {
  uint32_t readVal = readReg(getDeviceBaseNode(),regName);
  
  /*
  //check the transaction status
  //bit 31:27 - unused
  //bit 26 - error
  (readVal >> 26) & 0x1;
  //bit 25 - valid
  (readVal >> 25) & 0x0;
  //bit 24 - r/w
  (readVal >> 24) & 0x1;
  //bit 23:16 - VFAT number
  (readVal >> 16) & 0xff;
  //bit 15:8  - VFAT register
  (readVal >> 8) & 0xff;
  */
  //bit 7:0   - register value
  if ((readVal >> 26) & 0x1) {
    std::string msg = toolbox::toString("VFAT transaction error bit set reading register %s",regName.c_str());
    ++vfatErrors_.Error;
    ERROR(msg);
    XCEPT_RAISE(gem::hw::vfat::exception::TransactionError,msg);
  } else if ((readVal >> 25) & 0x0){
    std::string msg = toolbox::toString("VFAT transaction invalid bit set reading register %s",regName.c_str());
    ++vfatErrors_.Invalid;
    ERROR(msg);
    XCEPT_RAISE(gem::hw::vfat::exception::InvalidTransaction,msg);
  } else if ((readVal >> 24) & 0x0){
    std::string msg = toolbox::toString("VFAT read transaction returned write on register %s",regName.c_str());
    ++vfatErrors_.RWMismatch;
    ERROR(msg);
    XCEPT_RAISE(gem::hw::vfat::exception::WrongTransaction,msg);
  } else {
    return (readVal & 0xff);
  }
}

//
void gem::hw::vfat::HwVFAT2::readVFATRegs( vfat_reg_pair_list &regList) {
  register_pair_list fullRegList;
  for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
    fullRegList.push_back(std::make_pair(getDeviceBaseNode()+"."+curReg->first,static_cast<uint32_t>(curReg->second)));
  readRegs(fullRegList);
}

// read VFAT chipID and upset/hit counters into vfatParams_ object
//void gem::hw::vfat::HwVFAT2::readVFAT2Counters(gem::hw::vfat::VFAT2ControlParams &params)
void gem::hw::vfat::HwVFAT2::readVFAT2Counters()
{
  try {
    vfatParams_.chipID       = getChipID();
    vfatParams_.upsetCounter = getUpsetCount();
    vfatParams_.hitCounter   = getHitCount();
  } catch (gem::hw::vfat::exception::TransactionError& e) {
    DEBUG(e.what());
  } catch (gem::hw::vfat::exception::InvalidTransaction& e) {
    DEBUG(e.what());
  } catch (gem::hw::vfat::exception::WrongTransaction& e) {
    DEBUG(e.what());
  }
}

//void gem::hw::vfat::HwVFAT2::readVFAT2Channel(gem::hw::vfat::VFAT2ControlParams &params, uint8_t channel)
void gem::hw::vfat::HwVFAT2::readVFAT2Channel(uint8_t channel)
{
  uint8_t chanSettings = getChannelSettings(channel);
  if (channel>1)
    vfatParams_.activeChannel = (unsigned)channel;
  vfatParams_.channels[channel-1].fullChannelReg = chanSettings;
  vfatParams_.channels[channel-1].calPulse0 = false;
  if (channel == 1)
    vfatParams_.channels[channel-1].calPulse0 = ((chanSettings&VFAT2ChannelBitMasks::CHANCAL0)>>VFAT2ChannelBitShifts::CHANCAL0);
  vfatParams_.channels[channel-1].calPulse    = ((chanSettings&VFAT2ChannelBitMasks::CHANCAL )>>VFAT2ChannelBitShifts::CHANCAL );
  vfatParams_.channels[channel-1].mask        = ((chanSettings&VFAT2ChannelBitMasks::ISMASKED)>>VFAT2ChannelBitShifts::ISMASKED);
  vfatParams_.channels[channel-1].trimDAC     = ((chanSettings&VFAT2ChannelBitMasks::TRIMDAC )>>VFAT2ChannelBitShifts::TRIMDAC );
  DEBUG("readVFAT2Channel " << (unsigned)channel << " - 0x"
	<< std::hex << static_cast<unsigned>(vfatParams_.channels[channel-1].fullChannelReg) << std::dec << "::<"
	<< std::hex << static_cast<unsigned>(vfatParams_.channels[channel-1].calPulse0     ) << std::dec << ":"
	<< std::hex << static_cast<unsigned>(vfatParams_.channels[channel-1].calPulse      ) << std::dec << ":"
	<< std::hex << static_cast<unsigned>(vfatParams_.channels[channel-1].mask          ) << std::dec << ":"
	<< std::hex << static_cast<unsigned>(vfatParams_.channels[channel-1].trimDAC       ) << std::dec << ">"
	<< std::endl);
}

//void gem::hw::vfat::HwVFAT2::readVFAT2Channels(gem::hw::vfat::VFAT2ControlParams &params)
void gem::hw::vfat::HwVFAT2::readVFAT2Channels()
{
  for (uint8_t chan = 1; chan < 129; ++chan) {
    //readVFAT2Channel(vfatParams_,chan);
    readVFAT2Channel(chan);
    DEBUG("chan = "<< (unsigned)chan << "; activeChannel = " <<(unsigned)vfatParams_.activeChannel << std::endl);
  }
}

/////////************************///////////////
void gem::hw::vfat::HwVFAT2::setAllSettings(const gem::hw::vfat::VFAT2ControlParams &params) {
  //want to lock the hardware while performing this operation
  //when sending all these requests, might it be better to do a single transaction?
  
  //check that the hardware is alive
  //check that the settings are non-empty?
  uint8_t cont0 = 0x0;
  uint8_t cont1 = 0x0;
  uint8_t cont2 = 0x0;
  uint8_t cont3 = 0x0;
  
  setRunMode(        params.runMode   ,cont0);
  setTriggerMode(    params.trigMode  ,cont0);
  setMSPolarity(     params.msPol     ,cont0);
  setCalPolarity(    params.calPol    ,cont0);
  setCalibrationMode(params.calibMode ,cont0);
  writeVFATReg("ContReg0",cont0);
  
  setDACMode(          params.dacMode  ,cont1);
  setProbeMode(        params.probeMode,cont1);
  setLVDSMode(         params.lvdsMode ,cont1);
  setHitCountCycleTime(params.reHitCT  ,cont1);
  writeVFATReg("ContReg1",cont1);
  
  setHitCountMode( params.hitCountMode,cont2);
  setMSPulseLength(params.msPulseLen  ,cont2);
  setInputPadMode( params.digInSel    ,cont2);
  writeVFATReg("ContReg2",cont2);
  
  setTrimDACRange(   params.trimDACRange   ,cont3);
  setBandgapPad(     params.padBandGap     ,cont3);
  sendTestPattern   (params.sendTestPattern,cont3);
  writeVFATReg("ContReg3",cont3);

  setIPreampIn(  params.iPreampIn  );
  setIPreampFeed(params.iPreampFeed);
  setIPreampOut( params.iPreampOut );
  setIShaper(    params.iShaper    );
  setIShaperFeed(params.iShaperFeed);
  setIComp(      params.iComp      );
  
  setVCal(       params.vCal    );
  setVThreshold1(params.vThresh1);
  setVThreshold2(params.vThresh2);
  setCalPhase(   params.calPhase);

  //set the channel settings here
  //channel 0/1
  uint8_t chanReg = 0x0;
  chanReg|=(params.channels[0].trimDAC  <<VFAT2ChannelBitShifts::TRIMDAC );
  chanReg|=(params.channels[0].mask     <<VFAT2ChannelBitShifts::ISMASKED);
  chanReg|=(params.channels[0].calPulse <<VFAT2ChannelBitShifts::CHANCAL );
  chanReg|=(params.channels[0].calPulse0<<VFAT2ChannelBitShifts::CHANCAL0);
  writeVFATReg("VFATChannels.ChanReg1",chanReg);

  for (uint8_t chan = 2; chan < N_VFAT2_CHANNELS+1; ++chan) {
    chanReg = 0x0;
    chanReg|=(params.channels[chan-1].trimDAC <<VFAT2ChannelBitShifts::TRIMDAC );
    chanReg|=(params.channels[chan-1].mask    <<VFAT2ChannelBitShifts::ISMASKED);
    chanReg|=(params.channels[chan-1].calPulse<<VFAT2ChannelBitShifts::CHANCAL );
    writeVFATReg(toolbox::toString("VFATChannels.ChanReg%d",(unsigned)chan),(unsigned)chanReg);
  }
}

//void gem::hw::vfat::HwVFAT2::getAllSettings(gem::hw::vfat::VFAT2ControlParams &params) {
void gem::hw::vfat::HwVFAT2::getAllSettings() {
  //want to lock the hardware (and the params variable) while performing this operation
  //when sending all these requests, might it be better to do a single transaction?
  //check that the hardware is alive
  //maybe find a way to do a single transaction, using the vector read/write?
  //what is the performance cost/benefit

  DEBUG("getting all settings in HwVFAT2.cc");
  try {
    uint8_t cont0 = readVFATReg("ContReg0");
    uint8_t cont1 = readVFATReg("ContReg1");
    uint8_t cont2 = readVFATReg("ContReg2");
    uint8_t cont3 = readVFATReg("ContReg3");
  
    DEBUG("getting all settings in HwVFAT2.cc");
    vfatParams_.control0 = static_cast<unsigned>(cont0);
    DEBUG("getting all settings in HwVFAT2.cc");
    vfatParams_.control1 = static_cast<unsigned>(cont1);
    DEBUG("getting all settings in HwVFAT2.cc");
    vfatParams_.control2 = static_cast<unsigned>(cont2);
    DEBUG("getting all settings in HwVFAT2.cc");
    vfatParams_.control3 = static_cast<unsigned>(cont3);
    
    DEBUG("storing RunMode in HwVFAT2.cc");
    vfatParams_.runMode   = static_cast<VFAT2RunMode  >(getRunMode(        cont0));
    DEBUG("storing TriggerMode in HwVFAT2.cc");
    vfatParams_.trigMode  = static_cast<VFAT2TrigMode >(getTriggerMode(    cont0));
    DEBUG("storing MSPolarity in HwVFAT2.cc");
    vfatParams_.msPol     = static_cast<VFAT2MSPol    >(getMSPolarity(     cont0));
    DEBUG("storing CalPolarity in HwVFAT2.cc");
    vfatParams_.calPol    = static_cast<VFAT2CalPol   >(getCalPolarity(    cont0));
    DEBUG("storing CalibrationMode in HwVFAT2.cc");
    vfatParams_.calibMode = static_cast<VFAT2CalibMode>(getCalibrationMode(cont0));
    
    DEBUG("storing DACMode in HwVFAT2.cc");
    vfatParams_.dacMode   = static_cast<VFAT2DACMode  >(getDACMode(          cont1));
    DEBUG("storing ProbeMode in HwVFAT2.cc");
    vfatParams_.probeMode = static_cast<VFAT2ProbeMode>(getProbeMode(        cont1));
    DEBUG("storing LVDSMode in HwVFAT2.cc");
    vfatParams_.lvdsMode  = static_cast<VFAT2LVDSMode >(getLVDSMode(         cont1));
    DEBUG("storing HitCycleTime in HwVFAT2.cc");
    vfatParams_.reHitCT   = static_cast<VFAT2ReHitCT  >(getHitCountCycleTime(cont1));
    
    DEBUG("storing HitCountMode in HwVFAT2.cc");
    vfatParams_.hitCountMode = static_cast<VFAT2HitCountMode >(getHitCountMode( cont2));
    DEBUG("storing MSPulseLength in HwVFAT2.cc");
    vfatParams_.msPulseLen   = static_cast<VFAT2MSPulseLength>(getMSPulseLength(cont2));
    DEBUG("storing DigInSel in HwVFAT2.cc");
    vfatParams_.digInSel     = static_cast<VFAT2DigInSel     >(getInputPadMode( cont2));
    
    DEBUG("storing TrimDACRange in HwVFAT2.cc");
    vfatParams_.trimDACRange    = static_cast<VFAT2TrimDACRange >(getTrimDACRange(   cont3));
    DEBUG("storing PadBandgap in HwVFAT2.cc");
    vfatParams_.padBandGap      = static_cast<VFAT2PadBandgap   >(getBandgapPad(     cont3));
    DEBUG("storing DFTestPattern HwVFAT2.cc");
    vfatParams_.sendTestPattern = static_cast<VFAT2DFTestPattern>(getTestPatternMode(cont3));
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the control registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the control registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the control registers, wrong transaction bit set");
  }

  try {
    vfatParams_.latency = getLatency();
    
    vfatParams_.iPreampIn   = getIPreampIn()  ;
    vfatParams_.iPreampFeed = getIPreampFeed();
    vfatParams_.iPreampOut  = getIPreampOut() ;
    vfatParams_.iShaper     = getIShaper()    ;
    vfatParams_.iShaperFeed = getIShaperFeed();
    vfatParams_.iComp       = getIComp()      ;
    
    vfatParams_.vCal     = getVCal()       ;
    vfatParams_.vThresh1 = getVThreshold1();
    vfatParams_.vThresh2 = getVThreshold2();
    vfatParams_.calPhase = getCalPhase()   ;
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the analog settings registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the analog settings registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the analog settings registers, wrong transaction bit set");
  }

  //counters
  DEBUG("getting all counters in HwVFAT2.cc");
  //readVFAT2Counters(params);
  try {
    readVFAT2Counters();
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the VFAT counter registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the VFAT counter registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the VFAT counter registers, wrong transaction bit set");
  }

  //set the channel settings here
  DEBUG("getting all channel settings in HwVFAT2.cc");
  //readVFAT2Channels(params);
  try {
    readVFAT2Channels();
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the VFAT channel registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the VFAT channel registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the VFAT channel registers, wrong transaction bit set");
  }
  DEBUG("done getting all settings in HwVFAT2.cc");
}

//channel specific settings
void gem::hw::vfat::HwVFAT2::enableCalPulseToChannel(uint8_t channel, bool on) {
  DEBUG(toolbox::toString("setting cal pulse for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 0 to 128
  if (channel > 128) {
    std::string msg =
      toolbox::toString("Channel specified (%d) larger than expected (128)",channel);
    //XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    ERROR(msg);
    return;
  }
  
  std::string registerName = "VFATChannels.ChanReg1";
  if (channel > 1)
    registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  
  try {
    uint8_t channelSettings = readVFATReg(registerName);
    
    if (channel == 0) 
      writeVFATReg(registerName,(channelSettings&~VFAT2ChannelBitMasks::CHANCAL0)|(on ? 0x80 : 0x0));
    else
      writeVFATReg(registerName,(channelSettings&~VFAT2ChannelBitMasks::CHANCAL)|(on ? 0x40 : 0x0));
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the control registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the control registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the control registers, wrong transaction bit set");
  }

}

void gem::hw::vfat::HwVFAT2::maskChannel(uint8_t channel, bool on) {
  DEBUG(toolbox::toString("setting mask for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    //XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    ERROR(msg);
    return;
  }
  try {
    std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
    uint8_t channelSettings = (readVFATReg(registerName)&~VFAT2ChannelBitMasks::ISMASKED);
    writeVFATReg(registerName,channelSettings|(on ? 0x20 : 0x0));
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the control registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the control registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the control registers, wrong transaction bit set");
  }
}

uint8_t gem::hw::vfat::HwVFAT2::getChannelTrimDAC(uint8_t channel) {
  DEBUG(toolbox::toString("getting trim DAC for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    //XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return 0xff;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  return (readVFATReg(registerName)&VFAT2ChannelBitMasks::TRIMDAC);
}

void gem::hw::vfat::HwVFAT2::setChannelTrimDAC(uint8_t channel, uint8_t trimDAC) {
  DEBUG(toolbox::toString("setting trim DAC for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    //XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",channel);
  try {
    uint8_t channelSettings = (readVFATReg(registerName)&~VFAT2ChannelBitMasks::TRIMDAC)|trimDAC;
    writeVFATReg(registerName,channelSettings|trimDAC);
  } catch (gem::hw::vfat::exception::TransactionError const& e) {
    WARN("Problem reading the control registers, transaction error bit set");
  } catch (gem::hw::vfat::exception::InvalidTransaction const& e) {
    WARN("Problem reading the control registers, invalid transaction bit set");
  } catch (gem::hw::vfat::exception::WrongTransaction const& e) {
    WARN("Problem reading the control registers, wrong transaction bit set");
  }
}

/***
void gem::hw::vfat::HwVFAT2::setChannelTrimDAC(uint8_t channel, double trimDAC) {
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = "VFATChannels.ChanReg"+channel;
  uint8_t channelSettings = readVFATReg(registerName);
  writeVFATReg(registerName,channelSettings|0x20);
}
***/
