/**
 * class: GEMFSMApplication
 * description: Generic GEM application with FSM interface
 *              structure borrowed from TCDS core, with nods to HCAL and EMU code
 * author: 
 * date: 
 */

#include "gem/base/GEMFSMApplication.h"
#include "gem/base/GEMWebApplication.h"
#include "gem/base/GEMFSM.h"

#include "gem/utils/soap/GEMSOAPToolBox.h"

#include "gem/utils/exception/Exception.h"
#include "gem/base/exception/Exception.h"
#include "gem/base/utils/exception/Exception.h"

#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/InvalidInputEvent.h"
#include "toolbox/fsm/FailedEvent.h"

#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/BSem.h"

#include "toolbox/string.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/NamespaceURI.h"
#include "xcept/Exception.h"

#include "xgi/framework/Method.h"
#include "xgi/framework/UIManager.h"

#include "xgi/Input.h"
#include "xgi/Method.h"
#include "xgi/Output.h"

#include "xoap/Method.h"


gem::base::GEMFSMApplication::GEMFSMApplication(xdaq::ApplicationStub* stub)
  throw (xdaq::exception::Exception) :
  GEMApplication(stub),
  gemfsm_(this),//, &gemAppStateInfoSpace_)
  wl_semaphore_(toolbox::BSem::FULL)
{
  INFO("GEMFSMApplication ctor begin");

  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiInitialize, "Initialize" );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiEnable,     "Enable"     );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiConfigure,  "Configure"  );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiStart,      "Start"      );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiStop,       "Stop"       );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiPause,      "Pause"      );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiResume,     "Resume"     );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiHalt,       "Halt"       );
  xgi::framework::deferredbind(this, this, &GEMFSMApplication::xgiReset,      "Reset"      );

  // These bindings expose the state machine to the outside world. The
  // changeState() method simply forwards the calls to the GEMFSM
  // object.
  xoap::bind(this, &GEMFSMApplication::changeState, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Enable",     XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Configure",  XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Start",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Stop",       XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Pause",      XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Resume",     XDAQ_NS_URI);
  xoap::bind(this, &GEMFSMApplication::changeState, "Halt",       XDAQ_NS_URI);

  //benefit or disadvantage to setting up the workloop signatures this way?
  //hcal has done a forwarding which may be a clever solution, but to what problem?
  initSig_   = toolbox::task::bind(this, &GEMFSMApplication::initialize, "initialize");
  confSig_   = toolbox::task::bind(this, &GEMFSMApplication::configure,  "configure" );
  startSig_  = toolbox::task::bind(this, &GEMFSMApplication::start,      "start"     );
  stopSig_   = toolbox::task::bind(this, &GEMFSMApplication::stop,       "stop"      );
  pauseSig_  = toolbox::task::bind(this, &GEMFSMApplication::pause,      "pause"     );
  resumeSig_ = toolbox::task::bind(this, &GEMFSMApplication::resume,     "resume"    );
  haltSig_   = toolbox::task::bind(this, &GEMFSMApplication::halt,       "halt"      );
  resetSig_  = toolbox::task::bind(this, &GEMFSMApplication::reset,      "reset"     );
  
  std::stringstream tmpLoopName;
  uint32_t instanceNumber = this->getApplicationDescriptor()->getInstance();
  std::string className   = this->getApplicationDescriptor()->getClassName();

  //also want to get the name of the GEM FSM aplication to put it into this commandLoopName
  tmpLoopName << "urn:toolbox-task-workloop:"
	      << className << ":" << instanceNumber;
  workLoopName = tmpLoopName.str();
    
  //appStateInfoSpace_.setFSMState(gemfsm_.getCurrentStateName());

  INFO("GEMFSMApplication ctor end");
}

gem::base::GEMFSMApplication::~GEMFSMApplication()
{
  INFO("GEMFSMApplication dtor begin");
  INFO("GEMFSMApplication dtor end");
}

/**hyperdaq callbacks*/
void gem::base::GEMFSMApplication::xgiInitialize(xgi::Input* in, xgi::Output* out) {
  
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Initialize",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Initialize failed", e );
  }
  
  gemWebInterfaceP_->webInitialize(in,out);
}

void gem::base::GEMFSMApplication::xgiEnable(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Enable",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Enable failed", e );
  }
  
  gemWebInterfaceP_->webEnable(in,out);
}

void gem::base::GEMFSMApplication::xgiConfigure(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Configure",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Configure failed", e );
  }
  
  gemWebInterfaceP_->webConfigure(in,out);
}

void gem::base::GEMFSMApplication::xgiStart(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Start",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Start failed", e );
  }
  
  gemWebInterfaceP_->webStart(in,out);
}

void gem::base::GEMFSMApplication::xgiStop(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Stop",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Stop failed", e );
  }
  
  gemWebInterfaceP_->webStop(in,out);
}

void gem::base::GEMFSMApplication::xgiPause(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Pause",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Pause failed", e );
  }
  
  gemWebInterfaceP_->webPause(in,out);
}

