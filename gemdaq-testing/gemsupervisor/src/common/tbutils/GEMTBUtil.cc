#include "gem/supervisor/tbutils/GEMTBUtil.h"
#include "gem/hw/vfat/HwVFAT2.h"

//felipe 3
#include "gem/readout/GEMDataParker.h"
#include "gem/hw/glib/HwGLIB.h"
#include "gem/hw/optohybrid/HwOptoHybrid.h"
#include "gem/utils/GEMLogging.h"


#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TError.h"

#include <algorithm>
#include <ctime>


#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"
#include "xdata/Vector.h"
#include <string>

#include "gem/supervisor/tbutils/VFAT2XMLParser.h"

#include "TStopwatch.h"

//XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::GEMTBUtil)

void gem::supervisor::tbutils::GEMTBUtil::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  readoutDelay = 1U; //readout delay in milleseconds/microseconds?

  nTriggers = 1000U;

  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);
  std::string tmpFileName = "GEMTBUtil_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  //std::replace(tmpFileName.begin(), tmpFileName.end(), '\n', '_');

  outFileName  = tmpFileName;
  slotFileName = "slot_table_904_2.csv";
  //settingsFile = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/vfat/vfat_settings.xml";

  //  deviceIP      = "192.168.0.170";

  //stablish the number of VFATs and the entry is 
  for (int i = 0; i < 24; ++i) {
    deviceName.push_back("");
    deviceNum.push_back(-1);
  }

  triggerSource = 0x9;
  deviceChipID  = 0x0;

  triggersSeen = 0;
  //  m_confParams.bag.triggersSeen = 0;
  triggersSeenGLIB = 0;
  triggercount = 0;
  ADCVoltage = 0;
  ADCurrent = 0;
  ohGTXLink    = 3;

  bag->addField("readoutDelay", &readoutDelay);
  bag->addField("nTriggers",    &nTriggers);

  bag->addField("outFileName",  &outFileName );
  bag->addField("settingsFile", &settingsFile);

  bag->addField("deviceName",   &deviceName );
  bag->addField("deviceIP",     &deviceIP    );
  bag->addField("ohGTXLink",    &ohGTXLink   );

  bag->addField("deviceNum",    &deviceNum   );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("triggersSeen", &triggersSeen);
  bag->addField("triggercount", &triggercount);

  bag->addField("ADCVoltage",    &ADCVoltage);
  bag->addField("ADCurrent",     &ADCurrent);
  bag->addField("triggerSource", &triggerSource);
  bag->addField("slotFileName",  &slotFileName);

}

gem::supervisor::tbutils::GEMTBUtil::GEMTBUtil(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s),
  m_gemLogger(this->getApplicationLogger()),
  p_fsm(0),
  wl_semaphore_(toolbox::BSem::FULL),
  hw_semaphore_(toolbox::BSem::FULL),

  initSig_ (0),
  confSig_ (0),
  startSig_(0),
  stopSig_ (0),
  haltSig_ (0),
  resetSig_(0),
  //  runSig_  (0),
  readSig_ (0),
  m_readout_mask(0x0),
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false)


{
  gErrorIgnoreLevel = kWarning;
  
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams", &m_confParams);
  getApplicationInfoSpace()->fireItemAvailable("ipAddr",     &m_ipAddr);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &m_confParams);
  getApplicationInfoSpace()->fireItemValueRetrieve("ipAddr",     &m_ipAddr);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webInitialize,   "Initialize" );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webStart,        "Start"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webStop,         "Stop"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webHalt,         "Halt"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webReset,        "Reset"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webResetCounters,"ResetCounters");
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::GEMTBUtil::webSendFastCommands,"FastCommands");
  
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::GEMTBUtil::onReset,       "Reset",       XDAQ_NS_URI);
  
  initSig_  = toolbox::task::bind(this, &GEMTBUtil::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &GEMTBUtil::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &GEMTBUtil::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &GEMTBUtil::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &GEMTBUtil::halt,       "halt"      );
  resetSig_ = toolbox::task::bind(this, &GEMTBUtil::reset,      "reset"     );

  std::string className = getApplicationDescriptor()->getClassName();
  INFO("className " << className);
  className =
    className.substr(className.rfind("gem::supervisor::"),std::string::npos);
  INFO("className " << className);

  p_fsm = new
    toolbox::fsm::AsynchronousFiniteStateMachine("GEMTButilFSM:" + className);

  
  p_fsm->addState('I', "Initial",     this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  p_fsm->addState('H', "Halted",      this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  p_fsm->addState('C', "Configured",  this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  p_fsm->addState('E', "Running",     this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  
  p_fsm->setStateName('F', "Error");
  p_fsm->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::GEMTBUtil::transitionFailed);
  p_fsm->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  
  p_fsm->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::GEMTBUtil::initializeAction);
  p_fsm->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::GEMTBUtil::configureAction);
  p_fsm->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::GEMTBUtil::configureAction);
  p_fsm->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::GEMTBUtil::startAction);
  p_fsm->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::GEMTBUtil::stopAction);
  p_fsm->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  p_fsm->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  //p_fsm->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  p_fsm->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::GEMTBUtil::resetAction);
  p_fsm->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::GEMTBUtil::resetAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  p_fsm->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  p_fsm->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  p_fsm->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  p_fsm->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  p_fsm->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  p_fsm->addStateTransition('H', 'H', "Halt"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);

  p_fsm->setInitialState('I');
  p_fsm->reset();

  /*
    wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:GEMTBUtil","waiting");
    wl_->activate();
  */
  
}

