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

gem::hw::vfat::HwVFAT2::HwVFAT2(const log4cplus::Logger& vfatLogger,
				std::string const& vfatDevice):
  gem::hw::GEMHwDevice::GEMHwDevice(vfatLogger)
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
    //check if we can talk to the hardware (read the chipID using the full read)
    // possible failures are: unable to complete the transaction
    // chip not connected so the chipID is invalid and extended registers return the register number
    //(currently the implemented check)
    // treat each situation differently, also, how often does this need to be done? every transaction?
    //that's a ton of overhead...
    DEBUG("Checking hardware connection");
    uint32_t chipTest = readReg(getDeviceBaseNode(),"ChipID0");
    DEBUG("1) read chipID0 0x" << std::hex << chipTest << std::dec << std::endl);

    uint32_t test1 = readReg(getDeviceBaseNode(),"ContReg2");
    DEBUG("2) read ContReg2 (test1) 0x" << std::hex << test1 << std::dec << std::endl);
    writeReg(getDeviceBaseNode(),"ContReg2",0x000000FF&(~test1));
    uint32_t test2 = readReg(getDeviceBaseNode(),"ContReg2");
    DEBUG("3) read ContReg2 (test2) 0x" << std::hex << test2 << std::dec << std::endl);
    writeReg(getDeviceBaseNode(),"ContReg2",0x000000FF&test1);
    uint32_t test3 = readReg(getDeviceBaseNode(),"ContReg2");
    DEBUG("4) read ContReg2 0x" << std::hex << test3 << std::dec << std::endl);

    chipTest = readReg(getDeviceBaseNode(),"ChipID0");
    DEBUG("5) read chipID0 0x" << std::hex << chipTest << std::dec << std::endl);
    if (test1 == test2) {
      is_connected_ = false;
      return false;
    } else {
      is_connected_ = true;
      return true;
    } 
  } else {
    is_connected_ = false;
    return false;
  }
}


// read VFAT chipID and upset/hit counters into vfatParams_ object
//void gem::hw::vfat::HwVFAT2::readVFAT2Counters(gem::hw::vfat::VFAT2ControlParams &params)
void gem::hw::vfat::HwVFAT2::readVFAT2Counters()
{
  vfatParams_.chipID       = getChipID();
  vfatParams_.upsetCounter = getUpsetCount();
  vfatParams_.hitCounter   = getHitCount();
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

  //counters
  DEBUG("getting all counters in HwVFAT2.cc");
  //readVFAT2Counters(params);
  readVFAT2Counters();
  
  //set the channel settings here
  DEBUG("getting all channel settings in HwVFAT2.cc");
  //readVFAT2Channels(params);
  readVFAT2Channels();
  DEBUG("done getting all settings in HwVFAT2.cc");
}

//channel specific settings
void gem::hw::vfat::HwVFAT2::sendCalPulseToChannel(uint8_t channel, bool on) {
  DEBUG(toolbox::toString("setting cal pulse for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 0 to 128
  if (channel > 128) {
    std::string msg =
      toolbox::toString("Channel specified (%d) larger than expected (128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  
  std::string registerName = "VFATChannels.ChanReg1";
  //registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  if (channel > 1)
    registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  //registerName = "VFATChannels.ChanReg1";
  
  uint8_t channelSettings = readVFATReg(registerName);
  
  if (channel == 0) 
    writeVFATReg(registerName,(channelSettings&~VFAT2ChannelBitMasks::CHANCAL0)|(on ? 0x80 : 0x0));
  else
    writeVFATReg(registerName,(channelSettings&~VFAT2ChannelBitMasks::CHANCAL)|(on ? 0x40 : 0x0));
}

void gem::hw::vfat::HwVFAT2::maskChannel(uint8_t channel, bool on) {
  DEBUG(toolbox::toString("setting mask for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",(unsigned)channel);
  uint8_t channelSettings = (readVFATReg(registerName)&~VFAT2ChannelBitMasks::ISMASKED);
  writeVFATReg(registerName,channelSettings|(on ? 0x20 : 0x0));
}

uint8_t gem::hw::vfat::HwVFAT2::getChannelTrimDAC(uint8_t channel) {
  DEBUG(toolbox::toString("getting trim DAC for channel %d HwVFAT2.cc",(unsigned)channel));
  //channel should be 1 to 128
  if ((channel > 128) || (channel < 1)) {
    std::string msg =
      toolbox::toString("Channel specified (%d) outside expectation (1-128)",channel);
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
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
    XCEPT_RAISE(gem::hw::vfat::exception::NonexistentChannel,msg);
    return;
  }
  std::string registerName = toolbox::toString("VFATChannels.ChanReg%d",channel);
  uint8_t channelSettings = (readVFATReg(registerName)&~VFAT2ChannelBitMasks::TRIMDAC)|trimDAC;
  writeVFATReg(registerName,channelSettings|trimDAC);
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
