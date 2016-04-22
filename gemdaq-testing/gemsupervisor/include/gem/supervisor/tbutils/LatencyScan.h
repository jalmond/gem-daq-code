#ifndef gem_supervisor_tbutils_LatencyScan_h
#define gem_supervisor_tbutils_LatencyScan_h

#include "gem/supervisor/tbutils/GEMTBUtil.h"
#include "TStopwatch.h"


namespace gem {
  namespace supervisor {
    namespace tbutils {

      class LatencyScan : public GEMTBUtil
      {

	//friend class GEMTBUtil
	
      public:

	XDAQ_INSTANTIATOR();
	LatencyScan(xdaq::ApplicationStub * s)
	  throw (xdaq::exception::Exception);
	~LatencyScan();

	//SOAP MEssage AMC13	
	void sendConfigureMessageAMC13()
	  throw (xgi::exception::Exception);
	bool sendStartMessageAMC13()
	  throw (xgi::exception::Exception);
	void sendAMC13trigger()
	  throw (xgi::exception::Exception);
	void NTriggersAMC13()
	  throw (xgi::exception::Exception);

	//SOAP MEssage GLIB	
	void sendConfigureMessageGLIB()
	  throw (xgi::exception::Exception);
	bool sendStartMessageGLIB()
	  throw (xgi::exception::Exception);


	// HyperDAQ interface
	void webDefault(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webConfigure(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);
	void webStart(xgi::Input *in, xgi::Output *out)
	  throw (xgi::exception::Exception);

	//workloop functions
	bool run(       toolbox::task::WorkLoop* wl);

	// State transitions
	void configureAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void startAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);
	void resetAction(toolbox::Event::Reference e)
	  throw (toolbox::fsm::exception::Exception);

	//web display helpers
	void scanParameters(xgi::Output* out)
	  throw (xgi::exception::Exception);
        void selectTrigSource(xgi::Output* out)
	  throw (xgi::exception::Exception);
	    
      class ConfigParams 
      {
      public:

	void registerFields(xdata::Bag<ConfigParams> *bag);
	    
	xdata::String          slotFileName;

	xdata::UnsignedShort  stepSize;
	xdata::UnsignedShort  minLatency;
	xdata::UnsignedShort  maxLatency;
	xdata::UnsignedShort  nTriggers;
	//	xdata::UnsignedShort  triggerSource_;
	    
	xdata::Integer  threshold;
	xdata::Integer  deviceVT1;
	xdata::Integer  deviceVT2;
	xdata::Integer  VCal;
	xdata::Integer  MSPulseLength;
      };
	  
    private:
      //ConfigParams confParams_;
      xdata::Bag<ConfigParams> scanParams_;

      int minLatency_, maxLatency_, threshold_, MSPulseLength, VCal;
      uint8_t  currentLatency_;
      uint64_t stepSize_;
      int totaltriggers;

      protected:
	  
      };
    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
