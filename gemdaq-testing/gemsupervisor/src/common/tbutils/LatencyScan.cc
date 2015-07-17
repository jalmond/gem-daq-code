#include "gem/supervisor/tbutils/LatencyScan.h"

#include "gem/hw/vfat/HwVFAT2.h"

#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include <algorithm>
#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "cgicc/HTTPRedirectHeader.h"

#include "gem/supervisor/tbutils/VFAT2XMLParser.h"

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::LatencyScan)

void gem::supervisor::tbutils::LatencyScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  minLatency    = 1U;
  maxLatency    = 5U;
  stepSize      = 1U;
  triggerSource = 1U;

  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);
  std::string tmpFileName = "LatencyScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  //std::replace(tmpFileName.begin(), tmpFileName.end(), '\n', '_');

  outFileName  = tmpFileName;
  settingsFile = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/vfat/vfat_settings.xml";

  deviceIP      = "192.168.0.115";
  deviceName    = "";
  deviceNum     = -1;

  deviceChipID  = 0x0;

  nTriggers = 10;
  nEvents   = 2;

  bag->addField("minLatency",  &minLatency);
  bag->addField("maxLatency",  &maxLatency);
  bag->addField("stepSize",     &stepSize );

  bag->addField("triggerSource", &triggerSource );

  bag->addField("outFileName",   &outFileName );
  bag->addField("settingsFile",  &settingsFile);

  bag->addField("deviceName",   &deviceName  );
  bag->addField("deviceIP",     &deviceIP    );
  bag->addField("deviceNum",    &deviceNum   );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("nTriggers",    &nTriggers   );
  bag->addField("nEvents",      &nEvents     );
}

gem::supervisor::tbutils::LatencyScan::LatencyScan(xdaq::ApplicationStub * s)
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
  //deviceName_(""),
  //deviceChipID_(0x0),
  triggersSeen_(0),
  eventsSeen_(0),
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false),
  vfatDevice_(0)
{

  currentLatency_    = 0;
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemAvailable("ipAddr",     &ipAddr_);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("ipAddr",     &ipAddr_);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webInitialize,   "Initialize" );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webStart,        "Start"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webStop,         "Stop"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webHalt,         "Halt"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webReset,        "Reset"      );
  
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webResetCounters,   "ResetCounters");
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::LatencyScan::webSendFastCommands,"FastCommands" );
  
  xoap::bind(this, &gem::supervisor::tbutils::LatencyScan::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::LatencyScan::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::LatencyScan::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::LatencyScan::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::LatencyScan::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::LatencyScan::onReset,       "Reset",       XDAQ_NS_URI);
  
  initSig_  = toolbox::task::bind(this, &LatencyScan::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &LatencyScan::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &LatencyScan::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &LatencyScan::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &LatencyScan::halt,       "halt"      );
  resetSig_ = toolbox::task::bind(this, &LatencyScan::reset,      "reset"     );
  runSig_   = toolbox::task::bind(this, &LatencyScan::run,        "run"       );

  fsmP_ = new toolbox::fsm::AsynchronousFiniteStateMachine("GEMTestBeamLatencyScan");
  
  fsmP_->addState('I', "Initial",     this, &gem::supervisor::tbutils::LatencyScan::stateChanged);
  fsmP_->addState('H', "Halted",      this, &gem::supervisor::tbutils::LatencyScan::stateChanged);
  fsmP_->addState('C', "Configured",  this, &gem::supervisor::tbutils::LatencyScan::stateChanged);
  fsmP_->addState('E', "Running",     this, &gem::supervisor::tbutils::LatencyScan::stateChanged);
  
  fsmP_->setStateName('F', "Error");
  fsmP_->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::LatencyScan::transitionFailed);
  fsmP_->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::LatencyScan::stateChanged);
  
  fsmP_->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::LatencyScan::initializeAction);
  fsmP_->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::LatencyScan::configureAction);
  fsmP_->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::LatencyScan::configureAction);
  fsmP_->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::LatencyScan::startAction);
  fsmP_->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::LatencyScan::stopAction);
  fsmP_->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::LatencyScan::haltAction);
  fsmP_->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::LatencyScan::haltAction);
  fsmP_->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::LatencyScan::haltAction);
  fsmP_->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::LatencyScan::resetAction);
  fsmP_->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::LatencyScan::resetAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsmP_->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::LatencyScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::LatencyScan::noAction);
  fsmP_->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::LatencyScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::LatencyScan::noAction);
  fsmP_->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::LatencyScan::noAction);


  fsmP_->setInitialState('I');
  fsmP_->reset();
  
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  wl_->activate();

}

