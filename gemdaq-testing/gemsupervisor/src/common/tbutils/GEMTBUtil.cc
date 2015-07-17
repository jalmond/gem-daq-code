#include "gem/supervisor/tbutils/GEMTBUtil.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TError.h"

#include <algorithm>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"

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
  settingsFile = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/vfat/vfat_settings.xml";

  deviceIP      = "192.168.0.164";
  deviceName    = "";
  deviceNum     = -1;
  triggerSource = 0x0;
  deviceChipID  = 0x0;

  triggersSeen = 0;
  ADCVoltage = 0;
  ADCurrent = 0;

  bag->addField("readoutDelay", &readoutDelay);
  bag->addField("nTriggers",    &nTriggers);

  bag->addField("outFileName",  &outFileName );
  bag->addField("settingsFile", &settingsFile);

  bag->addField("deviceName",   &deviceName  );
  bag->addField("deviceIP",     &deviceIP    );
  bag->addField("deviceNum",    &deviceNum   );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("triggersSeen", &triggersSeen);
  bag->addField("ADCVoltage",   &ADCVoltage);
  bag->addField("ADCurrent",    &ADCurrent);

}

gem::supervisor::tbutils::GEMTBUtil::GEMTBUtil(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception) :
  xdaq::WebApplication(s),
  fsmP_(0),
  wl_semaphore_(toolbox::BSem::FULL),
  hw_semaphore_(toolbox::BSem::FULL),
  initSig_ (0),
  confSig_ (0),
  startSig_(0),
  stopSig_ (0),
  haltSig_ (0),
  resetSig_(0),
  runSig_  (0),
  readSig_ (0),
  //deviceName_(""),
  //deviceChipID_(0x0),
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false),
  vfatDevice_(0)
{
  gErrorIgnoreLevel = kWarning;
  
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemAvailable("ipAddr",     &ipAddr_);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("ipAddr",     &ipAddr_);

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

  fsmP_ = new toolbox::fsm::AsynchronousFiniteStateMachine("GEMTestBeamGEMTBUtil");
  
  fsmP_->addState('I', "Initial",     this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  fsmP_->addState('H', "Halted",      this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  fsmP_->addState('C', "Configured",  this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  fsmP_->addState('E', "Running",     this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  
  fsmP_->setStateName('F', "Error");
  fsmP_->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::GEMTBUtil::transitionFailed);
  fsmP_->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::GEMTBUtil::stateChanged);
  
  fsmP_->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::GEMTBUtil::initializeAction);
  fsmP_->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::GEMTBUtil::configureAction);
  fsmP_->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::GEMTBUtil::configureAction);
  fsmP_->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::GEMTBUtil::startAction);
  fsmP_->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::GEMTBUtil::stopAction);
  fsmP_->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  fsmP_->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  fsmP_->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::GEMTBUtil::haltAction);
  fsmP_->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::GEMTBUtil::resetAction);
  fsmP_->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::GEMTBUtil::resetAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsmP_->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);
  fsmP_->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::GEMTBUtil::noAction);

  fsmP_->setInitialState('I');
  fsmP_->reset();

  /*
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:GEMTBUtil","waiting");
  wl_->activate();
  */
  
}

gem::supervisor::tbutils::GEMTBUtil::~GEMTBUtil()
  
{
  /*
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:GEMTBUtil","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
  */
  
  LOG4CPLUS_INFO(getApplicationLogger(),"histo = 0x" << std::hex << histo << std::dec);
  if (histo)
    delete histo;
  histo = 0;

  for (int hi = 0; hi < 128; ++hi) {
    LOG4CPLUS_INFO(getApplicationLogger(),"histos[" << hi << "] = 0x" << std::hex << histos[hi] << std::dec);
    if (histos[hi])
      delete histos[hi];
    histos[hi] = 0;
  }

  LOG4CPLUS_INFO(getApplicationLogger(),"outputCanvas = 0x" << std::hex << outputCanvas << std::dec);
  if (outputCanvas)
    delete outputCanvas;
  outputCanvas = 0;
  
  //if (scanStream) {
  //  if (scanStream->is_open())
  //    scanStream->close();
  //  delete scanStream;
  //}
  //scanStream = 0;

  if (fsmP_)
    delete fsmP_;
  fsmP_ = 0;
  
}


