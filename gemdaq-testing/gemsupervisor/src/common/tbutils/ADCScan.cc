#include "gem/supervisor/tbutils/ADCScan.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include "TH1.h"
#include "TF1.h"
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

XDAQ_INSTANTIATOR_IMPL(gem::supervisor::tbutils::ADCScan)

void gem::supervisor::tbutils::ADCScan::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
  minDACValue = 0U;
  maxDACValue = 255U;
  stepSize    = 1U;

  time_t now  = time(0);
  tm    *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);
  std::string tmpFileName = "ADCScan_";
  tmpFileName.append(utcTime);
  tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
  tmpFileName.append(".dat");
  std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
  std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');
  //std::replace(tmpFileName.begin(), tmpFileName.end(), '\n', '_');

  outFileName  = tmpFileName;
  settingsFile = "${BUILD_HOME}/gemdaq-testing/gemhardware/xml/vfat/vfat_settings.xml";

  dacToScan = "IComp";

  deviceIP      = "192.168.0.115";
  deviceName    = "";
  deviceNum     = -1;

  deviceChipID  = 0x0;

  nSamples = 100;

  bag->addField("dacToScan",    &dacToScan);
  bag->addField("minDACValue",  &minDACValue);
  bag->addField("maxDACValue",  &maxDACValue);
  bag->addField("stepSize",     &stepSize );

  bag->addField("outFileName",   &outFileName );
  bag->addField("settingsFile",  &settingsFile);

  bag->addField("deviceName",   &deviceName  );
  bag->addField("deviceIP",     &deviceIP    );
  bag->addField("deviceNum",    &deviceNum   );
  bag->addField("deviceChipID", &deviceChipID);
  bag->addField("nSamples",     &nSamples);
}

