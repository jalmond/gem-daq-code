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

	  // HyperDAQ interface
	  void webDefault(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webConfigure(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webStart(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  /*
	  void webStop(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webInitialize(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webHalt(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webReset(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webResetCounters(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  void webSendFastCommands(xgi::Input *in, xgi::Output *out)
	    throw (xgi::exception::Exception);
	  */

	  //workloop functions
	  bool run(       toolbox::task::WorkLoop* wl);
	  bool readFIFO(  toolbox::task::WorkLoop* wl);

	  // State transitions
	  void configureAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  void startAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  void resetAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  /*
	  void initializeAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  void stopAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  void haltAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  void noAction(toolbox::Event::Reference e)
	    throw (toolbox::fsm::exception::Exception);
	  */
	  
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

	  int minThresh_, maxThresh_;
	  uint64_t stepSize_, latency_;
	  
	protected:
	  
	};

    } //end namespace gem::supervisor::tbutils
  } //end namespace gem::supervisor
} //end namespace gem
#endif