gem::supervisor::tbutils::GEMTBUtil::~GEMTBUtil()
  
{

  if (p_fsm)
    delete p_fsm;
  p_fsm = 0;
  
}


void gem::supervisor::tbutils::GEMTBUtil::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "m_ipAddr=[" << m_ipAddr.toString() << "]" << std::endl;
    ss << "slotFileName=["  << m_confParams.bag.slotFileName.toString()  << "]" << std::endl;

    slotInfo = std::unique_ptr<gem::readout::GEMslotContents>(new gem::readout::GEMslotContents(m_confParams.bag.slotFileName.toString()));
    
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), ss.str());
    m_confParams.bag.deviceIP = m_ipAddr;
  }
}

void gem::supervisor::tbutils::GEMTBUtil::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));  
  p_fsm->fireEvent(event);
}

void gem::supervisor::tbutils::GEMTBUtil::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;
  
  INFO("Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state = fsm.getStateName(fsm.getCurrentState());
  
  INFO( "StateChanged: " << (std::string)state);
  
}

void gem::supervisor::tbutils::GEMTBUtil::transitionFailed(toolbox::Event::Reference event)
{
  //keep_refresh_ = false;
  toolbox::fsm::FailedEvent &failed = dynamic_cast<toolbox::fsm::FailedEvent&>(*event);
  
  std::stringstream reason;
  reason << "<![CDATA["
         << std::endl
         << "Failure occurred when performing transition"
         << " from "        << failed.getFromState()
         << " to "          << failed.getToState()
         << ". Exception: " << xcept::stdformat_exception_history( failed.getException() )
         << std::endl
         << "]]>";
  
  ERROR(reason.str());
}



