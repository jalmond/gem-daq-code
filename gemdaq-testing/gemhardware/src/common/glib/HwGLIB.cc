#include <iomanip>

#include "gem/hw/glib/HwGLIB.h"

gem::hw::glib::HwGLIB::HwGLIB():
  gem::hw::GEMHwDevice::GEMHwDevice("HwGLIB"),
  //hwGLIB_(0),
  //monGLIB_(0),
  //is_connected_(false),
  links({0,0,0}),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)
{
  //use a connection file and connection manager?
  setDeviceID("GLIBHw");
  setAddressTableFileName("glib_address_table.xml");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();
}

gem::hw::glib::HwGLIB::HwGLIB(const int& crate, const int& slot):
  gem::hw::GEMHwDevice::GEMHwDevice("HwGLIB"),
  //hwGLIB_(0),
  //monGLIB_(0),
  //is_connected_(false),
  links({0,0,0}),
  m_controlLink(-1),
  m_crate(crate),
  m_slot(slot)
{
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot));
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml" );
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  //p_gemConnectionManager = new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml");
  //p_gemHW = new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID()));
  //setAddressTableFileName("glib_address_table.xml");
  //setDeviceIPAddress(toolbox::toString("192.168.0.%d",160+slot));
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();
//  
//  ipBusErrs.badHeader_     = 0;
//  ipBusErrs.readError_     = 0;
//  ipBusErrs.timeouts_      = 0;
//  ipBusErrs.controlHubErr_ = 0;
//  
//  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
//
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

bool gem::hw::glib::HwGLIB::isHwConnected() 
{
  if ( is_connected_ ) {
    INFO("basic check: HwGLIB connection good");
    return true;
  }
  
  //return gem::hw::GEMHwDevice::isHwConnected();
  else if (gem::hw::GEMHwDevice::isHwConnected()) {
    std::vector<linkStatus> tmp_activeLinks;
    tmp_activeLinks.reserve(3);
    for (unsigned int link = 0; link < 3; ++link) {
      if (this->getUserFirmware(link)) {
	links[link] = true;
	INFO("link" << link << " present(" << this->getUserFirmware(link) << ")");
	tmp_activeLinks.push_back(std::make_pair(link,this->LinkStatus(link)));
      } else {
	links[link] = false;
	INFO("link" << link << " not reachable");
      }
    }
    activeLinks = tmp_activeLinks;
    if (!activeLinks.empty()) {
      is_connected_ = true;
      m_controlLink = (activeLinks.begin())->first;
      INFO("connected - control link" << (int)m_controlLink);
      INFO("checked links: HwGLIB connection good");
      return true;
    } else {
      is_connected_ = false;
      INFO("not connected - control link" << (int)m_controlLink);
      return false;
    }
  } else if (m_controlLink < 0)
    return false;
  else
    return false;
}

std::string gem::hw::glib::HwGLIB::getBoardID()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.BOARD_ID");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getSystemID()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.SYSTEM_ID");
  res = uint32ToString(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getIPAddress()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.IP_INFO");
  res = uint32ToDottedQuad(val);
  return res;
}

std::string gem::hw::glib::HwGLIB::getMACAddress()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val1 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.UPPER");
  uint32_t val2 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.LOWER");
  res = uint32ToGroupedHex(val1,val2);
  return res;
}

std::string gem::hw::glib::HwGLIB::getFirmwareDate()
{
  // This returns the firmware build date. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  /*
    uint32_t yy = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.YY");
    uint32_t mm = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MM");
    uint32_t dd = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.DD");
    res << "20" << std::setfill('0') << std::setw(2) << yy
    << "-"      << std::setw(2) << mm
    << "-"      << std::setw(2) << dd;
  */
  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
  res << "20" << std::setfill('0') << std::setw(2) << (fwid&0x1f)
      << "-"  << std::setw(2) << ((fwid>>5)&0x0f)
      << "-"  << std::setw(2) << ((fwid>>9)&0x7f);
  return res.str();
}

