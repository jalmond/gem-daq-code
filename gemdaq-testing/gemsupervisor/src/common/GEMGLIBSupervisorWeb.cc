#include "gem/supervisor/GEMGLIBSupervisorWeb.h"
#include "gem/readout/GEMDataParker.h"
#include "gem/hw/vfat/HwVFAT2.h"

#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

std::string VFATnum[24] = { "VFAT0", "VFAT1", "VFAT2", "VFAT3", "VFAT4", "VFAT5", "VFAT6", "VFAT7",
                            "VFAT8", "VFAT9", "VFAT10","VFAT11","VFAT12","VFAT13","VFAT14","VFAT15",
                            "VFAT16","VFAT17","VFAT18","VFAT19","VFAT20","VFAT21","VFAT22","VFAT23"};

  
XDAQ_INSTANTIATOR_IMPL(gem::supervisor::GEMGLIBSupervisorWeb)

void gem::supervisor::GEMGLIBSupervisorWeb::ConfigParams::registerFields(xdata::Bag<ConfigParams> *bag)
{
    latency   = 12U;

    outFileName  = "";
    outputType   = "Hex";
    deviceIP     = "192.168.0.164";

    /*
    VAFT Devices List with are on GEB
    */
    deviceName[9]  = (xdata::String)VFATnum[9];
    deviceName[10] = (xdata::String)VFATnum[10];
    deviceName[11] = (xdata::String)VFATnum[11];
    deviceName[12] = (xdata::String)VFATnum[12];
    deviceName[13] = (xdata::String)VFATnum[13];

    for (int i=0; i<24; i++) deviceNum[i] = -1;

    triggerSource = 0x0; // 0x2; 
    deviceChipID  = 0x0; 
    deviceVT1     = 0x0; 
    deviceVT2     = 0x0; 

    bag->addField("latency",       &latency );
    bag->addField("outputType",    &outputType );
    bag->addField("outFileName",   &outFileName );

    bag->addField("deviceName",    &deviceName[0] );
    bag->addField("deviceNum",     &deviceNum[0]  );

    bag->addField("deviceIP",      &deviceIP    );
    bag->addField("triggerSource", &triggerSource );
    bag->addField("deviceChipID",  &deviceChipID);
    bag->addField("deviceVT1",     &deviceVT1   );
    bag->addField("deviceVT2",     &deviceVT2   );

}

