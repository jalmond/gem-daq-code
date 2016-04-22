#include <iomanip>

#include "gem/hw/glib/HwGLIB.h"

gem::hw::glib::HwGLIB::HwGLIB() :
  gem::hw::GEMHwDevice::GEMHwDevice("HwGLIB"),
  //monGLIB_(0),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)
{
  INFO("HwGLIB ctor");
  //use a connection file and connection manager?
  setDeviceID("GLIBHw");
  setAddressTableFileName("glib_address_table.xml");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();

  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              std::string const& connectionFile) :
  gem::hw::GEMHwDevice::GEMHwDevice(glibDevice, connectionFile),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)
{
  setDeviceBaseNode("GLIB");
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              std::string const& connectionURI,
                              std::string const& addressTable) :
  gem::hw::GEMHwDevice::GEMHwDevice(glibDevice, connectionURI, addressTable),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)

{
  INFO("trying to create HwGLIB(" << glibDevice << "," << connectionURI << "," <<addressTable);
  setDeviceBaseNode("GLIB");
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(std::string const& glibDevice,
                              uhal::HwInterface& uhalDevice) :
  gem::hw::GEMHwDevice::GEMHwDevice(glibDevice,uhalDevice),
  m_controlLink(-1),
  m_crate(-1),
  m_slot(-1)

{
  setDeviceBaseNode("GLIB");
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::HwGLIB(const int& crate, const int& slot) :
  gem::hw::GEMHwDevice::GEMHwDevice(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot)),
  //monGLIB_(0),
  m_controlLink(-1),
  m_crate(crate),
  m_slot(slot)
{
  INFO("HwGLIB ctor");
  //use a connection file and connection manager?
  setDeviceID(toolbox::toString("gem.shelf%02d.glib%02d",crate,slot));
  
  //uhal::ConnectionManager manager ( "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml" );
  INFO("getting the ConnectionManager pointer");
  p_gemConnectionManager.reset(new uhal::ConnectionManager("file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"));
  //p_gemConnectionManager.reset(new uhal::ConnectionManager("file://../data/connections_ch.xml"));
  INFO("getting HwInterface " << getDeviceID() << " pointer from ConnectionManager");
  p_gemHW.reset(new uhal::HwInterface(p_gemConnectionManager->getDevice(this->getDeviceID())));
  INFO("setting the device base node");
  setDeviceBaseNode("GLIB");
  //gem::hw::glib::HwGLIB::initDevice();
  //  
  //  ipBusErrs.badHeader_     = 0;
  //  ipBusErrs.readError_     = 0;
  //  ipBusErrs.timeouts_      = 0;
  //  ipBusErrs.controlHubErr_ = 0;
  //  
  //setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  
  for (unsigned li = 0; li < N_GTX; ++li) {
    b_links[li] = false;
    GLIBIPBusCounters tmpGTXCounter;
    m_ipBusCounters.push_back(tmpGTXCounter);
  }
  
  INFO("HwGLIB ctor done " << isHwConnected());
}

gem::hw::glib::HwGLIB::~HwGLIB()
{
  //releaseDevice();
}

//void gem::hw::glib::HwGLIB::configureDevice(std::string const& xmlSettings)
//{
//  //here load the xml file settings onto the board
//}
//
//void gem::hw::glib::HwGLIB::configureDevice()
//{
//  //determine the manner in which to configure the device (XML or DB parameters)
//}
//
//void gem::hw::glib::HwGLIB::connectDevice()
//{
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
  if ( b_is_connected ) {
    INFO("basic check: HwGLIB connection good");
    return true;
  } else if (gem::hw::GEMHwDevice::isHwConnected()) {
    std::vector<linkStatus> tmp_activeLinks;
    tmp_activeLinks.reserve(N_GTX);
    for (unsigned int gtx = 0; gtx < N_GTX; ++gtx) {
      //need to make sure that this works only for "valid" FW results
      // for the moment we can do a check to see that 2015/2016 appears in the string
      // this no longer will work as desired, how to get whether the GTX is active?
      // need to rethink this for future firmware versions and backwards compatibility
      // i.e., no longer check GLIB connection per GTX (maybe reserve some link check in the future)
      if ((this->getFirmwareVer()).rfind("5.")   != std::string::npos || 
          (this->getFirmwareVer()).rfind(".201") != std::string::npos || 
          (this->getBoardID()).rfind("GLIB")     != std::string::npos ) {
        b_links[gtx] = true;
        INFO("gtx" << gtx << " present(" << this->getFirmwareVer() << ")");
        tmp_activeLinks.push_back(std::make_pair(gtx,this->LinkStatus(gtx)));
      } else {
        b_links[gtx] = false;
        INFO("gtx" << gtx << " not reachable (unable to find 5 or 201 in the firmware string, " 
             << "or 'GLIB' in the board ID)"
             << " board ID "              << this->getBoardID()
             << " user firmware version " << this->getFirmwareVer());
      }
    }
    v_activeLinks = tmp_activeLinks;
    if (!v_activeLinks.empty()) {
      b_is_connected = true;
      m_controlLink = (v_activeLinks.begin())->first;
      INFO("connected - control gtx" << (int)m_controlLink);
      INFO("checked gtxs: HwGLIB connection good");
      return true;
    } else {
      b_is_connected = false;
      INFO("not connected - control gtx" << (int)m_controlLink);
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
  res = gem::utils::uint32ToString(val);
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getBoardIDRaw()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The board ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.BOARD_ID");
  return val;
}

std::string gem::hw::glib::HwGLIB::getSystemID()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  std::string res = "???";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.SYSTEM_ID");
  res = gem::utils::uint32ToString(val);
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getSystemIDRaw()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  // The system ID consists of four characters encoded as a 32-bit unsigned int
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.SYSTEM_ID");
  return val;
}

std::string gem::hw::glib::HwGLIB::getIPAddress()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.IP_INFO");
  res = gem::utils::uint32ToDottedQuad(val);
  return res;
}