void gem::supervisor::tbutils::GEMTBUtil::actionPerformed(xdata::Event& event)
{
  // This is called after all default configuration values have been
  // loaded (from the XDAQ configuration file).
  if (event.type() == "urn:xdaq-event:setDefaultValues") {
    std::stringstream ss;
    ss << "ipAddr_=[" << ipAddr_.toString() << "]" << std::endl;
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), ss.str());
    confParams_.bag.deviceIP = ipAddr_;
  }
}

void gem::supervisor::tbutils::GEMTBUtil::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));  
  fsmP_->fireEvent(event);
}

void gem::supervisor::tbutils::GEMTBUtil::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;
  
  LOG4CPLUS_INFO(getApplicationLogger(),"Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state_=fsm.getStateName (fsm.getCurrentState());
  
  LOG4CPLUS_INFO(getApplicationLogger(), "StateChanged: " << (std::string)state_);
  
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
  
  LOG4CPLUS_ERROR(getApplicationLogger(), reason.str());
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

////////////
void gem::supervisor::tbutils::GEMTBUtil::selectVFAT(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    std::string isDisabled = "";
    if (is_running_ || is_configured_ || is_initialized_)
      isDisabled = "disabled";
    
    LOG4CPLUS_DEBUG(getApplicationLogger(),"selected device is: "<<confParams_.bag.deviceName.toString());
    *out << cgicc::span() << std::endl
	 << "<table>"     << std::endl
	 << "<tr>"   << std::endl
	 << "<td>" << "Selected VFAT:" << "</td>" << std::endl
	 << "<td>" << "ChipID:"        << "</td>" << std::endl
	 << "</tr>"     << std::endl

	 << "<tr>" << std::endl
	 << "<td>" << std::endl
	 << cgicc::select().set("id","VFATDevice").set("name","VFATDevice")     << std::endl
      //here we should have all VFATs, all VFATs with disconnected ones greyed out, or all connected VFATs
      // the software shouldn't try to connect to an unavailable VFAT
	 << ((confParams_.bag.deviceName.toString().compare("VFAT8")) == 0 ?
	     (cgicc::option("VFAT8").set(isDisabled).set("value","VFAT8").set("selected")) :
	     (cgicc::option("VFAT8").set(isDisabled).set("value","VFAT8"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT9")) == 0 ?
	     (cgicc::option("VFAT9").set(isDisabled).set("value","VFAT9").set("selected")) :
	     (cgicc::option("VFAT9").set(isDisabled).set("value","VFAT9"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT10")) == 0 ?
	     (cgicc::option("VFAT10").set(isDisabled).set("value","VFAT10").set("selected")) :
	     (cgicc::option("VFAT10").set(isDisabled).set("value","VFAT10"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT11")) == 0 ?
	     (cgicc::option("VFAT11").set(isDisabled).set("value","VFAT11").set("selected")) :
	     (cgicc::option("VFAT11").set(isDisabled).set("value","VFAT11"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT12")) == 0 ?
	     (cgicc::option("VFAT12").set(isDisabled).set("value","VFAT12").set("selected")) :
	     (cgicc::option("VFAT12").set(isDisabled).set("value","VFAT12"))) << std::endl

	 << ((confParams_.bag.deviceName.toString().compare("VFAT13")) == 0 ?
	     (cgicc::option("VFAT13").set(isDisabled).set("value","VFAT13").set("selected")) :
	     (cgicc::option("VFAT13").set(isDisabled).set("value","VFAT13"))) << std::endl
	 << cgicc::select()<< std::endl
	 << "</td>" << std::endl
      
	 << "<td>" << std::endl
	 << cgicc::input().set("type","text").set("id","ChipID")
                          .set("name","ChipID").set("readonly")
                          .set("value",boost::str(boost::format("0x%04x")%(confParams_.bag.deviceChipID)))
	 << std::endl
	 << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</table>" << std::endl
	 << cgicc::span()  << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying VFATS(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::GEMTBUtil::showCounterLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/ResetCounters") << std::endl;
      
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
      //*out << cgicc::div().set("class","xdaq-tab").set("title","Counters")   << std::endl
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
	   << cgicc::td()    << "External"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.External") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1AExt")
	                                      .set("name","RstL1AExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Internal"    << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.Internal") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1AInt")
	                                      .set("name","RstL1AInt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Delayed"     << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.Delayed" ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1ADel")
	                                      .set("name","RstL1ADel")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Total"       << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"L1A.Total"   ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstL1ATot")
	                                      .set("name","RstL1ATot")
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
	   << cgicc::td()    << "External"  << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"CalPulse.External") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstCalPulseExt")
	                                      .set("name","RstCalPulseExt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Internal"  << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"CalPulse.Internal") << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstCalPulseInt")
	                                      .set("name","RstCalPulseInt")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "Total"     << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"CalPulse.Total"   ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstCalPulseTot")
	                                      .set("name","RstCalPulseTot")
	   << cgicc::td() << std::endl
	   << "</tr>"     << std::endl
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
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"Resync" ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstResync")
	                                      .set("name","RstResync")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()    << "BC0"       << cgicc::td() << std::endl
	   << cgicc::td()    << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"BC0"    ) << cgicc::td() << std::endl
	   << cgicc::td()    << cgicc::input().set("type","checkbox")
	                                      .set("id","RstBC0")
	                                      .set("name","RstBC0")
	   << cgicc::td()    << std::endl
	   << "</tr>" << std::endl

	   << "<tr>" << std::endl
	   << cgicc::td()  << "BXCount"   << cgicc::td() << std::endl
	   << cgicc::td()  << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"BXCount") << cgicc::td() << std::endl
	   << cgicc::td()  << "" << cgicc::td() << std::endl
	   << "</tr>"      << std::endl
	   << "</tbody>"   << std::endl
	   << "</table>"   << std::endl
	   << "</td>"      << std::endl
	   << "</tr>"      << std::endl
	   << cgicc::tbody() << std::endl
	   << "</table>"   << std::endl;

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();

      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Reset counters.")
	.set("value", "ResetCounters") << std::endl;

      *out << cgicc::form() << std::endl;
      
    }
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showCounterLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showCounterLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
} //end showCounterLayout


void gem::supervisor::tbutils::GEMTBUtil::showBufferLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands") << std::endl;
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
      *out << cgicc::label("FIFOOcc").set("for","FIFOOcc") << std::endl
	   << cgicc::input().set("id","FIFOOcc").set("name","FIFOOcc").set("type","text")
	                    .set("value",boost::str( boost::format("%d")%(
									 vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"TRK_FIFO.DEPTH")
									 ))) << std::endl;

      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
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
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showBufferLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying showBufferLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
} //end showBufferLayout


void gem::supervisor::tbutils::GEMTBUtil::fastCommandLayout(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    if (is_initialized_ && vfatDevice_) {

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/FastCommands") << std::endl;
      
      //hw_semaphore_.take();
      //vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
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
                            .set("id","GLIBsrc").set("value","GLIB")
	                    .set((unsigned)confParams_.bag.triggerSource == (unsigned)0x0 ? "checked" : "")

	   << cgicc::label("GLIB").set("for","GLIBSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc")
	                    .set("id","ExtSrc").set("value","Ext")
                            .set((unsigned)confParams_.bag.triggerSource == (unsigned)0x1 ? "checked" : "")
	   << cgicc::label("Ext (LEMO)").set("for","ExtSrc") << std::endl
	   << cgicc::br()
	   << cgicc::input().set("type","radio").set("name","trgSrc").set("checked")
                            .set("id","BothSrc").set("value","Both")
                            .set((unsigned)confParams_.bag.triggerSource == (unsigned)0x2 ? "checked" : "")
	   << cgicc::label("Both").set("for","BothSrc") << std::endl
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
	                .set("value",confParams_.bag.deviceNum.toString())
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
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying fastCommandLayout(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying fastCommandLayout(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
}


void gem::supervisor::tbutils::GEMTBUtil::displayHistograms(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  //needs to be explicitly defined in the derived class
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

      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Initialize hardware acces.")
	.set("value", "Initialize") << std::endl;

      *out << cgicc::form() << std::endl;
    }
    
    else if (!is_configured_) {
      //this will allow the parameters to be set to the chip and scan routine

      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Configure") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      //adding aysen's xml parser
      //std::string setConfFile = toolbox::toString("/%s/setConfFile",getApplicationDescriptor()->getURN().c_str());
      //*out << cgicc::form().set("method","POST").set("action",setConfFile) << std::endl ;
      
      *out << cgicc::input().set("type","text").set("name","xmlFilename").set("size","80")
 	                    .set("ENCTYPE","multipart/form-data").set("readonly")
                            .set("value",confParams_.bag.settingsFile.toString()) << std::endl;
      //*out << cgicc::input().set("type","submit").set("value","Set configuration file") << std::endl ;
      //*out << cgicc::form() << std::endl ;
      
      *out << cgicc::br() << std::endl;
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Configure threshold scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start threshold scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectVFAT(out);
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
    
    *out << "</div>" << std::endl;
    
    *out << "<div class=\"xdaq-tab\" title=\"Counters\">"  << std::endl;
    if (is_initialized_)
      showCounterLayout(out);
    *out << "</div>" << std::endl;

    *out << "<div class=\"xdaq-tab\" title=\"Fast Commands/Trigger Setup\">"  << std::endl;
    if (is_initialized_)
      fastCommandLayout(out);
    *out << "</div>" << std::endl;

    //place new div class=xdaq-tab here to hold the histograms
    /*
      display a single histogram and have a form that selects which channel you want to display
      use the file name of the histogram that is saved in readFIFO
    */
    *out << "<div class=\"xdaq-tab\" title=\"Channel histograms\">"  << std::endl;
    displayHistograms(out);
    
    *out << "</div>" << std::endl;
    *out << "</div>" << std::endl;
    //</div> //close the new div xdaq-tab

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
    
    if (is_initialized_ && vfatDevice_) {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("TEST");
      *out << "<tr>" << std::endl
	   << "<td>" << "GLIB" << "</td>"
	   << "<td>" << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"GLIB") << "</td>"
	   << "</tr>"   << std::endl
	
	   << "<tr>" << std::endl
	   << "<td>" << "OptoHybrid" << "</td>"
	   << "<td>" << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"OptoHybrid") << "</td>"
	   << "</tr>"       << std::endl
	
	   << "<tr>" << std::endl
	   << "<td>" << "VFATs" << "</td>"
	   << "<td>" << vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"VFATs") << "</td>"
	   << "</tr>"      << std::endl;
      
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
    *out << "</tbody>" << std::endl
	 << "</table>" << std::endl
	 << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << std::endl;
      //<< "</div>"   << std::endl;

    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","/gemdaq/gemsupervisor/html/scripts/tbutils/changeImage.js")
	 << cgicc::script() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying GEMTBUtil control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying GEMTBUtil control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::GEMTBUtil::webInitialize(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
    LOG4CPLUS_DEBUG(getApplicationLogger(), "debugging form entries");
    std::vector<cgicc::FormEntry>::const_iterator myiter = vfat2FormEntries.begin();
    
    std::string tmpDeviceName = "";
    cgicc::const_form_iterator name = cgi.getElement("VFATDevice");
    if (name != cgi.getElements().end())
      tmpDeviceName = name->getValue();

    //std::string tmpDeviceName = cgi["VFATDevice"]->getValue();
    LOG4CPLUS_DEBUG(getApplicationLogger(), "deviceName_::"             << confParams_.bag.deviceName.toString());
    LOG4CPLUS_DEBUG(getApplicationLogger(), "setting deviceName_ to ::" << tmpDeviceName);
    confParams_.bag.deviceName = tmpDeviceName;
    LOG4CPLUS_DEBUG(getApplicationLogger(), "deviceName_::"             << confParams_.bag.deviceName.toString());
    
    int tmpDeviceNum = -1;
    tmpDeviceName.erase(0,4);
    tmpDeviceNum = atoi(tmpDeviceName.c_str());
    tmpDeviceNum -= 8;
    LOG4CPLUS_DEBUG(getApplicationLogger(), "deviceNum_::"             << confParams_.bag.deviceNum.toString());
    LOG4CPLUS_DEBUG(getApplicationLogger(), "setting deviceNum_ to ::" << tmpDeviceNum);
    confParams_.bag.deviceNum = tmpDeviceNum;
    LOG4CPLUS_DEBUG(getApplicationLogger(), "deviceNum_::"             << confParams_.bag.deviceNum.toString());
    
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
    LOG4CPLUS_INFO(getApplicationLogger(), "resetting counters entries");
    
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.RESETS");

    if (cgi.queryCheckbox("RstL1AExt") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.External",0x1);
    if (cgi.queryCheckbox("RstL1AInt") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.Internal",0x1);
    if (cgi.queryCheckbox("RstL1ADel") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.Delayed",0x1);
    if (cgi.queryCheckbox("RstL1ATot") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"L1A.Total",0x1);

    if (cgi.queryCheckbox("RstCalPulseExt") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CalPulse.External",0x1);
    if (cgi.queryCheckbox("RstCalPulseInt") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CalPulse.Internal",0x1);
    if (cgi.queryCheckbox("RstCalPulseTot") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CalPulse.Total",0x1);

    if (cgi.queryCheckbox("RstResync") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Resync",0x1);
    if (cgi.queryCheckbox("RstBC0") ) 
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"BC0",0x1);

    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
  redirect(in,out);
}


//no need to redefine in the derived class
void gem::supervisor::tbutils::GEMTBUtil::webSendFastCommands(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> resetCounters = cgi.getElements();
    LOG4CPLUS_INFO(getApplicationLogger(), "resetting counters entries");
    
    std::string fastCommand = cgi["SendFastCommand"]->getValue();
    
    if (strcmp(fastCommand.c_str(),"FlushFIFO") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"FlushFIFO button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TRK_FIFO.FLUSH",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    if (strcmp(fastCommand.c_str(),"SendTestPackets") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"SendTestPackets button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      if (!is_running_) 
	vfatDevice_->setRunMode(0x1);
      vfatDevice_->sendTestPattern(0x1);
      //sleep(1);
      vfatDevice_->sendTestPattern(0x0);
      if (!is_running_) 
	vfatDevice_->setRunMode(0x0);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A+CalPulse") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send L1A+CalPulse button pressed");
      cgicc::const_form_iterator element = cgi.getElement("CalPulseDelay");
      uint8_t delay;
      if (element != cgi.getElements().end())
	delay = element->getIntegerValue();
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      for (unsigned int com = 0; com < 15; ++com)
	vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.L1ACalPulse",delay);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send L1A") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send L1A button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.L1A",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send CalPulse") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send CalPulse button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.CalPulse",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send Resync") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send Resync button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.Resync",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"Send BC0") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"Send BC0 button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.BC0",0x1);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }

    else if (strcmp(fastCommand.c_str(),"SetTriggerSource") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"SetTriggerSource button pressed");
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
      
      cgicc::form_iterator fi = cgi.getElement("trgSrc");
      if( !fi->isEmpty() && fi != (*cgi).end()) {  
	if (strcmp((**fi).c_str(),"GLIB") == 0) {
	  confParams_.bag.triggerSource = 0x0;
	  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"SOURCE",0x0);
	}
	else if (strcmp((**fi).c_str(),"Ext") == 0) {
	  confParams_.bag.triggerSource = 0x1;
	  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"SOURCE",0x1);
	}
	else if (strcmp((**fi).c_str(),"Both") == 0) {
	  confParams_.bag.triggerSource = 0x2;
	  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"SOURCE",0x2);
	}
      }
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
    else if (strcmp(fastCommand.c_str(),"SBitSelect") == 0) {
      LOG4CPLUS_INFO(this->getApplicationLogger(),"SBitSelect button pressed");
      uint32_t value = cgi["SBitSelect"]->getIntegerValue();
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",value);
      vfatDevice_->setDeviceBaseNode("GLIB");
      vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",value);
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
    }
    
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  hw_semaphore_.take();
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  hw_semaphore_.give();
  redirect(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::GEMTBUtil::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  //Need to know which device to connnect to here...
  //dropdown list from the web interface?
  //deviceName_ = "CMS_hybrid_J44";
  //here the connection to the device should be made
  setLogLevelTo(uhal::Debug());  // Set uHAL logging level Debug (most) to Error (least)
  hw_semaphore_.take();
  vfatDevice_ = new gem::hw::vfat::HwVFAT2(confParams_.bag.deviceName.toString());
  
  //vfatDevice_->setAddressTableFileName("allregsnonfram.xml");
  //vfatDevice_->setDeviceBaseNode("user_regs.vfats."+confParams_.bag.deviceName.toString());
  vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
  vfatDevice_->setDeviceIPAddress(confParams_.bag.deviceIP);
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //sleep(1);
  vfatDevice_->connectDevice();
  
  //read in default parameters from an xml file?
  //vfatDevice_->setRegisters(xmlFile);
  vfatDevice_->readVFAT2Counters();
  vfatDevice_->setRunMode(0);
  confParams_.bag.deviceChipID = vfatDevice_->getChipID();
  is_initialized_ = true;
  hw_semaphore_.give();

  //sleep(5);
  is_working_     = false;
}


void gem::supervisor::tbutils::GEMTBUtil::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  
  is_working_    = false;
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
    vfatDevice_->setRunMode(0);
    hw_semaphore_.give();
    is_running_ = false;
  }
  
  LOG4CPLUS_INFO(getApplicationLogger(),"histo = 0x" << std::hex << histo << std::dec);
  if (histo)
    delete histo;
  histo = 0;
  
  for (int hi = 0; hi < 128; ++hi) {
    LOG4CPLUS_INFO(getApplicationLogger(),"histos[" << hi << "] = 0x" << std::hex << histos[hi] << std::dec);
    if (histos[hi])
      delete histos[hi];
    histos[hi] = 0;
  }
  //if (scanStream->is_open())
  //  LOG4CPLUS_INFO(getApplicationLogger(),"Closling file");
  //scanStream->close();
  //delete scanStream;
  //scanStream = 0;
  
  //wl_->submit(stopSig_);
  //wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:GEMTBUtil","waiting");
  //wl_->cancel();
  sleep(0.001);
  is_working_ = false;
}


void gem::supervisor::tbutils::GEMTBUtil::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;

  if (is_running_) {
    hw_semaphore_.take();
    vfatDevice_->setRunMode(0);
    hw_semaphore_.give();
  }
  is_running_ = false;

  is_configured_ = false;

  LOG4CPLUS_INFO(getApplicationLogger(),"histo = 0x" << std::hex << histo << std::dec);
  if (histo)
    delete histo;
  histo = 0;
  
  for (int hi = 0; hi < 128; ++hi) {
    LOG4CPLUS_INFO(getApplicationLogger(),"histos[" << hi << "] = 0x" << std::hex << histos[hi] << std::dec);
    if (histos[hi])
      delete histos[hi];
    histos[hi] = 0;
  }

  //hw_semaphore_.take();
  //vfatDevice_->setRunMode(0);
  //hw_semaphore_.give();
  
  //wl_->submit(haltSig_);
  
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
  vfatDevice_->setRunMode(0);

  if (vfatDevice_->isHwConnected())
    vfatDevice_->releaseDevice();
  
  if (vfatDevice_)
    delete vfatDevice_;
  
  vfatDevice_ = 0;
  //sleep(2);
  hw_semaphore_.give();

  //reset parameters to defaults, allow to select new device
  confParams_.bag.nTriggers = 1000U;

  confParams_.bag.deviceName   = "";
  confParams_.bag.deviceChipID = 0x0;
  confParams_.bag.triggersSeen = 0;
  
  //wl_->submit(resetSig_);
  
  //sleep(5);
  sleep(0.001);
  is_working_     = false;
}


void gem::supervisor::tbutils::GEMTBUtil::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  //hw_semaphore_.take();
  ////vfatDevice_->setRunMode(0);
  //hw_semaphore_.give();
}