gem::supervisor::tbutils::ADCScan::ADCScan(xdaq::ApplicationStub * s)
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
  is_working_     (false),
  is_initialized_ (false),
  is_configured_  (false),
  is_running_     (false),
  vfatDevice_(0)
{

  curDACRegValue = 0;
  curDACValue    = 0;
  // Detect when the setting of default parameters has been performed
  this->getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  getApplicationInfoSpace()->fireItemAvailable("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemAvailable("ipAddr",     &ipAddr_);

  getApplicationInfoSpace()->fireItemValueRetrieve("confParams", &confParams_);
  getApplicationInfoSpace()->fireItemValueRetrieve("ipAddr",     &ipAddr_);

  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webDefault,      "Default"    );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webInitialize,   "Initialize" );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webConfigure,    "Configure"  );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webStart,        "Start"      );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webStop,         "Stop"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webHalt,         "Halt"       );
  xgi::framework::deferredbind(this, this, &gem::supervisor::tbutils::ADCScan::webReset,        "Reset"      );
  
  xoap::bind(this, &gem::supervisor::tbutils::ADCScan::onInitialize,  "Initialize",  XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ADCScan::onConfigure,   "Configure",   XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ADCScan::onStart,       "Start",       XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ADCScan::onStop,        "Stop",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ADCScan::onHalt,        "Halt",        XDAQ_NS_URI);
  xoap::bind(this, &gem::supervisor::tbutils::ADCScan::onReset,       "Reset",       XDAQ_NS_URI);
  
  initSig_  = toolbox::task::bind(this, &ADCScan::initialize, "initialize");
  confSig_  = toolbox::task::bind(this, &ADCScan::configure,  "configure" );
  startSig_ = toolbox::task::bind(this, &ADCScan::start,      "start"     );
  stopSig_  = toolbox::task::bind(this, &ADCScan::stop,       "stop"      );
  haltSig_  = toolbox::task::bind(this, &ADCScan::halt,       "halt"      );
  resetSig_ = toolbox::task::bind(this, &ADCScan::reset,      "reset"     );
  runSig_   = toolbox::task::bind(this, &ADCScan::run,        "run"       );

  fsmP_ = new toolbox::fsm::AsynchronousFiniteStateMachine("GEMTestBeamADCScan");
  
  fsmP_->addState('I', "Initial",     this, &gem::supervisor::tbutils::ADCScan::stateChanged);
  fsmP_->addState('H', "Halted",      this, &gem::supervisor::tbutils::ADCScan::stateChanged);
  fsmP_->addState('C', "Configured",  this, &gem::supervisor::tbutils::ADCScan::stateChanged);
  fsmP_->addState('E', "Running",     this, &gem::supervisor::tbutils::ADCScan::stateChanged);
  
  fsmP_->setStateName('F', "Error");
  fsmP_->setFailedStateTransitionAction(this,  &gem::supervisor::tbutils::ADCScan::transitionFailed);
  fsmP_->setFailedStateTransitionChanged(this, &gem::supervisor::tbutils::ADCScan::stateChanged);
  
  fsmP_->addStateTransition('I', 'H', "Initialize", this, &gem::supervisor::tbutils::ADCScan::initializeAction);
  fsmP_->addStateTransition('H', 'C', "Configure",  this, &gem::supervisor::tbutils::ADCScan::configureAction);
  fsmP_->addStateTransition('C', 'C', "Configure",  this, &gem::supervisor::tbutils::ADCScan::configureAction);
  fsmP_->addStateTransition('C', 'E', "Start",      this, &gem::supervisor::tbutils::ADCScan::startAction);
  fsmP_->addStateTransition('E', 'C', "Stop",       this, &gem::supervisor::tbutils::ADCScan::stopAction);
  fsmP_->addStateTransition('C', 'H', "Halt",       this, &gem::supervisor::tbutils::ADCScan::haltAction);
  fsmP_->addStateTransition('E', 'H', "Halt",       this, &gem::supervisor::tbutils::ADCScan::haltAction);
  fsmP_->addStateTransition('H', 'H', "Halt",       this, &gem::supervisor::tbutils::ADCScan::haltAction);
  fsmP_->addStateTransition('C', 'I', "Reset",      this, &gem::supervisor::tbutils::ADCScan::resetAction);
  fsmP_->addStateTransition('H', 'I', "Reset",      this, &gem::supervisor::tbutils::ADCScan::resetAction);

  // Define invalid transitions, too, so that they can be ignored, or else FSM will be unhappy when one is fired.
  fsmP_->addStateTransition('E', 'E', "Configure", this, &gem::supervisor::tbutils::ADCScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Start"    , this, &gem::supervisor::tbutils::ADCScan::noAction);
  fsmP_->addStateTransition('E', 'E', "Start"    , this, &gem::supervisor::tbutils::ADCScan::noAction);
  fsmP_->addStateTransition('H', 'H', "Stop"     , this, &gem::supervisor::tbutils::ADCScan::noAction);
  fsmP_->addStateTransition('C', 'C', "Stop"     , this, &gem::supervisor::tbutils::ADCScan::noAction);


  fsmP_->setInitialState('I');
  fsmP_->reset();
  
  dacMap["IPreampIn"]   = std::pair<std::string,std::string>("Current", "IPreampIn"  );
  dacMap["IPreampFeed"] = std::pair<std::string,std::string>("Current", "IPreampFeed");
  dacMap["IPreampOut"]  = std::pair<std::string,std::string>("Current", "IPreampOut" );
  dacMap["IShaper"]     = std::pair<std::string,std::string>("Current", "IShaper"    );
  dacMap["IShaperFeed"] = std::pair<std::string,std::string>("Current", "IShaperFeed");
  dacMap["IComp"]       = std::pair<std::string,std::string>("Current", "IComp"      );

  dacMap["VThreshold1"] = std::pair<std::string,std::string>("Voltage", "VThreshold1");
  dacMap["VThreshold2"] = std::pair<std::string,std::string>("Voltage", "VThreshold2");
  dacMap["VCal"]        = std::pair<std::string,std::string>("Voltage", "VCal"       );

  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ADCScan","waiting");
  wl_->activate();

}

gem::supervisor::tbutils::ADCScan::~ADCScan()
  