std::string gem::hw::glib::HwGLIB::getFirmwareVer()
{
  // This returns the firmware version number. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;
  /*
    uint32_t versionMajor = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MAJOR");
    uint32_t versionMinor = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.MINOR");
    uint32_t versionBuild = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.BUILD");
    res << versionMajor << "." << versionMinor << "." << versionBuild;
  */

  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
  res << ((fwid>>28)&0x0f) << "." 
      << ((fwid>>24)&0x0f) << "."
      << ((fwid>>16)&0xff);
  return res.str();
}

void gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t const& input, uint8_t const& output)
{
  if (xpoint2 && (input > 2 || output > 0)) {
    std::string msg = toolbox::toString("Invalid clock routing for XPoint2 %d -> %d",input,output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPoint2Routing,msg);
    return;
  }
  
  if ((input > 3 || output > 3)) {
    std::string msg = toolbox::toString( "Invalid clock routing for XPoint%d %d -> %d",xpoint2,input,output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPointRouting,msg);
    return;
  }
  
  std::stringstream regName;
  if (xpoint2)
    regName << "SYSTEM.CLK_CTRL.XPOINT2";
  else
    regName << "SYSTEM.CLK_CTRL.XPOINT1";
  
  switch(output) {
  case (0):
    regName << ".S1";
  case (1):
    regName << ".S2";
  case (2):
    regName << ".S3";
  case (3):
    regName << ".S4";
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

uint8_t gem::hw::glib::HwGLIB::XPointControl(bool xpoint2, uint8_t const& output)
{
  /*
    if (xpoint2 && output > 0) {
    std::string msg = toolbox::toString("Invalid clock output for XPoint2 %d",output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPoint2Routing,msg);
    return output;
    }
  */
  
  if (output > 3) {
    std::string msg = toolbox::toString( "Invalid clock output for XPoint%d %d",xpoint2,output);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidXPointRouting,msg);
    return output;
  }
  
  std::stringstream regName;
  if (xpoint2)
    regName << "SYSTEM.CLK_CTRL.XPOINT2";
  else
    regName << "SYSTEM.CLK_CTRL.XPOINT1";
  
  switch(output) {
  case (0):
    regName << ".S1";
  case (1):
    regName << ".S2";
  case (2):
    regName << ".S3";
  case (3):
    regName << ".S4";
  }
  uint8_t input = 0x0;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"0")&0x1)<<1;
  //input = input << 1;
  input |= (readReg(getDeviceBaseNode(),regName.str()+"1")&0x1);
  return input;
}

uint8_t gem::hw::glib::HwGLIB::SFPStatus(uint8_t const& sfpcage)
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.SFP" << (int)sfpcage << ".STATUS";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FMCPresence(bool fmc2)
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FMC" << (int)fmc2 << "_PRESENT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::GbEInterrupt()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.GBE_INT";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FPGAResetStatus()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FPGA_RESET";
  return (bool)readReg(getDeviceBaseNode(),regName.str());
}

uint8_t gem::hw::glib::HwGLIB::V6CPLDStatus()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.V6_CPLD";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::CDCELockStatus()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.CDCE_LOCK";
  return static_cast<bool>(readReg(getDeviceBaseNode(),regName.str()));
}

/** User core functionality **/
uint32_t gem::hw::glib::HwGLIB::getUserFirmware()
{
  // This returns the user firmware build date. 
  return getUserFirmware(m_controlLink);
}

uint32_t gem::hw::glib::HwGLIB::getUserFirmware(uint8_t const& link)
{
  // This returns the user firmware build date. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)link << ".USER_FW";
  uint32_t userfw = readReg(getDeviceBaseNode(),regName.str());
  INFO("GLIB link" << (int)link << " has firmware version 0x" 
       << std::hex << userfw << std::dec);
  return userfw;
}

std::string gem::hw::glib::HwGLIB::getUserFirmwareDate()
{
  // This returns the user firmware build date. 
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware(m_controlLink) << std::dec;
  return res.str();
}