//Actions (defined in the base class, not in the derived class)
bool gem::supervisor::tbutils::GEMTBUtil::initialize(toolbox::task::WorkLoop* wl)
{
  fireEvent("Initialize");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::configure(toolbox::task::WorkLoop* wl)
{
  fireEvent("Configure");
  m_counter = {0,0,0,0,0};
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
  return false;
}

bool gem::supervisor::tbutils::GEMTBUtil::stop(toolbox::task::WorkLoop* wl)
{
  fireEvent("Stop");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::halt(toolbox::task::WorkLoop* wl)
{
  fireEvent("Halt");
  return false; //do once?
}

bool gem::supervisor::tbutils::GEMTBUtil::reset(toolbox::task::WorkLoop* wl)
{
  fireEvent("Reset");
  return false; //do once?
}


// SOAP interface (defined in the base class, not in the derived class)
xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(initSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(confSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(startSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(stopSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(haltSig_);

  return message;
}

xoap::MessageReference gem::supervisor::tbutils::GEMTBUtil::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(resetSig_);

  return message;
}

void gem::supervisor::tbutils::GEMTBUtil::showCounterLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/ResetCounters") << std::endl;
      
      hw_semaphore_.take();
      
      *out << "<table class=\"xdaq-table\">" << std::endl
	//<< cgicc::caption("Counters")     << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "L1A"      << cgicc::th() << std::endl
	   << cgicc::th()    << "CalPulse" << cgicc::th() << std::endl
	   << cgicc::th()    << "Other"    << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl 

	   << cgicc::tbody() << std::endl;

      *out << "<tr>" << std::endl
	   << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << cgicc::th()    << "Reset"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl //close
	   << cgicc::thead() << std::endl //close
      
	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "TTC_on_GLIB"    << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getL1ACount(0x0) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1ATTC")
	.set("name","RstL1ATTC")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "T1_in_Firmware"    << cgicc::td() << std::endl
	   << cgicc::td()    <<  p_optohybridDevice->getL1ACount(0x1) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1AT1")
	.set("name","RstL1AT1")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "External"     << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getL1ACount(0x2) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1AExt")
	.set("name","RstL1AExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Loopback_sBits"       << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getL1ACount(0x3) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1Asbits")
	.set("name","RstL1Asbits")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Sent_along_GEB"     << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getL1ACount(0x4) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstL1AGEB")
	.set("name","RstL1AGEB")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "</tbody>" << std::endl
	   << "</table>"     << std::endl
	   << "</td>" << std::endl;

      *out << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << cgicc::th()    << "Reset"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl
	   << cgicc::thead() << std::endl

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	//	   << "<tr>" << std::endl
	   << cgicc::td()    << "TTC_on_GLIB"  << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getCalPulseCount(0x0)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseTTC")
	.set("name","RstCalPulseTTC")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl

	   << cgicc::td()    << "T1_in_Firmware"  << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getCalPulseCount(0x1)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseT1")
	.set("name","RstCalPulseT1")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl

	   << cgicc::td()    << "External"  << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getCalPulseCount(0x2)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseExt")
	.set("name","RstCalPulseExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Loopback_sBits"     << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getCalPulseCount(0x3) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseSbit")
	.set("name","RstCalPulseSbit")
	   << cgicc::td() << std::endl
	   << "</tr>"     << std::endl

	   << "<tr>" << std::endl

	   << cgicc::td()    << "Sent_along_GEB"  << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getCalPulseCount(0x4)  << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstCalPulseGEB")
	.set("name","RstCalPulseGEB")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "</tbody>"  << std::endl
	   << "</table>"  << std::endl
	   << "</td>"     << std::endl;
    
      *out << "<td>" << std::endl
	   << "<table class=\"xdaq-table\">" << std::endl
	   << cgicc::thead() << std::endl
	   << "<tr>" << std::endl
	   << cgicc::th()    << "Source" << cgicc::th() << std::endl
	   << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	   << cgicc::th()    << "Reset"  << cgicc::th() << std::endl
	   << "</tr>" << std::endl
	   << cgicc::thead() << std::endl

	   << "<tbody>" << std::endl
	   << "<tr>" << std::endl
	   << cgicc::td()    << "Resync"    << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getResyncCount() << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstResync")
	.set("name","RstResync")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "BC0"       << cgicc::td() << std::endl
	   << cgicc::td()    << p_optohybridDevice->getBC0Count() << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	.set("id","RstBC0")
	.set("name","RstBC0")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()  << "BXCount"   << cgicc::td() << std::endl
	   << cgicc::td()  << p_optohybridDevice->getBXCountCount() << cgicc::td() << std::endl
	   << cgicc::td()  << "" << cgicc::td() << std::endl
	   << "</tr>"      << std::endl
	   << "</tbody>"   << std::endl
	   << "</table>"   << std::endl
	   << "</td>"      << std::endl
	   << "</tr>"      << std::endl
	   << cgicc::tbody() << std::endl
	   << "</table>"   << std::endl;

      hw_semaphore_.give();

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset counters.")
	.set("value", "ResetCounters") << std::endl;

      *out << cgicc::form() << std::endl;
      
    }
  }
  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying showCounterLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    INFO("Something went wrong displaying showCounterLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  hw_semaphore_.give();
} //end showCounterLayout


void gem::supervisor::tbutils::GEMTBUtil::showBufferLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands") << std::endl;
      hw_semaphore_.take();
      hw_semaphore_.give();

      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("class","button").set("type","submit")
	.set("value","FlushFIFO").set("name","SendFastCommand")
	   << std::endl; 

      *out << cgicc::input().set("class","button").set("type","submit")
	.set("value","SendTestPackets").set("name","SendFastCommand")
	   << std::endl; 
     
      *out << cgicc::form() << std::endl
	   << cgicc::br()   << std::endl;
    }
  }
  
  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying showBufferLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    INFO("Something went wrong displaying showBufferLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  hw_semaphore_.give();
} //end showBufferLayout


void gem::supervisor::tbutils::GEMTBUtil::fastCommandLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands") << std::endl;
      
      *out << cgicc::table().set("class","xdaq-table") << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "L1A"          << cgicc::th() << std::endl
	   << cgicc::th()    << "CalPulse"     << cgicc::th() << std::endl
	   << cgicc::th()    << "Resync"       << cgicc::th() << std::endl
	   << cgicc::th()    << "BC0"          << cgicc::th() << std::endl
	   << cgicc::th()    << "L1A+CalPulse" << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl 

	   << cgicc::tbody() << std::endl;
      
      *out << cgicc::tr()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send L1A").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send CalPulse").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send Resync").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send BC0").set("name","SendFastCommand")
	   << cgicc::td()  << std::endl;
      *out << cgicc::td()  << cgicc::input().set("class","button").set("type","submit")
	.set("value","Send L1A+CalPulse").set("name","SendFastCommand")
	   << cgicc::br()  << std::endl
	   << cgicc::input().set("id","CalPulseDelay").set("name","CalPulseDelay")
	.set("type","number").set("min","0").set("max","255")
	.set("value","4")
	   << cgicc::td()  << std::endl;

      *out << cgicc::tr()    << std::endl
	   << cgicc::tbody() << std::endl
	   << cgicc::table() << std::endl;
	
      //trigger setup
      *out << cgicc::table().set("class","xdaq-table") << std::endl
	   << cgicc::thead() << std::endl
	   << cgicc::tr()    << std::endl //open
	   << cgicc::th()    << "Trigger Source Select" << cgicc::th() << std::endl
	   << cgicc::th()    << "SBit to TDC Select"    << cgicc::th() << std::endl
	   << cgicc::tr()    << std::endl //close
	   << cgicc::thead() << std::endl 

	   << cgicc::tbody() << std::endl;
      
      *out << cgicc::tr() << std::endl;
      *out << cgicc::td() << std::endl
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","GLIBsrc").set("value","TTC_GLIB_trsSrc")
	.set((unsigned)m_confParams.bag.triggerSource == (unsigned)0x0 ? "checked" : "")

	   << cgicc::label("TTC_GLIB_trsSrc").set("for","GLIBSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","T1_Src").set("value","T1_trgSrc")
	.set((unsigned)m_confParams.bag.triggerSource == (unsigned)0x1 ? "checked" : "")
	   << cgicc::label("T1_trgSrc").set("for","T1_Src") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","Ext").set("value","Ext_trgSrc")
	.set((unsigned)m_confParams.bag.triggerSource == (unsigned)0x2 ? "checked" : "")
	   << cgicc::label("Ext_trgSrc").set("for","Ext") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc").set("checked")
	.set("id","sBitSrc").set("value","sbits_trgSrc")
	.set((unsigned)m_confParams.bag.triggerSource == (unsigned)0x3 ? "checked" : "")
	   << cgicc::label("sbits_trgSrc").set("for","sBitSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	.set("id","Total").set("value","Total_trgSrc")
	.set((unsigned)m_confParams.bag.triggerSource == (unsigned)0x4 ? "checked" : "")
	   << cgicc::label("Total_trgSrc").set("for","Total") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("class","button").set("type","submit")
	.set("value","SetTriggerSource").set("name","SendFastCommand")
	   << cgicc::td() << std::endl;
      
      std::string isReadonly = "";
      if (is_running_ || is_configured_)
	isReadonly = "readonly";
      
      *out << cgicc::td() << std::endl
	   << cgicc::label("SBitSelect").set("for","SBitSelect") << std::endl
	   << cgicc::input().set("class","vfatBiasInput").set("id","SBitSelect" ).set("name","SBitSelect")
	.set("type","number").set("min","0").set("max","5")
	.set("value",m_confParams.bag.deviceNum.toString())
	.set(isReadonly)
	   << cgicc::input().set("class","button").set("type","submit")
	.set("value","SBitSelect").set("name","SendFastCommand")
	   << cgicc::td() << std::endl;

      *out << cgicc::tr()    << std::endl
	   << cgicc::tbody() << std::endl
	   << cgicc::table() << std::endl
	   << cgicc::form()  << std::endl;
    }
  }
  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying fastCommandLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    INFO("Something went wrong displaying fastCommandLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  hw_semaphore_.give();
}