uint32_t gem::hw::glib::HwGLIB::getIPAddressRaw()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uint32_t val = readReg(getDeviceBaseNode(),"SYSTEM.IP_INFO");
  return val;
}

std::string gem::hw::glib::HwGLIB::getMACAddress()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::string res = "N/A";
  uint32_t val1 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.UPPER");
  uint32_t val2 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.LOWER");
  res = gem::utils::uint32ToGroupedHex(val1,val2);
  return res;
}

uint64_t gem::hw::glib::HwGLIB::getMACAddressRaw()
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uint32_t val1 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.UPPER");
  uint32_t val2 = readReg(getDeviceBaseNode(),"SYSTEM.MAC.LOWER");
  return ((uint64_t)val1 << 32) + val2;
}

std::string gem::hw::glib::HwGLIB::getFirmwareDate()
{
  // This returns the firmware build date. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream res;
  std::stringstream regName;

  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.DATE");
  res <<         std::setfill('0') <<std::setw(2) << (fwid&0x1f)      // day
      << "-"  << std::setfill('0') <<std::setw(2) << ((fwid>>5)&0x0f) // month
      << "-"  << std::setw(4) << 2000+((fwid>>9)&0x7f);               // year
  return res.str();
}

uint32_t gem::hw::glib::HwGLIB::getFirmwareDateRaw()
{
  // This returns the firmware build date. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.DATE");
  return fwid;
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

  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.ID");
  res << ((fwid>>12)&0x0f) << "." 
      << ((fwid>>8) &0x0f) << "."
      << ((fwid)    &0xff);
  return res.str();
}

uint32_t gem::hw::glib::HwGLIB::getFirmwareVerRaw()
{
  // This returns the firmware version number. 
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uint32_t fwid = readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE.ID");
  return fwid;
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
  case (0) :
    regName << ".S1";
  case (1) :
    regName << ".S2";
  case (2) :
    regName << ".S3";
  case (3) :
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
  case (0) :
    regName << ".S1";
  case (1) :
    regName << ".S2";
  case (2) :
    regName << ".S3";
  case (3) :
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
  if (sfpcage < 1 || sfpcage > 4) {
    std::string msg = toolbox::toString("Status requested for SFP (%d): outside expectation (1,4)", sfpcage);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return 0;
  }
  std::stringstream regName;
  regName << "SYSTEM.STATUS.SFP" << (int)sfpcage << ".STATUS";
  return (uint8_t)readReg(getDeviceBaseNode(),regName.str());
}

bool gem::hw::glib::HwGLIB::FMCPresence(bool fmc2)
{
  //gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  std::stringstream regName;
  regName << "SYSTEM.STATUS.FMC" << (int)fmc2+1 << "_PRESENT";
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
  // This returns the firmware register (V2 removed the user firmware specific). 
  return readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
}

uint32_t gem::hw::glib::HwGLIB::getUserFirmware(uint8_t const& gtx)
{
  // This returns the firmware register (V2 removed the user firmware specific). 
  return readReg(getDeviceBaseNode(),"SYSTEM.FIRMWARE");
}

