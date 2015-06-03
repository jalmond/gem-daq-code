/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (DTCManager)
 * author: J. Sturdy
 * date: 
 */

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisor.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "gem/supervisor/exception/Exception.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{

  //getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  //getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);
  //initialize the AMC13Manager application objects

  //xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");

  LOG4CPLUS_DEBUG(getApplicationLogger(), "Creating the GEMSupervisorWeb interface");
  gemWebInterfaceP_ = new gem::supervisor::GEMSupervisorWeb(this);
  LOG4CPLUS_DEBUG(getApplicationLogger(), "done");
  //gemMonitorP_      = new gem generic system monitor
  
  //where can we get some nice PNG images for our different applications?
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
  init();
  
}

gem::supervisor::GEMSupervisor::~GEMSupervisor() {
  
}


// This is the callback used for handling xdata:Event objects
void gem::supervisor::GEMSupervisor::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMSupervisor::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    //gemMonitorP_->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

//void gem::supervisor::GEMSupervisor::xgiDefault(xgi::Input* in, xgi::Output* out) {
//  dynamic_cast<gem::supervisor::GEMSupervisorWeb*>(gemWebInterfaceP_)->controlPanel(in,out);
//}

void gem::supervisor::GEMSupervisor::init() {
  v_supervisedApps.clear();

  std::set<xdaq::ApplicationDescriptor*> used;
  std::set<std::string> groups = getApplicationContext()->getDefaultZone()->getGroupNames();
  for (std::set<std::string>::const_iterator i = groups.begin(); i != groups.end(); i++) {
    xdaq::ApplicationGroup* ag = getApplicationContext()->getDefaultZone()->getApplicationGroup(*i);
    std::set<xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
    
    
    for (std::set<xdaq::ApplicationDescriptor*>::const_iterator j=allApps.begin(); j!=allApps.end(); j++) {
      if (used.find(*j) != used.end()) continue; // no duplicates
      if ((*j) == appDescriptorP_ ) continue; // don't fire the command into the GEMSupervisor again
      //maybe just write a function that populates some vectors
      //with the application classes that we want to supervise
      //avoids the problem of picking up all the xDAQ related processes
      //if (isGEMSupervised(*j))
	v_supervisedApps.push_back(*j);
    }
  }
};

//workloops
bool gem::supervisor::GEMSupervisor::initialize(toolbox::task::WorkLoop *wl) {
  
  LOG4CPLUS_INFO(getApplicationLogger(),std::string("gem::supervisor::GEMSupervisor::initialize Initializing"));
  
  for (std::vector<xdaq::ApplicationDescriptor*>::iterator i=v_supervisedApps.begin(); i!=v_supervisedApps.end(); i++) {
    LOG4CPLUS_INFO(getApplicationLogger(),std::string("Initializing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize", appContextP_, appDescriptorP_, *i
						  //std::string params
						  );
  }
  return false;
}

bool gem::supervisor::GEMSupervisor::enable(    toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::configure( toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::start(     toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::pause(     toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::resume(    toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::stop(      toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::halt(      toolbox::task::WorkLoop *wl) {
  return false;
}

bool gem::supervisor::GEMSupervisor::reset(     toolbox::task::WorkLoop *wl) {
  return false;
}

//state transitions
void gem::supervisor::GEMSupervisor::initializeAction() {
  LOG4CPLUS_INFO(getApplicationLogger(),std::string("gem::supervisor::GEMSupervisor::initializeAction Initializing"));
  
  for (std::vector<xdaq::ApplicationDescriptor*>::iterator i=v_supervisedApps.begin(); i!=v_supervisedApps.end(); i++) {
    LOG4CPLUS_INFO(getApplicationLogger(),std::string("Initializing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",
						  appContextP_,
						  //getApplicationContext(),
						  appDescriptorP_,
						  //getApplicationDescriptor()
						  *i
						  //std::string params
						  );
  }
}

void gem::supervisor::GEMSupervisor::enableAction(    ) {}
void gem::supervisor::GEMSupervisor::configureAction( ) {}
void gem::supervisor::GEMSupervisor::startAction(     ) {}
void gem::supervisor::GEMSupervisor::pauseAction(     ) {}
void gem::supervisor::GEMSupervisor::resumeAction(    ) {}
void gem::supervisor::GEMSupervisor::stopAction(      ) {}
void gem::supervisor::GEMSupervisor::haltAction(      ) {}
void gem::supervisor::GEMSupervisor::noAction(        ) {}

void gem::supervisor::GEMSupervisor::failAction(      toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