void gem::supervisor::tbutils::GEMTBUtil::redirect(xgi::Input *in, xgi::Output* out) {
  //change the status to halting and make sure the page displays this information
  std::string redURL = "/" + getApplicationDescriptor()->getURN() + "/Default";
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;  
  this->webDefault(in,out);
}

// HyperDAQ interface
void gem::supervisor::tbutils::GEMTBUtil::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    ////update the page refresh 
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","2");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","30");
    }
    
    //generate the control buttons and display the ones that can be touched depending on the run mode
    *out << "<div class=\"xdaq-tab-wrapper\">"            << std::endl;
    *out << "<div class=\"xdaq-tab\" title=\"Control\">"  << std::endl;


    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Control" << cgicc::th() << std::endl
	 << cgicc::th()    << "Buffer"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << "<tbody>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;
    
    if (!is_initialized_) {
      //have a menu for selecting the VFAT
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Initialize") << std::endl;

      selectMultipleVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine
      
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
      
      selectMultipleVFAT(out);
      scanParameters(out);     
      
      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
        .set("ENCTYPE","multipart/form-data").set("readonly")
        .set("value",m_confParams.bag.settingsFile.toString()) << std::endl;
      
      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
        .set("name", "command").set("title", "Configure threshold scan.")
        .set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectMultipleVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
        .set("name", "command").set("title", "Start threshold scan.")
        .set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectMultipleVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop threshold scan.")
	.set("value", "Stop") << std::endl;
      *out << cgicc::form()   << std::endl;
    }
    
    *out << cgicc::comment() << "end the main commands, now putting the halt/reset commands" << cgicc::comment() << cgicc::br() << std::endl;
    *out << cgicc::span()  << std::endl
	 << "<table>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;
      
    //always should have a halt command
    *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Halt") << std::endl;
    
    *out << cgicc::input().set("type", "submit")
      .set("name", "command").set("title", "Halt threshold scan.")
      .set("value", "Halt") << std::endl;
    *out << cgicc::form() << std::endl
	 << "</td>" << std::endl;
    
    *out << "<td>"  << std::endl;

    if (!is_running_) {
      //comand that will take the system to initial and allow to change the hw device
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Reset") << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset device.")
	.set("value", "Reset") << std::endl;
      *out << cgicc::form() << std::endl;
    }
    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</table>" << std::endl
	 << cgicc::br() << std::endl
	 << cgicc::span()  << std::endl;

    *out << "</td>" << std::endl;

    *out << "<td>" << std::endl;
    if (is_initialized_)
      showBufferLayout(out);
    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl;
    
    *out << "</div>" << std::endl; //close control
    
    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl;//open countera
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;//close counters

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;//open fast commands
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl; //close fast commands

    *out << "</div>" << std::endl;    //</div> //close the new div xdaq-tab

    *out << cgicc::br() << cgicc::br() << std::endl;
    
    //*out << "<div class=\"xdaq-tab\" title=\"Status\">"  << std::endl
    //*out << cgicc::div().set("class","xdaq-tab").set("title","Status")   << std::endl
    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Program" << cgicc::th() << std::endl
	 << cgicc::th()    << "System"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << "<tbody>" << std::endl
	 << "<tr>"    << std::endl
	 << "<td>"    << std::endl;

    *out << "<table class=\"xdaq-table\">" << std::endl
	 << cgicc::thead() << std::endl
	 << cgicc::tr()    << std::endl //open
	 << cgicc::th()    << "Status" << cgicc::th() << std::endl
	 << cgicc::th()    << "Value"  << cgicc::th() << std::endl
	 << cgicc::tr()    << std::endl //close
	 << cgicc::thead() << std::endl 
      
	 << "<tbody>" << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_working_" << "</td>"
	 << "<td>" << is_working_   << "</td>"
	 << "</tr>"   << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_initialized_" << "</td>"
	 << "<td>" << is_initialized_   << "</td>"
	 << "</tr>"       << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_configured_" << "</td>"
	 << "<td>" << is_configured_   << "</td>"
	 << "</tr>"      << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << "is_running_" << "</td>"
	 << "<td>" << is_running_   << "</td>"
	 << "</tr>"   << std::endl

	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl
	 << "</td>"    << std::endl;
    
    *out  << "<td>"     << std::endl
	  << "<table class=\"xdaq-table\">" << std::endl
	  << cgicc::thead() << std::endl
	  << cgicc::tr()    << std::endl //open
	  << cgicc::th()    << "Device"     << cgicc::th() << std::endl
	  << cgicc::th()    << "Connected"  << cgicc::th() << std::endl
	  << cgicc::tr()    << std::endl //close
	  << cgicc::thead() << std::endl 
	  << "<tbody>" << std::endl;
    
    if (is_initialized_) {
      hw_semaphore_.take();
      hw_semaphore_.give();
    }
    
    *out << "</tbody>" << std::endl
	 << "</table>" << std::endl
	 << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << std::endl;

    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
      .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying GEMTBUtil control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    INFO("Something went wrong displaying GEMTBUtil control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::GEMTBUtil::webInitialize(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
    INFO( "debugging form entries");
    std::vector<cgicc::FormEntry>::const_iterator myiter = vfat2FormEntries.begin();

    //OH Devices
    cgicc::form_iterator oh = cgi.getElement("SetOH");
    if (strcmp((**oh).c_str(),"OH_0") == 0) {
      m_confParams.bag.ohGTXLink.value_= 0;
      INFO("OH_0 has been selected " << m_confParams.bag.ohGTXLink);
    }//if OH_0
    if (strcmp((**oh).c_str(),"OH_1") == 0) {
      m_confParams.bag.ohGTXLink.value_= 1;
      INFO("OH_1 has been selected " << m_confParams.bag.ohGTXLink);
    }//if OH_1

    for(int i = 0; i < 24; ++i) {    
      std::stringstream currentChipID;
      currentChipID << "VFAT" << i;

      std::stringstream form;
      form << "VFATDevice" << i;
      
      std::string tmpDeviceName = m_confParams.bag.deviceName[i].toString();
      //      std::string tmpDeviceName = "";
      cgicc::const_form_iterator name = cgi.getElement(form.str());
      if (name != cgi.getElements().end()) {
        INFO( "found form element::" << form.str());
        INFO( "has value::" << name->getValue());
	tmpDeviceName = name->getValue();
	m_confParams.bag.deviceName[i] = tmpDeviceName;
	INFO( "Web_deviceName::"             << m_confParams.bag.deviceName[i].toString());
	//	p_vfatDevice.push_back(m_confParams.bag.deviceName[i].toString());
      }
      
      int tmpDeviceNum = -1;
      tmpDeviceName.erase(0,4);
      tmpDeviceNum = atoi(tmpDeviceName.c_str());

      m_readout_mask = m_confParams.bag.ohGTXLink;

    }//end for
    

    //change the status to initializing and make sure the page displays this information
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(initSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::GEMTBUtil::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  wl_->submit(confSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::GEMTBUtil::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  wl_->submit(startSig_);
  
  redirect(in,out);
}

//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(stopSig_);
  
  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(haltSig_);
  
  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(resetSig_);
  
  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webResetCounters(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    INFO( "resetting counters entries");
    
    hw_semaphore_.take();
    
    INFO("GEMTBUtil::webResetCounters Reseting counters");
      
    if (cgi.queryCheckbox("RstL1ATTC") )
      p_optohybridDevice->resetL1ACount(0x0);
    
    if (cgi.queryCheckbox("RstL1AT1") ) 
      p_optohybridDevice->resetL1ACount(0x1);
    
    if (cgi.queryCheckbox("RstL1AExt") ) 
      p_optohybridDevice->resetL1ACount(0x2);

    if (cgi.queryCheckbox("RstL1Asbits") ) 
      p_optohybridDevice->resetL1ACount(0x3);

    if (cgi.queryCheckbox("RstL1AGEB") ) 
      p_optohybridDevice->resetL1ACount(0x4);

    if (cgi.queryCheckbox("RstCalPulseTTC") ) 
      p_optohybridDevice->resetCalPulseCount(0x0);

    if (cgi.queryCheckbox("RstCalPulseT1") ) 
      p_optohybridDevice->resetCalPulseCount(0x1);
    
    if (cgi.queryCheckbox("RstCalPulseExt") ) 
      p_optohybridDevice->resetCalPulseCount(0x2);
    
    if (cgi.queryCheckbox("RstCalPulseSbit") ) 
      p_optohybridDevice->resetCalPulseCount(0x3);

    if (cgi.queryCheckbox("RstCalPulseGEB") ) 
      p_optohybridDevice->resetCalPulseCount(0x4);
    
    if (cgi.queryCheckbox("RstResync") ) 
      p_optohybridDevice->resetResyncCount();
    
    if (cgi.queryCheckbox("RstBC0") ) 
      p_optohybridDevice->resetBC0Count();

    hw_semaphore_.give();

  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  hw_semaphore_.take();
  hw_semaphore_.give();
  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webSendFastCommands(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    INFO( "resetting counters entries");
    
    std::string fastCommand = cgi["SendFastCommand"]->getValue();
    
    if (strcmp(fastCommand.c_str(),"FlushFIFO") == 0) {
      INFO("FlushFIFO button pressed");
      hw_semaphore_.take();
      for (int i = 0; i < 2; ++i){
	p_glibDevice->flushFIFO(i);
      }
      hw_semaphore_.give();
    }

    if (strcmp(fastCommand.c_str(),"SendTestPackets") == 0) {
      INFO("SendTestPackets button pressed");
      hw_semaphore_.take();
      if (!is_running_) {
	for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip){
	  (*chip)->setRunMode(0x1);
	  (*chip)->sendTestPattern(0x1);
	  (*chip)->sendTestPattern(0x0);
	}
      }
      if (!is_running_) {
	for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
          (*chip)->setRunMode(0x0);
	}
      }
      hw_semaphore_.give();
      
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A+CalPulse") == 0) {
      INFO("Send L1A+CalPulse button pressed");
      cgicc::const_form_iterator element = cgi.getElement("CalPulseDelay");
      uint8_t delay;
      if (element != cgi.getElements().end())
	delay = element->getIntegerValue();
      hw_semaphore_.take();
      p_optohybridDevice->sendResync();
      for (unsigned int com = 0; com < 15; ++com)
	p_optohybridDevice->sendL1ACal(10, delay,1);
	hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A") == 0) {
      INFO("Send L1A button pressed");
      hw_semaphore_.take();
      p_optohybridDevice->sendL1A(10,1);
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send CalPulse") == 0) {
      INFO("Send CalPulse button pressed");
      hw_semaphore_.take();
      p_optohybridDevice->sendCalPulse(0x1,1);
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send Resync") == 0) {
      INFO("Send Resync button pressed");
      hw_semaphore_.take();
      p_optohybridDevice->sendResync();
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send BC0") == 0) {
      INFO("Send BC0 button pressed");
      hw_semaphore_.take();
      p_optohybridDevice->sendBC0();
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"SetTriggerSource") == 0) {
      INFO("SetTriggerSource button pressed");
      hw_semaphore_.take();
      cgicc::form_iterator fi = cgi.getElement("trgSrc");
      if( !fi->isEmpty() && fi != (*cgi).end()) {  
	if (strcmp((**fi).c_str(),"TTC_GLIB_trsSrc") == 0) {
	  m_confParams.bag.triggerSource = 0x0;
	  p_optohybridDevice->setTrigSource(0x0); 
	}
	else if (strcmp((**fi).c_str(),"T1_trgSrc") == 0) {
	  m_confParams.bag.triggerSource = 0x1;
	  p_optohybridDevice->setTrigSource(0x1);
	}
	else if (strcmp((**fi).c_str(),"Ext_trgSrc") == 0) {
	  m_confParams.bag.triggerSource = 0x2;
	  p_optohybridDevice->setTrigSource(0x2);
	}
	else if (strcmp((**fi).c_str(),"sbits_trgSrc") == 0) {
	  m_confParams.bag.triggerSource = 0x3;
	  p_optohybridDevice->setTrigSource(0x3);
	}
      }
      hw_semaphore_.give();
    }
    
    hw_semaphore_.take();
    hw_semaphore_.give();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  hw_semaphore_.take();
  hw_semaphore_.give();
  redirect(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::GEMTBUtil::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)

  hw_semaphore_.take();

  std::stringstream tmpURI;
  tmpURI << "chtcp-2.0://localhost:10203?target=" << m_confParams.bag.deviceIP.toString() << ":50001";

  p_glibDevice = glib_shared_ptr(new gem::hw::glib::HwGLIB("HwGLIB", tmpURI.str(),
                                                          "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

  /*    p_optohybridDevice = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid("HwOptoHybrid0", tmpURI.str(),
	"file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));*/

  std::string ohDeviceName = toolbox::toString("HwOptoHybrid%d",m_confParams.bag.ohGTXLink.value_);
  p_optohybridDevice = optohybrid_shared_ptr(new gem::hw::optohybrid::HwOptoHybrid(ohDeviceName, tmpURI.str(),
                                                                                  "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

  if (p_glibDevice->isHwConnected()) {
    INFO("GLIB device connected");
    if (p_optohybridDevice->isHwConnected()) {
      INFO("OptoHybrid device connected");

      for(int i=0;i<24;++i){
	//  int i=0;
	std::stringstream currentChipID;
	currentChipID << "VFAT" << i;
	
	std::string vfat;
	vfat=currentChipID.str();
	currentChipID.str("");

	vfat_shared_ptr tmpVFATDevice(new gem::hw::vfat::HwVFAT2(vfat, tmpURI.str(), "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"));

	if(tmpVFATDevice->isHwConnected()){
	tmpVFATDevice->setDeviceBaseNode(toolbox::toString("GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.%s",
							   m_confParams.bag.ohGTXLink.value_,
							   vfat.c_str()));
	

	  tmpVFATDevice->setDeviceIPAddress(m_confParams.bag.deviceIP);
	  tmpVFATDevice->setRunMode(0);
	  VFATdeviceConnected.push_back(tmpVFATDevice);
	  
	  std::string VfatName = m_confParams.bag.deviceName[i].toString();
	  if (VfatName != "") {
	    m_readout_mask = m_confParams.bag.ohGTXLink;
	    
	    INFO(" webConfigure : DeviceName " << VfatName );
	    INFO(" webConfigure : m_readout_mask 0x"  << std::hex << (int)m_readout_mask << std::dec );
	    
	    m_confParams.bag.deviceChipID = tmpVFATDevice->getChipID();
	    INFO(" CHIPID   :: " << m_confParams.bag.deviceChipID);      
	    // need to put all chips in sleep mode to start off
	    p_vfatDevice.push_back(tmpVFATDevice);
	  }//end if VfatName
	}//end for 

	for (auto chip = VFATdeviceConnected.begin(); chip != VFATdeviceConnected.end(); ++chip) {
	  if ((*chip)->isHwConnected()) {      
	    (*chip)->setRunMode(0);
	    int islot = slotInfo->GEBslotIndex( (uint32_t)((*chip)->getChipID()));
	    INFO( "vfatDevice Conected::" << islot);    
	  }
	}// end for  
	
	for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
	  int islot = slotInfo->GEBslotIndex( (uint32_t)((*chip)->getChipID()));
	  INFO( "vfatDevice selected::" << islot);    
	}	
      }// end for  
      
      //    }//end if vfat is connected	

   }//end if OH connected  
    else{
      INFO("OptoHybrid device not connected, breaking out");
      is_configured_  = false;
      is_working_     = false;    
      hw_semaphore_.give();
      return;
    }
        
  }// end if glib connected
  else {
    INFO("GLIB device not connected, breaking out");
    is_configured_  = false;
    is_working_     = false;    
    hw_semaphore_.give();
    return;
  }  

  
  is_initialized_ = true;
  hw_semaphore_.give();
      
  //sleep(5);
  is_working_     = false;
    
}


void gem::supervisor::tbutils::GEMTBUtil::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)

  hw_semaphore_.take();
  is_initialized_ = true;

  std::stringstream ss;
  auto num = m_confParams.bag.deviceNum.begin();
  for (auto chip = m_confParams.bag.deviceName.begin();
       chip != m_confParams.bag.deviceName.end(); ++chip, ++num) {
    ss << "Device name: " << chip->toString() << std::endl;
  }
  INFO(ss.str());


  hw_semaphore_.give();

  is_working_     = false;    

}


void gem::supervisor::tbutils::GEMTBUtil::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;

  //start scan routine
  wl_->submit(runSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::GEMTBUtil::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  if (is_running_) {
    hw_semaphore_.take();
    for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip) {
      (*chip)->setRunMode(0);
    }
    
    hw_semaphore_.give();
    is_running_ = false;
  }
  
  /*
  INFO("Closling file");
  scanStream->close();
  delete scanStream;
  scanStream = 0;*/
  wl_->submit(stopSig_);
  
  sleep(0.001);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::GEMTBUtil::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_    = true;
  is_configured_ = false;
  is_running_    = false;

  if (is_running_) {
    hw_semaphore_.take();
    /*int islot=0;
	for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip, ++islot) {
	(*chip)->setRunMode(0x0);
	}
*/
    hw_semaphore_.give();
  }
  is_running_ = false;

  is_configured_ = false;

  m_vfat = 0;
  m_event = 0;

  wl_->submit(haltSig_);
  
  //sleep(5);
  sleep(0.001);
  is_working_    = false;
}


