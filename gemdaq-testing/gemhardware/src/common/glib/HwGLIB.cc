#include <iomanip>

#include "gem/hw/glib/HwGLIB.h"

gem::hw::glib::HwGLIB::HwGLIB(const log4cplus::Logger& glibLogger):
  gem::hw::GEMHwDevice::GEMHwDevice(glibLogger)
  //logGLIB_(glibApp->getApplicationLogger()),
  //hwGLIB_(0),
  //monGLIB_(0)
{
  setDeviceID("GLIBHw");
  setAddressTableFileName("glib_address_table.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();
}

gem::hw::glib::HwGLIB::~HwGLIB()
{
  releaseDevice();
}

void gem::hw::glib::HwGLIB::configureDevice(std::string const& xmlSettings)
{
  //here load the xml file settings onto the board
  
}

void gem::hw::glib::HwGLIB::configureDevice()
{
  //determine the manner in which to configure the device (XML or DB parameters)
  
}

//void gem::hw::glib::HwGLIB::connectDevice()
//{
//  std::string const controlhubAddress = cfgInfoSpaceP_->getString("controlhubAddress");
//  std::string const device1Address    = cfgInfoSpaceP_->getString("deviceAddress");
//  uint32_t const    controlhubPort    = cfgInfoSpaceP_->getUInt32("controlhubPort");
//  uint32_t const    ipbusPort         = cfgInfoSpaceP_->getUInt32("ipbusPort");
//  
//  std::stringstream tmpUri;
//  if (controlhubAddress.size() > 0) {
//      INFO("Using control hub at address '" << controlhubAddress
//           << ", port number " << controlhubPort << "'.");
//      tmpUri << "chtcp-"
//             << getIPbusProtocolVersion()
//             << "://"
//             << controlhubAddress
//             << ":"
//             << controlhubPort
//             << "?target="
//             << deviceAddress
//             << ":"
//             << ipbusPort;
//    } else {
//      INFO("No control hub address specified -> "
//           "continuing with a direct connection.");
//      tmpUri << "ipbusudp-"
//             << getIPbusProtocolVersion()
//             << "://"
//             << deviceAddress
//             << ":"
//             << ipbusPort;
//    }
//  std::string const uri = tmpUri.str();
//  std::string const id = "HwGLIB";
//  
//}
//
//void gem::hw::glib::HwGLIB::releaseDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::enableDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::disableDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::pauseDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::startDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::stopDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::resumeDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::haltDevice()
//{
//
//}
//
//void gem::hw::glib::HwGLIB::initDevice() 
//{
//  char * val;
//  val = std::getenv( "BUILD_HOME" );
//  std::string dirVal = "";
//  if (val != NULL) {
//    dirVal = val;
//  } else {
//    std::cout<<"$BUILD_HOME not set, exiting"<<std::endl;
//    exit(1);
//  }
//  
//  //setLogLevelTo(uhal::Debug());  // Maximise uHAL logging
//  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
//  char connectionPath[128];
//  try {
//    sprintf(connectionPath,"file://%s/data/myconnections.xml;",dirVal.c_str());
//    manageGLIBConnection = new uhal::ConnectionManager( connectionPath );
//  } catch (const std::exception& e) {
//    std::cout << "Something went wrong initializing the connection: " << e.what() << std::endl;
//  }
//}


bool gem::hw::vfat::GLIB::isHwConnected() 
{
  //return gem::hw::GEMHwDevice::isHwConnected();
  if (gem::hw::GEMHwDevice::isHwConnected()) {
    return true;
  }
  else
    return false;
}


std::string gem::hw::glib::HwGLIB::getBoardID()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.BOARD_ID");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getSystemID()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.SYSTEM_ID");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getIPAddress()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.IP_INFO");
  res = uint32ToDottedQuad(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getMACAddress()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  /*
  std::stringstream res;
  res << std::hex << (uint8_t)readReg(getDeviceBaseNode(),"SYSTEM.MAC.B5") << std::dec << ":";
  res << std::hex << (uint8_t)readReg(getDeviceBaseNode(),"SYSTEM.MAC.B4") << std::dec << ":";
  res << std::hex << (uint8_t)readReg(getDeviceBaseNode(),"SYSTEM.MAC.B3") << std::dec << ":";
  res << std::hex << (uint8_t)readReg(getDeviceBaseNode(),"SYSTEM.MAC.B2") << std::dec << ":";
  res << std::hex << (uint8_t)readReg(getDeviceBaseNode(),"SYSTEM.MAC.B1") << std::dec << ":";
  res << std::hex << (uint8_t)readReg(getDeviceBaseNode(),"SYSTEM.MAC.B0") << std::dec;
  return res.str();
  */
  std::string res = "N/A";
  uint32_t val1 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.UPPER");
  uint32_t val2 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.LOWER");
  res = uint32ToGroupedHex(val1,val2);
  return res;
}

std::string gem::hw::glib::HwGLIB::getFirmwareDate()
{
  // This returns the firmware build date. 
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  /*
    uint32_t yy = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.YY");
    uint32_t mm = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MM");
    uint32_t dd = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.DD");
    res << "20" << std::setfill('0') << std::setw(2) << yy
    << "-"
    << std::setw(2) << mm
    << "-"
    << std::setw(2) << dd;
  */
  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
  res << "20" << std::setfill('0') << std::setw(2) << (fwid&0x1f)
      << "-"
      << std::setw(2) << ((fwid>>5)&0x0f)
      << "-"
      << std::setw(2) << ((fwid>>9)&0x7f);
  return res.str();
}

