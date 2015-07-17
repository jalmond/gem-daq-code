//General structure taken blatantly from tcds::utils::HwDeviceTCA as we're using the same card

#include "gem/hw/GEMHwDevice.h"

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& deviceName):
  //gemLogger_(gemLogger),
  //gemLogger_(log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(deviceName))),
  //p_gemConnectionManager(0),
  //p_gemHW(0),
  gemLogger_(log4cplus::Logger::getInstance(deviceName)),
  hwLock_(toolbox::BSem::FULL, true),
  is_connected_(false)
  //monGEMHw_(0)
{
  //need to grab these parameters from the xml file or from some configuration space/file/db
  //gemLogger_ = log4cplus::Logger::getInstance(deviceName);
  setAddressTableFileName("allregsnonfram.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceBaseNode("");
  setDeviceIPAddress("192.168.0.115");
  setDeviceID("GEMHwDevice");
  
  ipBusErrs_.BadHeader     = 0;
  ipBusErrs_.ReadError     = 0;
  ipBusErrs_.Timeout       = 0;
  ipBusErrs_.ControlHubErr = 0;
    
  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
  //gem::hw::GEMHwDevice::initDevice();
  /** 
   * what's the difference between connect, init, enable for GLIB, VFAT, other devices?
   * are all options necessary?
   * steps from nothing to running:
   * initDevice:
   * check that register values are hardware default values, if not, something may be amiss
   * enableDevice:
   * set register values to sw default values -> hardware is enabled!
   * configureDevice:
   * set register values to desired values -> hardware is configured!
   * startDevice:
   * set run bit -> hardware is running
   
   * in this model, a device can be running while the C++ object no longer exists
   * is this a good thing?  one can always at a later time connect again and turn the device off
   * however, if we define the sequences as init->enable->configure->start
   * then it will be non-trivial to connect to a running chip and set enable to off without
   * repeating the steps...
   **/
}

gem::hw::GEMHwDevice::GEMHwDevice(std::string const& connectionFile,
				  std::string const& cardName):
  //p_gemConnectionManager(0),
  //p_gemHW(0),
  hwLock_(toolbox::BSem::FULL, true),
  is_connected_(false)
  //monGEMHw_(0)
{
  gemLogger_ = log4cplus::Logger::getInstance(cardName);
  //set up device creation via connection manager
  uhal::ConnectionManager ConnectXML("file://"+connectionFile);

  //need to grab these parameters from the xml file or from some configuration space/file/db
  setAddressTableFileName("allregsnonfram.xml");
  setIPbusProtocolVersion("2.0");
  setDeviceBaseNode("");
  setDeviceIPAddress("192.168.0.115");
  setDeviceID("GEMHwDevice");
  
  ipBusErrs_.BadHeader     = 0;
  ipBusErrs_.ReadError     = 0;
  ipBusErrs_.Timeout       = 0;
  ipBusErrs_.ControlHubErr = 0;
  
  setLogLevelTo(uhal::Error());  // Minimise uHAL logging
}

gem::hw::GEMHwDevice::~GEMHwDevice()
{
  //if (p_gemHW)
  //  releaseDevice();
  //if (p_gemConnectionManager)
  //  delete p_gemConnectionManager;
  //p_gemConnectionManager = 0;
}

std::string gem::hw::GEMHwDevice::printErrorCounts() const {
  std::stringstream errstream;
  errstream << "errors while accessing registers:"              << std::endl 
	    << "Bad header:  "       <<ipBusErrs_.BadHeader     << std::endl
	    << "Read errors: "       <<ipBusErrs_.ReadError     << std::endl
	    << "Timeouts:    "       <<ipBusErrs_.Timeout       << std::endl
	    << "Controlhub errors: " <<ipBusErrs_.ControlHubErr << std::endl;
  INFO(errstream);
  return errstream.str();
}

// void gem::hw::GEMHwDevice::connectDevice(std::string const& devicename, uhal::HwInterface& hw_)
// {
// }

void gem::hw::GEMHwDevice::connectDevice()
{
  //std::string const addressTable      = "allregsnonfram.xml";    //cfgInfoSpaceP_->getString("addressTable");
  std::string const controlhubAddress = "localhost";    //cfgInfoSpaceP_->getString("controlhubAddress");
  std::string const deviceAddress     = deviceIPAddr_;  //cfgInfoSpaceP_->getString("deviceAddress");
  uint32_t    const controlhubPort    = 10203;          //cfgInfoSpaceP_->getUInt32("controlhubPort");
  uint32_t    const ipbusPort         = 50001;          //cfgInfoSpaceP_->getUInt32("ipbusPort");
  
  std::stringstream tmpUri;
  if (controlhubAddress.size() > 0) {
    DEBUG("Using control hub at address '" << controlhubAddress
	  << ", port number " << controlhubPort << "'.");
    tmpUri << "chtcp-"<< getIPbusProtocolVersion() << "://" << controlhubAddress << ":" << controlhubPort
	   << "?target=" << deviceAddress << ":" << ipbusPort;
  } else {
    DEBUG("No control hub address specified -> "
	  "continuing with a direct connection.");
      tmpUri << "ipbusudp-" << getIPbusProtocolVersion() << "://"
             << deviceAddress << ":" << ipbusPort;
  }
  std::string const uri = tmpUri.str();
  std::string const id  = getDeviceID();
  std::string const addressTable = getAddressTableFileName();

  INFO("uri, id, address table : " << uri << " " << id << " " << addressTable);
  
  //int retryCount = 0;
  
  std::shared_ptr<uhal::HwInterface> tmpHWP;
  
  try {
    tmpHWP.reset(new uhal::HwInterface(uhal::ConnectionManager::getDevice(id, uri, addressTable)));
  } catch (uhal::exception::FileNotFound const& err) {
    std::string msg = toolbox::toString("Could not find uhal address table file '%s' "
					"(or one of its included address table modules).",
					addressTable.c_str());
    ERROR(msg);
  } catch (uhal::exception::exception const& err) {
    std::string msgBase = "Could not obtain the uhal device from the connection manager";
    std::string msg = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
    ERROR(msg);
  } catch (std::exception const& err) {
    ERROR("Unknown std::exception caught from uhal");
    std::string msgBase = "Could not connect to th e hardware";
    std::string msg = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
    ERROR(msg);
  }
  
  p_gemHW.swap(tmpHWP);
  if (isHwConnected())
    INFO("connectDevice::HwDevice pointer active");
  else
    INFO("connectDevice::Unable to establish connection with the hardware.");
  //maybe raise exception here?
}

void gem::hw::GEMHwDevice::configureDevice()
{
  
}

void gem::hw::GEMHwDevice::releaseDevice()
{
  //if (p_gemHW != 0) {
  //  delete p_gemHW;
  //  p_gemHW = 0;
  //}
}

void gem::hw::GEMHwDevice::enableDevice()
{
  if (!isHwConnected()) {
    std::string msg = "Could not enable the hardware. " \
      "(No hardware is connected.)";
    ERROR(msg);
  }
}

//void gem::hw::GEMHwDevice::disableDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::pauseDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::startDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::stopDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::resumeDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::haltDevice()
//{
//
//}
//
//void gem::hw::GEMHwDevice::initDevice() 
//{
//
//}

uhal::HwInterface& gem::hw::GEMHwDevice::getGEMHwInterface() const
{
  if (p_gemHW == NULL) {
    std::string msg = "Trying to access hardware before connecting.";
    ERROR(msg);
    //XCEPT_RAISE(gem::hw::exception::UninitializedDevice, msg);
  } else {
    uhal::HwInterface& hw = static_cast<uhal::HwInterface&>(*p_gemHW);
    return hw;
  }
}

uint32_t gem::hw::GEMHwDevice::readReg(std::string const& name)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();

  int retryCount = 0;
  uint32_t res;
  DEBUG("gem::hw::GEMHwDevice::readReg " << name << std::endl);
  while (retryCount < MAX_IPBUS_RETRIES) {
    try {
      uhal::ValWord<uint32_t> val = hw.getNode(name).read();
      hw.dispatch();
      res = val.value();
      return res;
      //break;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
	++retryCount;
	if (retryCount > 4)
	  DEBUG("Failed to read register " << name <<
	       ", retrying. retryCount("<<retryCount<<")"
	       << std::endl);
	updateErrorCounters(errCode);
	continue;
      } else {
	ERROR(msg);
	//XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read register '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      ERROR(msg);
      //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, ubable to read register %s",name.c_str());
  ERROR(msg);
  //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
  return res;
}

void gem::hw::GEMHwDevice::readRegs(register_pair_list &regList)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();

  int retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    try {
      std::vector<std::pair<std::string,uhal::ValWord<uint32_t> > > vals;
      //vals.reserve(regList.size());
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	vals.push_back(std::make_pair(curReg->first,hw.getNode(curReg->first).read()));
      hw.dispatch();

      //would like to have these local to the loop, how to do...?
      auto curVal = vals.begin();
      auto curReg = regList.begin();
      for ( ; curReg != regList.end(); ++curVal,++curReg) 
	curReg->second = (curVal->second).value();
      return;
      //break;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
	++retryCount;
	/* would need to loop the debug message as curReg is out of scope here
	if (retryCount > 4)
	  for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	    DEBUG("Failed to read register " << curReg->first <<
	          ", retrying. retryCount("<<retryCount<<")"
	          << std::endl);
	*/
	updateErrorCounters(errCode);
	continue;
      } else {
	ERROR(msg);
	//XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = "Could not read from register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      ERROR(msg);
      //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeReg(std::string const& name, uint32_t const val)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  int retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    try {
      hw.getNode(name).write(val);
      hw.dispatch();
      return;
      //break;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
	++retryCount;
	if (retryCount > 4)
	  DEBUG("Failed to write value 0x" << std::hex<< val << std::dec << " to register " << name <<
	       ", retrying. retryCount("<<retryCount<<")"
	       << std::endl);
	updateErrorCounters(errCode);
	continue;
      } else {
	ERROR(msg);
	//XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to register '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      ERROR(msg);
      //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeRegs(register_pair_list const& regList)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  int retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    try {
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	hw.getNode(curReg->first).write(curReg->second);
      hw.dispatch();
      return;
      //break;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = "Could not write to register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg     = toolbox::toString("%s (uHAL): %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
	++retryCount;
	/* would need to loop the debug message as curReg is out of scope here
	if (retryCount > 4)
	  for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	    DEBUG("Failed to write value 0x" << std::hex <<
		  curReg->second << std::dec <<
		  " to register " << curReg->first <<
		  ", retrying. retryCount("<<retryCount<<")"
		  << std::endl);
	*/
	updateErrorCounters(errCode);
	continue;
      } else {
	ERROR(msg);
	//XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = "Could not write to register in list:";
      for (auto curReg = regList.begin(); curReg != regList.end(); ++curReg) 
	msgBase += toolbox::toString(" '%s'", curReg->first.c_str());
      std::string msg = toolbox::toString("%s (std): %s.", msgBase.c_str(), err.what());
      ERROR(msg);
      //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}

void gem::hw::GEMHwDevice::writeValueToRegs(std::vector<std::string> const& regNames, uint32_t const& regValue)
{
  register_pair_list regsToWrite;
  for (auto curReg = regNames.begin(); curReg != regNames.end(); ++curReg)
    regsToWrite.push_back(std::make_pair(*curReg,regValue));
  writeRegs(regsToWrite);
}

/*
void gem::hw::GEMHwDevice::zeroReg(std::string const& name)
{
  writeReg(name,0);
}
*/

void gem::hw::GEMHwDevice::zeroRegs(std::vector<std::string> const& regNames)
{
  register_pair_list regsToZero;
  for (auto curReg = regNames.begin(); curReg != regNames.end(); ++curReg)
    regsToZero.push_back(std::make_pair(*curReg,0x0));
  writeRegs(regsToZero);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  size_t numWords       = hw.getNode(name).getSize();
  DEBUG("reading block " << name << " which has size "<<numWords);
  return readBlock(name, numWords);
}

std::vector<uint32_t> gem::hw::GEMHwDevice::readBlock(std::string const& name, size_t const& numWords)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();

  std::vector<uint32_t> res(numWords);

  int retryCount = 0;
  if (numWords < 1) 
    return res;
  
  while (retryCount < MAX_IPBUS_RETRIES) {
    try {
      uhal::ValVector<uint32_t> values = hw.getNode(name).readBlock(numWords);
      hw.dispatch();
      std::copy(values.begin(), values.end(), res.begin());
      return res;
      //break;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read block '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
	++retryCount;
	if (retryCount > 4)
	  DEBUG("Failed to read block " << name << " with " << numWords << " words" <<
	       ", retrying. retryCount("<<retryCount<<")" << std::endl
	       << "error was " << errCode
	       << std::endl);
	updateErrorCounters(errCode);
	continue;
      } else {
	ERROR(msg);
	//XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not read block '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      ERROR(msg);
      //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
  std::string msg = toolbox::toString("Maximum number of retries reached, ubable to read block");
  ERROR(msg);
  //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
  return res;
}

void gem::hw::GEMHwDevice::writeBlock(std::string const& name, std::vector<uint32_t> const values)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  if (values.size() < 1) 
    return;
  
  uhal::HwInterface& hw = getGEMHwInterface();
  int retryCount = 0;
  while (retryCount < MAX_IPBUS_RETRIES) {
    try {
      hw.getNode(name).writeBlock(values);
      hw.dispatch();
      return;
      //break;
    } catch (uhal::exception::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to block '%s' (uHAL)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      std::string errCode = toolbox::toString("%s",err.what());
      if (knownErrorCode(errCode)) {
	++retryCount;
	if (retryCount > 4)
	  DEBUG("Failed to write block " << name <<
	       ", retrying. retryCount("<<retryCount<<")"
	       << std::endl);
	updateErrorCounters(errCode);
	continue;
      } else {
	ERROR(msg);
	//XCEPT_RAISE(gem::hw::exception::HardwareProblem, toolbox::toString("%s.", msgBase.c_str()));
      }
    } catch (std::exception const& err) {
      std::string msgBase = toolbox::toString("Could not write to block '%s' (std)", name.c_str());
      std::string msg     = toolbox::toString("%s: %s.", msgBase.c_str(), err.what());
      ERROR(msg);
      //XCEPT_RAISE(gem::hw::exception::HardwareProblem, msg);
    }
  }
}


bool gem::hw::GEMHwDevice::knownErrorCode(std::string const& errCode) const {
  return ((errCode.find("amount of data")              != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x4L")            != std::string::npos) ||
	  (errCode.find("INFO CODE = 0x6L")            != std::string::npos) ||
	  (errCode.find("timed out")                   != std::string::npos) ||
	  (errCode.find("had response field = 0x04")   != std::string::npos) ||
	  (errCode.find("had response field = 0x06")   != std::string::npos) ||
	  (errCode.find("ControlHub error code is: 4") != std::string::npos));
}


void gem::hw::GEMHwDevice::updateErrorCounters(std::string const& errCode) {
  if (errCode.find("amount of data")    != std::string::npos)
    ++ipBusErrs_.BadHeader;
  if (errCode.find("INFO CODE = 0x4L")  != std::string::npos)
    ++ipBusErrs_.ReadError;
  if ((errCode.find("INFO CODE = 0x6L") != std::string::npos) ||
      (errCode.find("timed out")        != std::string::npos))
    ++ipBusErrs_.Timeout;
  if (errCode.find("ControlHub error code is: 4") != std::string::npos)
    ++ipBusErrs_.ControlHubErr;
  if ((errCode.find("had response field = 0x04") != std::string::npos) ||
      (errCode.find("had response field = 0x06") != std::string::npos))
    ++ipBusErrs_.ControlHubErr;
}

void gem::hw::GEMHwDevice::zeroBlock(std::string const& name)
{
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(hwLock_);
  uhal::HwInterface& hw = getGEMHwInterface();
  size_t numWords = hw.getNode(name).getSize();
  std::vector<uint32_t> zeros(numWords, 0);
  return writeBlock(name, zeros);
}
