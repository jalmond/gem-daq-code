#ifndef gem_supervisor_GEMGLIBSupervisorWeb_h
#define gem_supervisor_GEMGLIBSupervisorWeb_h

#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"

#include "xdata/Float.h"
#include "xdata/String.h"
#include "xdata/Vector.h"
#include "xdata/Integer.h"
#include "xdata/UnsignedLong.h"
#include "xdata/UnsignedShort.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/UnsignedInteger64.h"

#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoop.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/Event.h"
#include "toolbox/BSem.h"

#include "xdaq/NamespaceURI.h"
#include "xoap/Method.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPSerializer.h"
#include "xoap/domutils.h"

#include "xoap/DOMParser.h"
#include "xoap/DOMParserFactory.h"
#include "xoap/domutils.h"
#include "xdata/soap/Serializer.h"

#include "xgi/framework/Method.h"
#include "cgicc/HTMLClasses.h"

#include <string>


namespace gem {
  namespace hw {
    class GEMHwDevice;
    namespace vfat {
      class HwVFAT2;
    }
    namespace optohybrid {
      class HwOptoHybrid;
    }
    namespace glib {
      class HwGLIB;
    }
  }
  namespace readout {
    class GEMDataParker;
  }

  namespace supervisor {

    class GEMGLIBSupervisorWeb: public xdaq::WebApplication, public xdata::ActionListener
      {
      public:

	XDAQ_INSTANTIATOR();

	GEMGLIBSupervisorWeb(xdaq::ApplicationStub * s);

	void setParameter(xgi::Input * in, xgi::Output * out ) ;

	// SOAP interface
	xoap::MessageReference onConfigure(xoap::MessageReference message);
	xoap::MessageReference onStart(xoap::MessageReference message);
	xoap::MessageReference onStop(xoap::MessageReference message);
	xoap::MessageReference onHalt(xoap::MessageReference message);

	// HyperDAQ interface
	/**
	 *    Generate main web interface
	 */
	void webDefault(xgi::Input *in, xgi::Output *out);
	/**
	 *    Initiate configuration workloop and return to main web interface
	 */
	void webConfigure(xgi::Input *in, xgi::Output *out);
	/**
	 *    Initiate start workloop and return to main web interface
	 */
	void webStart(xgi::Input *in, xgi::Output *out);
	/**
	 *    Initiate stop workloop and return to main web interface
	 */
	void webStop(xgi::Input *in, xgi::Output *out);
	/**
	 *    Initiate halt workloop and return to main web interface
	 */
	void webHalt(xgi::Input *in, xgi::Output *out);
	/**
	 *    Send L1A signal and return to main web interface
	 */
	void webTrigger(xgi::Input *in, xgi::Output *out);
	/**
	 *    Redirect to main web interface
	 */
	void webRedirect(xgi::Input *in, xgi::Output *out);

	// work loop call-back functions
	/**
	 *    Fire initialize action to FSM
	 */
	bool initializeAction(toolbox::task::WorkLoop *wl);
	/**
	 *    Fire configure action to FSM
	 */
	bool configureAction(toolbox::task::WorkLoop *wl);
	/**
	 *    Fire start action to FSM
	 */
	bool startAction(toolbox::task::WorkLoop *wl);
	/**
	 *    Fire stop action to FSM
	 */
	bool stopAction(toolbox::task::WorkLoop *wl);
	/**
	 *    Fire halt action to FSM
	 */
	bool haltAction(toolbox::task::WorkLoop *wl);
	/**
	 *    Check if GLIB has any data available
	 *    If it does, initiate read workloop
	 */
	bool runAction(toolbox::task::WorkLoop *wl);
	/**
	 *    Dump to disk all data available in GLIB data buffer
	 */
	bool readAction(toolbox::task::WorkLoop *wl);

	// State transitions
	/**
	 *    Prepare the system to data acquisition:
	 *      - Write predefined values to VFAT2 registers
	 *      - Create a new output file
	 */
	void configureAction(toolbox::Event::Reference e);
	/**
	 *    Prepare Opto-hybrid Board and GLIB to data acquisition
	 */
	void startAction(toolbox::Event::Reference e);
	/**
	 *    Action during transition to "Stopped" state
	 */
	void stopAction(toolbox::Event::Reference e);
	/**
	 *    Action during transition to "Halted" state
	 */
	void haltAction(toolbox::Event::Reference e);
	/**
	 *    Empty action for forbidden state transitions in FSM
	 */
	void noAction(toolbox::Event::Reference e);

	
	/**
	 *    Callback for action performed
	 */
	virtual void actionPerformed(xdata::Event& event);
	class ConfigParams 
	{   
	public:
	  void registerFields(xdata::Bag<ConfigParams> *bag);

	  xdata::String          deviceIP;
	  xdata::String          outFileName;
	  xdata::String          outputType;

	  xdata::Vector<xdata::String>  deviceName;
	  xdata::Vector<xdata::Integer> deviceNum;

	  xdata::UnsignedShort latency;
	  xdata::UnsignedShort triggerSource;
	  xdata::UnsignedShort deviceChipID;
	  xdata::UnsignedShort deviceVT1;
	  xdata::UnsignedShort deviceVT2;
	};

      private:

	log4cplus::Logger gemLogger_;
	
	toolbox::task::WorkLoop *wl_;

	toolbox::BSem wl_semaphore_;
	toolbox::BSem hw_semaphore_;

	toolbox::task::ActionSignature *configure_signature_;
	toolbox::task::ActionSignature *stop_signature_;
	toolbox::task::ActionSignature *halt_signature_;
	toolbox::task::ActionSignature *start_signature_;
	toolbox::task::ActionSignature *run_signature_;
	toolbox::task::ActionSignature *read_signature_;

	toolbox::fsm::FiniteStateMachine fsm_;

	xdata::Bag<ConfigParams> confParams_;

	FILE* outputFile;
	uint64_t latency_;
	bool is_working_, is_initialized_, is_configured_, is_running_;

	//supervisor application should not have any hw devices, should only send commands to manager applications
	//temporary fix just to get things working stably, should be using the manager
	gem::hw::glib::HwGLIB* glibDevice_;
	gem::hw::optohybrid::HwOptoHybrid* optohybridDevice_;
	std::vector< gem::hw::vfat::HwVFAT2* > vfatDevice_;
	//readout application should be running elsewhere, not tied to supervisor
	gem::readout::GEMDataParker* gemDataParker;

	int counter_;

	void fireEvent(std::string name);
	void stateChanged(toolbox::fsm::FiniteStateMachine &fsm);
	void transitionFailed(toolbox::Event::Reference event);
      };
  }
}
#endif