void gem::base::GEMFSMApplication::xgiResume(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Resume",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Resume failed", e );
  }
  
  gemWebInterfaceP_->webResume(in,out);
}

void gem::base::GEMFSMApplication::xgiHalt(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Halt",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Halt failed", e );
  }
  
  gemWebInterfaceP_->webHalt(in,out);
}

void gem::base::GEMFSMApplication::xgiReset(xgi::Input* in, xgi::Output* out) {
  try {
    INFO("Sending SOAP command to application");
    gem::utils::soap::GEMSOAPToolBox::sendCommand("Reset",
						  appContextP_,
						  appDescriptorP_,
						  appDescriptorP_
						  );
  } catch( toolbox::fsm::exception::Exception& e ){
    XCEPT_RETHROW( xgi::exception::Exception, "Reset failed", e );
  }
  
  gemWebInterfaceP_->webReset(in,out);
}


/**state transitions*/
void gem::base::GEMFSMApplication::transitionDriver(::toolbox::Event::Reference e)
  throw (::toolbox::fsm::exception::Exception) {
  try {
    if (e->type() == "Initialize" || e->type() == "Configure" || e->type() == "Start"  ||
	e->type() == "Stop"       || e->type() == "Pause"     || e->type() == "Resume" || 
	e->type() == "Halt"       || e->type() == "Reset" ) {
      workloopDriver(e->type());
    } else XCEPT_RAISE(::toolbox::fsm::exception::Exception,"Unknown transition command");
  } catch (gem::utils::exception::Exception& ex) {
    fireEvent("Fail");
    XCEPT_RETHROW(::toolbox::fsm::exception::Exception,"State Transition Failed",ex);
  }
}

void gem::base::GEMFSMApplication::workloopDriver(std::string const& command)
  throw (::toolbox::task::exception::Exception) {
  try {
    ::toolbox::task::WorkLoopFactory* wlf  = ::toolbox::task::WorkLoopFactory::getInstance();
    ::toolbox::task::WorkLoop*        loop = wlf->getWorkLoop(workLoopName,"waiting");
    if (!loop->isActive()) loop->activate();

    if      (command=="Initialize") loop->submit(initSig_  );
    else if (command=="Configure")  loop->submit(confSig_  );
    else if (command=="Start")      loop->submit(startSig_ );
    else if (command=="Stop")       loop->submit(stopSig_  );
    else if (command=="Pause")      loop->submit(pauseSig_ );
    else if (command=="Resume")     loop->submit(resumeSig_);
    else if (command=="Halt")       loop->submit(haltSig_  );
    else if (command=="Reset")      loop->submit(resetSig_ );
  } catch (::toolbox::task::exception::Exception& e) {
    XCEPT_RETHROW(gem::utils::exception::Exception,"Workloop failure",e);
  }
  
}

void gem::base::GEMFSMApplication::resetAction(::toolbox::Event::Reference e)
  throw (::toolbox::fsm::exception::Exception) {
}
	
void gem::base::GEMFSMApplication::failAction(::toolbox::Event::Reference e)
  throw (::toolbox::fsm::exception::Exception) {
}
	

void gem::base::GEMFSMApplication::stateChanged(::toolbox::fsm::FiniteStateMachine &fsm)
  throw (::toolbox::fsm::exception::Exception) {
}

void gem::base::GEMFSMApplication::transitionFailed(::toolbox::Event::Reference event)
  throw (::toolbox::fsm::exception::Exception) {
}

void gem::base::GEMFSMApplication::fireEvent(std::string event)
  throw (::toolbox::fsm::exception::Exception) {
  
  try {
    ::toolbox::Event::Reference e(new toolbox::Event(event,this));
    gemfsm_.fireEvent(e);
  } catch (::toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(::xoap::exception::Exception, "invalid command", e);
  }
}

 
/**SOAP callback*/
// This simply forwards the message to the GEMFSM object, since it is
// technically not possible to bind directly to anything but an
// xdaq::Application.
xoap::MessageReference gem::base::GEMFSMApplication::changeState(xoap::MessageReference msg)
{
  return gemfsm_.changeState(msg);
}


/** transition details*/
void gem::base::GEMFSMApplication::initializeAction() {
  LOG4CPLUS_INFO(getApplicationLogger(),std::string("gem::base::GEMFSMApplication::initializeAction Initializing"));
}
void gem::base::GEMFSMApplication::enableAction(    ) {}
void gem::base::GEMFSMApplication::configureAction( ) {}
void gem::base::GEMFSMApplication::startAction(     ) {}
void gem::base::GEMFSMApplication::pauseAction(     ) {}
void gem::base::GEMFSMApplication::resumeAction(    ) {}
void gem::base::GEMFSMApplication::stopAction(      ) {}
void gem::base::GEMFSMApplication::haltAction(      ) {}

void gem::base::GEMFSMApplication::noAction(        ) {} 
	