gem::supervisor::tbutils::LatencyScan::~LatencyScan()
  
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:LatencyScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
  
  if (histo) 
    delete histo;
  histo = 0;
  
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


void gem::supervisor::tbutils::LatencyScan::actionPerformed(xdata::Event& event)
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

void gem::supervisor::tbutils::LatencyScan::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));  
  fsmP_->fireEvent(event);
}

void gem::supervisor::tbutils::LatencyScan::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;
  
  LOG4CPLUS_INFO(getApplicationLogger(),"Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state_=fsm.getStateName (fsm.getCurrentState());
  
  LOG4CPLUS_INFO(getApplicationLogger(), "StateChanged: " << (std::string)state_);
  
}

void gem::supervisor::tbutils::LatencyScan::transitionFailed(toolbox::Event::Reference event)
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



//Actions
bool gem::supervisor::tbutils::LatencyScan::initialize(toolbox::task::WorkLoop* wl)
{
  fireEvent("Initialize");
  return false; //do once?
}

bool gem::supervisor::tbutils::LatencyScan::configure(toolbox::task::WorkLoop* wl)
{
  fireEvent("Configure");
  return false; //do once?
}

bool gem::supervisor::tbutils::LatencyScan::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
  return false;
}

bool gem::supervisor::tbutils::LatencyScan::stop(toolbox::task::WorkLoop* wl)
{
  fireEvent("Stop");
  return false; //do once?
}

bool gem::supervisor::tbutils::LatencyScan::halt(toolbox::task::WorkLoop* wl)
{
  fireEvent("Halt");
  return false; //do once?
}

bool gem::supervisor::tbutils::LatencyScan::reset(toolbox::task::WorkLoop* wl)
{
  fireEvent("Reset");
  return false; //do once?
}

bool gem::supervisor::tbutils::LatencyScan::run(toolbox::task::WorkLoop* wl)
{

  wl_semaphore_.take();
  if (!is_running_) {
    wl_semaphore_.give();
    return false;
  }
  
  //triggersSeen_ = 0;
  //eventsSeen_   = 0;
  //
  //hw_semaphore_.take();
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //vfatDevice_->setRunMode(1);
  //
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.RESETS.L1A");
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"External",0x1);
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Internal",0x1);
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Delayed", 0x1);
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Total",   0x1);
  //
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.Resync",0x1);
  //
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //hw_semaphore_.give();
  
  if (currentLatency_ <= confParams_.bag.maxLatency) {

    /*
    ////sbit way
    //while (triggersSeen_ < confParams_.bag.nTriggers) {
    if (triggersSeen_ < confParams_.bag.nTriggers) {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("GLIB.TRG_DATA");
      uint8_t trgWord = (vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"DATA"))&0x0000002F;
      if ((trgWord>>(confParams_.bag.deviceNum))&0x1)
	++eventsSeen_;
      vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.L1A");
      triggersSeen_ = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"Total");
      hw_semaphore_.give();
      wl_semaphore_.give();
      return true;
    }
    */
    if (triggersSeen_ < confParams_.bag.nTriggers) {
      bool hasData = true;
      while (hasData) {
	hw_semaphore_.take();
	vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.TRK_DATA.COL1");
	hasData = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"DATA_RDY");
	std::vector<uint32_t> data;
	if (hasData) {
	  for (int word = 0; word < 7; ++word) {
	    std::stringstream ss9;
	    ss9 << "DATA." << word;
	    data.push_back(vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),ss9.str()));
	  }
	  
	  uint32_t data1  = ((0x0000ffff & data.at(4)) << 16) | ((0xffff0000 & data.at(3)) >> 16);
	  uint32_t data2  = ((0x0000ffff & data.at(3)) << 16) | ((0xffff0000 & data.at(2)) >> 16);
	  uint32_t data3  = ((0x0000ffff & data.at(2)) << 16) | ((0xffff0000 & data.at(1)) >> 16);
	  uint32_t data4  = ((0x0000ffff & data.at(1)) << 16) | ((0xffff0000 & data.at(0)) >> 16);
	  
	  if (data1 || data2 || data3 || data4)
	    ++eventsSeen_;
	}
	vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.L1A");
	triggersSeen_ = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"Total");
	vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
	hw_semaphore_.give();
	wl_semaphore_.give();
	return true;
      }
      //hw_semaphore_.give();
    }

    LOG4CPLUS_INFO(getApplicationLogger(),"we've seen enough triggers, changing the latency");
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    vfatDevice_->setRunMode(0);
    if ((currentLatency_ + confParams_.bag.stepSize) < 0xFF) 
      vfatDevice_->setLatency(currentLatency_ + confParams_.bag.stepSize);
    else  
      vfatDevice_->setLatency(0xFF);

    currentLatency_ = vfatDevice_->getLatency();
    
    //flush fifo, reset l1a counter, send resync
    vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TRK_FIFO.FLUSH",0x1);
    
    //reset counters
    vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS.RESETS.L1A");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"External",0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Internal",0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Delayed", 0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Total",   0x1);
    
    vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.Resync",0x1);
  
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();
    
    histo->Fill((unsigned)currentLatency_,(eventsSeen_*1.0)/triggersSeen_);
    std::string imgName = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/latencyscan/"
      +confParams_.bag.deviceName.toString()+"_Latency_scan.png";
    outputCanvas->cd();
    histo->Draw("ep0");
    outputCanvas->Update();
    outputCanvas->SaveAs(TString(imgName));
   
    triggersSeen_ = 0;
    eventsSeen_   = 0;
    
    wl_semaphore_.give();
   
    return true;
  }
  else {
    LOG4CPLUS_INFO(getApplicationLogger(),"we've completed the scan");
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    vfatDevice_->setRunMode(0);
    hw_semaphore_.give();
    wl_semaphore_.give();
    wl_->submit(stopSig_);
    return false;
  }
}