std::string gem::hw::glib::HwGLIB::getFirmwareVer()
{
  // This returns the firmware version number. 
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  /*
    uint32_t versionMajor = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MAJOR");
    uint32_t versionMinor = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MINOR");
    uint32_t versionBuild = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.BUILD");
    res << versionMajor << "." << versionMinor << "." << versionBuild;
  */

  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
  res << ((fwid>>28)&0x0f)
      << "." 
      << ((fwid>>24)&0x0f)
      << "."
      << ((fwid>>16)&0xff);
  return res.str();
}

void gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t input, uint8_t output)
{
  if (xpoint2 && (input > 2 || output > 0)) {
    std::string msg = toolbox::toString("Invalid clock routing for XPoint2 %d -> %d",input,output);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidXPoint2Routing,msg);
    return;
  }
  
  if ((input > 3 || output > 3)) {
    std::string msg = toolbox::toString( "Invalid clock routing for XPoint%d %d -> %d",xpoint2,input,output);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidXPointRouting,msg);
    return;
  }
  
  std::stringstream regName;
  if (xpoint2)
    regName << "SYSTEM.CLK_CTRL.XPOINT2.";
  else
    regName << "SYSTEM.CLK_CTRL.XPOINT1.";
  
  switch(output) {
  case (0):
    regName << "S1";
  case (1):
    regName << "S2";
  case (2):
    regName << "S3";
  case (3):
    regName << "S4";
  }
  //input = b7b6b5b4b3b2b1b0 and all that matter are b1 and b0 -> 1 and 0 of, eg., S1
  // input == 0 -> b1b0 == 00
  // input == 1 -> b1b0 == 01
  // input == 2 -> b1b0 == 10
  // input == 3 -> b1b0 == 11
  // but the xpoint switch inverts b0 and b1 when routing outputs
  // thus to select input 3 for output 1, one sets S10=1 and S11=0
  writeReg(getDeviceBaseNode(),regName.str()+"1",input&0x01);
  writeReg(getDeviceBaseNode(),regName.str()+"0",(input&0x10)>>1);
}

uint8_t gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t output)
{
  /*
  if (xpoint2 && output > 0) {
    std::string msg = toolbox::toString("Invalid clock output for XPoint2 %d",output);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidXPoint2Routing,msg);
    return output;
  }
  */
  
  if (output > 3) {
    std::string msg = toolbox::toString( "Invalid clock output for XPoint%d %d",xpoint2,output);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidXPointRouting,msg);
    return output;
  }
  
  std::stringstream regName;
  if (xpoint2)
    regName << "SYSTEM.CLK_CTRL.XPOINT2.";
  else
    regName << "SYSTEM.CLK_CTRL.XPOINT1.";
  
  switch(output) {
  case (0):
    regName << "S1";
  case (1):
    regName << "S2";
  case (2):
    regName << "S3";
  case (3):
    regName << "S4";
  }
  uint8_t input = 0x0;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"0")&0x1)<<1;
  //input = input << 1;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"1")&0x1);
  return input;
}

uint8_t gem::hw::glib::HwGLIB::SFPStatus(uint8_t sfpcage)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.SFP" << (int)sfpcage << ".STATUS";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FMCPresence(bool fmc2)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FMC" << (int)fmc2 << "_PRESENT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::GbEInterrupt()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.GBE_INT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FPGAReset()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FPGA_RESET";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

uint8_t gem::hw::glib::HwGLIB::V6CPLD()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.V6_CPLD";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::CDCELocked()
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.CDCE_LOCK";
  return static_cast<bool>(readReg(getDeviceBaseNode(),regName.str()));
}

/** User core functionality **/
std::string gem::hw::glib::HwGLIB::getUserFirmware()
{
  // This returns the user firmware build date. 
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  uint32_t userfw = readReg(getDeviceBaseNode(),"GLIB_LINKS.LINK0.USER_FW");
  res << "0x"<< std::hex << userfw << std::dec;
  return res.str();
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::glib::HwGLIB::LinkStatus(uint8_t link) {
  
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
  } else {
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

void gem::hw::glib::HwGLIB::LinkReset(uint8_t link, uint8_t resets) {
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
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

uint32_t gem::hw::glib::HwGLIB::readTriggerFIFO() {
  std::stringstream regName;
  regName << "TRG_DATA.";
  uint32_t trgword = readReg(getDeviceBaseNode(),regName.str()+"DATA");
  return trgword;
}

void gem::hw::glib::HwGLIB::flushTriggerFIFO() {
  std::stringstream regName;
  regName << "TRG_DATA.";
  writeReg(getDeviceBaseNode(),regName.str()+"FIFO_FLUSH",0x1);
}

uint32_t gem::hw::glib::HwGLIB::getFIFOOccupancy(uint8_t link) {
  uint32_t fifocc = 0;
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return fifocc;
  }
  
  std::stringstream regName;
  regName << "OPTICAL_LINKS.LINK" << (int)link << ".TRK_FIFO.";
  fifocc = readReg(getDeviceBaseNode(),regName.str()+"DEPTH");
  
  return fifocc;
}

void gem::hw::glib::HwGLIB::flushFIFO(uint8_t link) {
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return;
  }

  std::stringstream regName;
  regName << "OPTICAL_LINKS.LINK" << (int)link << ".TRK_FIFO.";
  writeReg(getDeviceBaseNode(),regName.str()+"FLUSH",0x1);
}