{
  wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("urn:xdaq-workloop:GEMTestBeamSupervisor:ADCScan","waiting");
  //should we check to see if it's running and try to stop?
  wl_->cancel();
  wl_ = 0;
  
  //if (histo) 
  //  histo->Delete();
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


void gem::supervisor::tbutils::ADCScan::actionPerformed(xdata::Event& event)
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

void gem::supervisor::tbutils::ADCScan::fireEvent(const std::string& name)
{
  toolbox::Event::Reference event((new toolbox::Event(name, this)));  
  fsmP_->fireEvent(event);
}

void gem::supervisor::tbutils::ADCScan::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //keep_refresh_ = false;
  
  LOG4CPLUS_INFO(getApplicationLogger(),"Current state is: [" << fsm.getStateName (fsm.getCurrentState()) << "]");
  std::string state_=fsm.getStateName (fsm.getCurrentState());
  
  LOG4CPLUS_INFO(getApplicationLogger(), "StateChanged: " << (std::string)state_);
  
}

void gem::supervisor::tbutils::ADCScan::transitionFailed(toolbox::Event::Reference event)
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
bool gem::supervisor::tbutils::ADCScan::initialize(toolbox::task::WorkLoop* wl)
{
  fireEvent("Initialize");
  return false; //do once?
}

bool gem::supervisor::tbutils::ADCScan::configure(toolbox::task::WorkLoop* wl)
{
  fireEvent("Configure");
  return false; //do once?
}

bool gem::supervisor::tbutils::ADCScan::start(toolbox::task::WorkLoop* wl)
{
  fireEvent("Start");
  return false;
}

bool gem::supervisor::tbutils::ADCScan::stop(toolbox::task::WorkLoop* wl)
{
  fireEvent("Stop");
  return false; //do once?
}

bool gem::supervisor::tbutils::ADCScan::halt(toolbox::task::WorkLoop* wl)
{
  fireEvent("Halt");
  return false; //do once?
}

bool gem::supervisor::tbutils::ADCScan::reset(toolbox::task::WorkLoop* wl)
{
  fireEvent("Reset");
  return false; //do once?
}

bool gem::supervisor::tbutils::ADCScan::run(toolbox::task::WorkLoop* wl)
{

  wl_semaphore_.take();
  if (!is_running_) {
    wl_semaphore_.give();
    wl_->submit(stopSig_);
    return false;
  }
  
  if (samplesTaken_ < confParams_.bag.nSamples) {
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFAT_ADC");
    LOG4CPLUS_DEBUG(getApplicationLogger(), "trying to read the "
		   << dacMap[confParams_.bag.dacToScan.toString()].first
		   << " ADC for "
		   << confParams_.bag.dacToScan.toString());
    curDACValue = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),dacMap[confParams_.bag.dacToScan.toString()].first);
    
    ++samplesTaken_;
    vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
    hw_semaphore_.give();

    histo->Fill((unsigned)curDACRegValue, curDACValue);
    
    wl_semaphore_.give();
    return true;
  }
  else {
    //move to the next point
    std::string imgRoot = "${XDAQ_DOCUMENT_ROOT}/gemdaq/gemsupervisor/html/images/tbutils/dacscan/"+confParams_.bag.deviceName.toString()+"_";
    std::stringstream ss;
    ss << confParams_.bag.dacToScan.toString() << "_scan.png";
    std::string imgName = ss.str();
    //do a fit here to project the height of the image at the end
    TF1* imgFit = new TF1("pol1","pol1",
			  confParams_.bag.minDACValue-0.5,
			  confParams_.bag.maxDACValue+0.5);
    outputCanvas->cd();
    histo->Draw("colz");
    histo->Fit(imgFit,"QN");
    double projVal = imgFit->Eval(confParams_.bag.maxDACValue);
    LOG4CPLUS_INFO(getApplicationLogger(),"projected value a last step " << projVal);
    histo->SetMaximum(1.2*projVal);
    //histo->GetYaxis()->SetRangeUser(0., 1.2*projVal);
    outputCanvas->cd();
    //histo->SetMarkerStyle(23);
    //histo->SetMarkerSize(2);
    //outputCanvas->DrawFrame(-0.5,254.5,-0.5,1.2*projVal);
    histo->Draw("colz");
    outputCanvas->Update();
    outputCanvas->SaveAs(TString(imgRoot+imgName));
    delete imgFit;
    
    samplesTaken_ = 0;

    if (curDACRegValue < confParams_.bag.maxDACValue) {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      if ((curDACRegValue + confParams_.bag.stepSize) < 0xFF) 
	vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),confParams_.bag.dacToScan.toString(),curDACRegValue + confParams_.bag.stepSize);
      else  
	vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),confParams_.bag.dacToScan.toString(),0xFF);
      curDACRegValue = vfatDevice_->readVFATReg(confParams_.bag.dacToScan.toString());
      //vfatDevice_->setRunMode(0);
      hw_semaphore_.give();
      wl_semaphore_.give();
      return true;
    }
    else {
      hw_semaphore_.take();
      vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
      hw_semaphore_.give();
      wl_semaphore_.give();
      wl_->submit(stopSig_);
      return false;
    }
  }
}