//uint64_t gem::supervisor::tbutils::LatencyScan::countEvents(const int sBit)
//{
//  uint64_t nEvts = 0;
//  
//  hw_semaphore_.take();
//  vfatDevice_->setDeviceBaseNode("GLIB.TRG_DATA");
//  ////sbit way
//  while (true) {
//    uint8_t trgWord = (vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"DATA"))&0x0000002F;
//    if ((trgWord>>sBit)&0x1)
//      ++nEvts;
//  }
//  hw_semaphore_.give();
//
//  
//  return nEvts;
//}


// SOAP interface
xoap::MessageReference gem::supervisor::tbutils::LatencyScan::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(initSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::LatencyScan::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(confSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::LatencyScan::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(startSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::LatencyScan::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(stopSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::LatencyScan::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(haltSig_);

  return message;
}

xoap::MessageReference gem::supervisor::tbutils::LatencyScan::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(resetSig_);

  return message;
}

void gem::supervisor::tbutils::LatencyScan::selectVFAT(xgi::Output *out)
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


void gem::supervisor::tbutils::LatencyScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::span()   << std::endl
	 << cgicc::label("MinLatency").set("for","MinLatency") << std::endl
	 << cgicc::input().set("id","MinLatency").set("name","MinLatency")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.minLatency)))
	 << std::endl

	 << cgicc::label("MaxLatency").set("for","MaxLatency") << std::endl
	 << cgicc::input().set("id","MaxLatency").set("name","MaxLatency")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.maxLatency)))
	 << std::endl
	 << cgicc::br() << std::endl
      
	 << cgicc::label("Current Latency").set("for","CurrentLatency") << std::endl
	 << cgicc::input().set("id","CurrentLatency").set("name","CurrentLatency")
                          .set("type","text").set("readonly")
                          .set("value",boost::str(boost::format("%d")%((unsigned)currentLatency_)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("Latency Step").set("for","LatencyStep") << std::endl
	 << cgicc::input().set("id","LatencyStep").set("name","LatencyStep")
                          .set("type","number").set("min","1").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("Triggers to take").set("for","TriggersToTake") << std::endl
	 << cgicc::input().set("id","TriggersToTake").set("name","TriggersToTake")
                          .set("type","number").set("min","0")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.nTriggers)))
	 << cgicc::br() << std::endl
	 << cgicc::label("Triggers taken").set("for","TriggersSeen") << std::endl
	 << cgicc::input().set("id","TriggersSeen").set("name","TriggersSeen")
                          .set("type","text").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(triggersSeen_)))

	 << std::endl
	 << cgicc::label("Events with hits").set("for","EventsSeen") << std::endl
	 << cgicc::input().set("id","EventsSeen").set("name","EventsSeen")
                          .set("type","text").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(eventsSeen_)))
	 << cgicc::br()   << std::endl

	 << cgicc::span() << std::endl;
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


