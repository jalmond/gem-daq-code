/**
 * class: AMC13Manager
 * description: Manager application for AMC13 cards
 *              structure borrowed from TCDS core, with nods to HCAL (DTCManager)
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/amc13/AMC13ManagerWeb.h"
#include "gem/hw/amc13/AMC13Manager.h"

#include "gem/hw/amc13/exception/Exception.h"

#include "amc13/AMC13.hh"
#include "amc13/Status.hh"

XDAQ_INSTANTIATOR_IMPL(gem::hw::amc13::AMC13Manager);

gem::hw::amc13::AMC13Manager::AMC13Manager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub),
  deviceLock_(toolbox::BSem::FULL, true),
  amc13Device_(0)
{
  m_crateID = -1;
  m_slot = 13;
  
  getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);

  //initialize the AMC13Manager application objects
  LOG4CPLUS_DEBUG(getApplicationLogger(), "connecting to the AMC13ManagerWeb interface");
  gemWebInterfaceP_ = new gem::hw::amc13::AMC13ManagerWeb(this);
  //gemMonitorP_      = new gem::hw::amc13::AMC13HwMonitor(this);
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");

  LOG4CPLUS_DEBUG(getApplicationLogger(), "executing preInit for AMC13Manager");
  preInit();
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/html/images/amc13/AMC13Manager.png");
}

gem::hw::amc13::AMC13Manager::~AMC13Manager() {
  
}

// This is the callback used for handling xdata:Event objects
void gem::hw::amc13::AMC13Manager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "AMC13Manager::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    //gemMonitorP_->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::amc13::AMC13Manager::preInit()
  throw (gem::base::exception::Exception)
{
  std::string addressBase  = "${AMC13_ADDRESS_TABLE_PATH}/";
  std::string connection   = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/amc13/connectionSN170_ch.xml";
  std::string cardname = "gem.shelf01.amc13.";
  try {
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
    amc13Device_ = new ::amc13::AMC13(connection,cardname+"T1",cardname+"T2");
  } catch (uhal::exception::exception & e) {
    LOG4CPLUS_ERROR(getApplicationLogger(), std::string("AMC13::AMC13() failed, caught uhal::exception:") + e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (std::exception& e) {
    LOG4CPLUS_ERROR(getApplicationLogger(), std::string("AMC13::AMC13() failed, caught std::exception:") + e.what() );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create class: ")+e.what());
  } catch (...) {
    LOG4CPLUS_ERROR(getApplicationLogger(), std::string("AMC13::AMC13() failed, caught ...") );
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Unable to create AMC13 connection"));
  }

  LOG4CPLUS_DEBUG(getApplicationLogger(),"finished with AMC13::AMC13()");

  try {
    // just T2-related work here.
    gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
    amc13Device_->reset(::amc13::AMC13::T2);
    
    amc13Device_->enableAllTTC(); // this is convenient for debugging, works with _some_ firmwares
    
  } catch (uhal::exception::exception & e) {
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  } catch (std::exception& e) {
    XCEPT_RAISE(gem::hw::amc13::exception::HardwareProblem,std::string("Problem during preinit : ")+e.what());
  }
  LOG4CPLUS_DEBUG(getApplicationLogger(),"finished with AMC13Manager::preInit()");
}

void gem::hw::amc13::AMC13Manager::init()
  throw (gem::base::exception::Exception)
{
  gem::base::GEMFSMApplication::init();

  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::init()");
  if (amc13Device_==0) return;

  //have to set up the initialization of the AMC13 for the desired running situation
  //possibilities are TTC/TCDS mode, DAQ link, local trigger scheme
}

void gem::hw::amc13::AMC13Manager::enable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::enable()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  amc13Device_->startRun();
}

void gem::hw::amc13::AMC13Manager::disable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::amc13::AMC13Manager::disable()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  amc13Device_->endRun();
}

::amc13::Status* gem::hw::amc13::AMC13Manager::getHTMLStatus() const {
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(deviceLock_);
  return amc13Device_->getStatus(); 
}

/*
// work loop call-back functions
bool gem::hw::amc13::AMC13Manager::initializeAction(toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::enableAction(    toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::configureAction( toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::startAction(     toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::pauseAction(     toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::resumeAction(    toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::stopAction(      toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::haltAction(      toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::resetAction(     toolbox::task::WorkLoop *wl) {};
//bool gem::hw::amc13::AMC13Manager::noAction(        toolbox::task::WorkLoop *wl) {};
bool gem::hw::amc13::AMC13Manager::failAction(      toolbox::task::WorkLoop *wl) {};

//bool gem::hw::amc13::AMC13Manager::calibrationAction(toolbox::task::WorkLoop *wl) {};
//bool gem::hw::amc13::AMC13Manager::calibrationSequencer(toolbox::task::WorkLoop *wl) {};
*/
	
//state transitions
void gem::hw::amc13::AMC13Manager::initializeAction() {}
void gem::hw::amc13::AMC13Manager::enableAction(    ) {}
void gem::hw::amc13::AMC13Manager::configureAction( ) {}
void gem::hw::amc13::AMC13Manager::startAction(     ) {}
void gem::hw::amc13::AMC13Manager::pauseAction(     ) {}
void gem::hw::amc13::AMC13Manager::resumeAction(    ) {}
void gem::hw::amc13::AMC13Manager::stopAction(      ) {}
void gem::hw::amc13::AMC13Manager::haltAction(      ) {}
void gem::hw::amc13::AMC13Manager::noAction(        ) {}

void gem::hw::amc13::AMC13Manager::failAction(      toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::amc13::AMC13Manager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
