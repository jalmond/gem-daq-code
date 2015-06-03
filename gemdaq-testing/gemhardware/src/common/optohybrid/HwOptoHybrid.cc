#include "gem/hw/optohybrid/HwOptoHybrid.h"

gem::hw::optohybrid::HwOptoHybrid::HwOptoHybrid(const log4cplus::Logger& optoLogger):
  gem::hw::GEMHwDevice::GEMHwDevice(optoLogger)
  //logOptoHybrid_(optohybridApp->getApplicationLogger()),
  //hwOptoHybrid_(0),
  //monOptoHybrid_(0)
  
{
  setDeviceID("OptoHybridHw");
  setAddressTableFileName("optohybrid_address_table.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceBaseNode("OptoHybrid");
  //gem::hw::optohybrid::HwOptoHybrid::initDevice();
}

gem::hw::optohybrid::HwOptoHybrid::~HwOptoHybrid()
{
  releaseDevice();
}

void gem::hw::optohybrid::HwOptoHybrid::configureDevice(std::string const& xmlSettings)
{
  //here load the xml file settings onto the board
  
}

void gem::hw::optohybrid::HwOptoHybrid::configureDevice()
{
  //determine the manner in which to configure the device (XML or DB parameters)
  
}

//void gem::hw::optohybrid::HwOptoHybrid::releaseDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::enableDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::disableDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::pauseDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::startDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::stopDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::resumeDevice()
//{
//
//}
//
//void gem::hw::optohybrid::HwOptoHybrid::haltDevice()
//{
//
//}
//
gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::optohybrid::HwOptoHybrid::LinkStatus(uint8_t link) {
  
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    XCEPT_RAISE(gem::hw::optohybrid::exception::InvalidLink,msg);
  }
  else {
    std::stringstream regName;
    regName << "OPTICAL_LINKS.LINK" << (int)link << ".Counter.";
    linkStatus.linkErrCnt      = readReg(getDeviceBaseNode(),regName.str()+"LinkErr"      );
    linkStatus.linkVFATI2CRec  = readReg(getDeviceBaseNode(),regName.str()+"RecI2CReqests");
    linkStatus.linkVFATI2CSnt  = readReg(getDeviceBaseNode(),regName.str()+"SntI2CReqests");
    linkStatus.linkRegisterRec = readReg(getDeviceBaseNode(),regName.str()+"RecRegReqests");
    linkStatus.linkRegisterSnt = readReg(getDeviceBaseNode(),regName.str()+"SntRegReqests");
  }
  return linkStatus;
}

void gem::hw::optohybrid::HwOptoHybrid::LinkReset(uint8_t link, uint8_t resets) {
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    XCEPT_RAISE(gem::hw::optohybrid::exception::InvalidLink,msg);
    return;
  }
  
  std::stringstream regName;
  regName << "OPTICAL_LINKS.LINK" << (int)link << ".Resets.";
  if (resets&0x01)
    writeReg(getDeviceBaseNode(),regName.str()+"LinkErr",0x1);
  if (resets&0x02)
    writeReg(getDeviceBaseNode(),regName.str()+"RecI2CReqests",0x1);
  if (resets&0x04)
    writeReg(getDeviceBaseNode(),regName.str()+"SntI2CReqests",0x1);
  if (resets&0x08)
    writeReg(getDeviceBaseNode(),regName.str()+"RecRegReqests",0x1);
  if (resets&0x10)
    writeReg(getDeviceBaseNode(),regName.str()+"SntRegReqests",0x1);
}

//uint32_t gem::hw::optohybrid::HwOptoHybrid::readTriggerData() {
//  return uint32_t value;
//}