void gem::supervisor::tbutils::LatencyScan::showCounterLayout(xgi::Output *out)
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


void gem::supervisor::tbutils::LatencyScan::showBufferLayout(xgi::Output *out)
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


void gem::supervisor::tbutils::LatencyScan::fastCommandLayout(xgi::Output *out)
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
                            .set("value","25")
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


void gem::supervisor::tbutils::LatencyScan::displayHistograms(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::img().set("src","/gemdaq/gemsupervisor/html/images/tbutils/latencyscan/"+
			     confParams_.bag.deviceName.toString()+"_Latency_scan.png")
                        .set("id","vfatChannelHisto")
	 << cgicc::br()  << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying displayHistograms(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying displayHistograms(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void gem::supervisor::tbutils::LatencyScan::redirect(xgi::Input *in, xgi::Output* out) {
  std::string redURL = "/" + getApplicationDescriptor()->getURN() + "/Default";
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;
  this->webDefault(in,out);
}

// HyperDAQ interface
void gem::supervisor::tbutils::LatencyScan::webDefault(xgi::Input *in, xgi::Output *out)
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
      head.addHeader("Refresh","5");
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
	.set("name", "command").set("title", "Configure scan.")
	.set("value", "Configure") << std::endl;
      *out << cgicc::form()        << std::endl;
    }
    
    else if (!is_running_) {
      //hardware is initialized and configured, we can start the run
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Start") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Start scan.")
	.set("value", "Start") << std::endl;
      *out << cgicc::form()    << std::endl;
    }
    
    else if (is_running_) {
      *out << cgicc::form().set("method","POST").set("action", "/" + getApplicationDescriptor()->getURN() + "/Stop") << std::endl;
      
      selectVFAT(out);
      scanParameters(out);
      
      *out << cgicc::input().set("type", "submit")
	.set("name", "command").set("title", "Stop scan.")
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
      .set("name", "command").set("title", "Halt scan.")
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
    *out << "<div class=\"xdaq-tab\" title=\"Scan histogram\">"  << std::endl;
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
	 //<< "<tr>"    << std::endl
	 //<< "<td>" << "Status:"   << "</td>"
	 //<< "<td>" << "Value:"    << "</td>"
	 //<< "</tr>" << std::endl
      
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
	 //<< "<tr>"    << std::endl
	 //<< "<td>" << "Status:"   << "</td>"
	 //<< "<td>" << "Value:"    << "</td>"
	 //<< "</tr>" << std::endl
      
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

  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying LatencyScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying LatencyScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::LatencyScan::webInitialize(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  
  try {
    cgicc::Cgicc cgi(in);
    std::vector<cgicc::FormEntry> vfat2FormEntries = cgi.getElements();
    LOG4CPLUS_DEBUG(getApplicationLogger(), "debugging form entries");
    std::vector<cgicc::FormEntry>::const_iterator myiter = vfat2FormEntries.begin();
    
    //for (; myiter != vfat2FormEntries.end(); ++myiter ) {
    //  LOG4CPLUS_DEBUG(getApplicationLogger(), "form entry::" myiter->getName());
    //}

    
    std::string tmpDeviceName = "";
    cgicc::const_form_iterator name = cgi.getElement("VFATDevice");
    if (name != cgi.getElements().end())
      tmpDeviceName = name->getValue();

    confParams_.bag.deviceName = tmpDeviceName;
    
    int tmpDeviceNum = -1;
    tmpDeviceName.erase(0,4);
    tmpDeviceNum = atoi(tmpDeviceName.c_str());
    tmpDeviceNum -= 8;
    confParams_.bag.deviceNum = tmpDeviceNum;
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


void gem::supervisor::tbutils::LatencyScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();
    
    cgicc::const_form_iterator element  = cgi.getElement("MinLatency");
    if (element != cgi.getElements().end())
      confParams_.bag.minLatency = element->getIntegerValue();
    
    element = cgi.getElement("MaxLatency");
    if (element != cgi.getElements().end())
      confParams_.bag.maxLatency = element->getIntegerValue();

    element = cgi.getElement("LatencyStep");
    if (element != cgi.getElements().end())
      confParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("TriggersToTake");
    if (element != cgi.getElements().end())
      confParams_.bag.nTriggers  = element->getIntegerValue();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  
  wl_->submit(confSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::LatencyScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(startSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::LatencyScan::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(stopSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::LatencyScan::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(haltSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::LatencyScan::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(resetSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::LatencyScan::webResetCounters(xgi::Input *in, xgi::Output *out)
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


void gem::supervisor::tbutils::LatencyScan::webSendFastCommands(xgi::Input *in, xgi::Output *out)
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
      sleep(1);
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
      for (unsigned int pkt = 0; pkt < delay; ++pkt) {
	for (unsigned int com = 0; com < 15; ++com)
	  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.L1ACalPulse",com);
      }
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
void gem::supervisor::tbutils::LatencyScan::initializeAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;
  setLogLevelTo(uhal::Info());  // Set uHAL logging level Debug (most) to Error (least)
  hw_semaphore_.take();
  vfatDevice_ = new gem::hw::vfat::HwVFAT2(confParams_.bag.deviceName.toString());
  
  vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
  vfatDevice_->setDeviceIPAddress(confParams_.bag.deviceIP);
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  vfatDevice_->connectDevice();
  
  vfatDevice_->readVFAT2Counters();
  confParams_.bag.deviceChipID = vfatDevice_->getChipID();

  vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
  confParams_.bag.triggerSource = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"SOURCE");

  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());

  is_initialized_ = true;
  hw_semaphore_.give();

  is_working_     = false;
  wl_semaphore_.give();
}


void gem::supervisor::tbutils::LatencyScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;
  stepSize_   = confParams_.bag.stepSize;
  minLatency_ = confParams_.bag.minLatency;
  maxLatency_ = confParams_.bag.maxLatency;
  
  hw_semaphore_.take();
  LOG4CPLUS_INFO(getApplicationLogger(), "attempting to configure device");
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  //make sure device is not running
  vfatDevice_->setRunMode(0);

  /****unimplemented at the moment
  if ((confParams_.bag.settingsFile.toString()).rfind("xml") != std::string::npos) {
    LOG4CPLUS_INFO(getApplicationLogger(),"loading settings from XML file");
    gem::supervisor::tbutils::VFAT2XMLParser::VFAT2XMLParser theParser(confParams_.bag.settingsFile.toString(),
								       vfatDevice_);
    theParser.parseXMLFile();
  }
  */
  
  //else {
  LOG4CPLUS_INFO(getApplicationLogger(),"loading default settings");
  //default settings for the frontend
  vfatDevice_->setTriggerMode(    0x3); //set to S1 to S8
  vfatDevice_->setCalibrationMode(0x0); //set to normal
  vfatDevice_->setMSPolarity(     0x1); //negative
  vfatDevice_->setCalPolarity(    0x1); //negative
  
  vfatDevice_->setProbeMode(        0x0);
  vfatDevice_->setLVDSMode(         0x0);
  vfatDevice_->setHitCountCycleTime(0x0); //maximum number of bits
  
  vfatDevice_->setHitCountMode( 0x0);
  vfatDevice_->setMSPulseLength(0x3);
  vfatDevice_->setInputPadMode( 0x0);
  vfatDevice_->setTrimDACRange( 0x0);
  vfatDevice_->setBandgapPad(   0x0);
  vfatDevice_->sendTestPattern( 0x0);
  
  
  vfatDevice_->setIPreampIn(  168);
  vfatDevice_->setIPreampFeed(150);
  vfatDevice_->setIPreampOut(  80);
  vfatDevice_->setIShaper(    150);
  vfatDevice_->setIShaperFeed(100);
  vfatDevice_->setIComp(      120);
  
  vfatDevice_->setVThreshold1( 25);
  vfatDevice_->setVThreshold2(  0);
  //}
  
  LOG4CPLUS_INFO(getApplicationLogger(), "setting DAC mode to normal");
  vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));

  LOG4CPLUS_INFO(getApplicationLogger(), "setting starting latency value");
  vfatDevice_->setLatency(    confParams_.bag.minLatency);

  LOG4CPLUS_INFO(getApplicationLogger(), "reading back current latency value");
  currentLatency_ = vfatDevice_->getLatency();

  LOG4CPLUS_INFO(getApplicationLogger(), "device configured");
  is_configured_ = true;
  hw_semaphore_.give();
  
  //if (histo) 
  //  histo->Delete();
  //what happens at reconfigure? do I just make sure that other actions clean up the memory/
  //if (histo != 0) {
  //  LOG4CPLUS_INFO(getApplicationLogger(), "cleaning up pointers");
  //  delete histo;
  //  LOG4CPLUS_INFO(getApplicationLogger(), "deleted histo pointer");
  //}
  histo = 0;
  
  LOG4CPLUS_INFO(getApplicationLogger(), "setting up histogram");
  int minVal = confParams_.bag.minLatency;
  int maxVal = confParams_.bag.maxLatency;
  int nBins = (maxVal - minVal +1)/(confParams_.bag.stepSize);
  histo = new TH1D("LatencyScan", "Latency scan", nBins, minVal-0.5, maxVal+0.5);

  outputCanvas = new TCanvas("outputCanvas","outputCanvas",600,800);
  
  LOG4CPLUS_INFO(getApplicationLogger(), "configure routine completed");
  is_working_    = false;
  wl_semaphore_.give();
}


void gem::supervisor::tbutils::LatencyScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  wl_semaphore_.take();
  is_working_ = true;

  triggersSeen_ = 0;
  eventsSeen_   = 0;

  time_t now = time(0);
  // convert now to string form
  //char* dt = ctime(&now);
  
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "LatencyScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

  confParams_.bag.outFileName = tmpFileName;

  LOG4CPLUS_INFO(getApplicationLogger(),"Creating file " << confParams_.bag.outFileName.toString());
  //std::fstream scanStream(confParams_.bag.outFileName.c_str(),
  std::fstream scanStream(tmpFileName.c_str(),
			  std::ios::app | std::ios::binary);
  if (scanStream.is_open())
    LOG4CPLUS_DEBUG(getApplicationLogger(),"file " << confParams_.bag.outFileName.toString() << "opened");

  //write some global run information header
  
  //char data[128/8]
  is_running_ = true;
  hw_semaphore_.take();

  //set clock source
  vfatDevice_->setDeviceBaseNode("OptoHybrid.CLOCKING");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"VFAT.SOURCE",  0x0);
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"VFAT.FALLBACK",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CDCE.SOURCE",  0x0);
  //vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CDCE.FALLBACK",0x1);
  
  //send resync
  vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.Resync",0x1);

  //reset counters
  vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.External",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Internal",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Delayed", 0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Total",   0x1);
  
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.External",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.Internal",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.Total",   0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.Resync",0x1);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.BC0",   0x1);
  
  //set trigger source
  vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"SOURCE",   0x2);
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",(unsigned)confParams_.bag.deviceNum);
  
  vfatDevice_->setDeviceBaseNode("GLIB");
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",(unsigned)confParams_.bag.deviceNum);
  
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  vfatDevice_->setRunMode(1);
  hw_semaphore_.give();

  //start readout
  //////

  scanStream.close();

  is_working_ = false;
  //start scan routine
  //vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
  //for (unsigned int pkt = 0; pkt < delay; ++pkt) {
  //  for (unsigned int com = 0; com < 15; ++com)
  //    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.L1ACalPulse",com);
  wl_semaphore_.give();
  wl_->submit(runSig_);
}