std::string gem::hw::glib::HwGLIB::getUserFirmwareDate()
{
  // This returns the user firmware build date. 
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware(m_controlLink) << std::dec;
  return res.str();
}

std::string gem::hw::glib::HwGLIB::getUserFirmwareDate(uint8_t const& gtx)
{
  // This returns the user firmware build date. 
  std::stringstream res;
  res << "0x"<< std::hex << getUserFirmware(gtx) << std::dec;
  return res.str();
}

bool gem::hw::glib::HwGLIB::linkCheck(uint8_t const& gtx, std::string const& opMsg)
{
  if (gtx > N_GTX) {
    std::string msg = toolbox::toString("%s requested for gtx (%d): outside expectation (0-%d)",
                                        opMsg.c_str(), gtx, N_GTX);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return false;
  } else if (!b_links[gtx]) {
    std::string msg = toolbox::toString("%s requested inactive gtx (%d)",opMsg.c_str(), gtx);
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::glib::exception::InvalidLink,msg);
    return false;
  }
  return true;
}

gem::hw::GEMHwDevice::OpticalLinkStatus gem::hw::glib::HwGLIB::LinkStatus(uint8_t const& gtx)
{
  gem::hw::GEMHwDevice::OpticalLinkStatus linkStatus;

  if (linkCheck(gtx, "Link status")) {
    linkStatus.TRK_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRK_ERR",gtx));
    linkStatus.TRG_Errors   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRG_ERR",gtx));
    linkStatus.Data_Packets = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.DATA_Packets",gtx));
  }
  return linkStatus;
}

void gem::hw::glib::HwGLIB::LinkReset(uint8_t const& gtx, uint8_t const& resets)
{

  //right now this just resets the counters, but we need to be able to "reset" the link too
  if (linkCheck(gtx, "Link reset")) {
    if (resets&0x1)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRK_ERR.Reset",gtx),0x1);
    if (resets&0x2)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.TRG_ERR.Reset",gtx),0x1);
    if (resets&0x4)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.GTX%d.DATA_Packets.Reset",gtx),0x1);
  }
}


gem::hw::glib::HwGLIB::GLIBIPBusCounters gem::hw::glib::HwGLIB::getIPBusCounters(uint8_t const& gtx,
                                                                                 uint8_t const& mode)
{
  
  if (linkCheck(gtx, "IPBus counter")) {
    if (mode&0x01)
      m_ipBusCounters.at(gtx).OptoHybridStrobe = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",gtx));
    if (mode&0x02)
      m_ipBusCounters.at(gtx).OptoHybridAck    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",gtx));
    if (mode&0x04)
      m_ipBusCounters.at(gtx).TrackingStrobe   = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d",gtx));
    if (mode&0x08)
      m_ipBusCounters.at(gtx).TrackingAck      = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d",gtx));
    if (mode&0x10)
      m_ipBusCounters.at(gtx).CounterStrobe    = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters"));
    if (mode&0x20)
      m_ipBusCounters.at(gtx).CounterAck       = readReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters"));
  }
  return m_ipBusCounters.at(gtx);
}

void gem::hw::glib::HwGLIB::resetIPBusCounters(uint8_t const& gtx, uint8_t const& resets)
{  
  if (linkCheck(gtx, "Reset IPBus counters")) {
    if (resets&0x01)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.OptoHybrid_%d.Reset",gtx),0x1);
    if (resets&0x02)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.OptoHybrid_%d.Reset",gtx),0x1);
    if (resets&0x04)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.TRK_%d.Reset",gtx),0x1);
    if (resets&0x08)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.TRK_%d.Reset",gtx),0x1);
    if (resets&0x10)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Strobe.Counters.Reset"),0x1);
    if (resets&0x20)
      writeReg(getDeviceBaseNode(),toolbox::toString("COUNTERS.IPBus.Ack.Counters.Reset"),0x1);
  }
}

uint32_t gem::hw::glib::HwGLIB::readTriggerFIFO(uint8_t const& gtx)
{
  /*
    std::stringstream regName;
    regName << "GLIB_LINKS.TRG_DATA";
    uint32_t trgword = readReg(getDeviceBaseNode(),regName.str()+".DATA");
  */
  return 0;
}

void gem::hw::glib::HwGLIB::flushTriggerFIFO(uint8_t const& gtx)
{
  //V2 firmware hasn't got trigger fifo yet
  return;
  /*
  std::stringstream regName;
  regName << "GLIB_LINKS.LINK" << (int)gtx << ".TRIGGER";
  writeReg(getDeviceBaseNode(),regName.str()+".FIFO_FLUSH",0x1);
  */
}