// Main constructor
gem::supervisor::GEMGLIBSupervisorWeb::GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s)
    throw (xdaq::exception::Exception):
        xdaq::WebApplication(s),
        wl_semaphore_(toolbox::BSem::FULL),
        hw_semaphore_(toolbox::BSem::FULL),
        is_working_ (false),
        is_initialized_ (false),
        is_configured_ (false),
        is_running_ (false)
{
    // HyperDAQ bindings
    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webDefault,     "Default");
    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webConfigure,   "Configure");
    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webStart,       "Start");
    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webStop,        "Stop");
    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webHalt,        "Halt");
    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::webTrigger,     "Trigger");

    xgi::framework::deferredbind(this, this, &gem::supervisor::GEMGLIBSupervisorWeb::setParameter,   "setParameter");

    // SOAP bindings
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onConfigure,   "Configure",   XDAQ_NS_URI);
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStart,       "Start",       XDAQ_NS_URI);
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onStop,        "Stop",        XDAQ_NS_URI);
    xoap::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::onHalt,        "Halt",        XDAQ_NS_URI);

    // Initiate and activate main workloop
    wl_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("GEMGLIBSupervisorWebWorkLoop", "waiting");
    wl_->activate();

    // Workloop bindings
    configure_signature_   = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction, "configureAction");
    start_signature_       = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction,     "startAction");
    stop_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction,      "stopAction");
    halt_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction,      "haltAction");
    run_signature_         = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::runAction,       "runAction");
    read_signature_        = toolbox::task::bind(this, &gem::supervisor::GEMGLIBSupervisorWeb::readAction,      "readAction");

    // Define FSM states
    fsm_.addState('I', "Initial",    this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
    fsm_.addState('H', "Halted",     this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
    fsm_.addState('C', "Configured", this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);
    fsm_.addState('R', "Running",    this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

    // Define error FSM state
    fsm_.setStateName('F', "Error");
    fsm_.setFailedStateTransitionAction(this, &gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed);
    fsm_.setFailedStateTransitionChanged(this, &gem::supervisor::GEMGLIBSupervisorWeb::stateChanged);

    // Define allowed FSM state transitions
    fsm_.addStateTransition('H', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
    fsm_.addStateTransition('H', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
    fsm_.addStateTransition('C', 'C', "Configure", this, &gem::supervisor::GEMGLIBSupervisorWeb::configureAction);
    fsm_.addStateTransition('C', 'R', "Start",     this, &gem::supervisor::GEMGLIBSupervisorWeb::startAction);
    fsm_.addStateTransition('C', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);
    fsm_.addStateTransition('R', 'C', "Stop",      this, &gem::supervisor::GEMGLIBSupervisorWeb::stopAction);
    fsm_.addStateTransition('R', 'H', "Halt",      this, &gem::supervisor::GEMGLIBSupervisorWeb::haltAction);

    // Define forbidden FSM state transitions
    fsm_.addStateTransition('R', 'R', "Configure" , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('H', 'H', "Start"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('R', 'R', "Start"     , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('H', 'H', "Stop"      , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);
    fsm_.addStateTransition('C', 'C', "Stop"      , this, &gem::supervisor::GEMGLIBSupervisorWeb::noAction);

    // Set initial FSM state and reset FSM
    fsm_.setInitialState('H');
    fsm_.reset();

    counter_ = 0;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onConfigure(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(configure_signature_);
    return message;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStart(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(start_signature_);
    return message;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onStop(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(stop_signature_);
    return message;
}

    xoap::MessageReference gem::supervisor::GEMGLIBSupervisorWeb::onHalt(xoap::MessageReference message)
throw (xoap::exception::Exception)
{
    is_working_ = true;

    wl_->submit(halt_signature_);
    return message;
}

// HyperDAQ interface
    void gem::supervisor::GEMGLIBSupervisorWeb::webDefault(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Define how often main web interface refreshes
    if (!is_working_ && !is_running_) {
    }
    else if (is_working_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","7");
    }
    else if (is_running_) {
      cgicc::HTTPResponseHeader &head = out->getHTTPResponseHeader();
      head.addHeader("Refresh","30");
    }

    // If we are in "Running" state, check if GLIB has any data available
    if (is_running_) wl_->submit(run_signature_);

    // Page title
    *out << cgicc::h1("GEM DAQ Supervisor")<< std::endl;

    // Choose DAQ type: Spy or Global
    *out << "DAQ type: " << cgicc::select().set("name", "runtype");
    *out << cgicc::option().set("value", "Spy").set("selected","") << "Spy" << cgicc::option();
    *out << cgicc::option().set("value", "Global") << "Global" << cgicc::option();
    *out << cgicc::select() << std::endl;
    *out << cgicc::input().set("type", "submit").set("name", "command").set("title", "Set DAQ type").set("value", "Set DAQ type") 
         << cgicc::br() << cgicc::br();

    *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;") << std::endl;
    std::string method = toolbox::toString("/%s/setParameter",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::legend("Set Hex/Binary of output") << cgicc::p() << std::endl;
    *out << cgicc::form().set("method","GET").set("action", method) << std::endl;
    *out << cgicc::input().set("type","text").set("name","value").set("value", confParams_.bag.outputType.toString())   << std::endl;
    *out << cgicc::input().set("type","submit").set("value","Apply")  << std::endl;
    *out << cgicc::form() << std::endl;
    *out << cgicc::fieldset();

    // Show current state, counter, output filename
    *out << "Current state: " << fsm_.getStateName(fsm_.getCurrentState()) << cgicc::br();
    *out << "Current counter: " << counter_ << " events dumped to disk"  << cgicc::br();
    *out << "Output filename: " << confParams_.bag.outFileName.toString() << cgicc::br();
    *out << "Output type: " << confParams_.bag.outputType.toString() << cgicc::br();

    // Table with action buttons
    *out << cgicc::table().set("border","0");

    // Row with action buttons
    *out << cgicc::tr();

    // Configure button
    *out << cgicc::td();
    std::string configureButton = toolbox::toString("/%s/Configure",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",configureButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Configure") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Start button
    *out << cgicc::td();
    std::string startButton = toolbox::toString("/%s/Start",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",startButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Start") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Stop button
    *out << cgicc::td();
    std::string stopButton = toolbox::toString("/%s/Stop",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",stopButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Stop") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Halt button
    *out << cgicc::td();
    std::string haltButton = toolbox::toString("/%s/Halt",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",haltButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Halt") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Send L1A signal
    *out << cgicc::td();
    std::string triggerButton = toolbox::toString("/%s/Trigger",getApplicationDescriptor()->getURN().c_str());
    *out << cgicc::form().set("method","GET").set("action",triggerButton) << std::endl ;
    *out << cgicc::input().set("type","submit").set("value","Send L1A") << std::endl ;
    *out << cgicc::form();
    *out << cgicc::td();

    // Finish row with action buttons
    *out << cgicc::tr();

    // Finish table with action buttons
    *out << cgicc::table();

}

    void gem::supervisor::GEMGLIBSupervisorWeb::setParameter(xgi::Input * in, xgi::Output * out ) 
throw (xgi::exception::Exception)
{   try{
        cgicc::Cgicc cgi(in);
	confParams_.bag.outputType = cgi["value"]->getValue();
	// std::cout << " outputType " << confParams_.bag.outputType.toString() << std::endl;

	// re-display form page 
	this->webDefault(in,out);		
    }
    catch (const std::exception & e){
        XCEPT_RAISE(xgi::exception::Exception, e.what());
    }	
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webConfigure(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Derive device number from device name

    for (int i=0; i<24; i++){
      std::string tmpDeviceName = confParams_.bag.deviceName[i].toString();
      int tmpDeviceNum = -1;
      tmpDeviceName.erase(0,4);
      tmpDeviceNum = atoi(tmpDeviceName.c_str());
      tmpDeviceNum -= 8;

      if ( tmpDeviceNum >= 0 ) {
	confParams_.bag.deviceNum[i] = tmpDeviceNum;
      }
    }

    // Initiate configure workloop
    wl_->submit(configure_signature_);

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webStart(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Initiate start workloop
    wl_->submit(start_signature_);
    
    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webStop(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Initiate stop workloop
    wl_->submit(stop_signature_);

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webHalt(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Initiate halt workloop
    wl_->submit(halt_signature_);

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webTrigger(xgi::Input * in, xgi::Output * out )
throw (xgi::exception::Exception)
{
    // Send L1A signal
    hw_semaphore_.take();
    vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
    //for (unsigned int com = 0; com < 15; ++com) vfatDevice_->writeReg("Send.L1ACalPulse",1);

    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.L1A",0x1);

    for (int i=0; i<24; i++){
      std::string VfatName = confParams_.bag.deviceName[i].toString();
      if (VfatName != ""){
        //std::cout << " webTrigger : deviceName [" << i << "] " << VfatName << std::endl;
      }
    }


    hw_semaphore_.give();

    // Go back to main web interface
    this->webRedirect(in, out);
}

    void gem::supervisor::GEMGLIBSupervisorWeb::webRedirect(xgi::Input *in, xgi::Output* out) 
throw (xgi::exception::Exception)
{
    // Redirect to main web interface
    std::string url = "/" + getApplicationDescriptor()->getURN() + "/Default";
    *out << "<meta http-equiv=\"refresh\" content=\"0;" << url << "\">" << std::endl;

    this->webDefault(in,out);
}

bool gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::task::WorkLoop *wl)
{
    // fire "Configure" event to FSM
    fireEvent("Configure");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::task::WorkLoop *wl)
{
    // fire "Start" event to FSM
    fireEvent("Start");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::task::WorkLoop *wl)
{
    // Fire "Stop" event to FSM
    fireEvent("Stop");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::task::WorkLoop *wl)
{
    // Fire "Halt" event to FSM
    fireEvent("Halt");
    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::runAction(toolbox::task::WorkLoop *wl)
{
    wl_semaphore_.take();
    hw_semaphore_.take();

    // Get the size of GLIB data buffer
    vfatDevice_->setDeviceBaseNode("GLIB");
    uint32_t bufferDepth = vfatDevice_->readReg(vfatDevice_->getDeviceBaseNode(),"LINK1.TRK_FIFO.DEPTH");

    wl_semaphore_.give();
    hw_semaphore_.give();

    LOG4CPLUS_INFO(getApplicationLogger(),"bufferDepth = " << std::hex << bufferDepth << std::dec);

    // If GLIB data buffer has non-zero size, initiate read workloop
    if (bufferDepth) {
        wl_->submit(read_signature_);
    }

    return false;
}

bool gem::supervisor::GEMGLIBSupervisorWeb::readAction(toolbox::task::WorkLoop *wl)
{
    wl_semaphore_.take();
    hw_semaphore_.take();

    counter_ = gemDataParker->dumpDataToDisk();

    hw_semaphore_.give();
    wl_semaphore_.give();

    return false;
}

    // State transitions
    void gem::supervisor::GEMGLIBSupervisorWeb::configureAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception)
{
    is_working_ = true;
    counter_ = 0;
  
    hw_semaphore_.take();

    for (int i=0; i<24; i++){

      std::string VfatName = confParams_.bag.deviceName[i].toString();
      if (VfatName != ""){

	// Define device
        vfatDevice_ = new gem::hw::vfat::HwVFAT2(this->getApplicationLogger(), VFATnum[i]);

        vfatDevice_->setAddressTableFileName("testbeam_registers.xml");
        vfatDevice_->setDeviceIPAddress(confParams_.bag.deviceIP);

        vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName[i].toString());

	vfatDevice_->connectDevice();
	vfatDevice_->readVFAT2Counters();
	vfatDevice_->setRunMode(0);
	confParams_.bag.deviceChipID = vfatDevice_->getChipID();

	latency_   = confParams_.bag.latency;

	// Set VFAT2 registers
	vfatDevice_->loadDefaults();
	// vfatDevice_->setTriggerMode(    0x3); //set to S1 to S8
	// vfatDevice_->setCalibrationMode(0x0); //set to normal
	// vfatDevice_->setMSPolarity(     0x1); //negative
	// vfatDevice_->setCalPolarity(    0x1); //negative

	// vfatDevice_->setProbeMode(        0x0);
	// vfatDevice_->setLVDSMode(         0x0);
	// vfatDevice_->setDACMode(          0x0);
	// vfatDevice_->setHitCountCycleTime(0x0); //maximum number of bits

	// vfatDevice_->setHitCountMode( 0x0);
	// vfatDevice_->setMSPulseLength(0x3);
	// vfatDevice_->setInputPadMode( 0x0);
	// vfatDevice_->setTrimDACRange( 0x0);
	// vfatDevice_->setBandgapPad(   0x0);
	// vfatDevice_->sendTestPattern( 0x0);

	// vfatDevice_->setIPreampIn(  168);
	// vfatDevice_->setIPreampFeed(150);
	// vfatDevice_->setIPreampOut(  80);
	// vfatDevice_->setIShaper(    150);
	// vfatDevice_->setIShaperFeed(100);
	//vfatDevice_->setIComp(      120);

	vfatDevice_->setLatency(latency_);

	vfatDevice_->setVThreshold1(2);
	confParams_.bag.deviceVT1 = vfatDevice_->getVThreshold1();
	vfatDevice_->setVThreshold2(0);
	confParams_.bag.deviceVT2 = vfatDevice_->getVThreshold2();
	confParams_.bag.latency = vfatDevice_->getLatency();

      }
    }

    // Create a new output file
    time_t now  = time(0);
    tm    *gmtm = gmtime(&now);
    char* utcTime = asctime(gmtm);
    std::string tmpFileName = "GEM_DAQ_", tmpType = "";
    tmpFileName.append(utcTime);
    tmpFileName.erase(std::remove(tmpFileName.begin(), tmpFileName.end(), '\n'), tmpFileName.end());
    tmpFileName.append(".dat");
    std::replace(tmpFileName.begin(), tmpFileName.end(), ' ', '_' );
    std::replace(tmpFileName.begin(), tmpFileName.end(), ':', '-');

    confParams_.bag.outFileName = tmpFileName;
    std::ofstream outf(tmpFileName.c_str(), std::ios_base::app | std::ios::binary );

    tmpType = confParams_.bag.outputType.toString();

    // Book GEM Data Parker
    gemDataParker = new gem::readout::GEMDataParker(*vfatDevice_, tmpFileName, tmpType);

    // scanStream.close();
    outf.close();

    hw_semaphore_.give();

    is_configured_  = true;
    is_working_     = false;    

}

    void gem::supervisor::GEMGLIBSupervisorWeb::startAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception){
    is_working_ = true;

    is_running_ = true;
    hw_semaphore_.take();

    /*
    //set clock source
    vfatDevice_->setDeviceBaseNode("OptoHybrid.CLOCKING");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"VFAT.SOURCE",  0x0);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"CDCE.SOURCE",  0x0);
    */

    //send resync
    LOG4CPLUS_INFO(getApplicationLogger(),"deviceBaseNode = " << vfatDevice_->getDeviceBaseNode());
    vfatDevice_->setDeviceBaseNode("OptoHybrid.FAST_COM");
    LOG4CPLUS_INFO(getApplicationLogger(),"deviceBaseNode = " << vfatDevice_->getDeviceBaseNode());
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"Send.Resync",        0x1);

    //reset counters
    vfatDevice_->setDeviceBaseNode("OptoHybrid.COUNTERS");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.External",0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Internal",0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Delayed", 0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.L1A.Total",   0x1);

    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.Resync",      0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.BC0",         0x1);

    //flush FIFO
    vfatDevice_->setDeviceBaseNode("GLIB.LINK1");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TRK_FIFO.FLUSH",     0x1);

    /*
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.External",0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.Internal",0x1);
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"RESETS.CalPulse.Total",   0x1);
    */

    /*
    //set trigger source
    vfatDevice_->setDeviceBaseNode("OptoHybrid.TRIGGER");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"SOURCE",   0x0); //0x2 
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",(unsigned)confParams_.bag.deviceNum[11]);

    vfatDevice_->setDeviceBaseNode("GLIB");
    vfatDevice_->writeReg(vfatDevice_->getDeviceBaseNode(),"TDC_SBits",(unsigned)confParams_.bag.deviceNum[11]);
    */

    for (int i=0; i<24; i++){
      std::string VfatName = confParams_.bag.deviceName[i].toString();
      if (VfatName != ""){
        //std::cout << " startAction : deviceName [" << i << "] " << VfatName << std::endl;
        vfatDevice_->setDeviceBaseNode("OptoHybrid.GEB.VFATS."+confParams_.bag.deviceName[i].toString());
        vfatDevice_->setRunMode(1);
      }
    }

    hw_semaphore_.give();
    is_working_ = false;
}

    void gem::supervisor::GEMGLIBSupervisorWeb::stopAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception){
    is_running_ = false;
}

    void gem::supervisor::GEMGLIBSupervisorWeb::haltAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception){
    is_running_ = false;
    counter_ = 0;
    delete gemDataParker;
}

    void gem::supervisor::GEMGLIBSupervisorWeb::noAction(toolbox::Event::Reference evt)
throw (toolbox::fsm::exception::Exception){
}

    void gem::supervisor::GEMGLIBSupervisorWeb::fireEvent(std::string name){
       toolbox::Event::Reference event(new toolbox::Event(name, this));
       fsm_.fireEvent(event);
    }

    void gem::supervisor::GEMGLIBSupervisorWeb::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
throw (toolbox::fsm::exception::Exception){
}

    void gem::supervisor::GEMGLIBSupervisorWeb::transitionFailed(toolbox::Event::Reference event)
throw (toolbox::fsm::exception::Exception){
}