void gem::supervisor::tbutils::GEMTBUtil::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  is_initialized_ = false;
  is_configured_  = false;
  is_running_     = false;

  hw_semaphore_.take();
  for (auto chip = p_vfatDevice.begin(); chip != p_vfatDevice.end(); ++chip)
    (*chip)->setRunMode(0x0);
  
  for (int i = 0; i < 24; ++i)
    m_confParams.bag.deviceName[i] = ""; // ensure that the selected chips are reset
  
  m_confParams.bag.ohGTXLink = 0; // reset this to 0    
  
  //sleep(2);
  hw_semaphore_.give();

  //reset parameters to defaults, allow to select new device
  m_confParams.bag.nTriggers = 2U;

  //  m_confParams.bag.deviceName   = "";
  m_confParams.bag.deviceChipID = 0x0;
  m_confParams.bag.triggersSeen = 0;
  m_confParams.bag.triggersSeenGLIB = 0;
  m_confParams.bag.triggercount = 0;
  m_confParams.bag.triggerSource = 0x9;

  wl_->submit(resetSig_);
  
  //sleep(5);
  sleep(0.001);
  is_working_     = false;
}


void gem::supervisor::tbutils::GEMTBUtil::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  //hw_semaphore_.take();
  ////p_vfatDevice->setRunMode(0);
  //hw_semaphore_.give();
}