uint32_t gem::hw::glib::HwGLIB::getFIFOOccupancy(uint8_t const& gtx)
{
  uint32_t fifocc = 0;
  if (linkCheck(gtx, "FIFO occupancy")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)gtx;
    fifocc = readReg(getDeviceBaseNode(),regName.str()+".DEPTH");
    DEBUG(toolbox::toString("getFIFOOccupancy(%d) %s.%s%s:: %d", gtx, getDeviceBaseNode().c_str(),
                            regName.str().c_str(), ".DEPTH", fifocc));
  }
  //the fifo occupancy is in number of 32 bit words
  return fifocc;
}

uint32_t gem::hw::glib::HwGLIB::getFIFOVFATBlockOccupancy(uint8_t const& gtx)
{
  //what to return when the occupancy is not a full VFAT block?
  return getFIFOOccupancy(gtx)/7;
}

bool gem::hw::glib::HwGLIB::hasTrackingData(uint8_t const& gtx)
{
  bool hasData = false;
  if (linkCheck(gtx, "Tracking data")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)gtx << ".ISEMPTY";
    hasData = !readReg(getDeviceBaseNode(),regName.str());
  }
  //if the FIFO is fragmented, this will return true but we won't read a full block
  //what to do in this case?
  return hasData;
}

std::vector<uint32_t> gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& gtx, size_t const& nBlocks)
{
  if (!linkCheck(gtx, "Tracking data")) {
    //do we really want to return a huge vector of 0s?
    std::vector<uint32_t> data(7*nBlocks,0x0);
    return data;
  } 
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.OptoHybrid_" << (int)gtx << ".FIFO";
  //best way to read a real block? make getTrackingData ask for N blocks?
  //can we return the memory another way, rather than a vector?
  return readBlock(regName.str(),7*nBlocks);
}

uint32_t gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& gtx, uint32_t* data, size_t const& nBlocks)
{
  if (data==NULL) {
    std::string msg = toolbox::toString("Block read requested for null pointer");
    ERROR(msg);
    XCEPT_RAISE(gem::hw::glib::exception::NULLReadoutPointer,msg);
  } else if (!linkCheck(gtx, "Tracking data")) {
    return 0;
  }
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.OptoHybrid_" << (int)gtx << ".FIFO";
  //best way to read a real block? make getTrackingData ask for N blocks?
  //can we return the memory another way, rather than a vector?
  //readBlock(regName.str(),7*nBlocks);
  return nBlocks;
}

uint32_t gem::hw::glib::HwGLIB::getTrackingData(uint8_t const& gtx, std::vector<toolbox::mem::Reference*>& data,
                                                size_t const& nBlocks)
{
  if (!linkCheck(gtx, "Tracking data")) {
    return 0;
  } 
  
  std::stringstream regName;
  regName << getDeviceBaseNode() << ".TRK_DATA.OptoHybrid_" << (int)gtx << ".FIFO";
  //best way to read a real block? make getTrackingData ask for N blocks?
  //can we return the memory another way, rather than a vector?
  //readBlock(regName.str(),7*nBlocks);
  return nBlocks;
}

void gem::hw::glib::HwGLIB::flushFIFO(uint8_t const& gtx)
{
  if (linkCheck(gtx, "Flush FIFO")) {
    std::stringstream regName;
    regName << "TRK_DATA.OptoHybrid_" << (int)gtx;
    INFO("Tracking FIFO" << (int)gtx << ":"
         << " ISFULL 0x" << std::hex << readReg(getDeviceBaseNode(),regName.str()+".ISFULL")   << std::dec
         << " ISEMPTY 0x" << std::hex << readReg(getDeviceBaseNode(),regName.str()+".ISEMPTY") << std::dec
         << " Depth 0x"   << std::hex << getFIFOOccupancy(gtx) << std::dec);
    writeReg(getDeviceBaseNode(),regName.str()+".FLUSH",0x1);
  }
}


void gem::hw::glib::HwGLIB::enableDAQLink()
{
  writeReg(getDeviceBaseNode(),"DAQ.CONTROL.INPUT_KILL_MASK", 0x1);/*HACK to be fixed*/
  writeReg(getDeviceBaseNode(),"DAQ.CONTROL.DAQ_ENABLE", 0x1);
}