std::string gem::hw::glib::HwGLIB::getUserFirmwareDate(uint8_t const& link)
{
  // This returns the user firmware build date. 
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware(link) << std::dec;
  return res.str();
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::glib::HwGLIB::LinkStatus(uint8_t const& link) {
  
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
  } else {
    std::stringstream regName;
    regName << "GLIB_LINKS.LINK" << (int)link << ".OPTICAL_LINKS.Counter";
    linkStatus.Errors           = readReg(getDeviceBaseNode(),regName.str()+".LinkErr"       );
    linkStatus.I2CReceived      = readReg(getDeviceBaseNode(),regName.str()+".RecI2CRequests");
    linkStatus.I2CSent          = readReg(getDeviceBaseNode(),regName.str()+".SntI2CRequests");
    linkStatus.RegisterReceived = readReg(getDeviceBaseNode(),regName.str()+".RecRegRequests");
    linkStatus.RegisterSent     = readReg(getDeviceBaseNode(),regName.str()+".SntRegRequests");
  }
  return linkStatus;
}

void gem::hw::glib::HwGLIB::LinkReset(uint8_t const& link, uint8_t const& resets) {
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return;
  }
  
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)link << ".OPTICAL_LINKS.Resets";
  if (resets&0x01)
    writeReg(getDeviceBaseNode(),regName.str()+".LinkErr",0x1);
  if (resets&0x02)
    writeReg(getDeviceBaseNode(),regName.str()+".RecI2CRequests",0x1);
  if (resets&0x04)
    writeReg(getDeviceBaseNode(),regName.str()+".SntI2CRequests",0x1);
  if (resets&0x08)
    writeReg(getDeviceBaseNode(),regName.str()+".RecRegRequests",0x1);
  if (resets&0x10)
    writeReg(getDeviceBaseNode(),regName.str()+".SntRegRequests",0x1);
}

uint32_t gem::hw::glib::HwGLIB::readTriggerFIFO(uint8_t const& link) {
  std::stringstream regName;
  regName << "GLIB_LINKS.TRG_DATA";
  uint32_t trgword = readReg(getDeviceBaseNode(),regName.str()+".DATA");
  return trgword;
}

void gem::hw::glib::HwGLIB::flushTriggerFIFO(uint8_t const& link) {
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)link << ".TRIGGER";
  writeReg(getDeviceBaseNode(),regName.str()+".FIFO_FLUSH",0x1);
}

uint32_t gem::hw::glib::HwGLIB::getFIFOOccupancy(uint8_t const& link) {
  uint32_t fifocc = 0;
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return fifocc;
  }
  
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)link << ".TRK_FIFO";
  fifocc = readReg(getDeviceBaseNode(),regName.str()+".DEPTH");
  INFO(toolbox::toString("getFIFOOccupancy(%d) %s.%s%s:: %d",
			 link,
			 getDeviceBaseNode().c_str(),
			 regName.str().c_str(),
			 ".DEPTH",
			 fifocc));
  return fifocc;
}

bool gem::hw::glib::HwGLIB::hasTrackingData(uint8_t const& link) {
  if (link > 2) {
    std::string msg = toolbox::toString("Tracking data requested for column (%d): outside expectation (0-2)",link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return false;
  }
  
  std::stringstream regName;
  regName << "TRK_DATA.COL" << (int)link << ".DATA_RDY";
  return readReg(getDeviceBaseNode(),regName.str());
}

std::vector<uint32_t> gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& link) {
  if (link > 2) {
    std::string msg = toolbox::toString("Tracking data requested for column (%d): outside expectation (0-2)",link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    std::vector<uint32_t> data(7,0x0);
    return data;
  }
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.COL" << (int)link << ".DATA.";
  register_pair_list trackingData;
  for (int i = 0; i < 7; ++i) {
    std::stringstream trkWord;
    trkWord << regName.str() << i;
    trackingData.push_back(std::make_pair(trkWord.str(),0x0));
  }
  readRegs(trackingData);
  std::vector<uint32_t> data;
  for (auto word = trackingData.begin(); word != trackingData.end(); ++word)
    data.push_back(word->second);
  return data;
}


void gem::hw::glib::HwGLIB::flushFIFO(uint8_t const& link) {
  if (link > 2) {
    std::string msg = toolbox::toString("Link status requested for link (%d): outside expectation (0-2)",link);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return;
  }

  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)m_controlLink << ".TRK_FIFO";
  writeReg(getDeviceBaseNode(),regName.str()+".FLUSH",0x1);
}