void gem::supervisor::tbutils::LatencyScan::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;
  if (is_running_) {
    hw_semaphore_.take();
    vfatDevice_->setRunMode(0);
    is_running_ = false;
    hw_semaphore_.give();
  }

  if (histo) 
    delete histo;
  histo = 0;
  
  is_working_ = false;
  wl_semaphore_.give();
}


void gem::supervisor::tbutils::LatencyScan::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  is_configured_ = false;
  is_running_    = false;

  if (histo)
    delete histo;
  histo = 0;

  hw_semaphore_.take();
  vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
  
  is_working_    = false;
  wl_semaphore_.give();
}


void gem::supervisor::tbutils::LatencyScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  is_initialized_ = false;
  is_configured_  = false;
  is_running_     = false;

  hw_semaphore_.take();
  //vfatDevice_->setRunMode(0);

  if (vfatDevice_->isHwConnected())
    vfatDevice_->releaseDevice();
  
  if (vfatDevice_)
    delete vfatDevice_;
  
  vfatDevice_ = 0;

  sleep(1);
  hw_semaphore_.give();

  confParams_.bag.nTriggers  = 100U;
  confParams_.bag.nEvents    = 2U;
  confParams_.bag.minLatency = 0U;
  confParams_.bag.maxLatency = 25U;
  confParams_.bag.stepSize   = 1U;

  confParams_.bag.deviceName   = "";
  confParams_.bag.deviceChipID = 0x0;
  
  is_working_     = false;
  wl_semaphore_.give();
}


void gem::supervisor::tbutils::LatencyScan::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  hw_semaphore_.take();
  //vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
}