void gem::supervisor::tbutils::GEMTBUtil::selectMultipleVFAT(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    bool isDisabled = false;
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = true;
    
    const int nChips = 24;
    *out << cgicc::table();
    *out << cgicc::tr();
    
    
    for(int i = 0; i < nChips; ++i) {
      std::stringstream currentChipID;
      currentChipID << "VFAT" << i;

      std::stringstream form;
      form << "VFATDevice" << i;
      
      std::string label = "primary";
      cgicc::input vfatselection;
      *out << cgicc::td() << std::endl;
      
      *out << "<span class=\"label label-primary\">" << currentChipID.str() << "</span>" << std::endl;

      
      if(isDisabled){
        vfatselection.set("type","checkbox").set("name",form.str()).set("disabled","disabled");
      }else{
        vfatselection.set("type","checkbox").set("name",form.str());
      }
      *out << ((m_confParams.bag.deviceName[i].toString().compare(currentChipID.str())) == 0 ?
	       vfatselection.set("checked","checked").set("multiple","multiple") :
	       vfatselection.set("value",currentChipID.str())) << std::endl;
      
      *out << cgicc::td() << std::endl;
      if( i == 7 || i == 15) {
	*out << cgicc::tr() << std::endl //close
	     << cgicc::tr() << std::endl;//open
	//  }// end if 
	
     }//end else



    }// end if nChips
    
    *out << cgicc::tr()    << std::endl;
    *out << cgicc::table() << std::endl;
    

  }

  catch (const xgi::exception::Exception& e) {
    INFO("Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    INFO("Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::GEMTBUtil::selectOptohybridDevice(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    bool isDisabled = false;
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = true;
    
    // cgicc::input OHselection;
    *out   << "<table>"     << std::endl
	   << "<tr>"   << std::endl //open
	   << "<td>" << "OH GTX Link " << "</td>" << std::endl	 
	   << "</tr>"     << std::endl //close 

	   << "<tr>" << std::endl //open
	   << "<td>" << std::endl; //open
    if (isDisabled)
      *out << cgicc::select().set("name","SetOH").set("disabled","disabled") 
	   << cgicc::option("OH_0").set("value","OH_0")
	   << cgicc::option("OH_1").set("value","OH_1")
	   << cgicc::select().set("disabled","disabled") << std::endl
	   << "</td>"    << std::endl
	   << "</tr>"    << std::endl
	   << "</table>" << std::endl;
    else
      *out << cgicc::select().set("name","SetOH") << std::endl
	   << cgicc::option("OH_0").set("value","OH_0")
	   << cgicc::option("OH_1").set("value","OH_1")
	   << cgicc::select()<< std::endl
	   << "</td>"    << std::endl
	   << "</tr>"    << std::endl
	   << "</table>" << std::endl;
	
    /*      *out << "<tr><td class=\"title\"> Select Latency Scan: </td>"
	    << "<td class=\"form\">"*/
    
  }//end try
catch (const xgi::exception::Exception& e) {
  INFO("Something went wrong setting the trigger source): " << e.what());
  XCEPT_RAISE(xgi::exception::Exception, e.what());
 }
 catch (const std::exception& e) {
   INFO("Something went wrong setting the trigger source): " << e.what());
   XCEPT_RAISE(xgi::exception::Exception, e.what());
 }

}// end void selectoptohybrid

void gem::supervisor::tbutils::GEMTBUtil::dumpRoutinesData(uint8_t const& m_readout_mask, uint8_t value, uint8_t VT1, uint8_t VT2, bool isLatencyScan)
{

  INFO(" GEMTBUtitls INSIDE DUMPROUTINES ");
  //    int latency_m, VT1_m, VT2_m;
  // or   int vcal_m, VT1_m, VT2_m;
  

  for(int j = 0; j < 5; j++) {
    INFO(" before GEMTBUtils counter " << j <<  " "<< m_counter[j] );
  }

  uint32_t* pDQ = p_gemDataParker->selectData(m_counter);
  if (pDQ) {
    m_counter[0] = *(pDQ+0); // VFAT blocks dumped to disk
    m_counter[1] = *(pDQ+1); // Events counter
    m_counter[2] = *(pDQ+2); // VFATs counter, number of VFATS chips in the last event
    m_counter[3] = *(pDQ+3); // good VFAT blocks dumped to file  
    m_counter[4] = *(pDQ+4); // bad VFAT blocks dumped to error file 
    //m_counter[5] = *(pDQ+5); //out of range?
  }

  for(int j = 0; j < 5; j++){
    INFO("GEMTBUtils counter " << j <<  " " << m_counter[j] );
  }
  
  INFO(" GEMTBUtils ntriggers "     <<   m_confParams.bag.triggersSeen );
  INFO(" GEMTBUtils ntotalcounter " <<   m_confParams.bag.triggercount );

  bool finish = true;  
  INFO(" queueDepth " <<  p_gemDataParker->queueDepth()  );
  /*
  if(is_running_){
    finish = true;
  }else if (p_gemDataParker->queueDepth() > 0){
    INFO("Data Parker still reading");
    finish = false;
  }else if(m_counter[1] != m_confParams.bag.triggercount){
    INFO("nTrigegrs are not equal to number of stored events");
    finish = false;
  }
  */
  if(finish){
    INFO("DUMP DATA");
    if(isLatencyScan)  p_gemDataParker->ScanRoutines(value, VT1, VT2, true);
    else  p_gemDataParker->ScanRoutines(value, VT1, VT2, false);
    uint32_t* pDupm = p_gemDataParker->dumpData(m_readout_mask);
    if(pDupm) {
      if(isLatencyScan)   INFO( " Latency = " << (int)value << " VT1 = " << (int)VT1 << " VT2 = " << (int)VT2);
      else  INFO( " VCal = " << (int)value << " VT1 = " << (int)VT1 << " VT2 = " << (int)VT2);
    }
  }else{
    INFO("------NOT DUMP DATA---------");
  }
}