// SOAP interface
xoap::MessageReference gem::supervisor::tbutils::ADCScan::onInitialize(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(initSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ADCScan::onConfigure(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(confSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ADCScan::onStart(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(startSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ADCScan::onStop(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(stopSig_);

  return message;
}


xoap::MessageReference gem::supervisor::tbutils::ADCScan::onHalt(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(haltSig_);

  return message;
}

xoap::MessageReference gem::supervisor::tbutils::ADCScan::onReset(xoap::MessageReference message)
  throw (xoap::exception::Exception) {
  is_working_ = true;

  wl_->submit(resetSig_);

  return message;
}

void gem::supervisor::tbutils::ADCScan::selectVFAT(xgi::Output *out)
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


void gem::supervisor::tbutils::ADCScan::scanParameters(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    std::string isDisabled = "";
    std::string isReadonly = "";
    if (is_configured_) {
      isDisabled = "disabled";
      isReadonly = "readonly";
    }

    *out << cgicc::span()   << std::endl
	 << cgicc::label("DAC to Scan").set("for","DACToScan") << std::endl
	 << cgicc::select().set("id","DACToScan").set("name","DACToScan")
	 << ((confParams_.bag.dacToScan.toString().compare("IPreampIn")) == 0 ?
	     (cgicc::option("IPreampIn").set(isDisabled).set("value","IPreampIn").set("selected")) :
	     (cgicc::option("IPreampIn").set(isDisabled).set("value","IPreampIn"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("IPreampFeed")) == 0 ?
	     (cgicc::option("IPreampFeed").set(isDisabled).set("value","IPreampFeed").set("selected")) :
	     (cgicc::option("IPreampFeed").set(isDisabled).set("value","IPreampFeed"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("IPreampOut")) == 0 ?
	     (cgicc::option("IPreampOut").set(isDisabled).set("value","IPreampOut").set("selected")) :
	     (cgicc::option("IPreampOut").set(isDisabled).set("value","IPreampOut"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("IShaper")) == 0 ?
	     (cgicc::option("IShaper").set(isDisabled).set("value","IShaper").set("selected")) :
	     (cgicc::option("IShaper").set(isDisabled).set("value","IShaper"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("IShaperFeed")) == 0 ?
	     (cgicc::option("IShaperFeed").set(isDisabled).set("value","IShaperFeed").set("selected")) :
	     (cgicc::option("IShaperFeed").set(isDisabled).set("value","IShaperFeed"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("IComp")) == 0 ?
	     (cgicc::option("IComp").set(isDisabled).set("value","IComp").set("selected")) :
	     (cgicc::option("IComp").set(isDisabled).set("value","IComp"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("VThreshold1")) == 0 ?
	     (cgicc::option("VThreshold1").set(isDisabled).set("value","VThreshold1").set("selected")) :
	     (cgicc::option("VThreshold1").set(isDisabled).set("value","VThreshold1"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("VThreshold2")) == 0 ?
	     (cgicc::option("VThreshold2").set(isDisabled).set("value","VThreshold2").set("selected")) :
	     (cgicc::option("VThreshold2").set(isDisabled).set("value","VThreshold2"))) << std::endl
	 << ((confParams_.bag.dacToScan.toString().compare("VCal")) == 0 ?
	     (cgicc::option("VCal").set(isDisabled).set("value","VCal").set("selected")) :
	     (cgicc::option("VCal").set(isDisabled).set("value","VCal"))) << std::endl
	 << cgicc::select() << std::endl

	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("MinDACValue").set("for","MinDACValue") << std::endl
	 << cgicc::input().set("id","MinDACValue").set("name","MinDACValue")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.minDACValue)))
	 << std::endl

	 << cgicc::label("MaxDACValue").set("for","MaxDACValue") << std::endl
	 << cgicc::input().set("id","MaxDACValue").set("name","MaxDACValue")
                          .set("type","number").set("min","0").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.maxDACValue)))
	 << std::endl
	 << cgicc::br() << std::endl
      
	 << cgicc::label("ReadDACReg").set("for","ReadDACReg") << std::endl
	 << cgicc::input().set("id","ReadDACReg").set("name","ReadDACReg")
                          .set("type","text").set("readonly")
                          .set("value",boost::str(boost::format("%d")%((unsigned)curDACRegValue)))
	 << std::endl

	 << cgicc::label("ADCOutValue").set("for","ADCOutValue") << std::endl
	 << cgicc::input().set("id","ADCOutValue").set("name","ADCOutValue")
                          .set("type","text").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(curDACValue)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("DACStep").set("for","DACStep") << std::endl
	 << cgicc::input().set("id","DACStep").set("name","DACStep")
                          .set("type","number").set("min","1").set("max","255")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.stepSize)))
	 << std::endl
	 << cgicc::br() << std::endl

	 << cgicc::label("Samples to take").set("for","SamplesToTake") << std::endl
	 << cgicc::input().set("id","SamplesToTake").set("name","SamplesToTake")
                          .set("type","number").set("min","0")
                          .set("value",boost::str(boost::format("%d")%(confParams_.bag.nSamples)))
	 << cgicc::br() << std::endl
	 << cgicc::label("Samples taken").set("for","SamplesTaken") << std::endl
	 << cgicc::input().set("id","SamplesTaken").set("name","SamplesTaken")
                          .set("type","number").set("min","0").set("readonly")
                          .set("value",boost::str(boost::format("%d")%(samplesTaken_)))
	 << cgicc::br() << std::endl

	 << cgicc::span()   << std::endl;
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


void gem::supervisor::tbutils::ADCScan::displayHistograms(xgi::Output *out)
  throw (xgi::exception::Exception)
{
  try {
    *out << cgicc::img().set("src","/gemdaq/gemsupervisor/html/images/tbutils/dacscan/"+
			     confParams_.bag.deviceName.toString()+"_"+
			     confParams_.bag.dacToScan.toString()+"_scan.png")
                        .set("name",confParams_.bag.deviceName.toString()+"_"+
			     confParams_.bag.dacToScan.toString()+"_scan.png")
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

void gem::supervisor::tbutils::ADCScan::redirect(xgi::Input *in, xgi::Output* out) {
  std::string redURL = "/" + getApplicationDescriptor()->getURN() + "/Default";
  *out << "<meta http-equiv=\"refresh\" content=\"0;" << redURL << "\">" << std::endl;
  this->webDefault(in,out);
}

// HyperDAQ interface
void gem::supervisor::tbutils::ADCScan::webDefault(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception)
{

  try {
    ////update the page refresh 
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      LOG4CPLUS_DEBUG(this->getApplicationLogger()," why do we need &head ?: " << &head);
      //head.addHeader("Refresh","2");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      LOG4CPLUS_DEBUG(this->getApplicationLogger()," why do we need &head ?: " << &head);
      //head.addHeader("Refresh","5");
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

    *out << "</td>"    << std::endl
	 << "</tr>"    << std::endl
	 << "</tbody>" << std::endl
	 << "</table>" << cgicc::br() << std::endl;
    
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

    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jquery/1/jquery.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","http://ajax.googleapis.com/ajax/libs/jqueryui/1/jquery-ui.min.js")
	 << cgicc::script() << std::endl;
    *out << cgicc::script().set("type","text/javascript")
                           .set("src","/gemdaq/gemsupervisor/html/scripts/tbutils/adcScanImage.js")
	 << cgicc::script() << std::endl;
  }
  catch (const xgi::exception::Exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ADCScan control panel(xgi): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception& e) {
    LOG4CPLUS_INFO(this->getApplicationLogger(),"Something went wrong displaying ADCScan control panel(std): " << e.what());
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


void gem::supervisor::tbutils::ADCScan::webInitialize(xgi::Input *in, xgi::Output *out)
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


void gem::supervisor::tbutils::ADCScan::webConfigure(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    //aysen's xml parser
    confParams_.bag.settingsFile = cgi.getElement("xmlFilename")->getValue();
    
    cgicc::const_form_iterator element = cgi.getElement("DACToScan");
    if (element != cgi.getElements().end())
      confParams_.bag.dacToScan   = element->getValue();

    element = cgi.getElement("MinDACValue");
    if (element != cgi.getElements().end())
      confParams_.bag.minDACValue = element->getIntegerValue();
    
    element = cgi.getElement("MaxDACValue");
    if (element != cgi.getElements().end())
      confParams_.bag.maxDACValue = element->getIntegerValue();

    element = cgi.getElement("DACStep");
    if (element != cgi.getElements().end())
      confParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("SamplesToTake");
    if (element != cgi.getElements().end())
      confParams_.bag.nSamples  = element->getIntegerValue();
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


void gem::supervisor::tbutils::ADCScan::webStart(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {

  try {
    cgicc::Cgicc cgi(in);
    
    cgicc::const_form_iterator element = cgi.getElement("DACToScan");
    if (element != cgi.getElements().end())
      confParams_.bag.dacToScan   = element->getValue();

    element = cgi.getElement("MinDACValue");
    if (element != cgi.getElements().end())
      confParams_.bag.minDACValue = element->getIntegerValue();
    
    element = cgi.getElement("MaxDACValue");
    if (element != cgi.getElements().end())
      confParams_.bag.maxDACValue = element->getIntegerValue();

    element = cgi.getElement("DACStep");
    if (element != cgi.getElements().end())
      confParams_.bag.stepSize  = element->getIntegerValue();
        
    element = cgi.getElement("SamplesToTake");
    if (element != cgi.getElements().end())
      confParams_.bag.nSamples  = element->getIntegerValue();
  }
  catch (const xgi::exception::Exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
  catch (const std::exception & e) {
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

  wl_->submit(startSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ADCScan::webStop(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(stopSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ADCScan::webHalt(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(haltSig_);
  
  redirect(in,out);
}


void gem::supervisor::tbutils::ADCScan::webReset(xgi::Input *in, xgi::Output *out)
  throw (xgi::exception::Exception) {
  wl_->submit(resetSig_);
  
  redirect(in,out);
}


// State transitions
//is initialize different than halt? they come from different positions but put the software/hardware in the same state 'halted'
void gem::supervisor::tbutils::ADCScan::initializeAction(toolbox::Event::Reference e)
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
  //don't disturb the current device state until configure
  //vfatDevice_->setRunMode(0);
  confParams_.bag.deviceChipID = vfatDevice_->getChipID();
  is_initialized_ = true;
  hw_semaphore_.give();

  //sleep(5);
  is_working_     = false;
}


void gem::supervisor::tbutils::ADCScan::configureAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  stepSize_    = confParams_.bag.stepSize;
  minDACValue_ = confParams_.bag.minDACValue;
  maxDACValue_ = confParams_.bag.maxDACValue;
  
  hw_semaphore_.take();
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
  
  vfatDevice_->setLatency(     12);
  vfatDevice_->setVThreshold1( 25);
  vfatDevice_->setVThreshold2(  0);
  //}
  
  LOG4CPLUS_DEBUG(getApplicationLogger(),"trying to get an enum from ::" << confParams_.bag.dacToScan.toString());
  vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(confParams_.bag.dacToScan.toString())));
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),confParams_.bag.dacToScan.toString(),
			confParams_.bag.minDACValue);
  curDACRegValue = vfatDevice_->readVFATReg(confParams_.bag.dacToScan.toString());

  is_configured_ = true;
  hw_semaphore_.give();
  
  //if (histo) 
  //  histo->Delete();
  if (histo) 
    delete histo;
  histo = 0;
  
  TString histName  = confParams_.bag.dacToScan.toString();
  TString histTitle = confParams_.bag.dacToScan.toString()+"DAC scan";
  int minVal = confParams_.bag.minDACValue;
  int maxVal = confParams_.bag.maxDACValue;
  int nBins = (maxVal - minVal +1)/(confParams_.bag.stepSize);
  //((max-min)+1)/stepSize+1
  histo = new TH2F(histName, histTitle, nBins, minVal-0.5, maxVal+0.5, 1024, -0.5, 1023.5);

  outputCanvas = new TCanvas("outputCanvas","outputCanvas",600,800);
  
  is_working_    = false;
}


void gem::supervisor::tbutils::ADCScan::startAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {
  
  is_working_ = true;

  samplesTaken_ = 0;

  time_t now = time(0);
  // convert now to string form
  //char* dt = ctime(&now);
  
  tm *gmtm = gmtime(&now);
  char* utcTime = asctime(gmtm);

  std::string tmpFileName = "ADCScan_";
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
  
  vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(confParams_.bag.dacToScan.toString())));
  vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),confParams_.bag.dacToScan.toString(),
			confParams_.bag.minDACValue);
  curDACRegValue = vfatDevice_->readVFATReg(confParams_.bag.dacToScan.toString());
  
  vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName.toString());
  vfatDevice_->setRunMode(1);
  hw_semaphore_.give();

  //start readout

  scanStream.close();

  //if (histo) 
  //  histo->Delete();
  if (histo) 
    delete histo;
  histo = 0;
  
  TString histName  = confParams_.bag.dacToScan.toString();
  TString histTitle = confParams_.bag.dacToScan.toString()+"DAC scan";
  int minVal = confParams_.bag.minDACValue;
  int maxVal = confParams_.bag.maxDACValue;
  int nBins = (maxVal - minVal +1)/(confParams_.bag.stepSize);
  //((max-min)+1)/stepSize+1
  histo = new TH2F(histName, histTitle, nBins, minVal-0.5, maxVal+0.5, 1024, -0.5, 1023.5);

  //start scan routine
  wl_->submit(runSig_);
  
  is_working_ = false;
}


void gem::supervisor::tbutils::ADCScan::stopAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = true;
  wl_semaphore_.take();
  if (is_running_) {
    hw_semaphore_.take();
    vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(confParams_.bag.dacToScan.toString())));
    vfatDevice_->setRunMode(0);
    hw_semaphore_.give();
    is_running_ = false;
  }

  //here we delete the histogram, so it should be created at start
  //if (histo) 
  //  histo->Delete();
  if (histo) 
    delete histo;
  histo = 0;
  
  wl_semaphore_.give();
  is_working_ = false;
}


void gem::supervisor::tbutils::ADCScan::haltAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  is_configured_ = false;
  is_running_    = false;

  //here we delete the histogram, so it should be created at start
  if (histo)
    delete histo;
  histo = 0;

  hw_semaphore_.take();
  vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));
  //vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(confParams_.bag.dacToScan.toString())));
  vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
  
  //wl_->submit(haltSig_);
  
  //sleep(5);
  wl_semaphore_.give();
  is_working_    = false;
}


