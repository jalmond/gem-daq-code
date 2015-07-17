/**
 * class: GLIBManager
 * description: Manager application for GLIB cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/glib/GLIBManagerWeb.h"
#include "gem/hw/glib/GLIBManager.h"

#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/glib/exception/Exception.h"

gem::hw::glib::GLIBManager::GLIBInfo::GLIBInfo() {
  present = false;
  crateID = -1;
  slotID  = -1;
}

void gem::hw::glib::GLIBManager::GLIBInfo::registerFields(xdata::Bag<gem::hw::glib::GLIBManager::GLIBInfo>* bag) {
  bag->addField("crateID", &crateID);
  bag->addField("slot",    &slotID);
  bag->addField("present", &present);
}

gem::hw::glib::GLIBManager::GLIBManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  //maybe we put this type of stuff into a per GLIB infospace, in the monitor?
  // getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  // getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);

  //initialize the GLIB application objects
  LOG4CPLUS_DEBUG(getApplicationLogger(), "connecting to the GLIBManagerWeb interface");
  gemWebInterfaceP_ = new gem::hw::glib::GLIBManagerWeb(this);
  //gemMonitorP_      = new gem::hw::glib::GLIBHwMonitor(this);
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");
  
  for (int slot=1; slot <= MAX_AMCS_PER_CRATE; slot++)
    m_glibs[slot-1] = 0;

  //init();
  //getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/glib/GLIBManager.png");
}

gem::hw::glib::GLIBManager::~GLIBManager() {
  
}

// This is the callback used for handling xdata:Event objects
void gem::hw::glib::GLIBManager::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GLIBManager::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    //gemMonitorP_->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

void gem::hw::glib::GLIBManager::preInit()
  throw (gem::base::exception::Exception)
{
  
  for (int slot=1; slot <= MAX_AMCS_PER_CRATE; slot++) {
    
    //check if there is a GLIB in the specified slot, if not, do not initialize
    //set the web view to be empty or grey
    //if (!info.present.value_) continue;
    //gemWebInterfaceP_->glibInSlot(slot);
  }  
}

void gem::hw::glib::GLIBManager::init()
  throw (gem::base::exception::Exception)
{
  gem::base::GEMFSMApplication::init();

  uhal::setLogLevelTo( uhal::ErrorLevel() );
  
  int gemCrate = 1;
  
  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; slot++) {
    GLIBInfo& info = m_glibInfo[slot].bag;
    
    //check the config file if there should be a GLIB in the specified slot, if not, do not initialize
    //if slot empty
    //  continue;
    
    info.present = true;
    info.crateID = gemCrate;
    info.slotID  = slot+1;
    
    m_glibs[slot] = new gem::hw::glib::HwGLIB(gemCrate,slot+1);
    m_glibs[slot]->connectDevice();
    //set the web view to be empty or grey
    //if (!info.present.value_) continue;
    //gemWebInterfaceP_->glibInSlot(slot);
  }

  for (int slot = 0; slot < MAX_AMCS_PER_CRATE; slot++) {
    GLIBInfo& info = m_glibInfo[slot].bag;

    if (!info.present)
      continue;
    
    gem::hw::glib::HwGLIB* glib= m_glibs[slot];
    
  }
}

void gem::hw::glib::GLIBManager::enable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::glib::GLIBManager::enable()");
  //gem::base::GEMFSMApplication::enable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_deviceLock);
  //m_glibs[0]->startRun();
}

void gem::hw::glib::GLIBManager::disable()
  throw (gem::base::exception::Exception) {
  LOG4CPLUS_DEBUG(getApplicationLogger(),"Entering gem::hw::glib::GLIBManager::disable()");
  //gem::base::GEMFSMApplication::disable();
  gem::utils::LockGuard<gem::utils::Lock> guardedLock(m_deviceLock);
  //m_glibs[0]->endRun();
}

/*
// work loop call-back functions
bool gem::hw::glib::GLIBManager::initializeAction(toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::enableAction(    toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::configureAction( toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::startAction(     toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::pauseAction(     toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::resumeAction(    toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::stopAction(      toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::haltAction(      toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::resetAction(     toolbox::task::WorkLoop *wl) {};
//bool gem::hw::glib::GLIBManager::noAction(        toolbox::task::WorkLoop *wl) {};
bool gem::hw::glib::GLIBManager::failAction(      toolbox::task::WorkLoop *wl) {};

//bool gem::hw::glib::GLIBManager::calibrationAction(toolbox::task::WorkLoop *wl) {};
//bool gem::hw::glib::GLIBManager::calibrationSequencer(toolbox::task::WorkLoop *wl) {};
*/
	
//state transitions
void gem::hw::glib::GLIBManager::initializeAction() {}
void gem::hw::glib::GLIBManager::enableAction(    ) {}
void gem::hw::glib::GLIBManager::configureAction( ) {}
void gem::hw::glib::GLIBManager::startAction(     ) {}
void gem::hw::glib::GLIBManager::pauseAction(     ) {}
void gem::hw::glib::GLIBManager::resumeAction(    ) {}
void gem::hw::glib::GLIBManager::stopAction(      ) {}
void gem::hw::glib::GLIBManager::haltAction(      ) {}
void gem::hw::glib::GLIBManager::noAction(        ) {}

void gem::hw::glib::GLIBManager::failAction(      toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::hw::glib::GLIBManager::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
