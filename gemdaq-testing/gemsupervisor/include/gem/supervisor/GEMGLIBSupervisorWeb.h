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

#include "gem/readout/GEMslotContents.h"


//#include "gem/hw/amc13/Module.hh"
//#include "amc13/AMC13.hh"

#include <string>

namespace amc13 {
  class AMC13;
  class Module;
}

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

  typedef std::shared_ptr<hw::vfat::HwVFAT2 > vfat_shared_ptr;
  typedef std::shared_ptr<hw::glib::HwGLIB >  glib_shared_ptr;
  typedef std::shared_ptr<hw::optohybrid::HwOptoHybrid > optohybrid_shared_ptr;

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
         *    Send L1A+CalPulse signal and return to main web interface
         */
        void webL1ACalPulse(xgi::Input *in, xgi::Output *out);
        /**
         *    Send Resync signal and return to main web interface
         */
        void webResync(xgi::Input *in, xgi::Output *out);
        /**
         *    Send BC0 signal and return to main web interface
         */
        void webBC0(xgi::Input *in, xgi::Output *out);
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
        /**
         *    Select all data available in GLIB data buffer
         */
        bool selectAction(toolbox::task::WorkLoop *wl);

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
          xdata::String          slotFileName;
          xdata::String          outputType;

          xdata::Integer         ohGTXLink;

          xdata::Vector<xdata::String>  deviceName;
          xdata::Vector<xdata::Integer> deviceNum;

          xdata::UnsignedShort latency;
          xdata::UnsignedShort triggerSource;
          xdata::UnsignedShort deviceChipID;
          xdata::UnsignedShort deviceVT1;
          xdata::UnsignedShort deviceVT2;
        };

      private:

        std::unique_ptr<gem::readout::GEMslotContents> slotInfo;
        
        log4cplus::Logger m_gemLogger;
	
        toolbox::task::WorkLoopFactory* wlf_;
        toolbox::task::WorkLoop *wl_;

        toolbox::BSem wl_semaphore_;
        toolbox::BSem hw_semaphore_;

        toolbox::task::ActionSignature *configure_signature_;
        toolbox::task::ActionSignature *stop_signature_;
        toolbox::task::ActionSignature *halt_signature_;
        toolbox::task::ActionSignature *start_signature_;
        toolbox::task::ActionSignature *run_signature_;
        toolbox::task::ActionSignature *read_signature_;
        toolbox::task::ActionSignature *select_signature_;

        toolbox::fsm::FiniteStateMachine fsm_;

        uint32_t readout_mask;
        xdata::Bag<ConfigParams> confParams_;

        FILE* outputFile;
        uint64_t latency_;
        uint64_t deviceVT1_;
        bool is_working_, is_initialized_,  is_configured_, is_running_;

        //supervisor application should not have any hw devices, should only send commands to manager applications
        //temporary fix just to get things working stably, should be using the manager
        amc13::AMC13* amc13_;
        //amc13::Module* pMod;
        //std::shared_ptr<amc13::Module> pMod;
        //std::shared_ptr<amc13::AMC13> amc13_;
        glib_shared_ptr glibDevice_;
        optohybrid_shared_ptr optohybridDevice_;
        std::vector<vfat_shared_ptr> vfatDevice_;
        //readout application should be running elsewhere, not tied to supervisor
        std::shared_ptr<gem::readout::GEMDataParker> gemDataParker;

        // Counter
        uint32_t m_counter[5];

        // VFAT Blocks Counter
        int vfat_;

        // Events Counter     
        int event_;

        // VFATs counter per event
        int sumVFAT_;

        //all T1 signals have 5 sources TTC Firmware External Loopback Sent
        // L1A trigger counting
        uint32_t m_l1aCount[5];

        // CalPulse counting
        uint32_t m_calPulseCount[5];

        // Resync counting
        uint32_t m_resyncCount[5];

        // BC0 counting
        uint32_t m_bc0Count[5];

        void fireEvent(std::string name);
        void stateChanged(toolbox::fsm::FiniteStateMachine &fsm);
        void transitionFailed(toolbox::Event::Reference event);
      };
  }
}
#endif