void gem::supervisor::tbutils::ADCScan::resetAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  wl_semaphore_.take();
  is_working_ = true;

  is_initialized_ = false;
  is_configured_  = false;
  is_running_     = false;

  hw_semaphore_.take();
  vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at("OFF"));
  //vfatDevice_->setDACMode(gem::hw::vfat::StringToDACMode.at(boost::to_upper_copy(confParams_.bag.dacToScan.toString())));
  vfatDevice_->setRunMode(0);

  if (vfatDevice_->isHwConnected())
    vfatDevice_->releaseDevice();
  
  if (vfatDevice_)
    delete vfatDevice_;
  
  vfatDevice_ = 0;
  sleep(2);
  hw_semaphore_.give();

  confParams_.bag.nSamples    = 100U;
  confParams_.bag.minDACValue = 0U;
  confParams_.bag.maxDACValue = 25U;
  confParams_.bag.stepSize    = 1U;

  confParams_.bag.deviceName   = "";
  confParams_.bag.deviceChipID = 0x0;
  
  //wl_->submit(resetSig_);
  
  //sleep(5);
  wl_semaphore_.give();
  is_working_     = false;
}


void gem::supervisor::tbutils::ADCScan::noAction(toolbox::Event::Reference e)
  throw (toolbox::fsm::exception::Exception) {

  is_working_ = false;
  hw_semaphore_.take();
  //vfatDevice_->setRunMode(0);
  hw_semaphore_.give();
}

