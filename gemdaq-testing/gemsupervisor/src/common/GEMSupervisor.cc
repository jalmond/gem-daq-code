/**
 * class: GEMSupervisor
 * description: Supervisor application for GEM system
 *              structure borrowed from TCDS core, with nods to HCAL (hcalSupervisor)
 * author: J. Sturdy
 * date: 
 */

#include "gem/supervisor/GEMSupervisorWeb.h"
#include "gem/supervisor/GEMSupervisor.h"
#include "gem/supervisor/GEMSupervisorMonitor.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMSupervisor);

gem::supervisor::GEMSupervisor::GEMSupervisor(xdaq::ApplicationStub* stub) :
  gem::base::GEMFSMApplication(stub)
{

  //getApplicationInfoSpace()->fireItemAvailable("crateID", &m_crateID);
  //getApplicationInfoSpace()->fireItemAvailable("slot",    &m_slot);

  //xgi::framework::deferredbind(this, this, &GEMSupervisor::xgiDefault, "Default");

  DEBUG("Creating the GEMSupervisorWeb interface");
  p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this);
  // p_gemMonitor      = new gem::supervisor::GEMSupervisorMonitor(this->getApplicationLogger(),this);
  p_gemWebInterface = new gem::supervisor::GEMSupervisorWeb(this);
  DEBUG("done");
  //p_gemMonitor      = new gem generic system monitor
  
  v_supervisedApps.clear();
  // reset the GEMInfoSpaceToolBox object?
  //where can we get some nice PNG images for our different applications?
  getApplicationDescriptor()->setAttribute("icon","/gemdaq/gemsupervisor/images/supervisor/GEMSupervisor.png");
  init();
}

gem::supervisor::GEMSupervisor::~GEMSupervisor()
{
  // make sure to empty the v_supervisedApps  vector and free the pointers
  v_supervisedApps.clear();
}


// This is the callback used for handling xdata:Event objects
void gem::supervisor::GEMSupervisor::actionPerformed(xdata::Event& event)
{
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    DEBUG("GEMSupervisor::actionPerformed() setDefaultValues" << 
          "Default configuration values have been loaded from xml profile");
    importConfigurationParameters();
    importMonitoringParameters();
    //p_gemMonitor->startMonitoring();
  }
  // update monitoring variables
  gem::base::GEMApplication::actionPerformed(event);
}

bool gem::supervisor::GEMSupervisor::isGEMApplication(const std::string& classname) const
{
  if (classname.find("gem") != std::string::npos)
    return true; // handle all HCAL applications
  /*
    if (m_otherClassesToSupport.count(classname) != 0)
    return true; // include from list
  */
  return false;
}

bool gem::supervisor::GEMSupervisor::manageApplication(const std::string& classname) const
{
  if (classname == "GEMSupervisor")
    return false; // ignore ourself
  /*
    if (m_otherClassesToSupport.count(classname) != 0)
    return true; // include from list
  */
  if (classname.find("gem") != std::string::npos)
    return true; // handle all GEM applications
  if (classname.find("PeerTransport") != std::string::npos)
    return false; // ignore all peer transports
  /*
    if ((classname == "TTCciControl" || classname == "ttc::TTCciControl") && m_handleTTCci.value_)
    return true;
    if ((classname == "LTCControl" || classname == "ttc::LTCControl") && m_handleTTCci.value_)
    return true;
    if (classname.find("tcds") != std::string::npos && m_handleTCDS.value_)
    return true;
  */
  return false; // assume not ok.
}
void gem::supervisor::GEMSupervisor::init()
{
  v_supervisedApps.clear();
  v_supervisedApps.reserve(0);
  DEBUG("init:: looping over " << p_appZone->getGroupNames().size() << " groups");
  std::set<xdaq::ApplicationDescriptor*> used;
  std::set<std::string> groups = p_appZone->getGroupNames();
  //for (std::set<std::string>::const_iterator i = groups.begin(); i != groups.end(); i++) {
  for (auto i =groups.begin(); i != groups.end(); ++i) {
    DEBUG("init::xDAQ group: " << *i
          << "getApplicationGroup() " << p_appZone->getApplicationGroup(*i)->getName());
    
    xdaq::ApplicationGroup* ag = p_appZone->getApplicationGroup(*i);
    std::set<xdaq::ApplicationDescriptor*> allApps = ag->getApplicationDescriptors();
    
    DEBUG("init::getApplicationDescriptors() " << allApps.size());
    //for (std::set<xdaq::ApplicationDescriptor*>::const_iterator j=allApps.begin(); j!=allApps.end(); j++) {
    for (auto j = allApps.begin(); j != allApps.end(); ++j) {
      DEBUG("init::xDAQ application descriptor " << *j
            << " " << (*j)->getClassName()
            << " we are " << p_appDescriptor);
    
      if (used.find(*j) != used.end()) continue; // no duplicates
      if ((*j) == p_appDescriptor ) continue; // don't fire the command into the GEMSupervisor again
      //maybe just write a function that populates some vectors
      //with the application classes that we want to supervise
      //avoids the problem of picking up all the xDAQ related processes
      //if (isGEMSupervised(*j))
      if (manageApplication((*j)->getClassName())) {
        INFO("init::pushing " << (*j)->getClassName() << "(" << *j << ") to list of supervised applications");
        v_supervisedApps.push_back(*j);
        std::stringstream managedAppStateName;
        managedAppStateName << (*j)->getClassName() << "-lid:" << (*j)->getLocalId();
        std::stringstream managedAppStateURN;
        managedAppStateURN << (*j)->getURN();
        p_appStateInfoSpaceToolBox->createString(managedAppStateName.str(),managedAppStateURN.str());
      }
      DEBUG("done");
    } // done iterating over applications in group
    DEBUG("init::done iterating over applications in group");
  } // done iterating over groups in zone
  DEBUG("init::done iterating over groups in zone");
  
  DEBUG("init::starting the monitoring");

  // when to do this, have to make sure that all applications have been loaded...
  //p_gemMonitor->addInfoSpace("AppStateMonitoring", p_appStateInfoSpaceToolBox);
  dynamic_cast<gem::supervisor::GEMSupervisorMonitor*>(p_gemMonitor)->setupAppStateMonitoring();
  p_gemMonitor->startMonitoring();
};

//state transitions
void gem::supervisor::GEMSupervisor::initializeAction()
  throw (gem::supervisor::exception::Exception)
{
  INFO("gem::supervisor::GEMSupervisor::initializeAction Initializing");
  
  //for (std::vector<xdaq::ApplicationDescriptor*>::iterator i=v_supervisedApps.begin(); i!=v_supervisedApps.end(); i++) {
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Initializing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::configureAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Configuring ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::startAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Starting ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Start",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::pauseAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Pausing ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Pause",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::resumeAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Resuming ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Resume",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::stopAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Stopping ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::haltAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Halting ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt",p_appContext,p_appDescriptor,*i);
  }
}

void gem::supervisor::GEMSupervisor::resetAction()
  throw (gem::supervisor::exception::Exception)
{
  for (auto i = v_supervisedApps.begin(); i != v_supervisedApps.end(); ++i) {
    INFO(std::string("Resetting ")+(*i)->getClassName());
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset",p_appContext,p_appDescriptor,*i);
  }
  //gem::base::GEMFSMApplication::resetAction();
}

/*
  void gem::supervisor::GEMSupervisor::noAction()
  throw (gem::supervisor::exception::Exception)
  {
  }
*/

void gem::supervisor::GEMSupervisor::failAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}

void gem::supervisor::GEMSupervisor::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
}
