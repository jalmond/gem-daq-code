/**
 * class: GLIBManager
 * description: Manager application for GLIB cards
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: J. Sturdy
 * date: 
 */

#include "gem/hw/glib/GLIBManagerWeb.h"
#include "gem/hw/glib/GLIBManager.h"

gem::hw::glib::GLIBManager::GLIBManager(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{
  init();
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemhardware/images/glib/GLIBManager.png");
}

void gem::hw::glib::GLIBManager::init()
{
  //initialize the GLIB application objects
  gemWebInterfaceP_ = new gem::hw::glib::GLIBManagerWeb(this);
  //gemMonitorP_      = new gem::hw::glib::GLIBHwMonitor();
  
  for (int slot=1; slot <= MAX_GLIBS_PER_CRATE; slot++) {
    
    //check if there is a GLIB in the specified slot, if not, do not initialize
    //set the web view to be empty or grey
    //if (!info.present.value_) continue;
    //gemWebInterfaceP_->glibInSlot(slot);
  }  
}

// This is the callback used for handling xdata:Event objects
void gem::base::GEMApplication::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GLIBManager::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    //gemMonitorP_->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

/**hyperdaq callbacks, probably not necessary to implement in GLIBManager*/
/**
void gem::hw::glib::GLIBManager::xgiInitialize(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webInitialize(in,out);
}

void gem::hw::glib::GLIBManager::xgiEnable(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webEnable(in,out);
}

void gem::hw::glib::GLIBManager::xgiConfigure(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webConfigure(in,out);
}

void gem::hw::glib::GLIBManager::xgiStart(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webStart(in,out);
}

void gem::hw::glib::GLIBManager::xgiStop(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webStop(in,out);
}

void gem::hw::glib::GLIBManager::xgiPause(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webPause(in,out);
}

void gem::hw::glib::GLIBManager::xgiResume(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webResume(in,out);
}

void gem::hw::glib::GLIBManager::xgiHalt(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webHalt(in,out);
}

void gem::hw::glib::GLIBManager::xgiReset(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webReset(in,out);
}
**/
/**soap callbacks*/
/**
void gem::hw::glib::GLIBManager::initializeAction(toolbox::Event::Reference event)
{
  INFO("GEMFSMApplication::initializeAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->connectDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not connect to the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  //appStateInfoSpace_.setBool("gem_hw_connected", true, true);
}

void gem::hw::glib::GLIBManager::configureAction(toolbox::Event::Reference event)
{
  INFO("ConfigureAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
  try {
    //gemhwP_->configureDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not configure the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("GEMFSMApplication::ConfigureAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void gem::hw::glib::GLIBManager::enableAction(toolbox::Event::Reference event)
{
  INFO("EnableAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->enableDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not enable the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("EnableAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::hw::glib::GLIBManager::failAction(toolbox::Event::Reference event)
{
  INFO("FailAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
//  if (gemhwP_ != 0)
//    {
//      // BUG BUG BUG
//      // This is a mess (and may need a lock, actually).
//      //gemhwP_->releaseDevice();
//      // BUG BUG BUG end
//    }
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  //INFO("FailAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::hw::glib::GLIBManager::haltAction(toolbox::Event::Reference event)
{
  INFO("HaltAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP->haltDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not halt the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  try {
    //gemhwP_->releaseDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not release the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }

  //appStateInfoSpace_.setBool("gem_hw_connected", false, true);

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("HaltAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::hw::glib::GLIBManager::pauseAction(toolbox::Event::Reference event)
{
  INFO("PauseAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->pauseDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not pause the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }

  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("PauseAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::hw::glib::GLIBManager::resumeAction(toolbox::Event::Reference event)
{
  INFO("ResumeAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();

  try {
    //gemhwP_->resumeDevice();
    std::string msgBase = "Could not configure the hardware";
  }
  catch (gem::hw::exception::Exception& err) {
    std::string msgBase = "Could not resume the hardware";
    std::string msg = toolbox::toString("%s: '%s'.", msgBase.c_str(), err.what());
    FATAL(msg);
    XCEPT_DECLARE_NESTED(gem::hw::exception::HardwareProblem, top, msg, err);
    notifyQualified("fatal", top);
    gemfsm_.gotoFailed(top);
  }
  
  // BUG BUG BUG
  // Slow things down a bit while developing. Easier to follow the flow...
  ::sleep(2);
  // BUG BUG BUG end
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("ResumeAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}

void
gem::hw::glib::GLIBManager::stopAction(toolbox::Event::Reference event)
{
  INFO("StopAction()");
  toolbox::TimeVal timeBegin = toolbox::TimeVal::gettimeofday();
  this->haltAction(event);
  this->configureAction(event);
  toolbox::TimeVal timeEnd = toolbox::TimeVal::gettimeofday();
  toolbox::TimeVal deltaT = timeEnd - timeBegin;
  //INFO("StopAction() took " << tcds::utils::formatDeltaTString(timeBegin, timeEnd) << ".");
}
 
// This simply forwards the message to the FSM object, since it is
// technically not possible to bind directly to anything but an
// xdaq::Application.
xoap::MessageReference
gem::hw::glib::GLIBManager::changeState(xoap::MessageReference msg)
{
  return gemfsm_.changeState(msg);
}
*/ 
