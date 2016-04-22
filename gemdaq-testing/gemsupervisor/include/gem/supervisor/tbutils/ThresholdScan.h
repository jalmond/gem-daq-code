#ifndef gem_supervisor_tbutils_ThresholdScan_h
#define gem_supervisor_tbutils_ThresholdScan_h

#include "gem/supervisor/tbutils/GEMTBUtil.h"

#include "TStopwatch.h"

namespace gem {
  namespace supervisor {
    namespace tbutils {

      class ThresholdScan : public GEMTBUtil
      {
	  
        //	  friend class GEMTBUtil;
	  
      public:
	  
        XDAQ_INSTANTIATOR();
        ThresholdScan(xdaq::ApplicationStub * s)
          throw (xdaq::exception::Exception);
        ~ThresholdScan();

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
        void displayHistograms(xgi::Output* out)
          throw (xgi::exception::Exception);
	  
        class ConfigParams 
        {
        public:
          //void getFromFile(const std::string& fileName);
          void registerFields(xdata::Bag<ConfigParams> *bag);
	    
          xdata::UnsignedInteger latency;
          xdata::UnsignedShort   stepSize;

          xdata::Integer minThresh;
          xdata::Integer maxThresh;
	    
          xdata::UnsignedShort currentHisto;
          xdata::UnsignedShort deviceVT1;
          xdata::UnsignedShort deviceVT2;

        };

      private:

        //ConfigParams confParams_;
        xdata::Bag<ConfigParams> scanParams_;
	int totaltriggers;
        int minThresh_, maxThresh_;
        uint64_t stepSize_, latency_;
	  
      protected:
	  
      };

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
