/**
 * class: GEMApplication
 * description: Generic GEM application, all GEM applications should inherit
                from this class and define and extend as necessary
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: 
 * date: 
 */

// GEMApplication.cc

#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMApplication.h"
#include "gem/base/GEMMonitor.h"

#include "toolbox/string.h"

#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"
#include "xoap/Method.h"

#include "xcept/Exception.h"
#include "xcept/tools.h"

#include "xdaq/NamespaceURI.h"
#include "xdaq/Application.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptorImpl.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/XceptSerializer.h"
#include "xdata/Float.h" 
#include "xdata/Double.h" 
#include "xdata/Boolean.h"

gem::base::GEMApplication::GEMApplication(xdaq::ApplicationStub *stub)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(stub),
  gemLogger_(this->getApplicationLogger()),
  gemWebInterfaceP_(0),
  gemMonitorP_(0)
{
  DEBUG("called gem::base::GEMApplication constructor");
  
  gemWebInterfaceP_ = new GEMWebApplication(this);

  try {
    appInfoSpaceP_  = getApplicationInfoSpace();
    appDescriptorP_ = getApplicationDescriptor();
    appContextP_    = getApplicationContext();
    appZoneP_       = appContextP_->getDefaultZone();
    appGroupP_      = appZoneP_->getApplicationGroup("default");
    xmlClass_       = appDescriptorP_->getClassName();
    instance_       = appDescriptorP_->getInstance();
    urn_            = appDescriptorP_->getURN();
  }
  catch(xcept::Exception e) {
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to get GEM application information", e);
  }
  
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiDefault, "Default"    );
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiMonitor, "monitorView");
  xgi::framework::deferredbind(this, this, &GEMApplication::xgiExpert,  "expertView" );

  appInfoSpaceP_->addListener(this, "urn:xdaq-event:setDefaultValues");
  appInfoSpaceP_->fireItemAvailable("configuration:parameters", configInfoSpaceP_ );
  appInfoSpaceP_->fireItemAvailable("monitoring:parameters",    monitorInfoSpaceP_);
  //appInfoSpaceP_->fireItemAvailable("reasonForFailure", &reasonForFailure_);
  
  DEBUG("gem::base::GEMApplication constructed");
}


gem::base::GEMApplication::~GEMApplication() 
{
  INFO("gem::base::GEMApplication destructor called");
  
}

std::string gem::base::GEMApplication::getFullURL()
{
  std::string url = getApplicationDescriptor()->getContextDescriptor()->getURL();
  std::string urn = getApplicationDescriptor()->getURN();
  std::string fullURL = toolbox::toString("%s/%s", url.c_str(), urn.c_str());
  return fullURL;
}

// This is the callback used for handling xdata:Event objects
void gem::base::GEMApplication::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).  This should be implemented in all derived classes
  // followed by a call to gem::base::GEMApplication::actionPerformed(event)
  /*
  if (event.type() == "setDefaultValues" || event.type() == "urn:xdaq-event:setDefaultValues") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed() setDefaultValues" << 
		    "Default configuration values have been loaded from xml profile");
    //LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed()   --> starting monitoring");
    //monitorP_->startMonitoring();
    }
  */
  // update monitoring variables
  if (event.type() == "ItemRetrieveEvent" || event.type() == "urn:xdata-event:ItemRetrieveEvent") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed() ItemRetrieveEvent" << 
		    "");
  } else if (event.type() == "ItemGroupRetrieveEvent" || event.type() == "urn:xdata-event:ItemGroupRetrieveEvent") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed() ItemGroupRetrieveEvent" << 
		    "");
  }
  //item is changed, update it
  if (event.type()=="ItemChangedEvent" || event.type()=="urn:xdata-event:ItemChangedEvent") {
    LOG4CPLUS_DEBUG(getApplicationLogger(), "GEMApplication::actionPerformed() ItemChangedEvent" << 
		    "");

    /* from HCAL runInfoServer
    std::list<std::string> names;
    names.push_back(str_RUNNUMBER);

    try {
      getMonitorInfospace()->fireItemGroupChanged(names, this);
    } catch (xcept::Exception& e) {
      LOG4CPLUS_ERROR(getApplicationLogger(),xcept::stdformat_exception_history(e));
    }
    */
  }
}

void gem::base::GEMApplication::importConfigurationParameters() {
  //parse the xml configuration file or db configuration information
}


void gem::base::GEMApplication::fillConfigurationInfoSpace() {
  //put the configuration parameters into the configuration infospace
}


void gem::base::GEMApplication::updateConfigurationInfoSpace() {
  //update the configuration infospace object with new items
}


void gem::base::GEMApplication::importMonitoringParameters() {
  //parse the xml monitoring file or db monitoring information
}


void gem::base::GEMApplication::fillMonitoringInfoSpace() {
  //put the monitoring parameters into the monitoring infospace
}


void gem::base::GEMApplication::updateMonitoringInfoSpace() {
  //update the monitoring infospace object with new items
}


void gem::base::GEMApplication::xgiDefault(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->webDefault(in,out);
}

void gem::base::GEMApplication::xgiMonitor(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->monitorPage(in,out);
}

void gem::base::GEMApplication::xgiExpert(xgi::Input* in, xgi::Output* out) {
  gemWebInterfaceP_->expertPage(in,out);
}
// End of file