void gem::hw::glib::HwGLIB::resetDAQLink()
{
  writeReg(getDeviceBaseNode(),"DAQ.CONTROL.RESET", 0x1);
  writeReg(getDeviceBaseNode(),"DAQ.CONTROL.RESET", 0x0);
  writeReg(getDeviceBaseNode(),"DAQ.CONTROL.TTS_OVERRIDE", 0x8);/*HACK to be fixed*/
  writeReg(getDeviceBaseNode(),"DAQ.CONTROL.DAV_TIMEOUT", 0x3d090);
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkControl()
{
  return readReg(getDeviceBaseNode(),"DAQ.CONTROL");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkStatus()
{
  return readReg(getDeviceBaseNode(),"DAQ.STATUS");
}
/*
uint32_t gem::hw::glib::HwGLIB::getDAQLinkFlags()
{
  return readReg(getDeviceBaseNode(),"DAQ.FLAGS");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkCorruptCount()
{
  return readReg(getDeviceBaseNode(),"DAQ.CORRUPT_CNT");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkEventsBuilt()
{
  return readReg(getDeviceBaseNode(),"DAQ.EVT_BUILT");
}
*/
uint32_t gem::hw::glib::HwGLIB::getDAQLinkEventsSent()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_STATUS.EVT_SENT");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkL1AID()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_STATUS.L1AID");
}
/*
uint32_t gem::hw::glib::HwGLIB::getDAQLinkDebug(uint8_t const& mode)
{
  if (mode < 0 || mode > 6 ) {
    ERROR("Invalid DAQ LINK debug register (" << (int)mode << " specified. Expected 0-6");
    return 0;
  }
  std::stringstream regName;
  regName << "DAQ.DEBUG_" << (int)mode;
  return readReg(getDeviceBaseNode(),regName.str());
}
*/
uint32_t gem::hw::glib::HwGLIB::getDAQLinkDisperErrors()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_STATUS.DISPER_ERR");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkNonidentifiableErrors()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_STATUS.NOTINTABLE_ERR");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkInputMask()
{
  return readReg(getDeviceBaseNode(),"DAQ.CONTROL.INPUT_KILL_MASK");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkDAVTimeout()
{
  return readReg(getDeviceBaseNode(),"DAQ.CONTROL.DAV_TIMEOUT");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkDAVTimer(bool const& max)
{
  if (max)
    return readReg(getDeviceBaseNode(),"DAQ.EXT_STATUS.MAX_DAV_TIMER");
  else
    return readReg(getDeviceBaseNode(),"DAQ.EXT_STATUS.LAST_DAV_TIMER");
}

// GTX specific DAQ link information
uint32_t gem::hw::glib::HwGLIB::getDAQLinkStatus(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.GTX" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".STATUS");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkCounters(uint8_t const& gtx, uint8_t const& mode)
{
  std::stringstream regBase;
  regBase << "DAQ.GTX" << (int)gtx << ".COUNTERS";
  if (mode == 0)
    return readReg(getDeviceBaseNode(),regBase.str()+".CORRUPT_VFAT_BLK_CNT");
  else
    return readReg(getDeviceBaseNode(),regBase.str()+".EVN");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkLastBlock(uint8_t const& gtx)
{
  std::stringstream regBase;
  regBase << "DAQ.GTX" << (int)gtx;
  return readReg(getDeviceBaseNode(),regBase.str()+".LASTBLOCK");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkInputTimeout()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_CONTROL.INPUT_TIMEOUT");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkRunParameters()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_CONTROL.RUN_PARAMS");
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkRunParameter(uint8_t const& parameter)
{
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  return readReg(getDeviceBaseNode(),regBase.str());
}

uint32_t gem::hw::glib::HwGLIB::getDAQLinkRunType()
{
  return readReg(getDeviceBaseNode(),"DAQ.EXT_CONTROL.RUN_TYPE");
}

void gem::hw::glib::HwGLIB::setDAQLinkInputTimeout(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(),"DAQ.EXT_CONTROL.INPUT_TIMEOUT",value);
}

void gem::hw::glib::HwGLIB::setDAQLinkRunParameters(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(),"DAQ.EXT_CONTROL.RUN_PARAMS",value);
}

void gem::hw::glib::HwGLIB::setDAQLinkRunParameter(uint8_t const& parameter, uint8_t const& value)
{
  std::stringstream regBase;
  regBase << "DAQ.EXT_CONTROL.RUN_PARAM" << (int) parameter;
  return writeReg(getDeviceBaseNode(),regBase.str(),value);
}

void gem::hw::glib::HwGLIB::setDAQLinkRunType(uint32_t const& value)
{
  return writeReg(getDeviceBaseNode(),"DAQ.EXT_CONTROL.RUN_TYPE",value);
}

